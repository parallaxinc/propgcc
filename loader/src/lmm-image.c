#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "loadelf.h"
#include "loader.h"
#include "packet.h"
#include "PLoadLib.h"
#include "p1image.h"
#include "p2image.h"
#include "osint.h"

uint8_t *BuildInternalImage(BoardConfig *config, ElfContext *c, uint32_t *pStart, int *pImageSize, int *pCogImagesSize)
{
    uint32_t start, imageSize, cogImagesSize;
    uint8_t *imagebuf, *buf;
    ElfProgramHdr program;
    int i;

    /* get the total size of the program */
    if (!GetProgramSize(c, &start, &imageSize, &cogImagesSize))
        return NullError("can't get program size");
        
    /* check to see if cog images in eeprom are allowed */
    if (cogImagesSize > 0 && !pCogImagesSize)
        return NULL;
    
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
        if (program.paddr < COG_DRIVER_IMAGE_BASE)
            memcpy(&imagebuf[program.paddr - start], buf, program.filesz);
    }
    
    /* patch the program with values from the config file */
    PatchVariables(config, c, imagebuf, 0, NULL);
    
    /* update the image header */
    if (ELF_CHIP(&c->hdr) == ELF_CHIP_P2)
        p2_UpdateHeader(config, imagebuf, imageSize);
    else
        p1_UpdateHeader(config, imagebuf, imageSize);
    
    /* return the image */
    *pStart = start;
    *pImageSize = imageSize;
    if (pCogImagesSize)
	*pCogImagesSize = cogImagesSize;
    return imagebuf;
}

uint8_t *GetCogImages(BoardConfig *config, ElfContext *c)
{
    uint32_t start, imageSize, cogImagesSize;
    uint8_t *imagebuf, *buf;
    ElfProgramHdr program;
    int i;

    /* get the total size of the program */
    if (!GetProgramSize(c, &start, &imageSize, &cogImagesSize))
        return NullError("can't get program size");
    
    /* allocate a buffer big enough for the cog images */
    if (cogImagesSize > 0) {
        if (!(imagebuf = (uint8_t *)malloc(cogImagesSize)))
            return NullError("insufficient memory");
        memset(imagebuf, 0, cogImagesSize);
    }
    else
        imagebuf = NULL;
    
    /* load each program section */
    for (i = 0; i < c->hdr.phnum; ++i) {
        if (!LoadProgramTableEntry(c, i, &program)
        ||  !(buf = LoadProgramSegment(c, &program))) {
            free(imagebuf);
            return NullError("can't load program section %d", i);
        }
        if (program.paddr >= COG_DRIVER_IMAGE_BASE)
            memcpy(&imagebuf[program.paddr - COG_DRIVER_IMAGE_BASE], buf, program.filesz);
    }
    
    /* return the image */
    return imagebuf;
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
