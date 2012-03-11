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
#include <sys/timeb.h>
#include "interp.h"

extern int32_t printflag;
extern PasmVarsT PasmVars[8];
extern char *hubram;
extern int32_t memsize;
extern int32_t loopcount;
extern int32_t cycleaccurate;
extern int32_t proptwo;
extern int32_t pin_val;

extern char lockstate[8];
extern char lockalloc[8];

extern FILE *tracefile;

int32_t ExecuteMathOp(SpinVarsT *spinvars, int32_t opcode, int32_t parm1);
int32_t ExecuteExtraOp(SpinVarsT *spinvars, int32_t opcode, int32_t parm1, int32_t mask);
void ExecuteRegisterOp(SpinVarsT *spinvars, int32_t operand, int32_t msb, int32_t lsb);

// This routine checks for the system I/O address and
// maps it to the end of the memory
int32_t MAP_ADDR(int32_t addr)
{
    if ((addr & 0xfffffff0) == 0x12340000)
    {
	addr = memsize + (addr & 15);
    }
    else if (memsize == 65536)
    {
        addr &= 0xffff;
    }
    else if (((uint32_t)addr) >= memsize)
    {
      fprintf(tracefile, "MAP_ADDR: address out of bounds %8.8x\n", addr);
	addr = memsize + 12;
    }
    //fprintf(tracefile, "MAP_ADDR: %8.8x %8.8x\n", addr, ((uint32_t *)hubram)[addr>>2]);

    return addr;
}

int32_t GetCnt()
{
    int32_t cycles;

    if (cycleaccurate)
    {
	if (proptwo)
	    cycles = loopcount;
	else
	    cycles = loopcount * 4;
    }
    else
    {
        int64_t millisec;
        struct timeb timebuf;

        ftime(&timebuf);
        millisec = timebuf.time;
        millisec = (millisec * 1000) + timebuf.millitm;
        millisec *= LONG(0)/1000;
        cycles = millisec;
    }

    return cycles;
}

void UpdatePins(void)
{
    int32_t i;
    int32_t mask1;
    int32_t val = 0;
    int32_t mask = 0;

    for (i = 0; i < 8; i++)
    {
	if (PasmVars[i].state)
	{
	    mask1 = PasmVars[i].mem[0x1f6]; // dira
	    val |= mask1 & PasmVars[i].mem[0x1f4]; // outa
	    mask |= mask1;
	}
    }
    pin_val = (~mask) | val;
}

int32_t GetSignedOffset(int32_t *ppcurr)
{
    int32_t val;
    int32_t pcurr = *ppcurr;
    val = BYTE(pcurr++);
    if (val < 0x80)
    {
	val = (val << 25) >> 25;
    }
    else
    {
	val = (val << 8) | BYTE(pcurr++);
	val = (val << 17) >> 17;
    }
    *ppcurr = pcurr;
    return val;
}

int32_t GetUnsignedOffset(int32_t *ppcurr)
{
    int32_t pcurr = *ppcurr;
    int32_t val;
    val = BYTE(pcurr++);
    if (val & 0x80)
	val = ((val & 0x7f) << 8) | BYTE(pcurr++);
    *ppcurr = pcurr;
    return val;
}

int32_t bitrev(int32_t val, int32_t nbits)
{
    int32_t retval = 0;

    while (nbits-- > 0)
    {
        retval = (retval << 1) | (val & 1);
        val >>= 1;
    }

    return retval;
}

void StartCog(SpinVarsT *spinvars, int32_t par, int32_t cogid)
{
    memset(spinvars, 0, sizeof(SpinVarsT));
    spinvars->id = cogid;
    spinvars->par = par;
    spinvars->state = 1;
    spinvars->pbase = WORD(par + 2);
    spinvars->vbase = WORD(par + 4);
    spinvars->dbase = WORD(par + 6);
    spinvars->pcurr = WORD(par + 8);
    spinvars->dcurr = WORD(par + 10);
    spinvars->masklong = 0xffffffff;
    spinvars->masktop  = 0x80000000;
    spinvars->maskwr   = 0x00800000;
}

