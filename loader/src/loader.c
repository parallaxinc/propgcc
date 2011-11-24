/* loader.c - an elf and spin binary loader for the Parallax Propeller microcontroller

Copyright (c) 2011 David Michael Betz

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include "loadelf.h"
#include "loader.h"
#include "packet.h"
#include "PLoadLib.h"
#include "osint.h"
#include "pex.h"
#include "../sdloader/sd_loader.h"

/* maximum cog image size */
#define COG_IMAGE_MAX           (496 * 4)

/* spin object file header */
typedef struct {
    uint32_t clkfreq;
    uint8_t clkmode;
    uint8_t chksum;
    uint16_t pbase;
    uint16_t vbase;
    uint16_t dbase;
    uint16_t pcurr;
    uint16_t dcurr;
} SpinHdr;

/* spin object */
typedef struct {
    uint16_t next;
    uint8_t pubcnt;
    uint8_t objcnt;
    uint16_t pcurr;
    uint16_t numlocals;
} SpinObj;

/* DAT header in serial_helper.spin */
typedef struct {
    uint32_t baudrate;
    uint8_t rxpin;
    uint8_t txpin;
    uint8_t tvpin;
} SerialHelperDatHdr;

/* DAT header in flash_loader.spin */
typedef struct {
    uint32_t cache_size;
    uint32_t cache_param1;
    uint32_t cache_param2;
    uint32_t vm_code_off;
    uint32_t cache_code_off;
} FlashLoaderDatHdr;

/* DAT header in sd_driver.spin */
typedef struct {
    uint32_t jmp_init;
    uint32_t do_mask;
    uint32_t clk_mask;
    uint32_t di_mask;
    uint32_t cs_mask;
} SDDriverDatHdr;

/* DAT header in sd_cache.spin */
typedef struct {
    uint32_t jmp_init;
    uint32_t params_off;
} SDCacheDatHdr;

/* parameter structure in sd_cache.spin */
typedef struct {
    uint32_t do_mask;
    uint32_t clk_mask;
    uint32_t di_mask;
    uint32_t cs_clr_mask;
    uint32_t select_inc_mask;
    uint32_t select_mask;
} SDCacheParams;

/* target checksum for a binary file */
#define SPIN_TARGET_CHECKSUM    0x14

/* image header */
typedef struct {
    uint32_t entry;
    uint32_t initCount;
    uint32_t initTableOffset;
} ImageHdr;

/* init section */
typedef struct {
    uint32_t vaddr;
    uint32_t paddr;
    uint32_t size;
} InitSection;

/* packet types */
#define TYPE_VM_INIT            1
#define TYPE_CACHE_INIT         2
#define TYPE_FLASH_WRITE        3
#define TYPE_RAM_WRITE          4
#define TYPE_HUB_WRITE          5
#define TYPE_DATA               6
#define TYPE_EOF                7
#define TYPE_RUN                8

extern uint8_t serial_helper_array[];
extern int serial_helper_size;
extern uint8_t flash_loader_array[];
extern int flash_loader_size;

static int LoadElfFile(System *sys, BoardConfig *config, char *path, int flags, FILE *fp, ElfHdr *hdr);
static int LoadBinaryFile(System *sys, BoardConfig *config, char *path, int flags, FILE *fp);
static int LoadInternalImage(System *sys, BoardConfig *config, int flags, ElfContext *c);
static int WriteSpinBinaryFile(BoardConfig *config, char *path, ElfContext *c);
static uint8_t *BuildInternalImage(BoardConfig *config, ElfContext *c, uint32_t *pStart, int *pImageSize);
static void UpdateChecksum(uint8_t *imagebuf, int imageSize);
static int LoadExternalImage(System *sys, BoardConfig *config, int flags, ElfContext *c);
static int WriteExecutableFile(char *path, ElfContext *c);
static uint8_t *BuildExternalImage(ElfContext *c, uint32_t *pLoadAddress, int *pImageSize);
static int WriteFlashLoader(System *sys, BoardConfig *config, uint8_t *vm_array, int vm_size, int mode);
static int ReadCogImage(System *sys, char *name, uint8_t *buf, int *pSize);
static int WriteBuffer(uint8_t *buf, int size);
static char *ConstructOutputName(char *outfile, const char *infile, char *ext);
static int Error(char *fmt, ...);
static void *NullError(char *fmt, ...);

