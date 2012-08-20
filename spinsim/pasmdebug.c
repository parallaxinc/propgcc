/*******************************************************************************
' Author: Dave Hein
' Version 0.21
' Copyright (c) 2010, 2011
' See end of file for terms of use.
'******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "interp.h"

char *FindChar(char *str, int32_t val);
int32_t CheckWaitFlag(PasmVarsT *pasmvars, int mode);

extern char *hubram;
extern int32_t memsize;
extern int32_t loopcount;
extern int32_t cycleaccurate;

extern FILE *tracefile;

static char *condnames[16] = {
    "if_never    ", "if_nz_and_nc", "if_z_and_nc ", "if_nc       ",
    "if_nz_and_c ", "if_nz       ", "if_z_ne_c   ", "if_nz_or_nc ",
    "if_z_and_c  ", "if_z_eq_c   ", "if_z        ", "if_z_or_nc  ",
    "if_c        ", "if_nz_or_c  ", "if_z_or_c   ", "            "};


static char *opnames[64][2] = {
    {"wrbyte", "rdbyte"}, {"wrword", "rdword"}, {"wrlong", "rdlong"},
    {"hubop nr", "hubop"}, {"undef nr", "undef"}, {"undef nr", "undef"},
    {"undef nr", "undef"}, {"undef nr", "undef"}, {"ror nr", "ror"},
    {"rol nr", "rol"}, {"shr nr", "shr"}, {"shl nr", "shl"},
    {"rcr nr", "rcr"}, {"rcl nr", "rcl"}, {"sar nr", "sar"},
    {"rev nr", "rev"}, {"mins nr", "mins"}, {"maxs nr", "maxs"},
    {"min nr", "min"}, {"max nr", "max"}, {"movs nr", "movs"},
    {"movd nr", "movd"}, {"movi nr", "movi"}, {"jmp", "call"},
    {"test", "and"}, {"testn", "andn"}, {"or nr", "or"},
    {"xor nr", "xor"}, {"muxc nr", "muxc"}, {"muxnc nr", "muxnc"},
    {"muxz nr", "muxz"}, {"muxnz nr", "muxnz"}, {"add nr", "add"},
    {"cmp", "sub"}, {"addabs nr", "addabs"}, {"subabs nr", "subabs"},
    {"sumc nr", "sumc"}, {"sumnc nr", "sumnc"}, {"sumz nr", "sumz"},
    {"sumnz nr", "sumnz"}, {"mov nr", "mov"}, {"neg nr", "neg"},
    {"abs nr", "abs"}, {"absneg nr", "absneg"}, {"negc nr", "negc"},
    {"negnc nr", "negnc"}, {"negz nr", "negz"}, {"negnz nr", "negnz"},
    {"cmps", "cmps wr"}, {"cmpsx", "cmpsx wr"}, {"addx nr", "addx"},
    {"cmpx", "subx"}, {"adds nr", "adds"}, {"subs nr", "subs"},
    {"addsx nr", "addsx"}, {"subsx nr", "subsx"}, {"cmpsub nr", "cmpsub"},
    {"djnz nr", "djnz"}, {"tjnz", "tjnz wr"}, {"tjz", "tjz wr"},
    {"waitpeq", "waitpeq wr"}, {"waitpne", "waitpne wr"},
    {"waitcnt nr", "waitcnt"}, {"waitvid", "waitvid wr"}};

static char *hubops[8][2] = {
    {"clkset", "clkset wr"},   {"cogid nr", "cogid"},
    {"coginit", "coginit wr"}, {"cogstop", "cogstop wr"},
    {"locknew nr", "locknew"}, {"lockret", "lockret wr"},
    {"lockset", "lockset wr"}, {"lockclr", "lockclr wr"}};

static int32_t swapshift[32] = {
    28, 23,  8, 16, 21,  7, 21,  7,  3,  7, 13,  9, 19,  4,-13, 15,
    -8,-17, -3,  7,  2, -8,-16,  6,-21,-16, -8,-23,-26,-22,-25,-10};
    
static uint32_t swapmask[32] = {
    0x00000008, 0x00000080, 0x00200000, 0x00001000, 0x00000040, 0x00080000,
    0x00000010, 0x00020000, 0x00100000, 0x00008000, 0x00000100, 0x00000800,
    0x00000001, 0x00004000, 0x40000000, 0x00000002, 0x00800000, 0x80000000,
    0x00010000, 0x00000020, 0x00000200, 0x00040000, 0x02000000, 0x00000004,
    0x10000000, 0x00400000, 0x00002000, 0x08000000, 0x20000000, 0x01000000,
    0x04000000, 0x00000400};

static uint32_t swapit(uint32_t val1)
{
    int32_t i, shift;
    uint32_t mask;
    uint32_t val2;

    val2 = 0;
    for (i = 0; i < 32; i++)
    {
	mask = swapmask[i];
	shift = swapshift[i];
	if (shift >= 0)
	    val2 |= (val1 & mask) << shift;
        else
	    val2 |= (val1 & mask) >> -shift;
    }
    return val2;
}

void StartPasmCog(PasmVarsT *pasmvars, int32_t par, int32_t addr, int32_t cogid)
{
    int32_t i;

    pasmvars->waitflag = 0;
    pasmvars->cflag = 0;
    pasmvars->zflag = 0;
    pasmvars->pc = 0;
    pasmvars->pc1 = 512;
    pasmvars->pc2 = 512;
    pasmvars->instruct1 = 0;
    pasmvars->instruct2 = 0;
    pasmvars->cogid = cogid;
    pasmvars->state = 5;
    pasmvars->mem[496] = par;

    for (i = 0; i < 496; i++)
    {
	if (addr & 0x8000)
	    pasmvars->mem[i] = swapit(LONG(addr));
	else
	    pasmvars->mem[i] = LONG(addr);
	addr += 4;
    }
}

void DebugPasmInstruction(PasmVarsT *pasmvars)
{
    int32_t i;
    int32_t cflag = pasmvars->cflag;
    int32_t zflag = pasmvars->zflag;
    int32_t instruct, cond, pc;
    int32_t opcode, value2, value1, zcri;
    int32_t srcaddr, dstaddr;
    char *wzstr;
    char *wcstr;
    char *wrstr;
    char *ptr;
    char opstr[20];
    char *istr[2] = {" ", "#"};
    char *xstr[4] = {"X", " ", "W", "I"};
    int32_t xflag;

    // Fetch the instruction
    if (pasmvars->waitflag)
    {
	pc = pasmvars->pc2;
	instruct = pasmvars->instruct2;
    }
    else
    {
	pc = pasmvars->pc1;
	instruct = pasmvars->instruct1;
    }
    cond = (instruct >> 18) & 15;
    xflag = ((cond >> ((cflag << 1) | zflag)) & 1);

    // Check if the instruction is invalidated in the pipeline
    if (pc & 0xfffffe00)
    {
	pc &= 0x1ff;
	xflag = 3;
    }

    // Extract parameters from the instruction
    opcode = (instruct >> 26) & 63;
    srcaddr = instruct & 511;
    dstaddr = (instruct >> 9) & 511;
    zcri = (instruct >> 22) & 15;

    // Get the two operands
    value1 = pasmvars->mem[dstaddr];
    if (zcri & 1)
        value2 = srcaddr;
    else
        value2 = pasmvars->mem[srcaddr];

    // Check for wait cycles for djnz, tjnz, tjz or hubop
    if (cycleaccurate && xflag == 1)
    {
	if (opcode <= 0x07)        // hubop
        {
	    if (CheckWaitFlag(pasmvars, 3)) xflag = 2;
        }
        else if (opcode == 0x3e)   // waitcnt
        {
            int32_t result = GetCnt() - value1;
            if (result < 0 || result >= 4) xflag = 2;
        }
    }

    if (zcri&8) wzstr = " wz";
    else wzstr = "";
    if (zcri&4) wcstr = " wc";
    else wcstr = "";
    wrstr = "";

    if (opcode == 3)
        strcpy(opstr, hubops[value2 & 7][(zcri>>1)&1]);
    else
        strcpy(opstr, opnames[opcode][(zcri>>1)&1]);
    ptr = FindChar(opstr, ' ');
    if (*ptr)
    {
	*ptr++ = 0;
	if (*ptr == 'w')
	    wrstr = " wr";
	else if (*ptr == 'n')
	    wrstr = " nr";
    }

    i = strlen(opstr);

    while (i < 7) opstr[i++] = ' ';
    opstr[i] = 0;

#if 0
    fprintf(tracefile, "%3.3x %8.8x %d %d %s %s %s %3.3x %s%3.3x %8.8x %8.8x", pasmvars->pc,
        instruct, zflag, cflag, xstr[xflag], condnames[cond], opstr, dstaddr,
	istr[zcri & 1], srcaddr, value1, value2);
#else
    fprintf(tracefile, "%6d %3.3x %8.8x %s %s %s %3.3x, %s%3.3x%s%s%s", loopcount * 4, pc,
        instruct, xstr[xflag], condnames[cond], opstr, dstaddr,
	istr[zcri & 1], srcaddr, wzstr, wcstr, wrstr);
#endif
}
/*
+------------------------------------------------------------------------------------------------------------------------------+
|                                                   TERMS OF USE: MIT License                                                  |
+------------------------------------------------------------------------------------------------------------------------------+
|Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation    |
|files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,    |
|modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software|
|is furnished to do so, subject to the following conditions:                                                                   |
|                                                                                                                              |
|The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.|
|                                                                                                                              |
|THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE          |
|WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR         |
|COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,   |
|ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                         |
+------------------------------------------------------------------------------------------------------------------------------+
*/
