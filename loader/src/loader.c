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

/* SD parameters */
typedef struct {
    uint32_t do_mask;
    uint32_t clk_mask;
    uint32_t di_mask;
    uint32_t cs_clr_mask;
    uint32_t select_inc_mask;
    uint32_t select_mask;
    uint32_t select_address;
} SDParams;

/* DAT header in serial_helper.spin */
typedef struct {
    uint32_t baudrate;
    uint8_t rxpin;
    uint8_t txpin;
    uint8_t tvpin;
    uint8_t dopin;
    uint8_t clkpin;
    uint8_t dipin;
    uint8_t cspin;
    uint8_t select_address;
    uint32_t select_inc_mask;
    uint32_t select_mask;
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
#define TYPE_FILE_WRITE         3
#define TYPE_FLASH_WRITE        4
#define TYPE_RAM_WRITE          5
#define TYPE_HUB_WRITE          6
#define TYPE_DATA               7
#define TYPE_EOF                8
#define TYPE_RUN                9

extern uint8_t serial_helper_array[];
extern int serial_helper_size;
extern uint8_t flash_loader_array[];
extern int flash_loader_size;

static int LoadElfFile(System *sys, BoardConfig *config, char *path, int flags, FILE *fp, ElfHdr *hdr);
static int LoadBinaryFile(System *sys, BoardConfig *config, char *path, int flags, FILE *fp);
static int LoadInternalImage(System *sys, BoardConfig *config, char *path, int flags, ElfContext *c);
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
    
        if (flags & (LFLAG_WRITE_PEX | LFLAG_WRITE_SDLOADER | LFLAG_WRITE_SDCACHELOADER)) {
            char outfile[PATH_MAX];
        
            /* don't allow -r or -e with -x */
            if ((flags & LFLAG_WRITE_PEX) && (flags & (LFLAG_WRITE_EEPROM | LFLAG_RUN)))
                return Error("can't use -e or -r with -x");
                
            /* write an executable file */
            if (!WriteExecutableFile(path, config, c, outfile)) {
                CloseElfFile(c);
                return FALSE;
            }
            
            /* write the executable file to the sd card for -l or -z */
            if (flags & (LFLAG_WRITE_SDLOADER | LFLAG_WRITE_SDCACHELOADER)) {
                if (!WriteFileToSDCard(config, outfile, "autorun.pex")) {
                    fclose(fp);
                    return FALSE;
                }
            }
            
            /* write the sd_loader for -l */
            if (flags & LFLAG_WRITE_SDLOADER) {
                if (!LoadSDLoader(sys, config, "sd_loader.elf", flags))
                    return FALSE;
            }
            
            /* write the sd_cache_loader for -z */
            else if (flags & LFLAG_WRITE_SDCACHELOADER) {
                if (!LoadSDCacheLoader(sys, config, "sd_cache_loader.elf", flags))
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
    
        if (flags & LFLAG_WRITE_SDLOADER)
            return Error("SD loader can't be used with LMM programs");
        else if (flags & LFLAG_WRITE_SDCACHELOADER)
            return Error("SD cache can't be used with LMM programs");
    
        if (flags & LFLAG_WRITE_BINARY) {
            if (flags & (LFLAG_WRITE_EEPROM | LFLAG_RUN))
                return Error("can't use -e or -r with -s");
            if (!WriteSpinBinaryFile(config, path, c))
                return FALSE;
        }
        
        else {
            if (!LoadInternalImage(sys, config, path, flags, c)) {
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

static int LoadInternalImage(System *sys, BoardConfig *config, char *path, int flags, ElfContext *c)
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
    
    /* load the internal image */
    if (mode != SHUTDOWN_CMD && ploadbuf(path, imagebuf, imageSize, mode) != 0) {
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

    if (GetNumericConfigField(config, "sdspi-addr", &value))
        params->select_address = value;
        
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
    memset(info, 0, sizeof(SdLoaderInfo));
    
    if (GetNumericConfigField(config, "cache-size", &ivalue))
        info->cache_size = ivalue;
    if (GetNumericConfigField(config, "cache-param1", &ivalue))
        info->cache_param1 = ivalue;
    if (GetNumericConfigField(config, "cache-param2", &ivalue))
        info->cache_param2 = ivalue;
    if ((value = GetConfigField(config, "load-target")) != NULL) {
        if (strcasecmp(value, "ram") == 0)
            info->flags |= SD_LOAD_RAM;
    }
    

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
    
    if ((value = GetConfigField(config, "sd-driver")) == NULL)
        return Error("no sd_driver found in the configuration");

    SDDriverDatHdr *dat = (SDDriverDatHdr *)driverImage;
    if (!ReadCogImage(sys, value, driverImage, &driverSize))
        return Error("reading sd driver image failed: %s", value);
    if (!SetSDParams(config, &dat->sd_params))
        return FALSE;
    memcpy(&imagebuf[program.paddr - start], driverImage, driverSize);
            
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
    if (ploadbuf(path, imagebuf, imageSize, mode) != 0) {
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
    if (ploadbuf(path, imagebuf, imageSize, mode) != 0) {
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
    uint8_t cacheDriverImage[COG_IMAGE_MAX], *kernelbuf, *imagebuf;
    int cacheDriverImageSize, imageSize, target, ivalue;
    uint32_t loadAddress, params[3];
    ElfProgramHdr program_kernel;
    char *cacheDriver, *value;
    int eepromFirst = FALSE;

    /* check for a cache driver for loading the external image */
    if (!(cacheDriver = GetConfigField(config, "cache-driver")))
        return Error("no cache driver to load external image");
    
    /* build the external image */
    if (!(imagebuf = BuildExternalImage(config, c, &loadAddress, &imageSize)))
        return FALSE;
        
    /* get the target memory space */
    if ((value = GetConfigField(config, "load-target")) != NULL) {
        if (strcasecmp(value, "flash") == 0)
            target = TYPE_FLASH_WRITE;
        else if (strcasecmp(value, "ram") == 0)
            target = TYPE_RAM_WRITE;
        else
            return Error("unexpected value for 'load-target': %s", value);
    }
    
    /* no load target so check the address */
    else
        target = (loadAddress >= FLASH_BASE ? TYPE_FLASH_WRITE : TYPE_RAM_WRITE);

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
    
    /* load the serial helper program */
    if (!LoadSerialHelper(config, FALSE)) {
        free(kernelbuf);
        free(imagebuf);
        return FALSE;
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
    
int LoadSerialHelper(BoardConfig *config, int needsd)
{
    SpinHdr *hdr = (SpinHdr *)serial_helper_array;
    SpinObj *obj = (SpinObj *)(serial_helper_array + hdr->pbase);
    SerialHelperDatHdr *dat = (SerialHelperDatHdr *)((uint8_t *)obj + (obj->pubcnt + obj->objcnt) * sizeof(uint32_t));
    int ivalue;
    
    /* patch serial helper */
    if (GetNumericConfigField(config, "clkfreq", &ivalue))
        hdr->clkfreq = ivalue;
    if (GetNumericConfigField(config, "clkmode", &ivalue))
        hdr->clkmode = ivalue;
    if (GetNumericConfigField(config, "baudrate", &ivalue))
        dat->baudrate = ivalue;
    if (GetNumericConfigField(config, "rxpin", &ivalue))
        dat->rxpin = ivalue;
    if (GetNumericConfigField(config, "txpin", &ivalue))
        dat->txpin = ivalue;
    if (GetNumericConfigField(config, "tvpin", &ivalue))
        dat->tvpin = ivalue;
        
    if (needsd) {
    
        if (GetNumericConfigField(config, "sdspi-do", &ivalue))
            dat->dopin = ivalue;
        else
            return Error("missing sdspi-do pin configuration");
            
        if (GetNumericConfigField(config, "sdspi-clk", &ivalue))
            dat->clkpin = ivalue;
        else
            return Error("missing sdspi-clk pin configuration");
            
        if (GetNumericConfigField(config, "sdspi-di", &ivalue))
            dat->dipin = ivalue;
        else
            return Error("missing sdspi-di pin configuration");
            
        if (GetNumericConfigField(config, "sdspi-cs", &ivalue))
            dat->cspin = ivalue;
        else if (GetNumericConfigField(config, "sdspi-clr", &ivalue))
            dat->cspin = ivalue;
        else
            return Error("missing sdspi-cs or sdspi-clr pin configuration");

        if (GetNumericConfigField(config, "sdspi-sel", &ivalue))
            dat->select_inc_mask = ivalue;
        else if (GetNumericConfigField(config, "sdspi-inc", &ivalue))
            dat->select_inc_mask = 1 << ivalue;

        if (GetNumericConfigField(config, "sdspi-msk", &ivalue))
            dat->select_mask = ivalue;

        if (GetNumericConfigField(config, "sdspi-addr", &ivalue))
            dat->select_address = (uint8_t)ivalue;
    }
        
    /* recompute the checksum */
    UpdateChecksum(serial_helper_array, serial_helper_size);
    
    /* load the serial helper program */
    if (ploadbuf("the serial helper", serial_helper_array, serial_helper_size, DOWNLOAD_RUN_BINARY) != 0)
        return Error("helper load failed");

    /* wait for the serial helper to complete initialization */
    if (!WaitForInitialAck())
        return Error("failed to connect to helper");
    
    return TRUE;
}

static int WriteFlashLoader(System *sys, BoardConfig *config, uint8_t *vm_array, int vm_size, int mode)
{
    /* build the flash loader image */
    if (!BuildFlashLoaderImage(sys, config, vm_array, vm_size))
        return Error("building flash loader image failed");
        
    /* load the flash loader program */
    if (preset() != 0 || ploadbuf("the flash loader", flash_loader_array, flash_loader_size, mode) != 0)
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

/* variable patch table entry */
typedef struct {
    char *configName;
    char *variableName;
} VariablePatch;

/* These are user variables that, if they exist in the elf file, will be patched
   with the corresponding values from the board configuration file. */
VariablePatch variablePatchTable[] = {
/*      config entry    user variable           */
{       "baudrate",     "__serial_baudrate"     },
{       "rxpin",        "__serial_rxpin"        },
{       "txpin",        "__serial_txpin"        },
{       "tvpin",        "__tvpin"               },
{       "vgapin",       "__vgapin"              },
{       "sdspi-do",     "__sdspi_do"            },
{       "sdspi-clk",    "__sdspi_clk"           },
{       "sdspi-di",     "__sdspi_di"            },
{       "sdspi-cs",     "__sdspi_cs"            },
{       "sdspi-sel",    "__sdspi_sel"           },
{       "sdspi-clr",    "__sdspi_clr"           },
{       "sdspi-inc",    "__sdspi_inc"           },
{       "sdspi-mask",   "__sdspi_mask"          },
{       "sdspi-addr",   "__sdspi_addr"          },
{       NULL,           "__sdspi_config1"       },
{       NULL,           "__sdspi_config2"       }
};

#define  CS_CLR_PIN_MASK       0x01   // either CS or CLR
#define  INC_PIN_MASK          0x02   // for C3-style CS
#define  MUX_START_BIT_MASK    0x04   // low order bit of mux field
#define  MUX_WIDTH_MASK        0x08   // width of mux field
#define  ADDR_MASK             0x10   // device number for C3-style CS or value to write to the mux

/* GetVariableToPatch - get the name of a variable that the loader can patch */
char *GetVariableToPatch(int i)
{
    if (i < 0 || i > sizeof(variablePatchTable) / sizeof(VariablePatch))
        return NULL;
    return variablePatchTable[i].variableName;
}

/* GetVariableValue - get the value of a variable to patch */
int GetVariableValue(BoardConfig *config, int i, int *pValue)
{
    VariablePatch *patch = &variablePatchTable[i];
    int value, sts;
    if (patch->configName)
        sts = GetNumericConfigField(config, patch->configName, pValue);
    else {
        if (strcmp(patch->variableName, "__sdspi_config1") == 0) {
            if (!GetNumericConfigField(config, "sdspi-config1", pValue)) {
                int din = 0, clk = 0, dout = 0, protocol = 0, value;
                GetNumericConfigField(config, "sdspi-di", &din);
                GetNumericConfigField(config, "sdspi-clk", &clk);
                GetNumericConfigField(config, "sdspi-do", &dout);
                if (GetNumericConfigField(config, "sdspi-cs", &value)
                ||  GetNumericConfigField(config, "sdspi-clr", &value))
                    protocol |= CS_CLR_PIN_MASK;
                if (GetNumericConfigField(config, "sdspi-inc", &value))
                    protocol |= INC_PIN_MASK;
                if (GetNumericConfigField(config, "sdspi-start", &value))
                    protocol |= MUX_START_BIT_MASK;
                if (GetNumericConfigField(config, "sdspi-width", &value))
                    protocol |= MUX_WIDTH_MASK;
                if (GetNumericConfigField(config, "sdspi-addr", &value))
                    protocol |= ADDR_MASK;
                *pValue = (din << 24) | (dout << 16) | (clk << 8) | protocol;
            }
            sts = TRUE;
        }
        else if (strcmp(patch->variableName, "__sdspi_config2") == 0) {
            if (!GetNumericConfigField(config, "sdspi-config2", pValue)) {
                int aa, bb, cc, dd;
                if (GetNumericConfigField(config, "sdspi-cs", &value))
                    aa = value;
                if (GetNumericConfigField(config, "sdspi-clr", &value))
                    aa = value;
                if (GetNumericConfigField(config, "sdspi-inc", &value))
                    bb = value;
                if (GetNumericConfigField(config, "sdspi-start", &value))
                    bb = value;
                if (GetNumericConfigField(config, "sdspi-width", &value))
                    cc = value;
                if (GetNumericConfigField(config, "sdspi-addr", &value))
                    dd = value;
                *pValue = (aa << 24) | (bb << 16) | (cc << 8) | dd;
            }
            sts = TRUE;
        }
        else
            sts = FALSE;
    }
    return sts;
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
            return Error("SendPacket DATA failed");
    }
    printf("%d bytes sent             \n", size);

    if (!SendPacket(TYPE_EOF, (uint8_t *)"", 0))
        return Error("SendPacket EOF failed");

    return TRUE;
}

int WriteFileToSDCard(BoardConfig *config, char *path, char *target)
{
    uint8_t buf[PKTMAXLEN];
    size_t size, remaining, cnt;
    FILE *fp;

    /* open the file */
    if ((fp = fopen(path, "rb")) == NULL)
        return Error("can't open %s", path);
    
    if (!LoadSerialHelper(config, TRUE)) {
        fclose(fp);
        return Error("loading serial helper");
    }
    
    if (!target) {
        if (!(target = strrchr(path, '/')))
            target = path;
        else
            ++target; // skip past the slash
    }
    
    fseek(fp, 0, SEEK_END);
    size = remaining = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (!SendPacket(TYPE_FILE_WRITE, (uint8_t *)target, strlen(target) + 1)) {
        fclose(fp);
        return Error("SendPacket FILE_WRITE failed");
    }
    
    printf("Loading '%s' to SD card\n", path);
    while ((cnt = fread(buf, 1, PKTMAXLEN, fp)) > 0) {
        printf("%ld bytes remaining             \r", remaining); fflush(stdout);
        if (!SendPacket(TYPE_DATA, buf, cnt)) {
            fclose(fp);
            return Error("SendPacket DATA failed");
        }
        remaining -= cnt;
    }
    printf("%ld bytes sent             \n", size);

    fclose(fp);

    if (!SendPacket(TYPE_EOF, (uint8_t *)"", 0))
        return Error("SendPacket EOF failed");
    
    /*
       We send two EOF packets for SD card writes.  The reason is that the EOF
       packet does actual work, and that work takes time.  The packet
       transmission protocol uses read-ahead buffering on the receiving end.
       Therefore, we need to make sure the first EOF packet was received and
       processed before resetting the Prop!
    */
    if (!SendPacket(TYPE_EOF, (uint8_t *)"", 0))
        return Error("Second SendPacket EOF failed");
    
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