typedef struct {
    int baud;
    char *actualport;
} CheckPortInfo;

static int CheckPort(const char* port, void* data)
{
    CheckPortInfo* info = (CheckPortInfo*)data;
    int rc;
    if ((rc = popenport(port, info->baud)) != 0)
        return rc;
    strncpy(info->actualport, port, PATH_MAX - 1);
    info->actualport[PATH_MAX - 1] = '\0';
    return 0;
}

int InitPort(char *prefix, char *port, int baud, char *actualport)
{
    int rc;
    
    if (port) {
        strncpy(actualport, port, PATH_MAX - 1);
        actualport[PATH_MAX - 1] = '\0';
        rc = popenport(port, baud);
    }
    else {
        CheckPortInfo info;
        info.baud = baud;
        info.actualport = actualport;
        rc = serial_find(prefix, CheckPort, &info);
    }
        
    return rc == 0;
}

int LoadImage(System *sys, BoardConfig *config, char *path, int flags)
{    
    ElfHdr hdr;
    FILE *fp;
    int sts;
    
    /* open the image file */
    if (!(fp = fopen(path, "rb")))
        return Error("failed to open '%s'", path);
    
    /* check for an elf file */
    if (ReadAndCheckElfHdr(fp, &hdr))
        sts = LoadElfFile(sys, config, path, flags, fp, &hdr);
    else {
        char *end = strrchr(path, '.');
        if (end && strcasecmp(end, ".elf") == 0)
            return Error("bad elf file '%s'", path);
        sts = LoadBinaryFile(sys, config, path, flags, fp);
    }
    
    return sts;
}

static int LoadElfFile(System *sys, BoardConfig *config, char *path, int flags, FILE *fp, ElfHdr *hdr)
{
    ElfSectionHdr section;
    ElfContext *c;
    
    /* open the elf file */
    if (!(c = OpenElfFile(fp, hdr)))
        return Error("failed to open elf file");
        
    /* '.header' section used for external loads */
    if (FindSectionTableEntry(c, ".header", &section)) {
    
        if (flags & LFLAG_WRITE_PEX) {
            if (flags & (LFLAG_WRITE_EEPROM | LFLAG_RUN))
                return Error("can't use -e or -r with -x");
            if (!WriteExecutableFile(path, c)) {
                CloseElfFile(c);
                return FALSE;
            }
        }
        
        else {
            if (!LoadExternalImage(sys, config, flags, c)) {
                CloseElfFile(c);
                return FALSE;
            }
        }
    }
    
    /* no '.header' section for internal loads */
    else {
    
        if (flags & LFLAG_WRITE_BINARY) {
            if (flags & (LFLAG_WRITE_EEPROM | LFLAG_RUN))
                return Error("can't use -e or -r with -s");
            if (!WriteSpinBinaryFile(config, path, c))
                return FALSE;
        }
        
        else {
            if (!LoadInternalImage(sys, config, flags, c)) {
                CloseElfFile(c);
                return FALSE;
            }
        }
    }
    
    /* close the elf file */
    CloseElfFile(c);
    
    /* return successfully */
    return TRUE;
}

static int LoadBinaryFile(System *sys, BoardConfig *config, char *path, int flags, FILE *fp)
{
    int mode, sts;
    
    /* determine the download mode */
    if (flags & LFLAG_WRITE_EEPROM)
        mode = flags & LFLAG_RUN ? DOWNLOAD_RUN_EEPROM : DOWNLOAD_EEPROM;
    else if (flags & LFLAG_RUN)
        mode = DOWNLOAD_RUN_BINARY;
    else
        mode = SHUTDOWN_CMD;
    
    fseek(fp, 0, SEEK_SET);
    sts = ploadfp(path, fp, mode);
    fclose(fp);
    
    return sts == 0;
}

