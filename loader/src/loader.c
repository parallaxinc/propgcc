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
#include "p2image.h"
#include "osint.h"
#include "pex.h"
#include "../sdloader/sd_loader.h"

/* maximum cog image size */
#define COG_IMAGE_MAX           (496 * 4)

/* SD parameters */
typedef struct {
    uint32_t sdspi_config1;
    uint32_t sdspi_config2;
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
    uint32_t cache_param3;
    uint32_t cache_param4;
    uint32_t vm_code_off;
    uint32_t cache_code_off;
} FlashLoaderDatHdr;

/* DAT header in flash_loader2.spin */
typedef struct {
    uint32_t xmem_geometry;
    uint32_t vm_code_off;
    uint32_t cache_code_off;
} FlashLoader2DatHdr;

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
extern uint8_t serial_helper2_array[];
extern int serial_helper2_size;
extern uint8_t flash_loader_array[];
extern int flash_loader_size;
extern uint8_t flash_loader2_array[];
extern int flash_loader2_size;

static int LoadElfFile(System *sys, BoardConfig *config, char *path, int flags, FILE *fp, ElfHdr *hdr);
static int LoadBinaryFile(System *sys, BoardConfig *config, char *path, int flags, FILE *fp);
static int LoadInternalImage(System *sys, BoardConfig *config, char *path, int flags, ElfContext *c);
static int WriteSpinBinaryFile(BoardConfig *config, char *path, ElfContext *c);
static int LoadExternalImage(System *sys, BoardConfig *config, int flags, ElfContext *c);
static int WriteFlashLoader(System *sys, BoardConfig *config, uint8_t *vm_array, int vm_size, int mode, ElfContext *c);
static int BuildFlashLoaderImage(System *sys, BoardConfig *config, uint8_t *vm_array, int vm_size);
static int BuildFlashLoader2Image(System *sys, BoardConfig *config, uint8_t *vm_array, int vm_size);
static int PatchVariable(ElfContext *c, uint8_t *imagebuf, uint32_t imagebase, uint32_t addr, uint32_t value, TranslateTable *table);
static uint32_t TranslateAddress(TranslateTable *table, uint32_t addr);
static uint32_t Get_sdspi_config1(BoardConfig *config);
static uint32_t Get_sdspi_config2(BoardConfig *config);
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
        
    /* '.xmmkernel' section used for external loads */
    if (FindSectionTableEntry(c, ".xmmkernel", &section)) {
    
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
    
    /* no '.xmmkernel' section for internal loads */
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
    uint8_t image[32768];
    size_t size;
    int mode;
    
    /* determine the download mode */
    if (flags & LFLAG_WRITE_EEPROM)
        mode = flags & LFLAG_RUN ? DOWNLOAD_RUN_EEPROM : DOWNLOAD_EEPROM;
    else if (flags & LFLAG_RUN)
        mode = DOWNLOAD_RUN_BINARY;
    else
        mode = SHUTDOWN_CMD;
    
    /* get the size of the image */
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    if (size > 32768)
        return Error(".binary file is larger than 32k");
        
    /* read the image */
    fseek(fp, 0, SEEK_SET);
    if (fread(image, 1, size, fp) != size)
        return Error("reading image file");
    
    /* close the image file */
    fclose(fp);
   
/* This code doesn't work correctly because IsDefaultConfiguration was flawed.
   It will need to be rewritten if we want this feature. The best approach would
   be to provide a way to determine if a config variable has a default value or
   a user-supplied value */ 
#if 0
    /* patch the clkfreq and clkmode files if we're not using the default configuration (no -b on the command line) */
    if (!IsDefaultConfiguration(config)) {
        SpinHdr *hdr = (SpinHdr *)image;
        int ivalue;
    
        /* patch clkfreq and clkmode fields */
        if (GetNumericConfigField(config, "clkfreq", &ivalue)) {
            printf("patching clkfreq with %08x\n", ivalue);
            hdr->clkfreq = ivalue;
        }
        if (GetNumericConfigField(config, "clkmode", &ivalue)) {
            printf("patching clkmode with %02x\n", ivalue);
            hdr->clkmode = ivalue;
        }
            
        /* recompute the checksum */
        UpdateChecksum(image, size);
    }
#endif

    /* load the image */
    return ploadbuf(path, image, size, mode) == 0;
}