void ExecuteLowerOp(SpinVarsT *spinvars)
{
    int32_t pcurr = spinvars->pcurr;
    int32_t dcurr = spinvars->dcurr;
    int32_t opcode = BYTE(pcurr++);

    if (opcode <= 3) // ldfrmr, ldfrm, ldfrmar, ldfrma
    {
	opcode |= spinvars->pbase;
	WORD(dcurr) = opcode;
	dcurr += 2;
	WORD(dcurr) = spinvars->vbase;
	dcurr += 2;
	WORD(dcurr) = spinvars->dbase;
	dcurr += 2;
	WORD(dcurr) = spinvars->dcall;
	spinvars->dcall = dcurr;
	dcurr += 2;
	LONG(dcurr) = 0;
	dcurr += 4;
    }
    else if (opcode == 0x04) // jmp
    {
	int32_t jmpoff = GetSignedOffset(&pcurr);
	pcurr += jmpoff;
    }
    else if (opcode >= 0x05 && opcode <= 0x07) // call, callobj, callobjx
    {
	int32_t index, pubnum;

        if (opcode > 0x05)
	{
	    int32_t objnum = BYTE(pcurr++);
	    if (opcode == 0x07)
	    {
		dcurr -= 4;
		objnum += LONG(dcurr);
	    }
	    index = (objnum << 2) + spinvars->pbase;
	    spinvars->pbase += WORD(index);
	    spinvars->vbase += WORD(index+2);
	}

	pubnum = BYTE(pcurr++);
        spinvars->dbase = spinvars->dcall;
	spinvars->dcall = WORD(spinvars->dcall);
	WORD(spinvars->dbase) = pcurr;
	index = (pubnum << 2) + spinvars->pbase;
	pcurr = WORD(index) + spinvars->pbase;
	spinvars->dbase += 2;
	dcurr += WORD(index+2);
    }
    else if (opcode == 0x08) // tjz
    {
	int32_t jmpoff = GetSignedOffset(&pcurr);

	if (!LONG(dcurr-4))
	{
	    pcurr += jmpoff;
	    dcurr -= 4;
	}
    }
    else if (opcode == 0x09) // djnz
    {
	int32_t jmpoff = GetSignedOffset(&pcurr);
	if (--LONG(dcurr-4))
	{
	    pcurr += jmpoff;
	}
	else
	    dcurr -= 4;
    }
    else if (opcode == 0x0a) // jz
    {
	int32_t jmpoff = GetSignedOffset(&pcurr);
	dcurr -= 4;
	if (!LONG(dcurr))
	{
	    pcurr += jmpoff;
	}
    }
    else if (opcode == 0x0b) // jnz
    {
	int32_t jmpoff = GetSignedOffset(&pcurr);
	dcurr -= 4;
	if (LONG(dcurr))
	{
	    pcurr += jmpoff;
	}
    }
    else if (opcode >= 0x0c && opcode <= 0x15)
    {
	int32_t val, jmpoff, val2, val0;
	if (opcode == 0x0c) // casedone
	{
	    dcurr -= 8;
	    jmpoff = LONG(dcurr);
	    pcurr = spinvars->pbase + jmpoff;
	}
	else if (opcode == 0x0d) // casevalue
	{
	    jmpoff = GetSignedOffset(&pcurr);
	    dcurr -= 4;
	    val = LONG(dcurr);
	    if (val == LONG(dcurr - 4))
	    {
		pcurr += jmpoff;
	    }
	}
	else if (opcode == 0x0e) // caserange
	{
	    jmpoff = GetSignedOffset(&pcurr);
	    dcurr -= 4;
	    val = LONG(dcurr);
	    dcurr -= 4;
	    val2 = LONG(dcurr);
	    val0 = LONG(dcurr - 4);
	    if (val <= val2)
	    {
		if (val <= val0 && val0 <= val2) pcurr += jmpoff;
	    }
	    else
	    {
		if (val2 <= val0 && val0 <= val) pcurr += jmpoff;
	    }
	}
	else if (opcode == 0x0f) // lookdone
	{
	    dcurr -= 8;
	    LONG(dcurr - 4) = 0;
	}
	else if (opcode == 0x10) // lookupval
	{
	    dcurr -= 4;
	    val = LONG(dcurr);
	    val0 = LONG(dcurr - 4);
	    if (val0 < LONG(dcurr - 12))
	    {
		dcurr -= 8;
		pcurr = spinvars->pbase + LONG(dcurr);
		LONG(dcurr - 4) = 0;
	    }
	    else if (val0 == LONG(dcurr - 12))
	    {
		dcurr -= 8;
		pcurr = spinvars->pbase + LONG(dcurr);
		LONG(dcurr - 4) = val;
	    }
	    else
	    {
		LONG(dcurr - 12)++;
	    }
	}
	else if (opcode == 0x11) // lookdnval
	{
	    dcurr -= 4;
	    val = LONG(dcurr);
	    val0 = LONG(dcurr - 4);
	    if (val == val0)
	    {
		dcurr -= 8;
		pcurr = spinvars->pbase + LONG(dcurr);
	    }
	    else
	    {
		LONG(dcurr - 12)++;
	    }
	}
	else if (opcode == 0x12) // lookuprng
	{
	    int32_t num, count;
	    dcurr -= 4;
	    val = LONG(dcurr);
	    dcurr -= 4;
	    val2 = LONG(dcurr);
	    val0 = LONG(dcurr - 4);
	    num = val - val2;
	    if (num < 0) num = -num;
	    count = LONG(dcurr - 12);
	    if (val0 < count)
	    {
		dcurr -= 8;
		pcurr = spinvars->pbase + LONG(dcurr);
		LONG(dcurr - 4) = 0;
	    }
	    else if (val0 - num <= count)
	    {
		dcurr -= 8;
		pcurr = spinvars->pbase + LONG(dcurr);
		num = val0 - count;
		if (val >= val2)
		{
		    LONG(dcurr - 4) = val2 + num;
		}
		else
		{
		    LONG(dcurr - 4) = val - num;
		}
	    }
	    else
	    {
		LONG(dcurr - 12) += num + 1;
	    }
	}
	else if (opcode == 0x13) // lookdnrng
	{
	    dcurr -= 4;
	    val = LONG(dcurr);
	    dcurr -= 4;
	    val2 = LONG(dcurr);
	    if (val > val2)
	    {
		val0 = val;
		val = val2;
		val2 = val0;
	    }
	    val0 = LONG(dcurr - 4);
	    if (val <= val0 && val0 <= val2)
	    {
		dcurr -= 8;
		pcurr = spinvars->pbase + LONG(dcurr);
	    }
	    else
	    {
		LONG(dcurr - 12)++;
	    }
	}
	else if (opcode == 0x14) // pop
	{
	    dcurr -= 4;
	    val = LONG(dcurr);
	    dcurr -= val;
	}
	else if (opcode == 0x15) // run
	{
	    spinvars->lsb = pcurr;
	    pcurr = 0xfffc;
	}
	else
	  fprintf(tracefile, "%4.4x %2.2x - NOT IMPLEMENTED\n", pcurr - 1, opcode);
    }
    else if (opcode >= 0x16 && opcode <= 0x23)
    {
        if (opcode == 0x16) // strsize
        {
	    char *ptr = hubram;
	    dcurr -= 4;
	    LONG(dcurr) = strlen(ptr + LONG(dcurr));
	    dcurr += 4;
        }
        else if (opcode == 0x17) // strcomp
        {
	    char *ptr1 = hubram;
	    char *ptr2 = hubram;
	    dcurr -= 4;
	    ptr1 += LONG(dcurr);
	    dcurr -= 4;
	    ptr2 += LONG(dcurr);
	    if (strcmp(ptr1, ptr2) == 0)
	        LONG(dcurr) = -1;
	    else
	        LONG(dcurr) = 0;
	    dcurr += 4;
        }
        else if (opcode == 0x18) // bytefill
        {
	    int32_t val, num;
	    char *ptr = hubram;
	    dcurr -= 4;
	    num = LONG(dcurr);
	    dcurr -= 4;
	    val = LONG(dcurr);
	    dcurr -= 4;
	    ptr += LONG(dcurr);
	    memset(ptr, val, num);
        }
        else if (opcode == 0x19) // wordfill
        {
	    int32_t val, num, addr;
	    dcurr -= 4;
	    num = LONG(dcurr);
	    dcurr -= 4;
	    val = LONG(dcurr);
	    dcurr -= 4;
	    addr = LONG(dcurr);
	    while (num-- > 0)
	    {
		WORD(addr) = val;
		addr += 2;
	    }
        }
        else if (opcode == 0x1a) // longfill
        {
	    int32_t val, num, addr;
	    dcurr -= 4;
	    num = LONG(dcurr);
	    dcurr -= 4;
	    val = LONG(dcurr);
	    dcurr -= 4;
	    addr = LONG(dcurr);
	    while (num-- > 0)
	    {
		LONG(addr) = val;
		addr += 4;
	    }
        }
        else if (opcode == 0x1b) // waitpeq
        {
	    dcurr -= 12;
        }
        else if (opcode >= 0x1c && opcode <= 0x1e ) // bytemove, wordmove, longmove
        {
	    char *dst = hubram;
	    char *src = hubram;
	    int32_t num;
	    dcurr -= 4;
	    num = LONG(dcurr);
	    dcurr -= 4;
	    src += LONG(dcurr);
	    dcurr -= 4;
	    dst += LONG(dcurr);
	    if (opcode == 0x1d)
	        num <<= 1;
	    else if (opcode == 0x1e)
	        num <<= 2;
	    memmove(dst, src, num);
        }
        else if (opcode == 0x1f) // waitpne
        {
	    dcurr -= 12;
        }
        else if (opcode == 0x20) // clkset
        {
	    int32_t clkfreq, clkmode;
	    dcurr -= 4;
	    clkfreq = LONG(dcurr);
	    dcurr -= 4;
	    clkmode = LONG(dcurr);
	    if (clkmode & 0x80)
	    {
		RebootProp();
		return;
	    }
	    LONG(0) = clkfreq;
	    BYTE(4) = clkmode;
        }
        else if (opcode == 0x21) // cogstop
        {
	    int32_t cogid;
	    dcurr -= 4;
	    cogid = LONG(dcurr) & 7;
	    PasmVars[cogid].state = 0;
	    UpdatePins();
        }
        else if (opcode == 0x22) // lockret
        {
	    int32_t locknum;

	    dcurr -= 4;
	    locknum = LONG(dcurr) & 7;
	    lockalloc[locknum] = 0;
        }
        else if (opcode == 0x23) // waitcnt
        {
	    int32_t parm1;

	    dcurr -= 4;
	    parm1 = GetCnt() - LONG(dcurr);
	    if (parm1 < 0 || parm1 > 20000000)
	    {
		pcurr--;
		dcurr += 4;
		spinvars->state = 2;
	    }
	    else
	        spinvars->state = 1;
        }
    }
    else if (opcode >= 0x24 && opcode <= 0x2f)
    {
	if (opcode >= 0x24 && opcode <= 0x26) // ldregx, stregx, exregx (spr)
	{
	    int32_t operand;
	    dcurr -= 4;
	    operand = LONG(dcurr);
	    operand = ((opcode & 3) << 5) | 0x10 | (operand & 15);
            spinvars->pcurr = pcurr;
            spinvars->dcurr = dcurr;
	    ExecuteRegisterOp(spinvars, operand, 31, 0);
            pcurr = spinvars->pcurr;
            dcurr = spinvars->dcurr;
	}
	else if (opcode == 0x27) // waitvid
	{
	    dcurr -= 8;
	}
	else if (opcode == 0x28 || opcode == 0x2c) // coginitret, coginit
	{
	    int32_t cogid, addr, par;
	    dcurr -= 4;
	    par = LONG(dcurr);
	    dcurr -= 4;
	    addr = LONG(dcurr);
	    dcurr -= 4;
	    cogid = LONG(dcurr);
	    if (cogid < 0 || cogid > 7)
	    {
		for (cogid = 0; cogid < 8; cogid++)
		{
		    if (!PasmVars[cogid].state) break;
		}
	    }
	    if (cogid < 0 || cogid > 7) // || addr != 0xf004)
	        cogid = -1;
	    if (opcode == 0x28)
	    {
	        LONG(dcurr) = cogid;
	        dcurr += 4;
	    }
	    if (cogid != -1)
	    {
		if (addr == 0xf004)
		{
                    spinvars->pcurr = pcurr;
                    spinvars->dcurr = dcurr;
	            StartCog((SpinVarsT *)&PasmVars[cogid].mem[0x1e0], par, cogid);
                    pcurr = spinvars->pcurr;
                    dcurr = spinvars->dcurr;
		}
		else
		{
		    StartPasmCog(&PasmVars[cogid], par, addr, cogid);
		}
	        UpdatePins();
	    }
	}
	else if (opcode == 0x29 || opcode == 0x2d) // locknewret, locknew
	{
	    int32_t locknum;

	    for (locknum = 0; locknum < 8; locknum++)
	    {
		if (!lockalloc[locknum])
		{
		    lockalloc[locknum] = 1;
		    break;
		}
	    }
	    if (opcode == 0x29)
	    {
		if (locknum == 8) locknum = -1;
		LONG(dcurr) = locknum;
		dcurr += 4;
	    }
	}
	// locksetret, lockclrret, lockset, lockclr
	else if (opcode == 0x2a || opcode == 0x2b || opcode == 0x2e || opcode == 0x2f)
	{
	    int32_t locknum;

	    dcurr -= 4;
	    locknum = LONG(dcurr) & 7;
	    if (opcode <= 0x2b)
	    {
	        LONG(dcurr) = lockstate[locknum];
	        dcurr += 4;
	    }
	    lockstate[locknum] = (opcode & 1) - 1;
	}
    }
    else if (opcode >= 0x30 && opcode <= 0x33) // abort, abortval, ret, retval
    {
	int32_t retval, dbase, pbase;

	if (opcode & 1)
	    retval = LONG(dcurr - 4);
	else
	    retval = LONG(spinvars->dbase);

        dbase = spinvars->dbase;

        while (1)
	{
	    dcurr = dbase - 8;
	    pbase = WORD(dcurr);
	    dbase = WORD(dcurr + 4);
	    if ((opcode & 2) || (pbase & 2)) break;
	}

	spinvars->pbase = pbase & 0xfffc;
	spinvars->vbase = WORD(dcurr + 2);
	spinvars->dbase = dbase;
	pcurr = WORD(dcurr + 6);

        if (!(pbase & 1))
	{
	    LONG(dcurr) = retval;
	    dcurr += 4;
	}
    }
    else if (opcode >= 0x34 && opcode < 0x3c)
    {
	if (opcode == 0x35) LONG(dcurr) = 0;        // dli0
	else if (opcode == 0x36) LONG(dcurr) = 1;   // dli1
	else if (opcode == 0x34) LONG(dcurr) = -1;  // dlim1
	else if (opcode == 0x37) // ldlip
	{
	    int32_t val;
	    int32_t operand = BYTE(pcurr++);
	    int32_t rotate = operand & 31;

	    if (rotate == 31)
	        val = 1;
	    else
	        val = 2 << rotate;

	    if (operand & 0x20) val--;
	    if (operand & 0x40) val = ~val;
	    LONG(dcurr) = val;
	}
	else // ldbi, ldwi, ldmi, ldli
	{
	    int32_t operand = 0;
	    while (opcode-- >= 0x38) operand = (operand << 8) | BYTE(pcurr++);
	    LONG(dcurr) = operand;
	}
	dcurr += 4;
    }
    else if (opcode == 0x3d) // ldregbit, stregbit, exregbit
    {
	int32_t operand = BYTE(pcurr++);
	int32_t bitpos;

	dcurr -= 4;
	bitpos = LONG(dcurr) & 0x1f;

        spinvars->pcurr = pcurr;
        spinvars->dcurr = dcurr;
	ExecuteRegisterOp(spinvars, operand, bitpos, bitpos);
        pcurr = spinvars->pcurr;
        dcurr = spinvars->dcurr;
    }
    else if (opcode == 0x3e) // ldregbits, stregbits, exregbits
    {
	int32_t operand = BYTE(pcurr++);
	int32_t msb, lsb;

	dcurr -= 4;
	lsb = LONG(dcurr) & 0x1f;
	dcurr -= 4;
	msb = LONG(dcurr) & 0x1f;

        spinvars->pcurr = pcurr;
        spinvars->dcurr = dcurr;
	ExecuteRegisterOp(spinvars, operand, msb, lsb);
        pcurr = spinvars->pcurr;
        dcurr = spinvars->dcurr;
    }
    else if (opcode == 0x3f) // ldreg, streg, exreg
    {
	int32_t operand = BYTE(pcurr++);

        spinvars->pcurr = pcurr;
        spinvars->dcurr = dcurr;
	ExecuteRegisterOp(spinvars, operand, 31, 0);
        pcurr = spinvars->pcurr;
        dcurr = spinvars->dcurr;
    }
    else
    {
      fprintf(tracefile, "NOT PROCESSED\n");
    }
    spinvars->pcurr = pcurr;
    spinvars->dcurr = dcurr;
}