static int LoadInternalImage(System *sys, BoardConfig *config, int flags, ElfContext *c)
{
    uint8_t *imagebuf;
    uint32_t start;
    int imageSize;
    int mode;
    
    /* build the .binary image */
    if (!(imagebuf = BuildInternalImage(config, c, &start, &imageSize)))
        return FALSE;
    
    /* determine the download mode */
    if (flags & LFLAG_WRITE_EEPROM)
        mode = flags & LFLAG_RUN ? DOWNLOAD_RUN_EEPROM : DOWNLOAD_EEPROM;
    else if (flags & LFLAG_RUN)
        mode = DOWNLOAD_RUN_BINARY;
    else
        mode = SHUTDOWN_CMD;
    
    /* load the serial helper program */
    if (mode != SHUTDOWN_CMD && ploadbuf(imagebuf, imageSize, mode) != 0) {
        free(imagebuf);
        return Error("load failed");
    }
    
    /* free the image buffer */
    free(imagebuf);

    return TRUE;
}

int LoadSDLoader(System *sys, BoardConfig *config, char *path, int flags)
{
    uint8_t driverImage[COG_IMAGE_MAX];
    int imageSize, driverSize, mode;
    ElfProgramHdr program;
    SdLoaderInfo *info;
    uint8_t *imagebuf;
    uint32_t start;
    ElfContext *c;
    ElfHdr hdr;
    FILE *fp;
    
    /* open the sd loader executable */
    if (!(fp = xbOpenFileInPath(sys, path, "rb")))
        return Error("can't open '%s'", path);

    /* check for an elf file */
    if (!ReadAndCheckElfHdr(fp, &hdr))
        return Error("bad elf file '%s'", path);

    /* open the elf file */
    if (!(c = OpenElfFile(fp, &hdr)))
        return Error("failed to open elf file");
        
    /* build the .binary image */
    if (!(imagebuf = BuildInternalImage(config, c, &start, &imageSize)))
        return FALSE;
    
    if (FindProgramSegment(c, ".coguser0", &program) < 0)
        return Error("can't find info (.coguser0) segment");
    
    info = (SdLoaderInfo *)&imagebuf[program.paddr - start];
    info->cache_size = config->cacheSize;
    info->cache_param1 = config->cacheParam1;
    info->cache_param2 = config->cacheParam2;

    if (FindProgramSegment(c, ".coguser1", &program) < 0)
        return Error("can't find xmm_driver (.coguser1) segment");

    if (config->cacheDriver) {
        if (!ReadCogImage(sys, config->cacheDriver, driverImage, &driverSize))
            return Error("reading cache driver image failed: %s", config->cacheDriver);
        memcpy(&imagebuf[program.paddr - start], driverImage, driverSize);
    }
    else
        return Error("no cache driver");
        
    if (FindProgramSegment(c, ".coguser2", &program) < 0)
        return Error("can't find sd_driver (.coguser2) segment");
    
    if (config->sdDriver) {
        SDDriverDatHdr *dat = (SDDriverDatHdr *)driverImage;
        if (!ReadCogImage(sys, config->sdDriver, driverImage, &driverSize))
            return Error("reading sd driver image failed: %s", config->sdDriver);
        dat->do_mask = 1 << config->sdspiDO;
        dat->clk_mask = 1 << config->sdspiClk;
        dat->di_mask = 1 << config->sdspiDI;
        dat->cs_mask = 1 << config->sdspiCS;
        memcpy(&imagebuf[program.paddr - start], driverImage, driverSize);
        info->use_cache_driver_for_sd = FALSE;
    }
    else
        info->use_cache_driver_for_sd = TRUE;
            
    /* update the checksum */
    UpdateChecksum(imagebuf, imageSize);

    /* determine the download mode */
    if (flags & LFLAG_WRITE_EEPROM)
        mode = flags & LFLAG_RUN ? DOWNLOAD_RUN_EEPROM : DOWNLOAD_EEPROM;
    else if (flags & LFLAG_RUN)
        mode = DOWNLOAD_RUN_BINARY;
    else
        return Error("expecting -e and/or -r");
        
    /* load the serial helper program */
    if (ploadbuf(imagebuf, imageSize, mode) != 0) {
        free(imagebuf);
        return Error("load failed");
    }
    
    /* free the image buffer */
    free(imagebuf);

    return TRUE;
}

