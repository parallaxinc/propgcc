/*******************************************************************************
' Author: Dave Hein
' Version 0.21
' Copyright (c) 2010, 2011
' See end of file for terms of use.
'******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "interp.h"

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

char *rdwrnames[3][4] = { {"wrbyte", "rdbyte", "rdbyte", "rdbytec"}, {"wrword",
    "rdword", "rdword", "rdwordc"}, {"wrlong", "rdlong", "rdlong", "rdlongc"}};

char rdwrzcri[4] = {0, 2, 2, 6};

char lowerzcri[64] = {1, 3, 1, 1, 3, 1, 1, 1, 1, 3, 3, 3, 1, 1, 3, 3, 3,
    1, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};

char *lower[64] = { "clkset", "cogid", "coginit_old", "cogstop", "locknew",
    "lockret", "lockset", "lockclr", "sndser", "swapzc",
    "pushzc", "popzc", "cachex", "clracc", "getacc", "getcnt", "getlfsr",
    "gettops", "getptra", "getptrb", "getpix", "getspd", "getspa",
    "getspb", "popupa", "popupb", "popa", "popb", "reta", "retb",
    "retad", "retbd", "decod2", "decod3", "decod4", "decod5", "blmask",
    "not", "onecnt", "zercnt", "incpat", "decpat", "bingry", "grybin",
    "mergew", "splitw", "seussf", "seussr", "getmull", "getmulh", "getdivq",
    "getdivr", "getsqrt", "getcorx", "getcory", "getcorz", "getphsa",
    "getphza", "getcosa", "getsina", "getphsb", "getphzb", "getcosb",
    "getsinb"};

char *upper[96] = {"nopx", "setzc", "setspa", "setspb", "addspa", "addspb",
    "subspa", "subspb", "pushdna", "pushdnb", "pusha", "pushb", "calla",
    "callb", "callad", "callbd", "wrquad", "rdquad", "setptra", "setptrb",
    "addptra", "subptra", "addptrb", "subptrb", "setpix", "setpixu",
    "setpixv", "setpixz", "setpixr", "setpixg", "setpixb", "setpixa",
    "setmula", "setmulb", "setdiva", "setdivb", "setsqrt", "setcorx",
    "setcory", "setcorz", "cordrot", "cordatn", "cordexp", "cordlog",
    "cfgdac0", "cfgdac1", "cfgdac2", "cfgdac3", "setdac0", "setdac1",
    "setdac2", "setdac3", "cfgdacs", "setdacs", "getp", "getpn", "offp",
    "notp", "clrp", "setp", "setpc", "setpnc", "setpz", "setpnz", "setcog",
    "setmap", "setquad", "setport", "setpora", "setporb", "setporc",
    "setpord", "setxch", "setpnz", "setser", "setvid", "setvidm",
    "setvidy", "setvidi", "setvidq", "setctra", "setwava", "setfrqa",
    "setphsa", "addphsa", "subphsa", "synctra", "capctra", "setctrb",
    "setwavb", "setfrqb", "setphsb", "addphsb", "subphsb", "synctrb",
    "capctrb"};

char *bitinstr[] = { "isob", "notb", "clrb",
    "setb", "setbc", "setbnc", "setbz", "setbnz"};

char *legacy0[] = {"mac", "macs", "enc", "jmp", "ror", "rol", "shr", "shl",
    "rcr", "rcl", "sar", "rev", "mins", "maxs", "min", "max", "movs", "movd",
    "movi", "jmpd", "test", "testn", "or", "xor", "muxc", "muxnc", "muxz",
    "muxnz", "add", "cmp", "addabs", "subabs", "sumc", "sumnc", "sumz",
    "sumnz", "mov", "neg", "abs", "absneg", "negc", "negnc", "negz", "negnz",
    "cmps", "cmpsx", "addx", "cmpx", "adds", "subs", "addsx", "subsx",
    "subr", "cmpsub", "incmod", "decmod"};

char legops1[8] = {0x04, 0x05, 0x07, 0x17, 0x18, 0x19, 0x21, 0x33};

char legzcri[56] = {0, 0, 2, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 0, 0, 2, 0, 2, 2, 2, 2, 2, 2, 2, 2};

char *legacy1[] = {"mul", "muls", "jmpret",
    "jmpretd", "and", "andn", "sub", "subx"};

char *endops[] = {"ijz", "ijzd", "ijnz", "ijnzd", "djz", "djzd", "djnz",
    "djnzd", "tjz", "tjzd", "tjnz", "tjnzd", "waitcnt", "waitcnt",
    "waitpeq", "waitpne", "setinda", "setindb", "cfgpins", "waitvid"};

char endzcri[] = {2, 6, 0xa, 0xe, 2, 6, 0xa, 0xe,
    0, 4, 8, 0xc, 2, 2, 8, 0xa, 2, 6, 0xa, 0xe};

char *GetOpname2(unsigned int instr, int *pzcri)
{
    int index;
    int opcode = instr >> 26;
    int zcri = (instr >> 22) & 15;
    int src = instr & 0x1ff;
    
    if (opcode < 3)
    {
	*pzcri = rdwrzcri[(zcri >> 1) & 3];
        return rdwrnames[opcode][(zcri >> 1)&3];
    }
    else if (opcode == 3)
    {
	if ((zcri & 1) == 0)
	{
	    *pzcri = 2;
	    return "coginit";
	}
	else
	{
	    if (src < 0x40)
	    {
		if (src == 8 && (zcri & 2))
		{
		    *pzcri = 3;
		    return "rcvser";
		}
		*pzcri = lowerzcri[src];
		return lower[src];
	    }
	    else if (src >= 0x80 && src <= 0x9f)
	    {
		*pzcri = 1;
		return "rep";
	    }
	    else if (src >= 0x0a0 && src <= 0x0ff)
	    {
		*pzcri = 1;
		return upper[src-0x0a0];
	    }
	    else if (src & 0x100)
	    {
		*pzcri = 3;
		return bitinstr[(src >> 5) & 7];
	    }
	    else
	    {
		*pzcri = 0;
		return "undefined";
	    }
	}
    }
    else if (opcode >= 4 && opcode <= 0x3b)
    {
	for (index = 0; index < 8; index++)
	{
	    if (opcode == legops1[index]) break;
	}
	if (index < 8 && (zcri & 2))
	{
	    *pzcri = 2;
	    return legacy1[index];
	}
	else
	{
	    *pzcri = legzcri[opcode - 4];
	    return legacy0[opcode - 4];
	}
    }
    else
    {
	index = opcode - 0x3c;
	if (opcode == 0x3e && (zcri & 2)) index += 2;
	if (opcode == 0x3f)
	    index = (index << 2) + ((zcri >> 2) & 2) + ((zcri >> 1) & 1);
	else
	    index = (index << 2) + (zcri >> 2);
	*pzcri = endzcri[index];
	return endops[index];
    }
}

void StartPasmCog2(PasmVarsT *pasmvars, int32_t par, int32_t addr, int32_t cogid)
{
    int32_t i;

    pasmvars->waitflag = 0;
    pasmvars->cflag = 0;
    pasmvars->zflag = 0;
    pasmvars->pc = 0;
    pasmvars->cogid = cogid;
    pasmvars->state = 5;
    pasmvars->mem[496] = par;
    pasmvars->ptra = par;
    pasmvars->ptrb = par;
    pasmvars->spa = 0;
    pasmvars->spb = 0;
    pasmvars->inda = 0;
    pasmvars->indatop = 0;
    pasmvars->indabot = 0;
    pasmvars->indb = 0;
    pasmvars->indbtop = 0;
    pasmvars->indbbot = 0;
    pasmvars->repcnt = 0;
    pasmvars->repbot = 0;
    pasmvars->reptop = 0;
    pasmvars->cachehubaddr = 0xffffffff;
    pasmvars->cachecogaddr = 0xffffffff;
    pasmvars->instruct1 = 0;
    pasmvars->instruct2 = 0;
    pasmvars->instruct3 = 0;
    pasmvars->pc1 = 512;
    pasmvars->pc2 = 512;
    pasmvars->pc3 = 512;

    for (i = 0; i < 496; i++)
    {
	pasmvars->mem[i] = LONG(addr);
	addr += 4;
    }
    //for (i = 496; i < 512; i++) pasmvars->mem[i] = 0;
}

void DebugPasmInstruction2(PasmVarsT *pasmvars)
{
    int32_t i;
    int32_t cflag = pasmvars->cflag;
    int32_t zflag = pasmvars->zflag;
    int32_t instruct, pc, cond, xflag;
    int32_t opcode, value2, value1, zcri, zcri0;
    int32_t srcaddr, dstaddr;
    int32_t nbit = 0;
    char *wzstr = "";
    char *wcstr = "";
    char *wrstr = "";
    char opstr[20];
    char *istr[2] = {" ", "#"};
    char *xstr[4] = {"X", " ", "W", "I"};

    // Fetch the instruction
    if (pasmvars->waitflag)
    {
	pc = pasmvars->pc3;
	instruct = pasmvars->instruct3;
    }
    else
    {
	pc = pasmvars->pc2;
	instruct = pasmvars->instruct2;
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
    if (opcode == 3 && (zcri & 3) == 3 && (srcaddr >> 7) == 1)
    {
	nbit = 1;
        zcri &= 0xd;
	value1 = dstaddr;
    }
    else
        value1 = pasmvars->mem[dstaddr];

    if (zcri & 1)
        value2 = srcaddr;
    else
        value2 = pasmvars->mem[srcaddr];

    // Check for wait cycles for for a hub op
    if (cycleaccurate && xflag == 1)
    {
        if ((opcode <= 2) || (instruct & 0xfc4001f8) == 0x0c400000 ||
            (instruct & 0xfc4001fe) == 0x0c4000b0 ||
            (instruct & 0xfc400000) == 0x0c000000)
        {
	    // Exclude if rdxxxxc and cache hit
	    if (opcode > 2 || (zcri & 6) != 6 || (value2 & 0xffffff0) != pasmvars->cachehubaddr)
	    {
	        if (CheckWaitFlag(pasmvars, 3)) xflag = 2;
	    }
        }
	else if (opcode == 0x3f && (zcri & 8) == 0) // waitcnt
	{
	    if (value1 != GetCnt()) xflag = 2;
	}
    }

    strcpy(opstr, GetOpname2(instruct, &zcri0));

    if ((zcri ^ zcri0) & 8)
    {
	if (zcri & 8)
	    wzstr = " wz";
	else
	    wzstr = " nz";
    }

    if ((zcri ^ zcri0) & 4)
    {
	if (zcri & 4)
	    wcstr = " wc";
	else
	    wcstr = " nc";
    }

    if ((zcri ^ zcri0) & 2)
    {
	if (zcri & 2)
	    wrstr = " wr";
	else
	    wrstr = " nr";
    }

    i = strlen(opstr);

    while (i < 7) opstr[i++] = ' ';
    opstr[i] = 0;

    // Check for NOP
    if (!instruct)
    {
	cond = 15;
	if (xflag == 0) xflag = 1;
	strcpy(opstr, "nop    ");
    }


#if 0
    fprintf(tracefile, "%3.3x %8.8x %d %d %s %s %s %3.3x %s%3.3x %8.8x %8.8x", pasmvars->pc,
        instruct, zflag, cflag, xstr[xflag], condnames[cond], opstr, dstaddr,
	istr[zcri & 1], srcaddr, value1, value2);
#else
    fprintf(tracefile, "%3.3x %8.8x %s %s %s %s%3.3x, %s%3.3x%s%s%s", pc,
        instruct, xstr[xflag], condnames[cond], opstr, istr[nbit], dstaddr,
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