#define CCR_FLAG_LMMSTEP 0x80

static int PatchSectionForDebug(uint8_t *imagebuf, int imageSize, int offset, ElfContext *c)
{
    ElfSymbol symbol;
    if (!FindElfSymbol(c, "__ccr__", &symbol) ) {
      return Error("unable to debug this ELF file: __ccr__ symbol missing");
    }
    offset += symbol.value;

    printf("patching for debug at offset 0x%x\n", offset);
    if (offset < 0 || offset > imageSize) {
      return Error("Bad offset for debug symbol __ccr__");
    }

    /* set the flag to enter the debugger immediately */
    imagebuf[offset] |= CCR_FLAG_LMMSTEP;
    return TRUE;
}

static int PatchLMMImageForDebug(uint8_t *imagebuf, int imageSize, ElfContext *c)
{
    ElfProgramHdr kernel;
    int err;

    if (FindProgramSegment(c, ".lmmkernel", &kernel) < 0) {
      return Error(".lmmkernel section not found");
    }
    err = PatchSectionForDebug(imagebuf, imageSize, kernel.paddr, c);

    /* recompute the checksum */
    UpdateChecksum(imagebuf, imageSize);
    return err;
}

static int LoadInternalImage(System *sys, BoardConfig *config, char *path, int flags, ElfContext *c)
{
    uint32_t start;
    uint8_t *imagebuf;
    int imageSize, cogImagesSize;
    int mode;
    
    /* build the .binary image */
    if (!(imagebuf = BuildInternalImage(config, c, &start, &imageSize, &cogImagesSize)))
        return FALSE;

    if ( (flags & LFLAG_DEBUG) != 0) {
        if (!PatchLMMImageForDebug(imagebuf, imageSize, c))
            return FALSE;
    }

    /* load the eeprom cache driver if we need to write cog images to eeprom */
    if (cogImagesSize > 0) {
        char *cacheDriver = "eeprom_cache.dat";
        uint8_t cacheDriverImage[COG_IMAGE_MAX];
        int cacheDriverImageSize;
        uint8_t *cogimagesbuf;
        uint32_t params[3];
        
        /* get the cog images */
        if (!(cogimagesbuf = GetCogImages(config, c))) {
            free(imagebuf);
            return Error("geting cog images failed");
        }
        
        /* load the serial helper program */
        if (!LoadSerialHelper(config, serial_helper_array, serial_helper_size, FALSE)) {
            free(cogimagesbuf);
            free(imagebuf);
            return FALSE;
        }
        
        /* load the cache driver */
        if (!ReadCogImage(sys, cacheDriver, cacheDriverImage, &cacheDriverImageSize)) {
            free(cogimagesbuf);
            free(imagebuf);
            return Error("reading cache driver image failed: %s", cacheDriver);
        }
        printf("Loading cache driver '%s'\n", cacheDriver);
        params[0] = 8 * 1024;
        params[1] = 0x8000; // write cog images eeprom above the LMM program
        params[2] = 0;
        if (!SendPacket(TYPE_HUB_WRITE, (uint8_t *)"", 0)
        ||  !WriteBuffer(cacheDriverImage, cacheDriverImageSize)
        ||  !SendPacket(TYPE_CACHE_INIT, (uint8_t *)params, sizeof(params))) {
            free(cogimagesbuf);
            free(imagebuf);
            return Error("Loading cache driver failed");
        }
        
        /* write the cog images to eeprom */
        printf("Writing cog images to eeprom\n");
        if (!SendPacket(TYPE_FLASH_WRITE, (uint8_t *)"", 0)
        ||  !WriteBuffer(cogimagesbuf, cogImagesSize)) {
            free(cogimagesbuf);
            free(imagebuf);
            return Error("Writing cog images failed");
        }
        
        /* free the image buffer */
        free(cogimagesbuf);
    }

    /* handle propeller 2 loads */
    if (ELF_CHIP(&c->hdr) == ELF_CHIP_P2) {
        int baudrate;
        GetNumericConfigField(config, "baudrate", &baudrate);
        if (flags & LFLAG_WRITE_EEPROM)
            p2_FlashImage(imagebuf, imageSize, 0x1000, 0x20000, baudrate);
        else
            p2_LoadImage(imagebuf, imageSize, 0x1000, 0x20000, baudrate);
    }
    
    /* otherwise, handle propeller 1 loads */
    else {
    
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
    }
    
    /* free the image buffer */
    free(imagebuf);

    return TRUE;
}