int LoadSDCacheLoader(System *sys, BoardConfig *config, char *path, int flags)
{
    uint8_t driverImage[COG_IMAGE_MAX];
    int imageSize, driverSize, mode;
    ElfProgramHdr program;
    SdLoaderInfo *info;
    SDCacheDatHdr *dat;
    SDCacheParams *params;
    uint8_t *imagebuf;
    uint32_t start;
    ElfContext *c;
    ElfHdr hdr;
    FILE *fp;
    
    /* open the sd loader executable */
    if (!(fp = xbOpenFileInPath(sys, path, "rb")))
        return Error("can't open '%s'", path);

    /* check for an elf file */
    if (!ReadAndCheckElfHdr(fp, &hdr))
        return Error("bad elf file '%s'", path);

    /* open the elf file */
    if (!(c = OpenElfFile(fp, &hdr)))
        return Error("failed to open elf file");
        
    /* build the .binary image */
    if (!(imagebuf = BuildInternalImage(config, c, &start, &imageSize)))
        return FALSE;
    
    if (FindProgramSegment(c, ".coguser0", &program) < 0)
        return Error("can't find info (.coguser0) segment");
    
    info = (SdLoaderInfo *)&imagebuf[program.paddr - start];
    info->cache_size = config->cacheSize;
    info->cache_param1 = config->cacheParam1;
    info->cache_param2 = config->cacheParam2;

    if (FindProgramSegment(c, ".coguser1", &program) < 0)
        return Error("can't find xmm_driver (.coguser1) segment");

    if (!ReadCogImage(sys, "sd_cache.dat", driverImage, &driverSize))
        return Error("reading cache driver image failed: %s", config->cacheDriver);
        
    dat = (SDCacheDatHdr *)driverImage;
    params = (SDCacheParams *)(driverImage + dat->params_off);
    memset(params, 0, sizeof(SDCacheParams));
    params->do_mask = 1 << config->sdspiDO;
    params->clk_mask = 1 << config->sdspiClk;
    params->di_mask = 1 << config->sdspiDI;
    if (config->validMask & VALID_SDSPICS)
        params->cs_clr_mask = 1 << config->sdspiCS;
    else if (config->validMask & VALID_SDSPICLR)
        params->cs_clr_mask = 1 << config->sdspiClr;
    if (config->validMask & VALID_SDSPISEL)
        params->select_inc_mask = config->sdspiSel;
    else if (config->validMask & VALID_SDSPIINC)
        params->select_inc_mask = 1 << config->sdspiInc;
    params->select_mask = config->sdspiMsk;
    memcpy(&imagebuf[program.paddr - start], driverImage, driverSize);
    info->use_cache_driver_for_sd = TRUE;
            
    /* update the checksum */
    UpdateChecksum(imagebuf, imageSize);

    /* determine the download mode */
    if (flags & LFLAG_WRITE_EEPROM)
        mode = flags & LFLAG_RUN ? DOWNLOAD_RUN_EEPROM : DOWNLOAD_EEPROM;
    else if (flags & LFLAG_RUN)
        mode = DOWNLOAD_RUN_BINARY;
    else
        return Error("expecting -e and/or -r");
        
    /* load the serial helper program */
    if (ploadbuf(imagebuf, imageSize, mode) != 0) {
        free(imagebuf);
        return Error("load failed");
    }
    
    /* free the image buffer */
    free(imagebuf);

    return TRUE;
}

static int WriteSpinBinaryFile(BoardConfig *config, char *path, ElfContext *c)
{
    char outfile[PATH_MAX];
    uint8_t *imagebuf;
    uint32_t start;
    int imageSize;
    FILE *fp;
    
    /* build the .binary image */
    if (!(imagebuf = BuildInternalImage(config, c, &start, &imageSize)))
        return FALSE;
    
    /* write the spin .binary file */
    ConstructOutputName(outfile, path, ".binary");
    if (!(fp = fopen(outfile, "wb"))) {
        free(imagebuf);
        return Error("can't create '%s'", outfile);
    }
    
    if (fwrite(imagebuf, 1, imageSize, fp) != imageSize) {
        free(imagebuf);
        return Error("error writing '%s'", outfile);
    }
    
    fclose(fp);
    
    return TRUE;
}

