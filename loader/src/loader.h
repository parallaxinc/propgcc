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

#ifndef __LOADER_H__
#define __LOADER_H__

#include "config.h"
#include "system.h"
#include "loadelf.h"
#include "port.h"
#include "PLoadLib.h"

/* LoadXXX flags */
#define LFLAG_WRITE_EEPROM          (1 << 0)
#define LFLAG_RUN                   (1 << 1)
#define LFLAG_WRITE_BINARY          (1 << 2)
#define LFLAG_WRITE_PEX             (1 << 3)
#define LFLAG_WRITE_SDLOADER        (1 << 4)
#define LFLAG_WRITE_SDCACHELOADER   (1 << 5)
#define LFLAG_WRITE_SDFILE          (1 << 6)
#define LFLAG_DEBUG                 (1 << 7)

/* flags that need an open serial port */
#define NEED_PORT                   (LFLAG_WRITE_SDFILE | LFLAG_RUN | LFLAG_WRITE_EEPROM)

/* target checksum for a binary file */
#define SPIN_TARGET_CHECKSUM    0x14

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

typedef int PatchFcn(ElfContext *c, uint8_t *imagebuf, uint32_t imagebase, uint32_t addr, uint32_t value);

/* loader.c */
int LoadImage(System *sys, BoardConfig *config, char *path, int flags);
int LoadSDLoader(System *sys, BoardConfig *config, char *path, int flags);
int LoadSDCacheLoader(System *sys, BoardConfig *config, char *path, int flags);
int WriteFileToSDCard(BoardConfig *config, char *path, char *target);
int LoadSerialHelper(BoardConfig *config, int needsd);
void PatchVariables(BoardConfig *config, ElfContext *c, uint8_t *imagebuf, uint32_t imagebase);
int GetVariableValue(BoardConfig *config, const char *name, int *pValue);
char *ConstructOutputName(char *outfile, const char *infile, char *ext);
void *NullError(char *fmt, ...);
int Error(char *fmt, ...);

/* lmm-image.c */
uint8_t *BuildInternalImage(BoardConfig *config, ElfContext *c, uint32_t *pStart, int *pImageSize, int *pCogImagesSize);
uint8_t *GetCogImages(BoardConfig *config, ElfContext *c);
void UpdateChecksum(uint8_t *imagebuf, int imageSize);

/* xmm-image.c */
uint8_t *BuildExternalImage(BoardConfig *config, ElfContext *c, uint32_t *pLoadAddress, int *pImageSize);

/* pex-image.c */
int WriteExecutableFile(char *path, BoardConfig *config, ElfContext *c, char *outfile);

#endif