void ExecuteRegisterOp(SpinVarsT *spinvars, int32_t operand, int32_t msb, int32_t lsb)
{
    int32_t opcode;
    int32_t parm1, parm2;
    int32_t pcurr = spinvars->pcurr;
    int32_t dcurr = spinvars->dcurr;
    int32_t *reg = (int32_t *)spinvars;
    int32_t memfunc = (operand >> 5) & 3;
    int32_t revflag = 0;
    int32_t mask, nbits;

    if (lsb > msb)
    {
	revflag = msb;
	msb = lsb;
	lsb = revflag;
	revflag = 1;
    }

    nbits = msb - lsb + 1;

    if (nbits >= 32)
        mask = -1;
    else
        mask = (1 << nbits) - 1;

    operand &= 0x1f;

    if (memfunc == 1) // store
    {
        spinvars->dcurr -= 4;
	if (nbits == 32 && !revflag)
	{
            reg[operand] = LONG(spinvars->dcurr);
	}
	else
	{
            parm1 = reg[operand];
            parm2 = LONG(spinvars->dcurr);
	    parm2 &= mask;
	    if (revflag) parm2 = bitrev(parm2, nbits);
	    reg[operand] = (parm1 & ~(mask << lsb)) | (parm2 << lsb);
	}
        if (operand == 0x14 || operand == 0x16) UpdatePins();
        pcurr = spinvars->pcurr;
        dcurr = spinvars->dcurr;
    }
    else if (memfunc == 0) // load
    {
        if (operand == 0x11) // cnt = $1f1
            parm1 = GetCnt();
        else if (operand == 0x12) // ina = $1f2
	    parm1 = pin_val;
        else
            parm1 = reg[operand];
	parm1 = (parm1 >> lsb) & mask;
	if (revflag) parm1 = bitrev(parm1, nbits);
	LONG(dcurr) = parm1;
        dcurr += 4;
    }
    else if (memfunc == 2) // execute
    {
        opcode = BYTE(spinvars->pcurr++);

        if (opcode & 0x7e)
        {
            if (operand == 0x11) // cnt = $1f1
                parm1 = GetCnt();
            else if (operand == 0x12) // ina = $1f2
	        parm1 = pin_val;
            else
                parm1 = reg[operand];
        }
        else
            parm1 = 0; // Need to double check this

	parm2 = (parm1 >> lsb) & mask;
	if (revflag) parm2 = bitrev(parm2, nbits);

        parm2 = ExecuteExtraOp(spinvars, opcode, parm2, mask);

        parm2 &= mask;
        if (revflag) parm2 = bitrev(parm2, nbits);
        reg[operand] = (parm1 & ~(mask << lsb)) | (parm2 << lsb);
        if (operand == 0x14 || operand == 0x16) UpdatePins();

        pcurr = spinvars->pcurr;
        dcurr = spinvars->dcurr;
    }
    else
      fprintf(tracefile, "Undefined register operation\n");

    spinvars->pcurr = pcurr;
    spinvars->dcurr = dcurr;
}