static void SetSDParams(BoardConfig *config, SDParams *params)
{
    params->sdspi_config1 = Get_sdspi_config1(config);
    params->sdspi_config2 = Get_sdspi_config2(config);
}

#if 0

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
    if (!(imagebuf = BuildInternalImage(config, c, &start, &imageSize, NULL)))
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
    if (GetNumericConfigField(config, "cache-param3", &ivalue))
        info->cache_param3 = ivalue;
    if (GetNumericConfigField(config, "cache-param4", &ivalue))
        info->cache_param4 = ivalue;
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
        return Error("no sd-driver found in the configuration");

    SDDriverDatHdr *dat = (SDDriverDatHdr *)driverImage;
    if (!ReadCogImage(sys, value, driverImage, &driverSize))
        return Error("reading sd driver image failed: %s", value);
    SetSDParams(config, &dat->sd_params);
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

#else

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
    if (!(imagebuf = BuildInternalImage(config, c, &start, &imageSize, NULL)))
        return FALSE;
    
    if (FindProgramSegment(c, ".coguser0", &program) < 0)
        return Error("can't find info (.coguser0) segment");
    
    info = (SdLoaderInfo *)&imagebuf[program.paddr - start];
    memset(info, 0, sizeof(SdLoaderInfo));
    
    if (GetNumericConfigField(config, "cache-geometry", &ivalue))
        info->cache_geometry = ivalue;
    if ((value = GetConfigField(config, "load-target")) != NULL) {
        if (strcasecmp(value, "ram") == 0)
            info->flags |= SD_LOAD_RAM;
    }

    if (FindProgramSegment(c, ".coguser1", &program) < 0)
        return Error("can't find cache driver (.coguser1) segment");

    if ((value = GetConfigField(config, "xmem-driver")) != NULL) {
        uint32_t *xmem = (uint32_t *)driverImage;
        if (!ReadCogImage(sys, value, driverImage, &driverSize))
            return Error("reading external memory driver image failed: %s", value);
        if (GetNumericConfigField(config, "xmem-param1", &ivalue))
            xmem[1] = ivalue;
        if (GetNumericConfigField(config, "xmem-param2", &ivalue))
            xmem[2] = ivalue;
        if (GetNumericConfigField(config, "xmem-param3", &ivalue))
            xmem[3] = ivalue;
        if (GetNumericConfigField(config, "xmem-param4", &ivalue))
            xmem[4] = ivalue;
        memcpy(&imagebuf[program.paddr - start], driverImage, driverSize);
    }
    else
        return Error("no external memory driver");
        
    if (FindProgramSegment(c, ".coguser2", &program) < 0)
        return Error("can't find sd_driver (.coguser2) segment");
    
    if ((value = GetConfigField(config, "sd-driver")) == NULL)
        return Error("no sd-driver found in the configuration");

    SDDriverDatHdr *dat = (SDDriverDatHdr *)driverImage;
    if (!ReadCogImage(sys, value, driverImage, &driverSize))
        return Error("reading sd driver image failed: %s", value);
    SetSDParams(config, &dat->sd_params);
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

#endif

int LoadSDCacheLoader(System *sys, BoardConfig *config, char *path, int flags)
{
#if 0
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
    if (!(imagebuf = BuildInternalImage(config, c, &start, &imageSize, NULL)))
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
    if (GetNumericConfigField(config, "cache-param3", &ivalue))
        info->cache_param3 = ivalue;
    if (GetNumericConfigField(config, "cache-param4", &ivalue))
        info->cache_param4 = ivalue;

    if (FindProgramSegment(c, ".coguser1", &program) < 0)
        return Error("can't find cache driver (.coguser1) segment");

    if (!ReadCogImage(sys, "sd_cache.dat", driverImage, &driverSize))
        return Error("reading cache driver image failed: sd_cache.dat");
        
    dat = (SDCacheDatHdr *)driverImage;
    params = (SDCacheParams *)(driverImage + dat->params_off);
    memset(params, 0, sizeof(SDCacheParams));
    SetSDParams(config, &params->sd_params);
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
#else
    return FALSE;
#endif
}

