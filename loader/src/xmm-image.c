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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "loadelf.h"
#include "loader.h"
#include "packet.h"
#include "PLoadLib.h"
#include "osint.h"
#include "pex.h"
#include "../sdloader/sd_loader.h"

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

//#define DEBUG_BUILD_EXTERNAL_IMAGE

uint8_t *BuildExternalImage(BoardConfig *config, ElfContext *c, uint32_t *pLoadAddress, int *pImageSize)
{
    ElfProgramHdr program, program_kernel, program_header, program_hub;
    int dataSize, initTableSize, imageSize, ki, hi, si, i;
    InitSection *initSectionTable, *initSection;
    VariablePatch *patch = variablePatchTable;
    uint8_t *imagebuf, *buf;
    uint32_t endAddress;
    ImageHdr *image;
    
    /* find the .xmmkernel segment */
    if ((ki = FindProgramSegment(c, ".xmmkernel", &program_kernel)) < 0)
        return NullError("can't find .xmmkernel segment");
    
    /* find the .header segment */
    if ((hi = FindProgramSegment(c, ".header", &program_header)) < 0)
        return NullError("can't find .header segment");
        
#ifdef DEBUG_BUILD_EXTERNAL_IMAGE
    printf("header %08x\n", program_header.paddr);
#endif
    endAddress = program_header.paddr;
    
    /* find the .hub segment */
    if ((si = FindProgramSegment(c, ".hub", &program_hub)) < 0)
        return NullError("can't find .hub segment");
    
    /* determine the full image size including the hub/ram initializers */
#ifdef DEBUG_BUILD_EXTERNAL_IMAGE
    printf("determine sizes\n");
#endif
    for (i = initTableSize = 0; i < c->hdr.phnum; ++i) {
        if (!LoadProgramTableEntry(c, i, &program))
            return NullError("can't load program table entry %d", i);
        if (i != ki && program.paddr >= program_header.paddr) {
            if (program.filesz > 0) {
#ifdef DEBUG_BUILD_EXTERNAL_IMAGE
                printf("  %d S: vaddr %08x, paddr %08x, size %08x\n", i, program.vaddr, program.paddr, program.filesz);
#endif
                if (program.paddr + program.filesz > endAddress)
                    endAddress = program.paddr + program.filesz;
                if (i == si || (program.vaddr != program.paddr && program.vaddr != 0)) {
#ifdef DEBUG_BUILD_EXTERNAL_IMAGE
                    printf("  %d I: vaddr %08x, paddr %08x, size %08x\n", i, program.vaddr, program.paddr, program.filesz);
#endif
                    ++initTableSize;
                }
            }
        }
    }

    /* compute the total file size */
    dataSize = endAddress - program_header.paddr;
    imageSize = dataSize + initTableSize * sizeof(InitSection);
#ifdef DEBUG_BUILD_EXTERNAL_IMAGE
    printf("data size %08x, init entries %d, total %d\n", dataSize, initTableSize, imageSize);
#endif
    
    /* allocate a buffer big enough for the entire image */
    if (!(imagebuf = (uint8_t *)malloc(imageSize)))
        return NullError("insufficent memory for %d byte image", imageSize);
    memset(imagebuf, 0, imageSize);
    
    /* load the image data */
#ifdef DEBUG_BUILD_EXTERNAL_IMAGE
    printf("load data\n");
#endif
    for (i = 0; i < c->hdr.phnum; ++i) {
        if (!LoadProgramTableEntry(c, i, &program)) {
            free(imagebuf);
            return NullError("can't load program table entry %d", i);
        }
        if (i != ki && program.paddr >= program_header.paddr) {
            if (program.filesz > 0) {
                if (!(buf = LoadProgramSegment(c, &program))) {
                    free(imagebuf);
                    return NullError("can't load program section %d", i);
                }
#ifdef DEBUG_BUILD_EXTERNAL_IMAGE
                printf("  %d L: vaddr %08x, paddr %08x, size %08x\n", i, program.vaddr, program.paddr, program.filesz);
#endif
                memcpy(&imagebuf[program.paddr - program_header.paddr], buf, program.filesz);
                free(buf);
            }
        }
    }
    
    /* fill in the image header */
    image = (ImageHdr *)imagebuf;
    image->initCount = initTableSize;
    image->initTableOffset = dataSize;
    
    /* populate the init section table */
#ifdef DEBUG_BUILD_EXTERNAL_IMAGE
    printf("populate init table\n");
#endif
    initSectionTable = initSection = (InitSection *)(imagebuf + dataSize);
    for (i = 0; i < c->hdr.phnum; ++i) {
        if (!LoadProgramTableEntry(c, i, &program)) {
            free(imagebuf);
            return NullError("can't load program table entry %d", i);
        }
        if (i != ki && program.paddr >= program_header.paddr) {
            if (program.filesz > 0) {
                if (i == si || (program.vaddr != program.paddr && program.vaddr != 0)) {
                    initSection->vaddr = program.vaddr;
                    initSection->paddr = program.paddr;
                    initSection->size = program.filesz;
    #ifdef DEBUG_BUILD_EXTERNAL_IMAGE
                    printf("  %d T: vaddr %08x, paddr %08x, size %08x\n", i,initSection->vaddr, initSection->paddr, initSection->size);
    #endif
                    ++initSection;
                }
            }
        }
    }
    
    /* patch user variables with values from the configuration file */
    for (patch = variablePatchTable; patch->configName; ++patch) {
        int value;
        if (GetNumericConfigField(config, patch->configName, &value)) {
            ElfSymbol symbol;
            if (FindElfSymbol(c, patch->variableName, &symbol)) {
                for (i = 0; i < c->hdr.phnum; ++i) {
                    if (!LoadProgramTableEntry(c, i, &program)) {
                        free(imagebuf);
                        return NullError("can't load program table entry %d", i);
                    }
                    if (symbol.value >= program.vaddr && symbol.value < program.vaddr + program.filesz) {
                        uint32_t offset = symbol.value - program.vaddr + program.paddr - program_header.paddr;
                        *(uint32_t *)(imagebuf + offset) = value;
                        goto found;
                    }
                }
                printf("Unable to patch \"%s\"\n", patch->variableName);
            }
        }
found:
        continue;
    }

    /* return the image */
    *pLoadAddress = program_header.paddr;
    *pImageSize = imageSize;
    return imagebuf;
}