void ExecuteMemoryOp(SpinVarsT *spinvars)
{
    int32_t pcurr = spinvars->pcurr;
    int32_t dcurr = spinvars->dcurr;
    int32_t opcode, memfunc, memsize, membase, memaddr;

    opcode = BYTE(pcurr++);
    memfunc = opcode & 3;

    if (opcode < 0x80) // Compact offset
    {
	memsize = 3;
	memaddr = (opcode & 0x1c);
	membase = (opcode >> 5) & 3;
    }
    else
    {
	memsize = ((opcode >> 5) & 3) + 1;
	membase = (opcode & 0x0c) >> 2;

        // Check for index op
        if (opcode & 0x10)
        {
	    dcurr -= 4;
	    memaddr = LONG(dcurr) << (memsize - 1);
        }
	else
	    memaddr = 0;

	if (membase)
	    memaddr += GetUnsignedOffset(&pcurr);
	else
        {
	    dcurr -= 4;
	    memaddr += LONG(dcurr);
        }
    }

    if (membase == 1)
	memaddr += spinvars->pbase;
    else if (membase == 2)
	memaddr += spinvars->vbase;
    else if (membase == 3)
	memaddr += spinvars->dbase;

    if (memfunc == 3)      // la
    {
	LONG(dcurr) = memaddr;
	dcurr += 4;
    }
    else if (memfunc == 0) // ld
    {
	if (memsize == 1) LONG(dcurr) = BYTE(memaddr);
	else if (memsize == 2) LONG(dcurr) = WORD(memaddr);
	else LONG(dcurr) = LONG(memaddr);
	dcurr += 4;
    }
    else if (memfunc == 1) // st
    {
	dcurr -= 4;
	if (memsize == 1) BYTE(memaddr) = LONG(dcurr);
	else if (memsize == 2) WORD(memaddr) = LONG(dcurr);
	else LONG(memaddr) = LONG(dcurr);
    }
    else                    // ex
    {
        int32_t parm1 = 0;

	opcode = BYTE(pcurr++);

        if (opcode & 0x7f)
	{
	    if (memsize == 1) parm1 = BYTE(memaddr);
	    else if (memsize == 2) parm1 = WORD(memaddr);
	    else parm1 = LONG(memaddr);
	}

        spinvars->pcurr = pcurr;
        spinvars->dcurr = dcurr;
        parm1 = ExecuteExtraOp(spinvars, opcode, parm1, -1);
        pcurr = spinvars->pcurr;
        dcurr = spinvars->dcurr;

	if (memsize == 1) BYTE(memaddr) = parm1;
	else if (memsize == 2) WORD(memaddr) = parm1;
	else LONG(memaddr) = parm1;
    }
    spinvars->pcurr = pcurr;
    spinvars->dcurr = dcurr;
}