static int WriteSpinBinaryFile(BoardConfig *config, char *path, ElfContext *c)
{
    char outfile[PATH_MAX];
    uint8_t *imagebuf;
    uint32_t start;
    int imageSize;
    FILE *fp;
    
    /* build the .binary image */
    if (!(imagebuf = BuildInternalImage(config, c, &start, &imageSize, NULL)))
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
    uint8_t cacheDriverImage[COG_IMAGE_MAX], kernelImage[COG_IMAGE_MAX], *kernelbuf, *imagebuf, *helper_image;
    int cacheDriverImageSize, imageSize, target, ivalue;
    ElfProgramHdr program_kernel, program_start;
    char *cacheDriver, *value;
    int eepromFirst = FALSE;
    uint32_t loadAddress;
    int helper_size;
    
    /* build the external image */
    if (ELF_VERSION(&c->hdr) == ELF_VERSION_UNKNOWN) {
        if (!(cacheDriver = GetConfigField(config,  "cache-driver")))
            return Error("no cache driver to load external image");
        if (!(imagebuf = BuildExternalImage(config, c, &loadAddress, &imageSize)))
            return FALSE;
    }
    else {
        if (!(cacheDriver = GetConfigField(config, "xmem-driver")))
            return Error("no external memory driver to load external image");
        if (!(imagebuf = BuildExternalImage2(config, c, &loadAddress, &imageSize)))
            return FALSE;
    }
    
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
    
    /* copy the kernel to a full size COG image buffer */
    memset(kernelImage, 0, sizeof(kernelImage));
    memcpy(kernelImage, kernelbuf, program_kernel.filesz);
    free(kernelbuf);
    
    /* find the .start.kerext segment */
    if (FindProgramSegment(c, ".start.kerext", &program_start) >= 0) {
    
        /* load the .start.kerext section */
        if (!(kernelbuf = LoadProgramSegment(c, &program_start))) {
            free(imagebuf);
            return Error("can't load .start.kerext section");
        }
        
        /* preload .start.kerext */
        memcpy(&kernelImage[program_start.vaddr], kernelbuf, program_start.filesz);
        free(kernelbuf);
    }
    
    /* if debugging requested, patch */
    if ( (flags & LFLAG_DEBUG) != 0) {
        if (!(PatchSectionForDebug(kernelImage, sizeof(kernelImage), 0, c))) {
            free(imagebuf);
            return Error("can't patch .xmmkernel section");
        }
    }

    /* handle downloads to eeprom that must be done before the external memory download */
    GetNumericConfigField(config, "eeprom-first", &eepromFirst);
    if (eepromFirst && (flags & LFLAG_WRITE_EEPROM)) {
        if (target == TYPE_FLASH_WRITE) {
            if (!WriteFlashLoader(sys, config, kernelImage, sizeof(kernelImage), DOWNLOAD_EEPROM, c)) {
                free(imagebuf);
                return Error("can't load '.xmmkernel' section into eeprom");
            }
            preset();
        }
        else
            return Error("no external ram eeprom loader is currently available");
    }
    
    /* choose the serial helper program based on the elf version */
    if (ELF_VERSION(&c->hdr) == ELF_VERSION_UNKNOWN) {
        helper_image = serial_helper_array;
        helper_size = serial_helper_size;
    }
    else {
        helper_image = serial_helper2_array;
        helper_size = serial_helper2_size;
    }
    
    /* load the serial helper program */
    if (!LoadSerialHelper(config, helper_image, helper_size, FALSE)) {
        free(imagebuf);
        return FALSE;
    }
        
    /* get the cache driver image */
    if (!ReadCogImage(sys, cacheDriver, cacheDriverImage, &cacheDriverImageSize)) {
        free(imagebuf);
        return Error("reading cache driver image failed: %s", cacheDriver);
    }
    
    /* setup the cache initialization parameters */
    
    /* this branch handles the old cache drivers that include tag handling */
    if (ELF_VERSION(&c->hdr) == ELF_VERSION_UNKNOWN) {
        uint32_t params[5];
        memset(params, 0, sizeof(params));
        if (GetNumericConfigField(config, "cache-size", &ivalue))
            params[0] = ivalue;
        if (GetNumericConfigField(config, "cache-param1", &ivalue))
            params[1] = ivalue;
        if (GetNumericConfigField(config, "cache-param2", &ivalue))
            params[2] = ivalue;
        if (GetNumericConfigField(config, "cache-param3", &ivalue))
            params[3] = ivalue;
        if (GetNumericConfigField(config, "cache-param4", &ivalue))
            params[4] = ivalue;
    
        /* load the cache driver */
        printf("Loading cache driver '%s'\n", cacheDriver);
        if (!SendPacket(TYPE_HUB_WRITE, (uint8_t *)"", 0)
        ||  !WriteBuffer(cacheDriverImage, cacheDriverImageSize)
        ||  !SendPacket(TYPE_CACHE_INIT, (uint8_t *)params, sizeof(params))) {
            free(imagebuf);
            return Error("Loading cache driver failed");
        }
    }
    
    /* this branch handles the external memory drivers where the tag handling is in the kernel */
    else {
        uint32_t *xmem = (uint32_t *)cacheDriverImage;
        uint32_t params[1];
        memset(params, 0, sizeof(params));
        if (GetNumericConfigField(config, "cache-geometry", &ivalue))
            params[0] = ivalue;
        if (GetNumericConfigField(config, "xmem-param1", &ivalue))
            xmem[1] = ivalue;
        if (GetNumericConfigField(config, "xmem-param2", &ivalue))
            xmem[2] = ivalue;
        if (GetNumericConfigField(config, "xmem-param3", &ivalue))
            xmem[3] = ivalue;
        if (GetNumericConfigField(config, "xmem-param4", &ivalue))
            xmem[4] = ivalue;
    
        /* load the external memory driver */
        printf("Loading external memory driver '%s'\n", cacheDriver);
        if (!SendPacket(TYPE_HUB_WRITE, (uint8_t *)"", 0)
        ||  !WriteBuffer(cacheDriverImage, cacheDriverImageSize)
        ||  !SendPacket(TYPE_CACHE_INIT, (uint8_t *)params, sizeof(params))) {
            free(imagebuf);
            return Error("Loading external memory driver failed");
        }
    }
            
    /* write the full image to memory */
    printf("Loading program image to %s\n", target == TYPE_FLASH_WRITE ? "flash" : "RAM");
    if (!SendPacket(target, (uint8_t *)"", 0)
    ||  !WriteBuffer(imagebuf, imageSize)) {
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
                if (!WriteFlashLoader(sys, config, kernelImage, sizeof(kernelImage), mode, c))
                    return Error("can't load '.xmmkernel' section into eeprom");
            }
            else
                return Error("no external ram eeprom loader is currently available");
        }
    }
    
    /* handle downloads to hub memory */
    else if (flags & LFLAG_RUN) {
        printf("Loading .xmmkernel\n");
        if (!SendPacket(TYPE_HUB_WRITE, (uint8_t *)"", 0)
        ||  !WriteBuffer(kernelImage, sizeof(kernelImage))
        ||  !SendPacket(TYPE_VM_INIT, (uint8_t *)"", 0))
            return Error("can't loading xmm kernel");
        if (!SendPacket(TYPE_RUN, (uint8_t *)"", 0))
            return Error("can't run program");
    }
    
    return TRUE;
}

