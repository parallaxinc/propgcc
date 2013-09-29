/*
    Create an image to load into external memory.
    
    For programs with code in external flash memory:
    
        Load all sections with flash load addresses into flash.
        Create images of all sections with hub/ram addresses in flash.
        
        Create an initialization entry for every section that must be
        relocated to either external RAM or hub memory at startup.
        
    For programs with code in external RAM:
    
        Load all sections with external RAM load addresses into external RAM.
        Create images of all sections with hub addresses in external RAM.
        
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

uint8_t *BuildExternalImage2(BoardConfig *config, ElfContext *c, uint32_t *pLoadAddress, int *pImageSize)
{
    char *initSectionName = ELF_VERSION(&c->hdr) == ELF_VERSION_UNKNOWN ? ".header" : ".init";
    ElfProgramHdr program, program_kernel, program_header;
    int initTableSize, ki, hi, i;
    InitSection *initSectionTable, *initSection;
    uint8_t *imagebuf, *buf;
    uint32_t endAddress, textSize, dataSize, imageSize, dataOffset;
    TranslateTable table;
    TranslateEntry *entry;
    ImageHdr *image;
    
    /* find the .xmmkernel segment */
    if ((ki = FindProgramSegment(c, ".xmmkernel", &program_kernel)) < 0)
        return NullError("can't find .xmmkernel segment");
    
    /* find the .header or .init segment */
    if ((hi = FindProgramSegment(c, initSectionName, &program_header)) < 0)
        return NullError("can't find %s segment", initSectionName);
        
#ifdef DEBUG_BUILD_EXTERNAL_IMAGE
    printf("header %08x\n", program_header.paddr);
#endif
    endAddress = program_header.paddr;
    initTableSize = 0;
    table.count = 0;
    dataSize = 0;
    
    /* determine the full image size including the hub/ram initializers */
#ifdef DEBUG_BUILD_EXTERNAL_IMAGE
    printf("determine sizes\n");
#endif
    for (i = 0; i < c->hdr.phnum; ++i) {
        if (!LoadProgramTableEntry(c, i, &program))
            return NullError("can't load program table entry %d", i);
        
        /* section loads above the program header */
        if (i != ki && program.paddr >= program_header.paddr) {
            if (program.filesz > 0) {
#ifdef DEBUG_BUILD_EXTERNAL_IMAGE
                printf("  %d S: vaddr %08x, paddr %08x, filesz %08x, memsz %08x\n", i, program.vaddr, program.paddr, program.filesz, program.memsz);
#endif
                if (program.paddr + program.memsz > endAddress)
                    endAddress = program.paddr + program.memsz;
            }
        }
        
        /* section loads below the program header and needs to be initialized */
        else if (i != ki) {
            if (program.filesz > 0) {
#ifdef DEBUG_BUILD_EXTERNAL_IMAGE
                printf("  %d I: vaddr %08x, paddr %08x, filesz %08x, memsz %08x\n", i, program.vaddr, program.paddr, program.filesz, program.memsz);
#endif
                dataSize += program.filesz;
                ++initTableSize;
                ++table.count;
            }
            if (program.memsz > program.filesz) {
#ifdef DEBUG_BUILD_EXTERNAL_IMAGE
                printf("  %d Z: vaddr %08x, paddr %08x, filesz %08x, memsz %08x\n", i, program.vaddr + program.filesz, program.paddr + program.filesz, 0, program.memsz - program.filesz);
#endif
                ++initTableSize;
            }
        }
    }
    

    /* compute the total file size */
    textSize = endAddress - program_header.paddr;
    imageSize = textSize + dataSize + initTableSize * sizeof(InitSection);
#ifdef DEBUG_BUILD_EXTERNAL_IMAGE
    printf("text size %08x, data size %08x, init entries %d, total %d\n", textSize, dataSize, initTableSize, imageSize);