static uint8_t *BuildInternalImage(BoardConfig *config, ElfContext *c, uint32_t *pStart, int *pImageSize)
{
    uint32_t start, imageSize;
    uint8_t *imagebuf, *buf;
    ElfProgramHdr program;
    SpinHdr *hdr;
    int i;

    /* get the total size of the program */
    if (!GetProgramSize(c, &start, &imageSize))
        return NullError("can't get program size");
    
    /* allocate a buffer big enough for the entire image */
    if (!(imagebuf = (uint8_t *)malloc(imageSize)))
        return NullError("insufficient memory");
    memset(imagebuf, 0, imageSize);
        
    /* load each program section */
    for (i = 0; i < c->hdr.phnum; ++i) {
        if (!LoadProgramTableEntry(c, i, &program)
        ||  !(buf = LoadProgramSegment(c, &program))) {
            free(imagebuf);
            return NullError("can't load program section %d", i);
        }
        memcpy(&imagebuf[program.paddr - start], buf, program.filesz);
    }
    
    /* fixup the header to point past the spin bytecodes and generated PASM code */
    hdr = (SpinHdr *)imagebuf;
    hdr->clkfreq = config->clkfreq;
    hdr->clkmode = config->clkmode;
    hdr->vbase = imageSize;
    hdr->dbase = imageSize + 2 * sizeof(uint32_t); // stack markers
    hdr->dcurr = hdr->dbase + sizeof(uint32_t);
    
    /* update the checksum */
    UpdateChecksum(imagebuf, imageSize);
    
    /* return the image */
    *pStart = start;
    *pImageSize = imageSize;
    return imagebuf;
}

static void UpdateChecksum(uint8_t *imagebuf, int imageSize)
{
    SpinHdr *hdr = (SpinHdr *)imagebuf;
    uint32_t cnt;
    uint8_t *p;
    int chksum;
    
    /* first zero out the checksum */
    hdr->chksum = 0;
    
    /* compute the checksum */
    for (chksum = 0, p = imagebuf, cnt = imageSize; cnt > 0; --cnt)
        chksum += *p++;
        
    /* store the checksum in the header */
    hdr->chksum = SPIN_TARGET_CHECKSUM - chksum;
}

