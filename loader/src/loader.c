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

/* target checksum for a binary file */
#define SPIN_TARGET_CHECKSUM    0x14

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

static int LoadElfFile(System *sys, BoardConfig *config, char *port, char *path, int flags, FILE *fp, ElfHdr *hdr);
static int LoadBinaryFile(System *sys, BoardConfig *config, char *port, char *path, int flags, FILE *fp);
static int LoadInternalImage(System *sys, BoardConfig *config, char *port, char *path, int flags, ElfContext *c);
static int LoadExternalImage(System *sys, BoardConfig *config, char *port, char *path, int flags, ElfContext *c);
static int WriteFlashLoader(System *sys, BoardConfig *config, char *port, uint8_t *vm_array, int vm_size, int mode);
static int ReadCogImage(System *sys, char *name, uint8_t *buf, int *pSize);
static int WriteBuffer(uint8_t *buf, int size);
static char *ConstructOutputName(char *outfile, const char *infile, char *ext);
static int Error(char *fmt, ...);

int InitPort(char *port, int baud)
{
	return serial_init(port, baud);
}

int LoadImage(System *sys, BoardConfig *config, char *port, char *path, int flags)
{    
    ElfHdr hdr;
    FILE *fp;
    int sts;
    
    /* open the image file */
    if (!(fp = fopen(path, "rb")))
        return Error("failed to open '%s'", path);
    
    /* check for an elf file */
    if (ReadAndCheckElfHdr(fp, &hdr))
        sts = LoadElfFile(sys, config, port, path, flags, fp, &hdr);
    else {
        char *end = strrchr(path, '.');
        if (end && strcasecmp(end, ".elf") == 0)
            return Error("bad elf file '%s'", path);
        sts = LoadBinaryFile(sys, config, port, path, flags, fp);
    }
    
    return sts;
}

static int LoadElfFile(System *sys, BoardConfig *config, char *port, char *path, int flags, FILE *fp, ElfHdr *hdr)
{
    ElfSectionHdr section;
    ElfContext *c;
    
    /* open the elf file */
    if (!(c = OpenElfFile(fp, hdr)))
        return Error("failed to open elf file");
        
    /* load the '.init' section */
    if (!FindSectionTableEntry(c, ".init", &section)
    &&  !FindSectionTableEntry(c, ".text", &section)) {
        CloseElfFile(c);
        return Error("can't find an '.init' or '.text' section");
    }
    
    /* check for loading into hub or external memory */
	if (section.addr >= EXTERNAL_BASE) {
        if (!LoadExternalImage(sys, config, port, path, flags, c)) {
            CloseElfFile(c);
            return FALSE;
        }
    }
    else {
        if (!LoadInternalImage(sys, config, port, path, flags, c)) {
            CloseElfFile(c);
            return FALSE;
        }
    }
    
    /* close the elf file */
    CloseElfFile(c);
    
	/* return successfully */
	return TRUE;
}

static int LoadBinaryFile(System *sys, BoardConfig *config, char *port, char *path, int flags, FILE *fp)
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
    sts = ploadfp(path, fp, port, mode);
    fclose(fp);
    
    return sts == 0;
}

static int LoadInternalImage(System *sys, BoardConfig *config, char *port, char *path, int flags, ElfContext *c)
{
    uint8_t *imagebuf, *buf, *p;
    uint32_t start, size, cnt;
    ElfProgramHdr program;
    int chksum, mode, i;
    SpinHdr *hdr;
    
    /* get the total size of the program */
    if (!GetProgramSize(c, &start, &size))
        return Error("can't get program size");
    
    /* allocate a buffer big enough for the entire image */
    if (!(imagebuf = (uint8_t *)malloc(size)))
        return Error("insufficient memory");
    memset(imagebuf, 0, size);
        
    /* load each program section */
    for (i = 0; i < c->hdr.phnum; ++i) {
        if (!LoadProgramTableEntry(c, i, &program)
        ||  !(buf = LoadProgramSection(c, &program))) {
            free(imagebuf);
            return Error("can't load program section %d", i);
        }
        memcpy(&imagebuf[program.paddr - start], buf, program.filesz);
    }
    
    /* fixup the header to point past the spin bytecodes and generated PASM code */
    hdr = (SpinHdr *)imagebuf;
    hdr->clkfreq = config->clkfreq;
    hdr->clkmode = config->clkmode;
    hdr->vbase = size;
    hdr->dbase = size + 2 * sizeof(uint32_t); // stack markers
    hdr->dcurr = hdr->dbase + sizeof(uint32_t);
    
    /* compute the checksum */
    for (chksum = 0, p = imagebuf, cnt = size; cnt > 0; --cnt)
        chksum += *p++;
        
    /* store the checksum in the header */
    hdr->chksum = SPIN_TARGET_CHECKSUM - chksum;
    
    /* write the spin binary file if requested */
    if (flags & LFLAG_WRITE_BINARY) {
        char outfile[PATH_MAX];
        FILE *fp;
        ConstructOutputName(outfile, path, ".binary");
        if (!(fp = fopen(outfile, "wb"))) {
            free(imagebuf);
            return Error("can't create '%s'", outfile);
        }
        if (fwrite(imagebuf, 1, size, fp) != size) {
            free(imagebuf);
            return Error("error writing '%s'", outfile);
        }
        fclose(fp);
    }
    
    /* determine the download mode */
    if (flags & LFLAG_WRITE_EEPROM)
        mode = flags & LFLAG_RUN ? DOWNLOAD_RUN_EEPROM : DOWNLOAD_EEPROM;
    else if (flags & LFLAG_RUN)
        mode = DOWNLOAD_RUN_BINARY;
    else
        mode = SHUTDOWN_CMD;
    
	/* load the serial helper program */
    if (mode != SHUTDOWN_CMD && ploadbuf(imagebuf, size, port, mode) != 0) {
        free(imagebuf);
		return Error("load failed");
    }
    
    /* free the image buffer */
    free(imagebuf);

    return TRUE;
}

