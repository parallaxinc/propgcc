/*******************************************************************************
' Author: Dave Hein
' Version 0.21
' Copyright (c) 2010, 2011
' See end of file for terms of use.
'******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef LINUX
#include "conion.h"
#else
#include <conio.h>
#endif
#include <ctype.h>
#include "interp.h"
#include "opcodes.h"

#define OP_NONE                      0
#define OP_UNSIGNED_OBJ_OFFSET       1
#define OP_UNSIGNED_VAR_OFFSET       2
#define OP_UNSIGNED_LOC_OFFSET       3
#define OP_DATA                      4
#define OP_SIGNED_JMP_OFFSET         5
#define OP_MEMORY_OPCODE_WRITE       6
#define OP_OBJ_CALL_PAIR             7
#define OP_SIGNED_OFFSET             8
#define OP_PACKED_LITERAL            9
#define OP_BYTE_LITERAL             10
#define OP_WORD_LITERAL             11
#define OP_NEAR_LONG_LITERAL        12
#define OP_LONG_LITERAL             13
#define OP_MEMORY_OPCODE            14
#define OP_MEMORY_OPCODE_READ       15
#define OP_COMPACT_VAR_OFFSET       16
#define OP_COMPACT_LOC_OFFSET       17

extern char *hubram;
extern int32_t memsize;
extern int32_t symflag;

extern FILE *tracefile;

void RemoveCRLF(char *str)
{
    int32_t len = strlen(str);

    if(len == 0) return;

    str += len-1;

    while (len > 0)
    {
	if (*str != 10 && *str != 13) break;
	*str-- = 0;
	len--;
    }
}

char *FindOpcode(int32_t pcurr, int32_t *ops_format, int32_t mode)
{
    int32_t opcode;
    int32_t i;

    opcode = BYTE(pcurr);

    if (mode)
    {
	opcode &= 0x7f;
	if (opcode >= 0x40)
	    opcode += 0xe0 - 0x40;
	else if (opcode >= 0x20)
	    opcode &= 0x78;
	else if (opcode >= 0x08)
	    opcode &= 0x7c;
	else
	    opcode &= 0x7e;
    }
    else
    {
        if (opcode >= 0x40 && opcode <= 0x7f) opcode &= 0x63;
    }

    for (i = 0; optable[i].opname; i++)
    {
	if (opcode != optable[i].opcode) continue;
	if (mode == 0 || (optable[i].opform >> 5) >= 2) break;
    }
    if (!optable[i].opname) return 0;
    *ops_format = optable[i].opform;
    return optable[i].opname;
}

extern char objname[100][20];
extern int32_t methodnum[100];
extern int32_t methodlev;

char *FindChar(char *str, int32_t val)
{
    while (*str && *str != val)
        str++;
    return str;
}

void ProcessRet(void)
{
    if (!symflag) return;
    methodlev--;
    fprintf(tracefile, "return %s\n\n", objname[methodlev]);
}

static char linebuf[200];

void ProcessCall(int32_t subnum, int32_t mode)
{
    int32_t methnum = 0;
    FILE *infile;

    if (!symflag) return;

    infile = fopen(objname[methodlev], "r");
    if (mode)
    {
	methodlev++;
        strcpy(objname[methodlev], objname[methodlev-1]);
    }
    methodnum[methodlev] = subnum;
    if (!infile) return;

    // Count pubs
    while (fgets(linebuf, 200, infile))
    {
	if (strncmp(linebuf, "PUB", 3) == 0 || strncmp(linebuf, "pub", 3) == 0)
	{
	    methnum++;
	    if (methnum == subnum)
	    {
	      fprintf(tracefile, "call %s:%s\n", objname[methodlev], linebuf);
		fclose(infile);
		return;
	    }
	}
    }
    fclose(infile);
    infile = fopen(objname[methodlev], "r");

    // Count pris
    while (fgets(linebuf, 200, infile))
    {
	if (strncmp(linebuf, "PRI", 3) == 0 || strncmp(linebuf, "pri", 3) == 0)
	{
	    methnum++;
	    if (methnum == subnum)
	    {
	      fprintf(tracefile, "call %s:%s\n", objname[methodlev], linebuf);
		fclose(infile);
		return;
	    }
	}
    }
}

void ProcessObjCall(int32_t objnum, int32_t subnum)
{
    int32_t mode = 0;
    int32_t methnum = 0;
    int32_t commentmode = 0;
    int32_t found = 0;
    FILE *infile;

    if (!symflag) return;

    infile = fopen(objname[methodlev], "r");
    methodlev++;
    strcpy(objname[methodlev], objname[methodlev-1]);
    methodnum[methodlev] = subnum;
    if (!infile) return;

    // Count pubs and pris
    while (fgets(linebuf, 200, infile))
    {
	if (linebuf[0] == '}')
	{
	    commentmode--;
	    if (commentmode < 0) commentmode = 0;
	    continue;
	}
	else if (linebuf[0] == '{')
	{
	    if (*FindChar(linebuf, '}') == 0)
	        commentmode++;
	}
	if (commentmode) continue;
	if (strncmp(linebuf, "PUB", 3) == 0 || strncmp(linebuf, "pub", 3) == 0 ||
	    strncmp(linebuf, "PRI", 3) == 0 || strncmp(linebuf, "pri", 3) == 0)
	{
	    methnum++;
	}
    }
    fclose(infile);
    infile = fopen(objname[methodlev], "r");

    // Count objs
    commentmode = 0;
    while (fgets(linebuf, 200, infile))
    {
	if (linebuf[0] == '}')
	{
	    commentmode--;
	    if (commentmode < 0) commentmode = 0;
	    continue;
	}
	else if (linebuf[0] == '{')
	{
	    if (*FindChar(linebuf, '}') == 0)
	        commentmode++;
	}
	if (commentmode) continue;
	if (mode == 0)
	{
	    if (strncmp(linebuf, "OBJ", 3) == 0 || strncmp(linebuf, "obj", 3) == 0)
	        mode = 1;
	}
	else
	{
	    methnum++;
	    if (linebuf[0] != ' ') break;
	    if (methnum == objnum)
	    {
		char *ptr1;
		char *ptr2;
		int32_t num;

		// Locate filename
		ptr1 = FindChar(linebuf, '"');
		if (*ptr1)
		    ptr1++;
		    ptr2 = FindChar(ptr1, '"');
		    if (*ptr2) ptr2--;
		else
		    ptr2 = ptr1;
		num = ptr2 - ptr1 + 1;
		if (num < 1) num = 1;
		memcpy(objname[methodlev], ptr1, num);
		objname[methodlev][num] = 0;
		if (symflag == 2)
		    strcat(objname[methodlev], ".spn");
		else
		    strcat(objname[methodlev], ".spin");
		fclose(infile);
		found = 1;
		break;
	    }
	}
    }
    if (!found)
    {
        fclose(infile);
	return;
    }
    ProcessCall(subnum, 0);
}

static int GetOpIndex(int opcode);
static int GetExOpIndex(int opcode);

void PrintOp(SpinVarsT *spinvars)
{
    long pcurr = spinvars->pcurr;
    int32_t opcode, opform;
    char *opstr;
    int exop1, exop2;
    int32_t val;
    int32_t operand;
    char *regop[] = {"ldreg", "streg", "exreg", "??reg"};
    char *regname[] = {"par", "cnt", "ina", "inb", "outa", "outb", "dira",
	"dirb", "ctra", "ctrb", "frqa", "frqb", "phsa", "phsb", "vcfg", "vscl"};
    char bytestr[40], symstr[100];

    if (spinvars->state != 1) return;

    opcode = BYTE(pcurr);
    exop1 = GetOpIndex(opcode);
    exop2 = -1;
    opstr = FindOpcode(pcurr, &opform, 0);

    memset(bytestr, ' ', 40);
    bytestr[20] = 0;
    sprintf(bytestr, "%4.4x %2.2x", (unsigned int)pcurr, opcode);
    symstr[0] = 0;

    switch (opform & 0x1f)
    {
        case OP_NONE:
	    strcpy(symstr, opstr);
	    if (strncmp(opstr, "ret", 3) == 0)
	        ProcessRet();
            pcurr++;
            break;
        case OP_UNSIGNED_OBJ_OFFSET:
        case OP_UNSIGNED_VAR_OFFSET:
        case OP_UNSIGNED_LOC_OFFSET:
	    operand = BYTE(pcurr+1);
	    if (operand & 0x80)
	    {
	        operand = ((operand & 0x7f) << 8) | BYTE(pcurr+2);
		sprintf(bytestr + strlen(bytestr), " %2.2x %2.2x", BYTE(pcurr+1), BYTE(pcurr+2));
                pcurr += 3;
	    }
	    else
	    {
		sprintf(bytestr + strlen(bytestr), " %2.2x", BYTE(pcurr+1));
                pcurr += 2;
	    }
	    sprintf(symstr, "%s $%x", opstr, operand);
            break;
        case OP_DATA:
	    sprintf(symstr, "%s - ******** TBD ********", opstr);
            pcurr++;
            break;
        case OP_SIGNED_OFFSET:
        case OP_SIGNED_JMP_OFFSET:
	    operand = BYTE(pcurr+1);
	    if (operand & 0x80)
	    {
	        operand = ((operand & 0x7f) << 8) | BYTE(pcurr+2);
		operand = (operand << 17) >> 17;
		sprintf(bytestr + strlen(bytestr), " %2.2x %2.2x", BYTE(pcurr+1), BYTE(pcurr+2));
                pcurr += 3;
	    }
	    else
	    {
		operand = (operand << 25) >> 25;
		sprintf(bytestr + strlen(bytestr), " %2.2x", BYTE(pcurr+1));
                pcurr += 2;
	    }
	    sprintf(symstr, "%s %d", opstr, operand);
            break;
        case OP_MEMORY_OPCODE_READ:
        case OP_MEMORY_OPCODE_WRITE:
	    operand = (BYTE(pcurr+1) >> 5) & 3;
	    opstr = regop[operand];
	    if (operand == 2) opform |= 1 << 5;
	    operand = (BYTE(pcurr+1) & 31);
	    sprintf(bytestr + strlen(bytestr), " %2.2x", BYTE(pcurr+1));
	    if (operand <= 15)
	    {
	        sprintf(symstr, "%s $%3.3x", opstr, operand + 0x1e0);
	    }
	    else
	    {
	        sprintf(symstr, "%s %s", opstr, regname[operand-16]);
	    }
            pcurr += 2;
            break;
        case OP_OBJ_CALL_PAIR:
	    sprintf(bytestr + strlen(bytestr), " %2.2x %2.2x", BYTE(pcurr+1), BYTE(pcurr+2));
	    sprintf(symstr, "%s %d %d", opstr, BYTE(pcurr+1), BYTE(pcurr+2));
	    ProcessObjCall(BYTE(pcurr+1), BYTE(pcurr+2));
            pcurr += 3;
            break;
        case OP_PACKED_LITERAL:
	    operand = BYTE(pcurr+1);
	    val = 2 << (operand & 31);
	    if (operand & 0x20) val--;
	    if (operand & 0x40) val = ~val;
	    sprintf(bytestr + strlen(bytestr), " %2.2x", operand);
	    sprintf(symstr, "%s $%x", opstr, val);
            pcurr += 2;
            break;
        case OP_BYTE_LITERAL:
	    operand = BYTE(pcurr+1);
	    sprintf(bytestr + strlen(bytestr), " %2.2x", operand);
	    sprintf(symstr, "%s %d", opstr, operand);
	    if (strcmp(opstr, "call") == 0)
	        ProcessCall(operand, 1);
            pcurr += 2;
            break;
        case OP_WORD_LITERAL:
	    operand = BYTE(pcurr+1);
	    operand = (operand << 8) | BYTE(pcurr+2);
	    sprintf(bytestr + strlen(bytestr), " %2.2x %2.2x", BYTE(pcurr+1), BYTE(pcurr+2));
	    sprintf(symstr, "%s %d", opstr, operand);
            pcurr += 3;
            break;
        case OP_NEAR_LONG_LITERAL:
	    operand = BYTE(pcurr+1);
	    operand = (operand << 8) | BYTE(pcurr+2);
	    operand = (operand << 8) | BYTE(pcurr+3);
	    sprintf(bytestr + strlen(bytestr), " %2.2x %2.2x %2.2x",
	        BYTE(pcurr+1), BYTE(pcurr+2), BYTE(pcurr+3));
	    sprintf(symstr, "%s %d", opstr, operand);
            pcurr += 4;
            break;
        case OP_LONG_LITERAL:
	    operand = BYTE(pcurr+1);
	    operand = (operand << 8) | BYTE(pcurr+2);
	    operand = (operand << 8) | BYTE(pcurr+3);
	    operand = (operand << 8) | BYTE(pcurr+4);
	    sprintf(bytestr + strlen(bytestr), " %2.2x %2.2x %2.2x %2.2x",
	        BYTE(pcurr+1), BYTE(pcurr+2), BYTE(pcurr+3), BYTE(pcurr+4));
	    sprintf(symstr, "%s %d", opstr, operand);
            pcurr += 5;
            break;
        case OP_MEMORY_OPCODE:
	    sprintf(symstr, "%s - ******** TBD ********", opstr);
            pcurr++;
            break;
        case OP_COMPACT_VAR_OFFSET:
        case OP_COMPACT_LOC_OFFSET:
	    operand = opcode & 0x1c;
	    sprintf(symstr, "%s $%x", opstr, operand);
            pcurr++;
            break;
    }
    if ((opform >> 5) == 1)
    {
	char *loadstr = "";
	opcode = BYTE(pcurr);
	exop2 = GetExOpIndex(opcode);
	if (opcode & 0x80)
	{
	    loadstr = "load";
	    opcode &= 0x7f;
	}
        opstr = FindOpcode(pcurr, &opform, 1);
	if (opcode != 0x02)
	{
	    sprintf(symstr + strlen(symstr), " %s %s", opstr, loadstr);
	    sprintf(bytestr + strlen(bytestr), " %2.2x", BYTE(pcurr));
	}
	else
	{
	    operand = BYTE(pcurr+1);
	    if (operand < 0x80)
	    {
	        operand = (operand << 25) >> 25;
	        sprintf(bytestr + strlen(bytestr), " %2.2x %2.2x",
	            BYTE(pcurr), BYTE(pcurr+1));
	        pcurr++;
	    }
	    else
	    {
	        operand = (operand << 8) | BYTE(pcurr+2);
	        operand = (operand << 17) >> 17;
	        sprintf(bytestr + strlen(bytestr), " %2.2x %2.2x %2.2x",
	            BYTE(pcurr), BYTE(pcurr+1), BYTE(pcurr+2));
	        pcurr += 2;
	    }
	    sprintf(symstr + strlen(symstr), " %s %d %s", opstr, operand, loadstr);
	}
	pcurr++;
    }
    bytestr[strlen(bytestr)] = ' ';
    fprintf(tracefile, "%s %s\n", bytestr, symstr);
    //fprintf(tracefile, "%s [%2d,%2d] %s\n", bytestr, exop1, exop2, symstr);
}

static int opcount[256];
static int exopcount[27][45];

static int GetExOpIndex(int opcode)
{
    int index;

    opcode &= 0x7f;
    if (opcode >= 0x40)
        index = (opcode & 0x1f) + 13;
    else if (opcode >= 0x20)
        index = (opcode >> 3) + 5;
    else if (opcode >= 0x02)
        index = (opcode >> 2) + 1;
    else
    	index = 0;

    return index;
}
static int GetOpIndex(int opcode)
{
    int index = -1;

    if (opcode == 0x26)
        index = 0;
    else if ((opcode >= 0x40 && opcode <= 0x7f) && (opcode & 3) == 2)
        index = ((opcode - 0x40) >> 7) + 1;
    else if ((opcode >= 0x80 && opcode < 0xe0) && (opcode & 3) == 2)
        index = ((opcode - 0x80) >> 2) + 3;

    return index;
}

void ResetStats(void)
{
    memset(opcount, 0, 256 * 4);
    memset(exopcount, 0, 27 * 45 * 4);
}

void CountOp(SpinVarsT *spinvars)
{
    int pcurr = spinvars->pcurr;
    int opcode = BYTE(pcurr++);
    int opindex, exopindex;

    if (opcode >= 0x40 && opcode <= 0x7f) opcode &= 0x63;
    opcount[opcode]++;
    opindex = GetOpIndex(opcode);
    if (opindex < 0) return;
    if (opcode >= 0x80 && ((opcode >> 2) & 3))
    {
        opcode = BYTE(pcurr++);
	if (opcode & 0x80) pcurr++;
    }
    opcode = BYTE(pcurr);
    exopindex = GetExOpIndex(opcode);
    exopcount[opindex][exopindex]++;
}

void PrintStats(void)
{
    int i, j;
    int opcode;
    int opindex;
    int exop, k;
    char *exname;
    char *opname;
    static char *exopname[13] = {
	"store", "repeat", "repeats", "randf", "randr", "sexb", "sexw",
	"postclr", "postset", "preinc", "postinc", "predec", "postdec"};
    static unsigned char exopcode[13] = { 0x00, 0x02, 0x06, 0x08,
	0x0c, 0x10, 0x14, 0x18, 0x1c, 0x20, 0x28, 0x30, 0x38};

    for (i = 0; i < 256; i++)
    {
	if (opcount[i] == 0) continue;
	opcode = i;
        if (opcode >= 0x40 && opcode <= 0x7f) opcode &= 0x63;

        for (j = 0; optable[j].opname; j++)
        {
	    if (opcode == optable[j].opcode) break;
        }
	opname = optable[j].opname;
	opindex = GetOpIndex(i);
	if (opindex >= 0)
	{
	    for (j = 0; j < 45; j++)
	    {
		if (exopcount[opindex][j] == 0) continue;
		if (j < 13)
		{
		    exop = exopcode[j];
		    exname = exopname[j];
		}
		else
		{
		    exop = j - 13 + 0xe0;
                    for (k = 0; optable[k].opname; k++)
                    {
	                if (exop == optable[k].opcode) break;
                    }
		    exname = optable[k].opname;
		}
                fprintf(tracefile, "%10d, %2.2x:%2.2x, %s:%s\n",
		    exopcount[opindex][j], i, exop, opname, exname);

	    }
	}
	else
	{
	  fprintf(tracefile, "%10d, %2.2x,    %s\n", opcount[i], i, opname);
	}
    }
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