#endif
    
    /* allocate a buffer big enough for the entire image */
    if (!(imagebuf = (uint8_t *)malloc(imageSize)))
        return NullError("insufficent memory for %d byte image", imageSize);
    memset(imagebuf, 0, imageSize);
    
    /* allocate space for the translate table */
    if (!(table.entries = (TranslateEntry *)malloc(sizeof(TranslateEntry) * table.count))) {
        free(imagebuf);
        return NullError("insufficent memory for %d byte image", imageSize);
    }
    
    /* load the image text */
#ifdef DEBUG_BUILD_EXTERNAL_IMAGE
    printf("load text\n");
#endif
    for (i = 0; i < c->hdr.phnum; ++i) {
        if (!LoadProgramTableEntry(c, i, &program)) {
            free(imagebuf);
            free(table.entries);
            return NullError("can't load program table entry %d", i);
        }
        if (i != ki && program.paddr >= program_header.paddr) {
            if (program.filesz > 0) {
                if (!(buf = LoadProgramSegment(c, &program))) {
                    free(imagebuf);
                    free(table.entries);
                    return NullError("can't load program section %d", i);
                }
#ifdef DEBUG_BUILD_EXTERNAL_IMAGE
                printf("  %d L: vaddr %08x, paddr %08x, filesz %08x, memsz %08x\n", i, program.vaddr, program.paddr, program.filesz, program.memsz);
#endif
                memcpy(&imagebuf[program.paddr - program_header.paddr], buf, program.filesz);
                free(buf);
            }
        }
    }
    
    /* fill in the image header */
    image = (ImageHdr *)imagebuf;
    image->initCount = initTableSize;
    image->initTableOffset = textSize + dataSize;
    initSectionTable = (InitSection *)(imagebuf + image->initTableOffset);
    
    /* load the image data */
#ifdef DEBUG_BUILD_EXTERNAL_IMAGE
    printf("load data\n");
#endif
    initSection = initSectionTable;
    dataOffset = textSize;
    entry = table.entries;
    for (i = 0; i < c->hdr.phnum; ++i) {
        if (!LoadProgramTableEntry(c, i, &program)) {
            free(imagebuf);
            free(table.entries);
            return NullError("can't load program table entry %d", i);
        }
        if (i != ki && program.paddr < program_header.paddr) {
            if (program.filesz > 0) {
                if (!(buf = LoadProgramSegment(c, &program))) {
                    free(imagebuf);
                    free(table.entries);
                    return NullError("can't load program section %d", i);
                }
                memcpy(&imagebuf[dataOffset], buf, program.filesz);
                free(buf);
                initSection->vaddr = program.paddr;
                initSection->paddr = program_header.paddr + dataOffset;
                initSection->size = program.filesz;
                entry->paddr = initSection->vaddr;
                entry->laddr = initSection->paddr;
                entry->size = program.filesz;
                ++entry;
#ifdef DEBUG_BUILD_EXTERNAL_IMAGE
                printf("  %d I: vaddr %08x, paddr %08x, laddr %08x, size %08x\n", i, program.vaddr, initSection->vaddr, initSection->paddr, initSection->size);
#endif
                dataOffset += program.filesz;
                ++initSection;
            }
            if (program.memsz > program.filesz) {
                initSection->vaddr = program.paddr + program.filesz;
                initSection->paddr = initSection->vaddr; // vaddr == paddr means to zero the section
                initSection->size = program.memsz - program.filesz;
#ifdef DEBUG_BUILD_EXTERNAL_IMAGE
                printf("  %d Z: vaddr %08x, paddr %08x, laddr %08x, size %08x\n", i, program.vaddr + program.filesz, initSection->vaddr, initSection->paddr, initSection->size);
#endif
                ++initSection;
            }
        }
    }
    
    /* patch user variables with values from the configuration file */
    PatchVariables(config, c, imagebuf, program_header.paddr, &table);
    
    /* free the address translation table */
    free(table.entries);

#ifdef DEBUG_BUILD_EXTERNAL_IMAGE
{
    FILE *fp = fopen("image.xmm", "wb");
    if (fp) {
        fwrite(imagebuf, 1, imageSize, fp);
        fclose(fp);
    }
}
#endif

    /* return the image */
    *pLoadAddress = program_header.paddr;
    *pImageSize = imageSize;
    return imagebuf;
}