static int LoadExternalImage(System *sys, BoardConfig *config, int flags, ElfContext *c)
{
    SpinHdr *hdr = (SpinHdr *)serial_helper_array;
    SpinObj *obj = (SpinObj *)(serial_helper_array + hdr->pbase);
    SerialHelperDatHdr *dat = (SerialHelperDatHdr *)((uint8_t *)obj + (obj->pubcnt + obj->objcnt) * sizeof(uint32_t));
    uint8_t cacheDriverImage[COG_IMAGE_MAX], *buf, *imagebuf;
    int imageSize, chksum, target, i;
    uint32_t loadAddress, params[3];
    ElfProgramHdr program_kernel;

    /* check for a cache driver for loading the external image */
    if (!config->cacheDriver)
        return Error("no cache driver to load external image");
    
    /* patch serial helper for clock mode and frequency */
    hdr->clkfreq = config->clkfreq;
    hdr->clkmode = config->clkmode;
    dat->baudrate = config->baudrate;
    dat->rxpin = config->rxpin;
    dat->txpin = config->txpin;
    dat->tvpin = config->tvpin;
        
    /* recompute the checksum */
    hdr->chksum = 0;
    for (chksum = i = 0; i < serial_helper_size; ++i)
        chksum += serial_helper_array[i];
    hdr->chksum = SPIN_TARGET_CHECKSUM - chksum;
    
    /* load the serial helper program */
    if (ploadbuf(serial_helper_array, serial_helper_size, DOWNLOAD_RUN_BINARY) != 0)
        return Error("helper load failed");

    /* wait for the serial helper to complete initialization */
    if (!WaitForInitialAck())
        return Error("failed to connect to helper");
    
    /* load the cache driver */
    if (!ReadCogImage(sys, config->cacheDriver, cacheDriverImage, &imageSize))
        return Error("reading cache driver image failed: %s", config->cacheDriver);
    printf("Loading cache driver\n");
    params[0] = config->cacheSize;
    params[1] = config->cacheParam1;
    params[2] = config->cacheParam2;
    if (!SendPacket(TYPE_HUB_WRITE, (uint8_t *)"", 0)
    ||  !WriteBuffer(cacheDriverImage, imageSize)
    ||  !SendPacket(TYPE_CACHE_INIT, (uint8_t *)params, sizeof(params)))
        return Error("Loading cache driver failed");
            
    /* build the external image */
    if (!(imagebuf = BuildExternalImage(c, &loadAddress, &imageSize)))
        return FALSE;
        
    /* write the full image to memory */
    printf("Loading program image\n");
    target = (loadAddress >= FLASH_BASE ? TYPE_FLASH_WRITE : TYPE_RAM_WRITE);
    if (!SendPacket(target, (uint8_t *)"", 0)
    ||  !WriteBuffer(imagebuf, imageSize)) {
        free(imagebuf);
        return Error("Loading program image failed");
    }
    
    /* free the image buffer */
    free(imagebuf);
    
    /* find the .xmmkernel segment */
    if (FindProgramSegment(c, ".xmmkernel", &program_kernel) < 0)
        return Error("can't find .xmmkernel segment");
    
    /* load the .kernel section */
    if (!(buf = LoadProgramSegment(c, &program_kernel)))
        return Error("can't load .xmmkernel section");

    /* handle downloads to eeprom */
    if (flags & LFLAG_WRITE_EEPROM) {
        int mode = (flags & LFLAG_RUN ? DOWNLOAD_RUN_EEPROM : DOWNLOAD_EEPROM);
        if (target == TYPE_FLASH_WRITE) {
            if (!WriteFlashLoader(sys, config, buf, program_kernel.filesz, mode)) {
                free(buf);
                return Error("can't load '.xmmkernel' section into eeprom");
            }
        }
        else
            return Error("no external ram eeprom loader is currently available");
    }
    
    /* handle downloads to hub memory */
    else if (flags & LFLAG_RUN) {
        printf("Loading .xmmkernel\n");
        if (!SendPacket(TYPE_HUB_WRITE, (uint8_t *)"", 0)
        ||  !WriteBuffer(buf, program_kernel.filesz)
        ||  !SendPacket(TYPE_VM_INIT, (uint8_t *)"", 0))
            return Error("can't loading xmm kernel");
        if (!SendPacket(TYPE_RUN, (uint8_t *)"", 0))
            return Error("can't run program");
    }
    
    /* free the '.xmmkernel' section data */
    free(buf);
    
    return TRUE;
}
    
static int WriteExecutableFile(char *path, ElfContext *c)
{
    char outfile[PATH_MAX];
    ElfProgramHdr program_kernel;
    uint8_t *imagebuf, *buf;
    uint32_t loadAddress;
    PexeFileHdr hdr;
    int imageSize;
    FILE *fp;
    
    /* build the external image */
    if (!(imagebuf = BuildExternalImage(c, &loadAddress, &imageSize)))
        return FALSE;
        
    /* find the .xmmkernel segment */
    if (FindProgramSegment(c, ".xmmkernel", &program_kernel) < 0)
        return Error("can't find .xmmkernel segment");
    
    /* load the .kernel section */
    if (!(buf = LoadProgramSegment(c, &program_kernel)))
        return Error("can't load .xmmkernel section");

    memset(&hdr, 0, sizeof(hdr));
    strcpy(hdr.tag, PEXE_TAG);
    hdr.version = PEXE_VERSION;
    hdr.loadAddress = loadAddress;
    memcpy(hdr.kernel, buf, program_kernel.filesz);
    
    ConstructOutputName(outfile, path, ".pex");
    if (!(fp = fopen(outfile, "wb"))) {
        free(imagebuf);
        return Error("can't create '%s'", outfile);
    }
    
    /* write the header */
    if (fwrite(&hdr, 1, sizeof(hdr), fp) != sizeof(hdr)) {
        free(imagebuf);
        return Error("error writing '%s'", outfile);
    }
    
    /* write the image */
    if (fwrite(imagebuf, 1, imageSize, fp) != imageSize) {
        free(imagebuf);
        return Error("error writing '%s'", outfile);
    }
    
    fclose(fp);
    
    return TRUE;
}
    
