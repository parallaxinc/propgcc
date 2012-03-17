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

/* SD parameters */
typedef struct {
    uint32_t do_mask;
    uint32_t clk_mask;
    uint32_t di_mask;
    uint32_t cs_clr_mask;
    uint32_t select_inc_mask;
    uint32_t select_mask;
} SDParams;

/* DAT header in sd_driver.spin */
typedef struct {
    uint32_t jmp_init;
    SDParams sd_params;
} SDDriverDatHdr;

/* DAT header in sd_cache.spin */
typedef struct {
    uint32_t jmp_init;
    uint32_t params_off;
} SDCacheDatHdr;

/* parameter structure in sd_cache.spin */
typedef struct {
    SDParams sd_params;
} SDCacheParams;

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
static int LoadExternalImage(System *sys, BoardConfig *config, int flags, ElfContext *c);
static int WriteFlashLoader(System *sys, BoardConfig *config, uint8_t *vm_array, int vm_size, int mode);
static int BuildFlashLoaderImage(System *sys, BoardConfig *config, uint8_t *vm_array, int vm_size);
static int ReadCogImage(System *sys, char *name, uint8_t *buf, int *pSize);
static int WriteBuffer(uint8_t *buf, int size);

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

static int SetSDParams(BoardConfig *config, SDParams *params)
{
    int value;
    
    if (!GetNumericConfigField(config, "sdspi-do", &value))
        return Error("missing configuration parameter: sdspi-do");
    params->do_mask = 1 << value;

    if (!GetNumericConfigField(config, "sdspi-clk", &value))
        return Error("missing configuration parameter: sdspi-clk");
    params->clk_mask = 1 << value;
    
    if (!GetNumericConfigField(config, "sdspi-di", &value))
        return Error("missing configuration parameter: sdspi-di");
    params->di_mask = 1 << value;

    if (GetNumericConfigField(config, "sdspi-cs", &value))
        params->cs_clr_mask = 1 << value;
    else if (GetNumericConfigField(config, "sdspi-clr", &value))
        params->cs_clr_mask = 1 << value;
        
    if (GetNumericConfigField(config, "sdspi-sel", &value))
        params->select_inc_mask = value;
    else if (GetNumericConfigField(config, "sdspi-inc", &value))
        params->select_inc_mask = 1 << value;
        
    if (GetNumericConfigField(config, "sdspi-msk", &value))
        params->select_mask = value;
        
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
    char *value;
    int ivalue;
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
    if (GetNumericConfigField(config, "cache-size", &ivalue))
        info->cache_size = ivalue;
    if (GetNumericConfigField(config, "cache-param1", &ivalue))
        info->cache_param1 = ivalue;
    if (GetNumericConfigField(config, "cache-param2", &ivalue))
        info->cache_param2 = ivalue;

    if (FindProgramSegment(c, ".coguser1", &program) < 0)
        return Error("can't find cache driver (.coguser1) segment");

    if ((value = GetConfigField(config, "cache-driver")) != NULL) {
        if (!ReadCogImage(sys, value, driverImage, &driverSize))
            return Error("reading cache driver image failed: %s", value);
        memcpy(&imagebuf[program.paddr - start], driverImage, driverSize);
    }
    else
        return Error("no cache driver");
        
    if (FindProgramSegment(c, ".coguser2", &program) < 0)
        return Error("can't find sd_driver (.coguser2) segment");
    
    if ((value = GetConfigField(config, "sd-driver")) != NULL) {
        SDDriverDatHdr *dat = (SDDriverDatHdr *)driverImage;
        if (!ReadCogImage(sys, value, driverImage, &driverSize))
            return Error("reading sd driver image failed: %s", value);
        if (!SetSDParams(config, &dat->sd_params))
            return FALSE;
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
        
    /* load the program */
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
    int ivalue;
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
    if (GetNumericConfigField(config, "cache-size", &ivalue))
        info->cache_size = ivalue;
    if (GetNumericConfigField(config, "cache-param1", &ivalue))
        info->cache_param1 = ivalue;
    if (GetNumericConfigField(config, "cache-param2", &ivalue))
        info->cache_param2 = ivalue;

    if (FindProgramSegment(c, ".coguser1", &program) < 0)
        return Error("can't find cache driver (.coguser1) segment");

    if (!ReadCogImage(sys, "sd_cache.dat", driverImage, &driverSize))
        return Error("reading cache driver image failed: sd_cache.dat");
        
    dat = (SDCacheDatHdr *)driverImage;
    params = (SDCacheParams *)(driverImage + dat->params_off);
    memset(params, 0, sizeof(SDCacheParams));
    if (!SetSDParams(config, &params->sd_params))
        return FALSE;
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
        
    /* load the program */
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

static int LoadExternalImage(System *sys, BoardConfig *config, int flags, ElfContext *c)
{
    SpinHdr *hdr = (SpinHdr *)serial_helper_array;
    SpinObj *obj = (SpinObj *)(serial_helper_array + hdr->pbase);
    SerialHelperDatHdr *dat = (SerialHelperDatHdr *)((uint8_t *)obj + (obj->pubcnt + obj->objcnt) * sizeof(uint32_t));
    uint8_t cacheDriverImage[COG_IMAGE_MAX], *kernelbuf, *imagebuf;
    int cacheDriverImageSize, imageSize, target, ivalue;
    uint32_t loadAddress, params[3];
    ElfProgramHdr program_kernel;
    int eepromFirst = FALSE;
    char *cacheDriver;

    /* check for a cache driver for loading the external image */
    if (!(cacheDriver = GetConfigField(config, "cache-driver")))
        return Error("no cache driver to load external image");
    
    /* build the external image */
    if (!(imagebuf = BuildExternalImage(c, &loadAddress, &imageSize)))
        return FALSE;
        
#if 0
    char *value; // move above if this code is enabled
    /* get the target memory space */
    if ((value = GetConfigField(config, "load-target")) != NULL) {
        if (strcasecmp(value, "flash") == 0)
            target = TYPE_FLASH_WRITE;
        else if (strcasecmp(value, "ram") == 0)
            target = TYPE_RAM_WRITE;
        else
            return Error("unexpected value for 'load-target': %s", value);
    }
    
    /* no load target so assume flash */
    else
        target = TYPE_FLASH_WRITE;
#else
    target = (loadAddress >= FLASH_BASE ? TYPE_FLASH_WRITE : TYPE_RAM_WRITE);
#endif

    /* find the .xmmkernel segment */
    if (FindProgramSegment(c, ".xmmkernel", &program_kernel) < 0) {
        free(imagebuf);
        return Error("can't find .xmmkernel segment");
    }
    
    /* load the .xmmkernel section */
    if (!(kernelbuf = LoadProgramSegment(c, &program_kernel))) {
        free(imagebuf);
        return Error("can't load .xmmkernel section");
    }

    /* handle downloads to eeprom that must be done before the external memory download */
    GetNumericConfigField(config, "eeprom-first", &eepromFirst);
    if (eepromFirst && (flags & LFLAG_WRITE_EEPROM)) {
        if (target == TYPE_FLASH_WRITE) {
            if (!WriteFlashLoader(sys, config, kernelbuf, program_kernel.filesz, DOWNLOAD_EEPROM)) {
                free(kernelbuf);
                free(imagebuf);
                return Error("can't load '.xmmkernel' section into eeprom");
            }
            preset();
        }
        else
            return Error("no external ram eeprom loader is currently available");
    }
    
    /* patch serial helper for clock mode and frequency */
    GetNumericConfigField(config, "clkfreq", &ivalue);
    hdr->clkfreq = ivalue;
    GetNumericConfigField(config, "clkmode", &ivalue);
    hdr->clkmode = ivalue;
    GetNumericConfigField(config, "baudrate", &ivalue);
    dat->baudrate = ivalue;
    GetNumericConfigField(config, "rxpin", &ivalue);
    dat->rxpin = ivalue;
    GetNumericConfigField(config, "txpin", &ivalue);
    dat->txpin = ivalue;
    if (GetNumericConfigField(config, "clkfreq", &ivalue))
        dat->tvpin = ivalue;
        
    /* recompute the checksum */
    UpdateChecksum(serial_helper_array, serial_helper_size);
    
    /* load the serial helper program */
    if (ploadbuf(serial_helper_array, serial_helper_size, DOWNLOAD_RUN_BINARY) != 0) {
        free(kernelbuf);
        free(imagebuf);
        return Error("helper load failed");
    }

    /* wait for the serial helper to complete initialization */
    if (!WaitForInitialAck()) {
        free(kernelbuf);
        free(imagebuf);
        return Error("failed to connect to helper");
    }
    
    /* load the cache driver */
    if (!ReadCogImage(sys, cacheDriver, cacheDriverImage, &cacheDriverImageSize)) {
        free(kernelbuf);
        free(imagebuf);
        return Error("reading cache driver image failed: %s", cacheDriver);
    }
    printf("Loading cache driver '%s'\n", cacheDriver);
    if (GetNumericConfigField(config, "cache-size", &ivalue))
        params[0] = ivalue;
    if (GetNumericConfigField(config, "cache-param1", &ivalue))
        params[1] = ivalue;
    if (GetNumericConfigField(config, "cache-param2", &ivalue))
        params[2] = ivalue;
    if (!SendPacket(TYPE_HUB_WRITE, (uint8_t *)"", 0)
    ||  !WriteBuffer(cacheDriverImage, cacheDriverImageSize)
    ||  !SendPacket(TYPE_CACHE_INIT, (uint8_t *)params, sizeof(params))) {
        free(kernelbuf);
        free(imagebuf);
        return Error("Loading cache driver failed");
    }
            
    /* write the full image to memory */
    printf("Loading program image to %s\n", target == TYPE_FLASH_WRITE ? "flash" : "RAM");
    if (!SendPacket(target, (uint8_t *)"", 0)
    ||  !WriteBuffer(imagebuf, imageSize)) {
        free(kernelbuf);
        free(imagebuf);
        return Error("Loading program image failed");
    }
    
    /* free the image buffer */
    free(imagebuf);
    
    /* handle downloads to eeprom */
    if (flags & LFLAG_WRITE_EEPROM) {
        if (eepromFirst)
            preset(); // just reset to start the program loaded into eeprom above
        else {
            int mode = (flags & LFLAG_RUN ? DOWNLOAD_RUN_EEPROM : DOWNLOAD_EEPROM);
            if (target == TYPE_FLASH_WRITE) {
                if (!WriteFlashLoader(sys, config, kernelbuf, program_kernel.filesz, mode)) {
                    free(kernelbuf);
                    return Error("can't load '.xmmkernel' section into eeprom");
                }
            }
            else
                return Error("no external ram eeprom loader is currently available");
        }
    }
    
    /* handle downloads to hub memory */
    else if (flags & LFLAG_RUN) {
        printf("Loading .xmmkernel\n");
        if (!SendPacket(TYPE_HUB_WRITE, (uint8_t *)"", 0)
        ||  !WriteBuffer(kernelbuf, program_kernel.filesz)
        ||  !SendPacket(TYPE_VM_INIT, (uint8_t *)"", 0))
            return Error("can't loading xmm kernel");
        if (!SendPacket(TYPE_RUN, (uint8_t *)"", 0))
            return Error("can't run program");
    }
    
    /* free the '.xmmkernel' section data */
    free(kernelbuf);
    
    return TRUE;
}
    
static int WriteFlashLoader(System *sys, BoardConfig *config, uint8_t *vm_array, int vm_size, int mode)
{
    /* build the flash loader image */
    if (!BuildFlashLoaderImage(sys, config, vm_array, vm_size))
        return Error("building flash loader image failed");
        
    /* load the flash loader program */
    if (preset() != 0 || ploadbuf(flash_loader_array, flash_loader_size, mode) != 0)
        return Error("loader load failed");
    
    /* return successfully */
    return TRUE;
}

static int BuildFlashLoaderImage(System *sys, BoardConfig *config, uint8_t *vm_array, int vm_size)
{
    SpinHdr *hdr = (SpinHdr *)flash_loader_array;
    SpinObj *obj = ( SpinObj *)(flash_loader_array + hdr->pbase);
    FlashLoaderDatHdr *dat = (FlashLoaderDatHdr *)((uint8_t *)obj + (obj->pubcnt + obj->objcnt) * sizeof(uint32_t));
    uint8_t cacheDriverImage[COG_IMAGE_MAX];
    int imageSize, ivalue;
    char *cacheDriver;
    
    if (!(cacheDriver = GetConfigField(config, "cache-driver")))
        return Error("no cache driver in board configuration");
        
    if (!ReadCogImage(sys, cacheDriver, cacheDriverImage, &imageSize))
        return Error("reading cache driver image failed: %s", cacheDriver);
        
    /* patch flash loader for clock mode and frequency */
    GetNumericConfigField(config, "clkfreq", &ivalue);
    hdr->clkfreq = ivalue;
    GetNumericConfigField(config, "clkmode", &ivalue);
    hdr->clkmode = ivalue;
    
    /* copy the vm image to the binary file */
    memcpy((uint8_t *)dat + dat->vm_code_off, vm_array, vm_size);
    
    /* copy the cache driver image to the binary file */
    memcpy((uint8_t *)dat + dat->cache_code_off, cacheDriverImage, imageSize);
    
    /* get the cache size */
    if (GetNumericConfigField(config, "cache-size", &ivalue))
        dat->cache_size = ivalue;
    if (GetNumericConfigField(config, "cache-param1", &ivalue))
        dat->cache_param1 = ivalue;
    if (GetNumericConfigField(config, "cache-param2", &ivalue))
        dat->cache_param2 = ivalue;
    
    /* recompute the checksum */
    UpdateChecksum(flash_loader_array, flash_loader_size);
    
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
char *ConstructOutputName(char *outfile, const char *infile, char *ext)
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

int Error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    printf("error: ");
    vprintf(fmt, ap);
    putchar('\n');
    va_end(ap);
    return FALSE;
}

void *NullError(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    printf("error: ");
    vprintf(fmt, ap);
    putchar('\n');
    va_end(ap);
    return NULL;
}