int32_t ExecuteExtraOp(SpinVarsT *spinvars, int32_t opcode, int32_t parm1, int32_t mask)
{
    int32_t parm2;
    int32_t pcurr = spinvars->pcurr;
    int32_t dcurr = spinvars->dcurr;
    int32_t loadflag = opcode & 0x80;
    int32_t size = (opcode >> 1) & 3;

    if (size == 1)
        mask = 0xff;
    else if (size == 2)
        mask = 0xffff;
    else if (size == 3)
        mask = 0xffffffff;

    opcode &= 0x7f;

    if (opcode >= 0x40 && opcode < 0x60) // math op
    {
        parm2 = parm1 = ExecuteMathOp(spinvars, opcode, parm1);
        pcurr = spinvars->pcurr;
        dcurr = spinvars->dcurr;
    }
    else if ((opcode & 0x7e) == 0x00) // store
    {
        dcurr -= 4;
        parm2 = parm1 = LONG(dcurr);
    }
    else if ((opcode & 0x7a) == 0x02) // repeat, repeats
    {
        int32_t first, last, step, jmpoff;

        dcurr -= 4;
        last = LONG(dcurr);
        dcurr -= 4;
        first = LONG(dcurr);
        if (opcode == 0x06)
        {
	    dcurr -= 4;
	    step = LONG(dcurr);
        }
        else step = 1;
        jmpoff = GetSignedOffset(&pcurr);
        if (last >= first)
        {
	    parm1 += step;
	    if (parm1 >= first && parm1 <= last)
	        pcurr += jmpoff;
        }
        else
        {
	    parm1 -= step;
	    if (parm1 <= first && parm1 >= last)
	        pcurr += jmpoff;
        }
        parm2 = parm1;
    }
    else if ((opcode & 0x78) == 8) // randf, randr
    {
	uint32_t a, c, x, y, z;

        x = parm1;
	z = (opcode & 0x04) == 0;
	if (!x) x = 1;
	y = 32;
	a = 0x17;
	if (!z) a = (a >> 1) | (a << 31);
	while (y--)
	{
	    c = x & a;
	    while (c & 0xfffffffe) c = (c >> 1) ^ (c & 1);
	    if (z)
	        x = (x >> 1) | (c << 31);
	    else
	        x = (x << 1) | c;
	}
        parm2 = parm1 = x;
    }
    else if ((opcode & 0x7c) == 0x10) // sexb
    {
        parm2 = parm1 = (parm1 << 24) >> 24;
    }
    else if ((opcode & 0x7c) == 0x14) // sexw
    {
        parm2 = parm1 = (parm1 << 16) >> 16;
    }
    else if ((opcode & 0x7c) == 0x18) // postclr
    {
	parm2 = parm1;
	parm1 = 0;
    }
    else if ((opcode & 0x7c) == 0x1c) // postset
    {
	parm2 = parm1;
	parm1 = -1;
    }
    else if ((opcode & 0x78) == 0x20) // preinc
    {
        parm2 = ++parm1;
	parm2 &= mask;
    }
    else if ((opcode & 0x78) == 0x28) // postinc
    {
        parm2 = parm1++;
	parm2 &= mask;
    }
    else if ((opcode & 0x78) == 0x30) // predec
    {
        parm2 = --parm1;
	parm2 &= mask;
    }
    else if ((opcode & 0x78) == 0x38) // postdec
    {
        parm2 = parm1--;
	parm2 &= mask;
    }
    else
    {
      fprintf(tracefile, "NOT IMPLEMENTED\n");
        parm2 = 0;
    }

    if (loadflag)
    {
        LONG(dcurr) = parm2;
        dcurr += 4;
    }

    spinvars->pcurr = pcurr;
    spinvars->dcurr = dcurr;

    return parm1;
}

