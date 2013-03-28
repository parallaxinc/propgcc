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

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

/* prototypes */
static void Usage(void);

int main(int argc, char *argv[])
{
    uint32_t hubStart, hubEnd, ramStart, ramEnd, flashStart, flashEnd;
    char *infile;
    int verbose, i;
    ElfHdr elfHdr;
    ElfContext *c;
    ElfProgramHdr program;
    FILE *fp;
    
    /* initialize */
    infile = NULL;
    verbose = FALSE;
    
    /* get the arguments */
    for (i = 1; i < argc; ++i) {

        /* handle switches */
        if(argv[i][0] == '-') {
            switch(argv[i][1]) {
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
    
    /* open the binary */
    if (!(fp = fopen(infile, "rb"))) {
        printf("error: can't open '%s'\n", infile);
        return 1;
    }
    
    /* check for an elf file */
    if (!ReadAndCheckElfHdr(fp, &elfHdr)) {
        printf("error: invalid input file\n");
        return 1;
    }
        
    /* open the elf file */
    if (!(c = OpenElfFile(fp, &elfHdr)))
        return FALSE;
        
#define HUB_BASE    0x00000000
#define RAM_BASE    0x20000000
#define FLASH_BASE  0x30000000
#define COG_BASE    0xc0000000

    hubStart   = 0xffffffff;
    hubEnd     = 0x00000000;
    ramStart   = 0xffffffff;
    ramEnd     = 0x00000000;
    flashStart = 0xffffffff;
    flashEnd   = 0x00000000;
    
    for (i = 0; i < c->hdr.phnum; ++i) {
        if (!LoadProgramTableEntry(c, i, &program)) {
            printf("error: can't read program header %d\n", i);
            return FALSE;
        }
        if (program.vaddr < RAM_BASE) {
            if (program.vaddr < hubStart)
                hubStart = program.vaddr;
            if (program.vaddr + program.memsz > hubEnd)
                hubEnd = program.vaddr + program.memsz;
        }
        else if (program.vaddr < FLASH_BASE) {
            if (program.vaddr < ramStart)
                ramStart = program.vaddr;
            if (program.vaddr + program.memsz > ramEnd)
                ramEnd = program.vaddr + program.memsz;
        }
        else if (program.vaddr < COG_BASE) {
            if (program.vaddr < flashStart)
                flashStart = program.vaddr;
            if (program.vaddr + program.memsz > flashEnd)
                flashEnd = program.vaddr + program.memsz;
        }
    }
    
    if (hubStart < 0xffffffff) {
        printf("hub:   ");
        if (verbose)
            printf("%08x %08x ", hubStart, hubEnd);
        printf("%d bytes\n", hubEnd - hubStart);
    }
    if (ramStart < 0xffffffff) {
        printf("ram:   ");
        if (verbose)
            printf("%08x %08x ", ramStart, ramEnd);
        printf("%d bytes\n", ramEnd - ramStart);
    }
    if (flashStart < 0xffffffff) {
        printf("flash: ");
        if (verbose)
            printf("%08x %08x ", flashStart, flashEnd);
        printf("%d bytes\n", flashEnd - flashStart);
    }

    /* close the elf file */
    CloseElfFile(c);
    
    return 0;
}

/* Usage - display a usage message and exit */
static void Usage(void)
{
printf("\
usage: propeller-elf-image-size\n\
         [ -v ]            verbose output\n\
         [ -? ]            display a usage message and exit\n\
         <name>            file to load\n");
    exit(1);
}
