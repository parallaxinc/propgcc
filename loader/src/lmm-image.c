#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "loadelf.h"
#include "loader.h"
#include "packet.h"
#include "PLoadLib.h"
#include "osint.h"

static void PatchLmmImageVariables(BoardConfig *config, ElfContext *c, uint8_t *imagebuf, uint32_t start);

uint8_t *BuildInternalImage(BoardConfig *config, ElfContext *c, uint32_t *pStart, int *pImageSize)
{
    uint32_t start, imageSize;
    uint8_t *imagebuf, *buf;
    ElfProgramHdr program;
    int ivalue, i;
    SpinHdr *hdr;

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
    
    /* patch the program with values from the config file */
    PatchLmmImageVariables(config, c, imagebuf, start);
    
    /* fixup the header to point past the spin bytecodes and generated PASM code */
    hdr = (SpinHdr *)imagebuf;
    GetNumericConfigField(config, "clkfreq", &ivalue);
    hdr->clkfreq = ivalue;
    GetNumericConfigField(config, "clkmode", &ivalue);
    hdr->clkmode = ivalue;
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

static void PatchLmmImageVariables(BoardConfig *config, ElfContext *c, uint8_t *imagebuf, uint32_t start)
{
    char *variableName;
    int i;
    for (i = 0; (variableName = GetVariableToPatch(i)) != NULL; ++i) {
        ElfSymbol symbol;
        if (FindElfSymbol(c, variableName, &symbol)) {
            int value;
            if (GetVariableValue(config, i, &value))
                *(uint32_t *)(imagebuf + symbol.value) = value;
        }
    }
}

void UpdateChecksum(uint8_t *imagebuf, int imageSize)
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