int32_t ExecuteMathOp(SpinVarsT *spinvars, int32_t opcode, int32_t parm1)
{
    int32_t parm3;
    int32_t parm2 = 0;
    int32_t execflag = 0;
    int32_t unary;
    int32_t dcurr = spinvars->dcurr;

    unary = (0x810a02c0 >> (opcode & 0x1f)) & 1;

    // Get the parameters from the stack
    if (opcode < 0xe0)
    {
	execflag = 1;
	if (!unary)
	{
	    if ((opcode >> 5) & 1) // Swap parms
	    {
		parm2 = parm1;
                dcurr -= 4;
                parm1 = LONG(dcurr);
	    }
	    else
	    {
                dcurr -= 4;
                parm2 = LONG(dcurr);
	    }
	}
	opcode += 0xe0 - 0x40;
    }
    else
    {
        if (!unary)
        {
            dcurr -= 4;
            parm2 = LONG(dcurr);
	    dcurr -= 4;
	    parm1 = LONG(dcurr);
        }
        else
	{
            dcurr -= 4;
            parm1 = LONG(dcurr);
	}
    }

    // Execute the math op
    switch (opcode)
    {
	case 0xe0: // ror
        parm1 = (((uint32_t)parm1) >> parm2) | (parm1 << (32 - parm2));
	break;

	case 0xe1: // rol
        parm1 = (((uint32_t)parm1) >> (32 - parm2)) | (parm1 << parm2);
	break;

	case 0xe2: // shr
        parm1 = ((uint32_t)parm1) >> parm2;
	break;

	case 0xe3: // shl
        parm1 <<= parm2;
	break;

	case 0xe4: // min
	if (parm2 > parm1) parm1 = parm2;
	break;

	case 0xe5: // max
	if (parm2 < parm1) parm1 = parm2;
	break;

	case 0xe6: // neg
        parm1 = -parm1;
	break;

	case 0xe7: // com
        parm1 = ~parm1;
	break;

	case 0xe8: // and
        parm1 &= parm2;
	break;

	case 0xe9: // abs
        if (parm1 < 0) parm1 = -parm1;
	break;

	case 0xea: // or
        parm1 |= parm2;
	break;

	case 0xeb: // xor
        parm1 ^= parm2;
	break;

	case 0xec: // add
        parm1 += parm2;
	break;

	case 0xed: // sub
        parm1 -= parm2;
	break;

	case 0xee: // sar
        parm1 >>= parm2;
	break;

	case 0xef: // rev
	parm1 = bitrev(parm1, parm2);
	break;

	case 0xf0: // andl
	parm1 = (parm1 && parm2) ? -1 : 0;
	break;

	case 0xf1: // encode
	for (parm2 = 32; parm2 >= 1; parm2--)
	{
	    if (parm1 & 0x80000000) break;
	    parm1 <<= 1;
	}
	parm1 = parm2;
	break;

	case 0xf4: // mul
        parm1 *= parm2;
	break;

	case 0xf5: // mulh
        {
	    int64_t parm1a = parm1;
	    int64_t parm2a = parm2;
	    parm1 = (parm1a * parm2a) >> 32;
        }
	break;

	case 0xf2: // orl
        parm1 = (parm1 || parm2) ? -1 : 0;
	break;

	case 0xf3: // decode
        parm1 = 1 << parm1;
	break;

	case 0xf6: // div
	if (parm2)
            parm1 /= parm2;
	else
	    parm1 = 0;
	break;

	case 0xf7: // mod
	if (parm2)
            parm1 %= parm2;
	else
	    parm1 = 0;
	break;

	case 0xf8: // sqrt
        parm2 = 0;
        parm3 = 1 << 30;
        while (parm3)
        {
	    parm2 |= parm3;
	    if (parm2 <= parm1)
	    {
	        parm1 -= parm2;
	        parm2 += parm3;
	    }
	    else
	        parm2 -= parm3;
	    parm2 >>= 1;
	    parm3 >>= 2;
        }
	parm1 = parm2;
	break;

	case 0xf9: // cmplt
        parm1 = (parm1 < parm2) ? -1 : 0;
	break;

	case 0xfa: // cmpgt
        parm1 = (parm1 > parm2) ? -1 : 0;
	break;

	case 0xfb: // cmpne
        parm1 = (parm1 != parm2) ? -1 : 0;
	break;

	case 0xfc: // cmpeq
        parm1 = (parm1 == parm2) ? -1 : 0;
	break;

	case 0xfd: // cmple
        parm1 = (parm1 <= parm2) ? -1 : 0;
	break;

	case 0xfe: // cmpgr
        parm1 = (parm1 >= parm2) ? -1 : 0;
	break;

	case 0xff: // notl
        parm1 = parm1 ? 0 : -1;
	break;

	default:
	  fprintf(tracefile, "NOT PROCESSED\n");
    }

    // Push the result back to the stack
    if (!execflag)
    {
        LONG(dcurr) = parm1;
        dcurr += 4;
        spinvars->pcurr++;
    }

    spinvars->dcurr = dcurr;
    return parm1;
}

void ExecuteOp(SpinVarsT *spinvars)
{
    int32_t opcode;

    if (!spinvars->state) return;

    opcode = BYTE(spinvars->pcurr);

    if (opcode < 0x40)
        ExecuteLowerOp(spinvars);
    else if (opcode < 0xe0)
        ExecuteMemoryOp(spinvars);
    else
        ExecuteMathOp(spinvars, opcode, 0);
}
/*
+ -----------------------------------------------------------------------------------------------------------------------------+
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