int LoadSerialHelper(BoardConfig *config, uint8_t  *image, int image_size, int needsd)
{
    SpinHdr *hdr = (SpinHdr *)image;
    SpinObj *obj = (SpinObj *)(image + hdr->pbase);
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
    UpdateChecksum(image, image_size);
    
    /* load the serial helper program */
    if (ploadbuf("the serial helper", image, image_size, DOWNLOAD_RUN_BINARY) != 0)
        return Error("helper load failed");

    /* wait for the serial helper to complete initialization */
    if (!WaitForInitialAck())
        return Error("failed to connect to helper");
    
    return TRUE;
}

static int WriteFlashLoader(System *sys, BoardConfig *config, uint8_t *vm_array, int vm_size, int mode, ElfContext *c)
{
    uint8_t *loader_image;
    int loader_size;

    /* choose the flash loader based on the elf version */
    if (ELF_VERSION(&c->hdr) == ELF_VERSION_UNKNOWN) {
        if (!BuildFlashLoaderImage(sys, config, vm_array, vm_size))
            return Error("building flash loader image failed");
        loader_image = flash_loader_array;
        loader_size = flash_loader_size;
    }
    else {
        if (!BuildFlashLoader2Image(sys, config, vm_array, vm_size))
            return Error("building flash loader image failed");
        loader_image = flash_loader2_array;
        loader_size = flash_loader2_size;
    }
    
    /* load the flash loader program */
    if (preset() != 0 || ploadbuf("the flash loader", loader_image, loader_size, mode) != 0)
        return Error("flash loader load failed");
    
    /* return successfully */
    return TRUE;
}