static int LoadExternalImage(System *sys, BoardConfig *config, char *port, char *path, int flags, ElfContext *c)
{
	SpinHdr *hdr = (SpinHdr *)serial_helper_array;
    SpinObj *obj = (SpinObj *)(serial_helper_array + hdr->pbase);
    SerialHelperDatHdr *dat = (SerialHelperDatHdr *)((uint8_t *)obj + (obj->pubcnt + obj->objcnt) * sizeof(uint32_t));
    uint8_t cacheDriverImage[COG_IMAGE_MAX], *buf, *buf2;
    int imageSize, chksum, target, i;
    ElfProgramHdr program;
    uint32_t size, size2;

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
    if (ploadbuf(serial_helper_array, serial_helper_size, port, DOWNLOAD_RUN_BINARY) != 0)
		return Error("helper load failed");

	/* wait for the serial helper to complete initialization */
    if (!WaitForInitialAck())
		return Error("failed to connect to helper");
    
    /* load the cache driver */
    if (config->cacheDriver) {
        uint32_t params[3];
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
    }
    
    /* no cache driver for loading external image */
    else
        return Error("no cache driver to load external image");
    
    /* load the '.init' section */
    if (!FindProgramSection(c, ".init", &program)) {
        CloseElfFile(c);
        return Error("can't find '.init' section");
    }
    
    /* load the '.init' section data */
    if (!(buf = LoadProgramSection(c, &program)))
        return Error("can't load '.init' section");
    size = program.filesz;
    
    /* load the '.data' section initializers */
    if (!FindProgramSection(c, ".data", &program)) {
        CloseElfFile(c);
        return Error("can't find '.data' section image");
    }
    
    /* load the '.data' section data initializers */
    if (!(buf2 = LoadProgramSection(c, &program)))
        return Error("can't load '.data' section image");
    size2 = program.filesz;
    
    /* combine the buffers */
    if (!(buf = (uint8_t *)realloc(buf, size + size2))) {
        free(buf);
        free(buf2);
        return Error("Insufficient memory for '.init' section buffer: %d", size + size2);
    }
    memcpy(&buf[size], buf2, size2);
    free(buf2);
    
    /* write the '.init' section to memory */
	printf("Loading .init\n");
    target = (program.paddr >= FLASH_BASE ? TYPE_FLASH_WRITE : TYPE_RAM_WRITE);
    if (!SendPacket(target, (uint8_t *)"", 0)
    ||  !WriteBuffer(buf, size + size2)) {
        free(buf);
        return Error("Loading '.init' section failed");
	}
    
    /* free the '.init' section data and the .data section initializers */
    free(buf);

    /* load the '.xmmkernel' section */
    if (!FindProgramSection(c, ".xmmkernel", &program)
    ||  !(buf = LoadProgramSection(c, &program)))
        return Error("can't load '.xmmkernel' section");
    
    /* handle downloads to eeprom */
    if (flags & LFLAG_WRITE_EEPROM) {
        int mode = (flags & LFLAG_RUN ? DOWNLOAD_RUN_EEPROM : DOWNLOAD_EEPROM);
        if (target == TYPE_FLASH_WRITE) {
            if (!WriteFlashLoader(sys, config, port, buf, program.filesz, mode)) {
                free(buf);
                return Error("can't load '.xmmkernel' section into eeprom");
            }
        }
        else
            return Error("no external ram eeprom loader is currently available");
    }
    
    /* handle downloads to hub memory */
    else if (flags & LFLAG_RUN) {
        if (!SendPacket(TYPE_HUB_WRITE, (uint8_t *)"", 0)
        ||  !WriteBuffer(buf, program.filesz)
        ||  !SendPacket(TYPE_VM_INIT, (uint8_t *)"", 0))
            return Error("can't loading xmm kernel");
        if (!SendPacket(TYPE_RUN, (uint8_t *)"", 0))
            return Error("can't run program");
    }
    
    /* free the '.xmmkernel' section data */
    free(buf);
    
    return TRUE;
}
    
static int WriteFlashLoader(System *sys, BoardConfig *config, char *port, uint8_t *vm_array, int vm_size, int mode)
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
    if (ploadbuf(flash_loader_array, flash_loader_size, port, mode) != 0)
		return Error("loader load failed");
	
	/* return successfully */
	return TRUE;
}

static int ReadCogImage(System *sys, char *name, uint8_t *buf, int *pSize)
{
    void *file;
    if (!(file = xbOpenFileInPath(sys, name, "rb")))
        return Error("can't open cache driver: %s", name);
    *pSize = xbReadFile(file, buf, COG_IMAGE_MAX);
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
