/* p2load.c - propeller ii two-stage loader */
/*
    Copyright (c) 2012, David Betz
    
    Based on code by Chip Gracey from the
    Propeller II ROM loader
      Copyright (c) 2012, Parallax, Inc.
      
    MIT License
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include "loadelf.h"
#include "osint.h"

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

/* port prefix */
#if defined(CYGWIN) || defined(WIN32) || defined(MINGW)
  #define PORT_PREFIX ""
#elif defined(LINUX)
  #define PORT_PREFIX "ttyUSB"
#elif defined(MACOSX)
  #define PORT_PREFIX "cu.usbserial"
#else
  #define PORT_PREFIX ""
#endif

/* base of hub ram */
#define BASE        0x0e80
#define COGIMAGE_LO BASE
#define COGIMAGE_HI 0x1000

/* defaults */
#define BAUD_RATE   115200

/* CheckPort state structure */
typedef struct {
    int baud;
    int verbose;
    char *actualport;
} CheckPortInfo;

/* CheckPort result codes */
enum {
    CHECK_PORT_OK,
    CHECK_PORT_OPEN_FAILED,
    CHECK_PORT_NO_PROPELLER
};

/* globals */
int baudRate, baudRate2, baudRate3;

/* locals */
static int version;

/* prototypes */
static void Usage(void);
static void *LoadElfFile(FILE *fp, ElfHdr *hdr, int *pImageSize);

int p2_HardwareFound(int *pVersion);
int p2_LoadImage(uint8_t *imageBuf, int imageSize, uint32_t cogImage, uint32_t stackTop);

static int ShowPort(const char* port, void* data);
static void ShowPorts(char *prefix);
static int CheckPort(const char* port, void* data);
static int InitPort(char *prefix, char *port, int baud, int verbose, char *actualport);
static int OpenPort(const char* port, int baud);

int main(int argc, char *argv[])
{
    char actualPort[PATH_MAX], *port, *infile, *p;
    int verbose, strip, terminalMode, imageSize, err, i;
    uint32_t cogImage = COGIMAGE_LO;
    uint32_t stackTop = 0x20000;
    uint8_t *imageBuf, *ptr;
    ElfHdr elfHdr;
    FILE *fp;
    
    /* initialize */
    baudRate = baudRate2 = baudRate3 = BAUD_RATE;
    port = infile = NULL;
    verbose = strip = terminalMode = FALSE;
    
    /* get the arguments */
    for(i = 1; i < argc; ++i) {

        /* handle switches */
        if(argv[i][0] == '-') {
            switch(argv[i][1]) {
            case 'b':
                if(argv[i][2])
                    p = &argv[i][2];
                else if(++i < argc)
                    p= argv[i];
                else
                    Usage();
                if (*p != ':')
                    baudRate = baudRate2 = baudRate3 = atoi(p);
                if ((p = strchr(p, ':')) != NULL) {
                    if (*++p != ':' && *p != '\0')
                        baudRate2 = baudRate3 = atoi(p);
                    if ((p = strchr(p, ':')) != NULL) {
                        if (*++p != '\0')
                            baudRate3 = atoi(p);
                    }
                }
                break;
            case 'h':
                cogImage = COGIMAGE_HI;
                break;
            case 'n':
                stackTop = 0x8000;
                break;
            case 'p':
                if(argv[i][2])
                    port = &argv[i][2];
                else if(++i < argc)
                    port = argv[i];
                else
                    Usage();
#if defined(CYGWIN) || defined(WIN32) || defined(LINUX)
                if (isdigit((int)port[0])) {
#if defined(CYGWIN) || defined(WIN32)
                    static char buf[10];
                    sprintf(buf, "COM%d", atoi(port));
                    port = buf;
#endif
#if defined(LINUX)
                    static char buf[64];
                    sprintf(buf, "/dev/ttyUSB%d", atoi(port));
                    port = buf;
#endif
                }
#endif
#if defined(MACOSX)
                if (port[0] != '/') {
                    static char buf[64];
                    sprintf(buf, "/dev/cu.usbserial-%s", port);
                    port = buf;
                }
#endif
                break;
            case 'P':
                ShowPorts(PORT_PREFIX);
                break;
            case 's':
                strip = TRUE;
                break;
            case 't':
                terminalMode = TRUE;
                break;
            case 'v':
                verbose = TRUE;
                break;
            case '?':
                /* fall through */
            default:
                Usage();
                break;
            }
        }

        /* handle the input filename */
        else {
            if (infile)
                Usage();
            infile = argv[i];
        }
    }
    
    /* exit early if there is no file to load */
    if (!infile)
        return 0;
    
    switch (InitPort(PORT_PREFIX, port, baudRate, verbose, actualPort)) {
    case CHECK_PORT_OK:
        printf("Found propeller version %d\n", version);
        break;
    case CHECK_PORT_OPEN_FAILED:
        printf("error: opening serial port '%s'\n", port);
        perror("Error is ");
        return 1;
    case CHECK_PORT_NO_PROPELLER:
        if (port)
            printf("error: no propeller chip on port '%s'\n", port);
        else
            printf("error: can't find a port with a propeller chip\n");
        return 1;
    }
    
    printf("Loading '%s' on port %s\n", infile, actualPort);
    
    /* open the binary */
    if (!(fp = fopen(infile, "rb"))) {
        printf("error: can't open '%s'\n", infile);
        return 1;
    }
    
    /* check for an elf file */
    if (ReadAndCheckElfHdr(fp, &elfHdr)) {
        
        /* load the elf image */
        if (!(imageBuf = LoadElfFile(fp, &elfHdr, &imageSize))) {
            printf("error: can't load elf file '%s'\n", infile);
            return 1;
        }
        
        /* skip over the rom image at the start */
        ptr = imageBuf + BASE;
        imageSize -= BASE;
        
        /* propgcc always creates images that start at 0x1000 */
        cogImage = COGIMAGE_HI;
    }
    
    /* load a binary file */
    else {
    
        /* get the size of the binary file */
        fseek(fp, 0, SEEK_END);
        imageSize = (int)ftell(fp);
    
        /* strip off the space occupied by the ROM */
        if (strip) {
            if (fseek(fp, BASE, SEEK_SET) != 0) {
                printf("error: can't skip past ROM space\n");
                return 1;
            }
            if ((imageSize -= BASE) < 0) {
                printf("error: file too small\n");
                return 1;
            }
        }
        
        /* load the complete file */
        else {
            fseek(fp, 0, SEEK_SET);
        }
        
        /* allocate a buffer big enough for the entire image */
        if (!(imageBuf = (uint8_t *)malloc(imageSize))) {
            printf("error: insufficient memory\n");
            return 1;
        }
        ptr = imageBuf;
            
        /* read the binary file */
        if (fread(imageBuf, 1, imageSize, fp) != imageSize) {
            printf("error: can't load binary file '%s'\n", infile);
            return 1;
        }
        
        /* close the binary file */
        fclose(fp);
    }
    
    /* load the image */
    if ((err = p2_LoadImage(ptr, imageSize, cogImage, stackTop)) != 0)
        return err;
    
    /* free the image buffer */
    free(imageBuf);
        
    /* enter terminal mode if requested */
    if (terminalMode) {
        printf("[ Entering terminal mode. Type ESC or Control-C to exit. ]\n");
        fflush(stdout);
        if (baudRate3 != baudRate2)
            serial_baud(baudRate3);
        terminal_mode(FALSE);
    }

    return 0;
}