static int BuildFlashLoaderImage(System *sys, BoardConfig *config, uint8_t *vm_array, int vm_size)
{
    SpinHdr *hdr = (SpinHdr *)flash_loader_array;
    SpinObj *obj = (SpinObj *)(flash_loader_array + hdr->pbase);
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
    if (GetNumericConfigField(config, "cache-param3", &ivalue))
        dat->cache_param3 = ivalue;
    if (GetNumericConfigField(config, "cache-param4", &ivalue))
        dat->cache_param4 = ivalue;
    
    /* recompute the checksum */
    UpdateChecksum(flash_loader_array, flash_loader_size);
    
    /* return successfully */
    return TRUE;
}

static int BuildFlashLoader2Image(System *sys, BoardConfig *config, uint8_t *vm_array, int vm_size)
{
    SpinHdr *hdr = (SpinHdr *)flash_loader2_array;
    SpinObj *obj = (SpinObj *)(flash_loader2_array + hdr->pbase);
    FlashLoader2DatHdr *dat = (FlashLoader2DatHdr *)((uint8_t *)obj + (obj->pubcnt + obj->objcnt) * sizeof(uint32_t));
    uint8_t xmemDriverImage[COG_IMAGE_MAX];
    uint32_t *xmem = (uint32_t *)xmemDriverImage;
    int imageSize, ivalue;
    char *xmemDriver;
    
    if (!(xmemDriver = GetConfigField(config, "xmem-driver")))
        return Error("no xmem driver in board configuration");
        
    if (!ReadCogImage(sys, xmemDriver, xmemDriverImage, &imageSize))
        return Error("reading xmem driver image failed: %s", xmemDriver);
        
    /* patch flash loader for clock mode and frequency */
    GetNumericConfigField(config, "clkfreq", &ivalue);
    hdr->clkfreq = ivalue;
    GetNumericConfigField(config, "clkmode", &ivalue);
    hdr->clkmode = ivalue;
    
    /* copy the vm image to the binary file */
    memcpy((uint8_t *)dat + dat->vm_code_off, vm_array, vm_size);
    
    /* patch the external memory driver with its initialization parameters */
    if (GetNumericConfigField(config, "xmem-param1", &ivalue))
        xmem[1] = ivalue;
    if (GetNumericConfigField(config, "xmem-param2", &ivalue))
        xmem[2] = ivalue;
    if (GetNumericConfigField(config, "xmem-param3", &ivalue))
        xmem[3] = ivalue;
    if (GetNumericConfigField(config, "xmem-param4", &ivalue))
        xmem[4] = ivalue;

    /* copy the external memory driver image to the flash loader image */
    memcpy((uint8_t *)dat + dat->cache_code_off, xmemDriverImage, imageSize);
    
    /* get the cache geometry */
    if (GetNumericConfigField(config, "cache-geometry", &ivalue))
        dat->xmem_geometry = ivalue;
    
    /* recompute the checksum */
    UpdateChecksum(flash_loader2_array, flash_loader2_size);
    
    /* return successfully */
    return TRUE;
}

