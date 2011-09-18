/* loadelf.h - an elf loader for the Parallax Propeller microcontroller

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

#ifndef __LOADELF_H__
#define __LOADELF_H__

#include <stdint.h>

typedef struct {
    uint8_t     ident[16];
    uint16_t    type;
    uint16_t    machine;
    uint32_t    version;
    uint32_t    entry;
    uint32_t    phoff;
    uint32_t    shoff;
    uint32_t    flags;
    uint16_t    ehsize;
    uint16_t    phentsize;
    uint16_t    phnum;
    uint16_t    shentsize;
    uint16_t    shnum;
    uint16_t    shstrndx;
} ElfHdr;

typedef struct {
    uint32_t    name;
    uint32_t    type;
    uint32_t    flags;
    uint32_t    addr;
    uint32_t    offset;
    uint32_t    size;
    uint32_t    link;
    uint32_t    info;
    uint32_t    addralign;
    uint32_t    entsize;
} ElfSectionHdr;

typedef struct {
    uint32_t    type;
    uint32_t    offset;
    uint32_t    vaddr;
    uint32_t    paddr;
    uint32_t    filesz;
    uint32_t    memsz;
    uint32_t    flags;
    uint32_t    align;
} ElfProgramHdr;

typedef struct {
    ElfHdr hdr;
    uint32_t stringOff;
    FILE *fp;
} ElfContext;

int ReadAndCheckElfHdr(FILE *fp, ElfHdr *hdr);
ElfContext *OpenElfFile(FILE *fp, ElfHdr *hdr);
void CloseElfFile(ElfContext *c);
int GetProgramSize(ElfContext *c, uint32_t *pStart, uint32_t *pSize);
int FindSectionTableEntry(ElfContext *c, const char *name, ElfSectionHdr *section);
int FindProgramSection(ElfContext *c, const char *name, ElfProgramHdr *program);
uint8_t *LoadProgramSection(ElfContext *c, ElfProgramHdr *program);
int LoadSectionTableEntry(ElfContext *c, int i, ElfSectionHdr *section);
int LoadProgramTableEntry(ElfContext *c, int i, ElfProgramHdr *program);
void ShowElfFile(ElfContext *c);

#endif