/*
    Create an image to load into external memory.
    
    For programs with code in external flash memory:
    
        Load all sections with flash load addresses into flash.
        
        Create an initialization entry for every section that must be
        relocated to either external RAM or hub memory at startup.
        
    For programs with code in external RAM:
    
        Load all sections with external RAM load addresses into external RAM.
        
        Create an initialization entry for every section that must be
        relocated to hub memory at startup.
        
    Do not generate initialization entries for cogsysn or cogusern
    sections that will eventually be loaded into a COG using coginit.
    These sections have vaddr == 0 and PF_X set.
    
    Do not load the kernel section as this is handled separately by the 
    loader.
*/

static uint8_t *BuildExternalImage(ElfContext *c, uint32_t *pLoadAddress, int *pImageSize)
{
    ElfProgramHdr program, program_kernel, program_header, program_hub;
    int imageSize, initTableSize, ki, hi, si, i;
    InitSection *initSection;
    uint8_t *imagebuf, *buf;
    ImageHdr *image;
    
    /* find the .xmmkernel segment */
    if ((ki = FindProgramSegment(c, ".xmmkernel", &program_kernel)) < 0)
        return NullError("can't find .xmmkernel segment");
    
    /* find the .header segment */
    if ((hi = FindProgramSegment(c, ".header", &program_header)) < 0)
        return NullError("can't find .header segment");
    
    /* find the .hub segment */
    if ((si = FindProgramSegment(c, ".hub", &program_hub)) < 0)
        return NullError("can't find .hub segment");
    
    /* determine the full image size including the hub/ram initializers */
    for (i = imageSize = initTableSize = 0; i < c->hdr.phnum; ++i) {
        if (!LoadProgramTableEntry(c, i, &program))
            return NullError("can't load program table entry %d", i);
        if (i != ki && program.offset != 0) {
            //printf("S: vaddr %08x, paddr %08x, size %08x\n", program.vaddr, program.paddr, program.filesz);
            imageSize += program.filesz;
        }
        if (i != ki && i != hi && program.offset != 0) {
            if (i == si || program.filesz == 0 || (program.vaddr != program.paddr && program.vaddr > 0)) {
                //printf("I: vaddr %08x, paddr %08x, size %08x\n", program.vaddr, program.paddr, program.filesz);
                ++initTableSize;
            }
        }
    }
    //printf("size %08x, init entries %d\n", imageSize, initTableSize);
    
    /* allocate a buffer big enough for the entire image */
    if (!(imagebuf = (uint8_t *)malloc(imageSize + initTableSize * sizeof(InitSection))))
        return NullError("insufficent memory for %d byte image", imageSize + initTableSize * sizeof(InitSection));
    
    /* load the image data */
    for (i = 0; i < c->hdr.phnum; ++i) {
        if (!LoadProgramTableEntry(c, i, &program)) {
            free(imagebuf);
            return NullError("can't load program table entry %d", i);
        }
        if (i != ki && (i == hi || program.paddr >= program_header.paddr) && program.offset != 0) {
            if (program.filesz > 0) {
                if (!(buf = LoadProgramSegment(c, &program))) {
                    free(imagebuf);
                    return NullError("can't load program section %d", i);
                }
                //printf("L: vaddr %08x, paddr %08x, size %08x\n", program.vaddr, program.paddr, program.filesz);
                memcpy(&imagebuf[program.paddr - program_header.paddr], buf, program.filesz);
                free(buf);
            }
        }
    }
    
    /* fill in the image header */
    image = (ImageHdr *)imagebuf;
    image->initCount = initTableSize;
    image->initTableOffset = imageSize;
    
    /* populate the init section table */
    initSection = (InitSection *)(imagebuf + imageSize);
    for (i = 0; i < c->hdr.phnum; ++i) {
        if (!LoadProgramTableEntry(c, i, &program)) {
            free(imagebuf);
            return NullError("can't load program table entry %d", i);
        }
        if (i != ki && i != hi) {
            if (i == si || program.filesz == 0 || (program.vaddr != program.paddr && program.vaddr > 0)) {
                initSection->vaddr = program.vaddr;
                initSection->paddr = program.paddr;
                if (program.vaddr == program.paddr)
                    initSection->size = program.memsz;
                else
                    initSection->size = program.filesz;
                //printf("T: vaddr %08x, paddr %08x, size %08x\n", initSection->vaddr, initSection->paddr, initSection->size);
                imageSize += sizeof(InitSection);
                ++initSection;
            }
        }
    }
    
    /* return the image */
    *pLoadAddress = program_header.paddr;
    *pImageSize = imageSize;
    return imagebuf;
}