/* PatchVariables - patch user variables based on config file values */
void PatchVariables(BoardConfig *config, ElfContext *c, uint8_t *imagebuf, uint32_t imagebase, TranslateTable *table)
{
    int i;
    for (i = 1; i < c->symbolCnt; ++i) {
        char cname[ELFNAMEMAX], vname[ELFNAMEMAX], *p;
        ElfSymbol symbol;
        if (LoadElfSymbol(c, i, vname, &symbol) == 0 && strncmp(vname, "__cfg_", 6) == 0) {
            int value;
            
            /* remove the '__cfg_' prefix */
            strcpy(cname, &vname[6]);
            
            /* translate underscores to dashes */
            for (p = cname; *p != '\0'; ++p) {
                if (*p == '_')
                    *p = '-';
            }
            
            /* get the variable value */
            if (GetVariableValue(config, cname, &value)) {
                if (PatchVariable(c, imagebuf, imagebase, symbol.value, value, table))
                    printf("Patching %s with %08x\n", vname, value);
                else
                    printf("Unable to patch %s\n", vname);
            }
            
            /* can't find variable to patch */
            else
                printf("No value for %s in the configuration file\n", vname);
        }
    }    
}

/* PatchVariable - patch a single variable */
static int PatchVariable(ElfContext *c, uint8_t *imagebuf, uint32_t imagebase, uint32_t addr, uint32_t value, TranslateTable *table)
{
    ElfProgramHdr program;
    int i;
    for (i = 0; i < c->hdr.phnum; ++i) {
        if (LoadProgramTableEntry(c, i, &program)) {
            if (addr >= program.vaddr && addr < program.vaddr + program.filesz) {
                uint32_t offset = addr - program.vaddr + TranslateAddress(table, program.paddr) - imagebase;
                *(uint32_t *)(imagebuf + offset) = value;
                return TRUE;
            }
        }
    }
    return FALSE;
}

/* GetVariableValue - get the value of a variable to patch */
int GetVariableValue(BoardConfig *config, const char *name, int *pValue)
{
    int sts;
    if (strcmp(name, "sdspi-config1") == 0) {
        *pValue = Get_sdspi_config1(config);
        sts = TRUE;
    }
    else if (strcmp(name, "sdspi-config2") == 0) {
        *pValue = Get_sdspi_config2(config);
        sts = TRUE;
    }
    else
        sts = GetNumericConfigField(config, name, pValue);
    return sts;
}

/* TranslateAddress - translate a paddr to an laddr */
static uint32_t TranslateAddress(TranslateTable *table, uint32_t addr)
{
    if (table) {
        TranslateEntry *entry = table->entries;
        int count = table->count;
        while (--count >= 0) {
            if (addr >= entry->paddr && addr < entry->paddr + entry->size) {
                addr = addr - entry->paddr + entry->laddr;
                break;
            }
            ++entry;
        }
    }
    return addr;
}

#define  CS_CLR_PIN_MASK       0x01   // either CS or CLR
#define  INC_PIN_MASK          0x02   // for C3-style CS
#define  MUX_START_BIT_MASK    0x04   // low order bit of mux field
#define  MUX_WIDTH_MASK        0x08   // width of mux field
#define  ADDR_MASK             0x10   // device number for C3-style CS or value to write to the mux

static uint32_t Get_sdspi_config1(BoardConfig *config)
{
    int value;
    if (!GetNumericConfigField(config, "sdspi-config1", &value)) {
        int din = 0, clk = 0, dout = 0, protocol = 0;
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
        value = (din << 24) | (dout << 16) | (clk << 8) | protocol;
    }
    return (uint32_t)value;
}

static uint32_t Get_sdspi_config2(BoardConfig *config)
{
    int value;
    if (!GetNumericConfigField(config, "sdspi-config2", &value)) {
        int aa = 0, bb = 0, cc = 0, dd = 0;
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
        value = (aa << 24) | (bb << 16) | (cc << 8) | dd;
    }
    return (uint32_t)value;
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
    
    if (!LoadSerialHelper(config, serial_helper_array, serial_helper_size, TRUE)) {
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