/* Usage - display a usage message and exit */
static void Usage(void)
{
printf("\
usage: p2load\n\
         [ -b <baud> ]     baud rate (default is %d)\n\
         [ -h ]            cog image is at $1000 instead of $0e80\n\
         [ -n ]            set stack top to $8000 for the DE0-Nano\n\
         [ -p <port> ]     serial port (default is to auto-detect the port)\n\
         [ -P ]            list available serial ports\n\
         [ -s ]            strip $0e80 bytes from the start of the file before loading\n\
         [ -t ]            enter terminal mode after running the program\n\
         [ -v ]            verbose output\n\
         [ -? ]            display a usage message and exit\n\
         <name>            file to load\n", BAUD_RATE);
    exit(1);
}

static void *LoadElfFile(FILE *fp, ElfHdr *hdr, int *pImageSize)
{
    uint32_t start, imageSize, cogImagesSize;
    uint8_t *imageBuf, *buf;
    ElfContext *c;
    ElfProgramHdr program;
    int i;

    /* open the elf file */
    if (!(c = OpenElfFile(fp, hdr)))
        return NULL;
        
    /* get the total size of the program */
    if (!GetProgramSize(c, &start, &imageSize, &cogImagesSize)) {
        CloseElfFile(c);
        return NULL;
    }
        
    /* check to see if cog images in eeprom are allowed */
    if (cogImagesSize > 0) {
        CloseElfFile(c);
        return NULL;
    }
    
    /* allocate a buffer big enough for the entire image */
    if (!(imageBuf = (uint8_t *)malloc(imageSize)))
        return NULL;
    memset(imageBuf, 0, imageSize);
        
    /* load each program section */
    for (i = 0; i < c->hdr.phnum; ++i) {
        if (!LoadProgramTableEntry(c, i, &program)
        ||  !(buf = LoadProgramSegment(c, &program))) {
            CloseElfFile(c);
            free(imageBuf);
            return NULL;
        }
        if (program.paddr < COG_DRIVER_IMAGE_BASE)
            memcpy(&imageBuf[program.paddr - start], buf, program.filesz);
    }
    
    /* close the elf file */
    CloseElfFile(c);
    
    /* return the image */
    *pImageSize = imageSize;
    return (void *)imageBuf;
}

static int ShowPort(const char* port, void* data)
{
    printf("%s\n", port);
    return 1;
}

static void ShowPorts(char *prefix)
{
    serial_find(prefix, ShowPort, NULL);
}

static int CheckPort(const char* port, void* data)
{
    CheckPortInfo* info = (CheckPortInfo*)data;
    int rc;
    if (info->verbose)
        printf("Trying %s                    \n", port); fflush(stdout);
    if ((rc = OpenPort(port, info->baud)) != CHECK_PORT_OK)
        return rc;
    if (info->actualport) {
        strncpy(info->actualport, port, PATH_MAX - 1);
        info->actualport[PATH_MAX - 1] = '\0';
    }
    return 0;
}

static int InitPort(char *prefix, char *port, int baud, int verbose, char *actualport)
{
    int result;
    
    if (port) {
        if (actualport) {
            strncpy(actualport, port, PATH_MAX - 1);
            actualport[PATH_MAX - 1] = '\0';
        }
        result = OpenPort(port, baud);
    }
    else {
        CheckPortInfo info;
        info.baud = baud;
        info.verbose = verbose;
        info.actualport = actualport;
        if (serial_find(prefix, CheckPort, &info) == 0)
            result = CHECK_PORT_OK;
        else
            result = CHECK_PORT_NO_PROPELLER;
    }
        
    return result;
}

static int OpenPort(const char* port, int baud)
{
    /* open the port */
    if (serial_init(port, baud) == 0)
        return CHECK_PORT_OPEN_FAILED;
        
    /* check for a propeller on this port */
    if (!p2_HardwareFound(&version)) {
        serial_done();
        return CHECK_PORT_NO_PROPELLER;
    }

    return CHECK_PORT_OK;
}