static int WriteFlashLoader(System *sys, BoardConfig *config, uint8_t *vm_array, int vm_size, int mode)
{
    SpinHdr *hdr = (SpinHdr *)flash_loader_array;
    SpinObj *obj = (SpinObj *)(flash_loader_array + hdr->pbase);
    FlashLoaderDatHdr *dat = (FlashLoaderDatHdr *)((uint8_t *)obj + (obj->pubcnt + obj->objcnt) * sizeof(uint32_t));
    uint8_t cacheDriverImage[COG_IMAGE_MAX];
    int imageSize, chksum, i;
    
    if (!ReadCogImage(sys, config->cacheDriver, cacheDriverImage, &imageSize))
        return Error("reading cache driver image failed: %s", config->cacheDriver);
        
    /* patch flash loader for clock mode and frequency */
    hdr->clkfreq = config->clkfreq;
    hdr->clkmode = config->clkmode;
    
    /* copy the vm image to the binary file */
    memcpy((uint8_t *)dat + dat->vm_code_off, vm_array, vm_size);
    
    /* copy the cache driver image to the binary file */
    memcpy((uint8_t *)dat + dat->cache_code_off, cacheDriverImage, imageSize);
    
    /* get the cache size */
    dat->cache_size = config->cacheSize;
    dat->cache_param1 = config->cacheParam1;
    dat->cache_param2 = config->cacheParam2;
    
    /* recompute the checksum */
    hdr->chksum = 0;
    for (chksum = i = 0; i < flash_loader_size; ++i)
        chksum += flash_loader_array[i];
    hdr->chksum = SPIN_TARGET_CHECKSUM - chksum;
    
    /* load the lflash oader program */
    if (ploadbuf(flash_loader_array, flash_loader_size, mode) != 0)
        return Error("loader load failed");
    
    /* return successfully */
    return TRUE;
}

static int ReadCogImage(System *sys, char *name, uint8_t *buf, int *pSize)
{
    void *file;
    if (!(file = xbOpenFileInPath(sys, name, "rb")))
        return Error("can't open driver: %s", name);
    *pSize = (int)xbReadFile(file, buf, COG_IMAGE_MAX);
    xbCloseFile(file);
    return *pSize > 0;
}

static int WriteBuffer(uint8_t *buf, int size)
{
    int remaining, cnt;

    for (remaining = size; remaining > 0; remaining -= cnt, buf += cnt) {
        if ((cnt = remaining) > PKTMAXLEN)
            cnt = PKTMAXLEN;
        printf("%d bytes remaining             \r", remaining); fflush(stdout);
        if (!SendPacket(TYPE_DATA, buf, cnt))
            return Error("SendPacket DATA failed\n");
    }
    printf("%d bytes sent             \n", size);

    if (!SendPacket(TYPE_EOF, (uint8_t *)"", 0))
        return Error("SendPacket EOF failed");

    return TRUE;
}

/* ConstructOutputName - construct an output filename from an input filename */
static char *ConstructOutputName(char *outfile, const char *infile, char *ext)
{
    char *end = strrchr(infile, '.');
    if (end && !strchr(end, '/') && !strchr(end, '\\')) {
        strncpy(outfile, infile, end - infile);
        outfile[end - infile] = '\0';
    }
    else
        strcpy(outfile, infile);
    strcat(outfile, ext);
    return outfile;
}

static int Error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    printf("error: ");
    vprintf(fmt, ap);
    putchar('\n');
    va_end(ap);
    return FALSE;
}

static void *NullError(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    printf("error: ");
    vprintf(fmt, ap);
    putchar('\n');
    va_end(ap);
    return NULL;
}
