/*******************************************************************************
' Author: Dave Hein
' Version 0.21
' Copyright (c) 2010, 2011
' See end of file for terms of use.
'******************************************************************************/
#include <stdio.h>
#include "interp.h"

extern char *hubram;
extern int32_t memsize;
extern char lockstate[8];
extern char lockalloc[8];
extern PasmVarsT PasmVars[8];
extern int32_t pasmspin;
extern int32_t cycleaccurate;
extern int32_t loopcount;
extern int32_t proptwo;
extern int32_t pin_val;

extern FILE *tracefile;

static int32_t parity(int32_t val)
{
    val ^= val >> 16;
    val ^= val >> 8;
    val ^= val >> 4;
    val ^= val >> 2;
    val ^= val >> 1;
    return val & 1;
}

static int32_t abs(int32_t val)
{
    return val < 0 ? -val : val;
}

int32_t CheckWaitFlag(PasmVarsT *pasmvars, int mode)
{
    int32_t hubmode = mode & 1;
    int32_t debugmode = mode & 2;
    int32_t waitflag = pasmvars->waitflag;

    if (waitflag)
    {
        waitflag--;
    }
    else if (hubmode)
    {
	if (proptwo)
	    waitflag = (pasmvars->cogid - loopcount) & 7;
	else
	    waitflag = ((pasmvars->cogid >> 1) - loopcount) & 3;
	waitflag++;
    }
    else
    {
        if (proptwo)
            waitflag = 2;
        else
            waitflag = 1;
    }
    if (!debugmode)
    {
        if (waitflag)
            pasmvars->pc = (pasmvars->pc - 1) & 511;
	pasmvars->waitflag = waitflag;
    }

    return waitflag;
}

int32_t ExecutePasmInstruction(PasmVarsT *pasmvars)
{
    int32_t cflag = pasmvars->cflag;
    int32_t zflag = pasmvars->zflag;
    int32_t instruct = pasmvars->mem[pasmvars->pc];
    int32_t cond = (instruct >> 18) & 15;
    int32_t opcode, value2, value1, zcri;
    int32_t srcaddr, dstaddr;
    int32_t result = 0;

    // Increment the program counter
    pasmvars->pc = (pasmvars->pc + 1) & 511;

    // Return if not executed
    if (!((cond >> ((cflag << 1) | zflag)) & 1))
    {
	return 0;
    }

    // Check for a hub wait
    if (cycleaccurate && !(instruct & 0xe0000000))
    {
	if (CheckWaitFlag(pasmvars, 1)) return 0;
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
    else if (srcaddr == 0x1f1)
        value2 = GetCnt();
    else if (srcaddr == 0x1f2)
        value2 = pin_val;
    else
        value2 = pasmvars->mem[srcaddr];

    // Decode the three most significant bits of the opcode
    switch(opcode >> 3)
    {
	// Hub access opcodes
	case 0:
	switch (opcode & 7)
	{
	    case 0: // wrbyte, rdbyte
	    case 1: // wrword, rdword
	    case 2: // wrlong, rdlong
	    if (zcri & 2) // read
	    {
		if ((opcode & 7) == 0)
		    result = BYTE(value2);
		else if ((opcode & 7) == 1)
		    result = WORD(value2);
		else
		    result = LONG(value2);
		zflag = (result == 0);
	    }
	    else          // write
	    {
		if ((opcode & 7) == 0)
		    BYTE(value2) = pasmvars->mem[dstaddr];
		else if ((opcode & 7) == 1)
		    WORD(value2) = pasmvars->mem[dstaddr];
		else
		    LONG(value2) = pasmvars->mem[dstaddr];
	    }
	    break;

	    case 3: // misc hubops
	    switch (value2 & 7)
	    {
		int32_t par, addr;
		case 0: // clkset
		result = value1 & 0xff;
		if (result & 0x80)
		{
		    reboot();
		    return 0;
		}
		break;

		case 1: // cogid
		result = pasmvars->cogid;
		//pasmvars->mem[dstaddr] = result;
		break;

		case 2: // coginit
		par = (value1 >> 16) & 0xfffc;
		addr = (value1 >> 2) & 0xfffc;
		if (value1 & 8) // Start new cog
		{
		    // Look for next available cog
		    for (result = 0; result < 8; result++)
		    {
		        if (!PasmVars[result].state) break;
		    }
		    if (result == 8) // Check if none available
		    {
			cflag = 1;
			result = 7;
			zflag = 0;
			break;
		    }
		}
		else
		{
		    result = value1 & 7;
		}
		cflag = 0;
		zflag = (result == 0);
		if (addr == 0xf004 && !pasmspin)
		{
		    SpinVarsT *spinvars = (SpinVarsT *)&PasmVars[result].mem[0x1e0];
		    StartCog(spinvars, par, result);
		}
		else
		{
		    StartPasmCog(&PasmVars[result], par, addr, result);
		}
	        UpdatePins();
		// Return without saving if we restarted this cog
		if (result == pasmvars->cogid) return result;
		break;

		case 3: // cogstop
		for (result = 0; result < 8; result++)
		{
		    if (!PasmVars[result].state) break;
		}
		cflag = (result == 8);
		result = value1 & 7;
		zflag = (result == 0);
                PasmVars[result].state = 0;
		UpdatePins();
		// Return without saving if we stopped this cog
		if (result == pasmvars->cogid) return result;
		break;

		case 4: // locknew
		for (result = 0; result < 8; result++)
		{
		    if (!lockalloc[result]) break;
		}
		if (result == 8)
		{
		    cflag = 1;
		    result = 7;
		}
		else
		{
		    cflag = 0;
		    lockalloc[result] = 1;
		}
		zflag = (result == 0);
		break;

		case 5: // lockret
		for (result = 0; result < 8; result++)
		{
		    if (!lockalloc[result]) break;
		}
		cflag = (result == 8);
		result = value1 & 7;
		zflag = (result == 0);
		lockalloc[result] = 0;
		break;

		case 6: // lockset
		result = value1 & 7;
		zflag = (result == 0);
		cflag = lockstate[result] & 1;
		lockstate[result] = -1;
		break;

		case 7: // lockclr
		result = value1 & 7;
		zflag = (result == 0);
		cflag = lockstate[result] & 1;
		lockstate[result] = 0;
		break;
	    }
	    break;

	    default: // Not defined
	    //printf("Undefined op - %8.8x\n", instruct);
	    break;
	}
	break;

	// Rotate and shift
	case 1:
	value2 &= 0x1f; // Get five LSB's
	switch (opcode & 7)
	{
	    case 0: // ror
	    result = (((uint32_t)value1) >> value2) | (value1 << (32 - value2));
	    cflag = value1 & 1;
	    break;

	    case 1: // rol
	    result = (((uint32_t)value1) >> (32 - value2)) | (value1 << value2);
	    cflag = (value1 >> 31) & 1;
	    break;

	    case 2: // shr
	    result = (((uint32_t)value1) >> value2);
	    cflag = value1 & 1;
	    break;

	    case 3: // shl
	    result = (value1 << value2);
	    cflag = (value1 >> 31) & 1;
	    break;

	    case 4: // rcr
	    if (value2)
	    {
	        result = (cflag << 31) | (((uint32_t)value1) >> 1);
		result >>= (value2 - 1);
	    }
	    else
	        result = value1;
	    cflag = value1 & 1;
	    break;

	    case 5: // rcl
	    result = cflag ? (1 << value2) - 1 : 0;
	    result |= (value1 << value2);
	    cflag = (value1 >> 31) & 1;
	    break;

	    case 6: // sar
	    result = value1 >> value2;
	    cflag = value1 & 1;
	    break;

	    case 7: // rev
	    cflag = value1 & 1;
	    value2 = 32 - value2;
	    result = 0;
	    while (value2-- > 0)
	    {
		result = (result << 1) | (value1 & 1);
		value1 >>= 1;
	    }
	    break;
	}
	zflag = (result == 0);
	break;

	// Jump, call, return and misc.
	case 2:
	switch (opcode & 7)
	{
	    case 0: // mins
	    cflag = (value1 < value2);
	    zflag = (value2 == 0);
	    result = cflag ? value2 : value1;
	    break;

	    case 1: // maxs
	    cflag = (value1 < value2);
	    zflag = (value2 == 0);
	    result = cflag ? value1 : value2;
	    break;

	    case 2: // min
	    cflag = (((uint32_t)value1) < ((uint32_t)value2));
	    zflag = (value2 == 0);
	    result = cflag ? value2 : value1;
	    break;

	    case 3: // max
	    cflag = (((uint32_t)value1) < ((uint32_t)value2));
	    zflag = (value2 == 0);
	    result = cflag ? value1 : value2;
	    break;

	    case 4: // movs
	    cflag = ((uint32_t)value1) < ((uint32_t)value2);
	    result = (value1 & 0xfffffe00) | (value2 &0x1ff);
	    zflag = (result == 0);
	    break;

	    case 5: // movd
	    cflag = ((uint32_t)value1) < ((uint32_t)value2);
	    result = (value1 & 0xfffc01ff) | ((value2 &0x1ff) << 9);
	    zflag = (result == 0);
	    break;

	    case 6: // movi
	    cflag = ((uint32_t)value1) < ((uint32_t)value2);
	    result = (value1 & 0x007fffff) | ((value2 &0x1ff) << 23);
	    zflag = (result == 0);
	    break;

	    case 7: // ret, jmp, call, jmpret
	    cflag = ((uint32_t)value1) < ((uint32_t)value2);
	    result = (value1 & 0xfffffe00) | pasmvars->pc;
	    pasmvars->pc = value2 & 0x1ff;
	    zflag = (result == 0);
	    break;
	}
	break;

	// Logical operations
	case 3:
	switch (opcode & 7)
	{
	    case 0: // test, and
	    result = value1 & value2;
	    break;

	    case 1: // testn, andn
	    result = value1 & (~value2);
	    break;

	    case 2: // or
	    result = value1 | value2;
	    break;

	    case 3: // xor
	    result = value1 ^ value2;
	    break;

	    case 4: // muxc
	    result = (value1 & (~value2)) | (value2 & (-cflag));
	    break;

	    case 5: // muxnc
	    result = (value1 & (~value2)) | (value2 & (~(-cflag)));
	    break;

	    case 6: // muxz
	    result = (value1 & (~value2)) | (value2 & (-zflag));
	    break;

	    case 7: // muxnz
	    result = (value1 & (~value2)) | (value2 & (~(-zflag)));
	    break;
	}
	zflag = (result == 0);
	cflag = parity(result);
	break;

	// Add and subtract
	case 4:
	switch (opcode & 7)
	{
	    case 0: // add
	    result = value1 + value2;
	    cflag = (((value1 & value2) | ((value1 | value2) & (~result))) >> 31) & 1;
	    break;

	    case 1: // cmp, sub
	    result = value1 - value2;
	    cflag = ((uint32_t)value1) < ((uint32_t)value2);
	    break;

	    case 2: // addabs
	    cflag = (value2 >> 31) & 1;
	    value2 = abs(value2);
	    result = value1 + value2;
	    cflag ^= (((value1 & value2) | ((value1 | value2) & (~result))) >> 31) & 1;
	    break;

	    case 3: // subabs
	    result = abs(value2);
	    cflag = ((value2 >> 31) & 1) ^
	        (((uint32_t)value1) < ((uint32_t)result));
	    result = value1 - result;
	    break;

	    case 4: // sumc
	    result = cflag ? value1 - value2 : value1 + value2;
	    cflag = (~cflag) << 31;
	    cflag = (((cflag ^ value1 ^ value2) & (value1 ^ result)) >> 31) & 1;
	    break;

	    case 5: // sumnc
	    result = cflag ? value1 + value2 : value1 - value2;
	    cflag = cflag << 31;
	    cflag = (((cflag ^ value1 ^ value2) & (value1 ^ result)) >> 31) & 1;
	    break;

	    case 6: // sumz
	    result = zflag ? value1 - value2 : value1 + value2;
	    cflag = (~zflag) << 31;
	    cflag = (((cflag ^ value1 ^ value2) & (value1 ^ result)) >> 31) & 1;
	    break;

	    case 7: // sumnz
	    result = zflag ? value1 + value2 : value1 - value2;
	    cflag = zflag << 31;
	    cflag = (((cflag ^ value1 ^ value2) & (value1 ^ result)) >> 31) & 1;
	    break;
	}
	zflag = (result == 0);
	break;

	// Move, absolute and negate
	case 5:
	switch (opcode & 7)
	{
	    case 0: // mov
	    result = value2;
	    cflag = (value2 >> 31) & 1;
	    break;

	    case 1: // neg
	    cflag = value2 < 0;
	    result = -value2;
	    break;

	    case 2: // abs
	    cflag = (value2 >> 31) & 1;
	    result = abs(value2);
	    break;

	    case 3: // absneg
	    cflag = (value2 >> 31) & 1;
	    result = -abs(value2);
	    break;

	    case 4: // negc
	    result = cflag ? -value2 : value2;
	    cflag = (value2 >> 31) & 1;
	    break;

	    case 5: // negnc
	    result = cflag ? value2 : -value2;
	    cflag = (value2 >> 31) & 1;
	    break;

	    case 6: // negz
	    result = zflag ? -value2 : value2;
	    cflag = (value2 >> 31) & 1;
	    break;

	    case 7: // negnz
	    result = zflag ? value2 : -value2;
	    cflag = (value2 >> 31) & 1;
	    break;
	}
	zflag = (result == 0);
	break;

	// More add and subtract
	case 6:
	switch (opcode & 7)
	{
	    case 0: // cmps
	    result = value1 - value2;
	    cflag = value1 < value2;
	    zflag = (result == 0);
	    break;

	    case 1: // cmpsx
	    result = value1 - value2 - cflag;
	    cflag = value1 < (value2 + cflag);
	    zflag = (result == 0) & zflag;
	    break;

	    case 2: // addx
	    result = value1 + value2 + cflag;
	    cflag = (((value1 & value2) | ((value1 | value2) & (~result))) >> 31) & 1;
	    zflag = (result == 0) & zflag;
	    break;

	    case 3: // cmpx, subx
	    result = value1 - value2 - cflag;
	    if (value2 != 0xffffffff || !cflag)
	        cflag = ((uint32_t)value1) < ((uint32_t)(value2 + cflag));
	    zflag = (result == 0) & zflag;
	    break;

	    case 4: // adds
	    result = value1 + value2;
	    cflag = (((~(value1 ^ value2)) & (value1 ^ result)) >> 31) & 1;
	    zflag = (result == 0);
	    break;

	    case 5: // subs
	    result = value1 - value2;
	    zflag = (result == 0);
	    cflag = (((value1 ^ value2) & (value1 ^ result)) >> 31) & 1;
	    break;

	    case 6: // addsx
	    result = value1 + value2 + cflag;
	    cflag = (((~(value1 ^ value2)) & (value1 ^ result)) >> 31) & 1;
	    zflag = (result == 0) & zflag;
	    break;

	    case 7: // subsx
	    result = value1 - value2 - cflag;
	    cflag = (((value1 ^ value2) & (value1 ^ result)) >> 31) & 1;
	    zflag = (result == 0) & zflag;
	    break;
	}
	break;

	// Test and jump and wait ops
	case 7:
	switch (opcode & 7)
	{
	    case 0: // cmpsub
	    cflag = (((uint32_t)value1) >= ((uint32_t)value2));
	    result = cflag ? value1 - value2 : value1;
	    zflag = (result == 0);
	    break;

	    case 1: // djnz
	    result = value1 - 1;
	    zflag = (result == 0);
	    cflag = (result == -1);
	    if (!zflag) pasmvars->pc = srcaddr;
	    else if (cycleaccurate)
	    {
	        if (CheckWaitFlag(pasmvars, 0)) return 0;
	    }
	    break;

	    case 2: // tjnz
	    result = value1;
	    zflag = (result == 0);
	    cflag = 0;
	    if (!zflag) pasmvars->pc = srcaddr;
	    else if (cycleaccurate)
	    {
	        if (CheckWaitFlag(pasmvars, 0)) return 0;
	    }
	    break;

	    case 3: // tjz
	    result = value1;
	    zflag = (result == 0);
	    cflag = 0;
	    if (zflag) pasmvars->pc = srcaddr;
	    else if (cycleaccurate)
	    {
	        if (CheckWaitFlag(pasmvars, 0)) return 0;
	    }
	    break;

	    case 4: // waitpeq - result, zflag and cflag not validated
            result = (pin_val & value2) ^ value1;
	    if (result)
	    {
		pasmvars->state = 6;
    		pasmvars->pc = (pasmvars->pc - 1) & 511;
		return 0;
	    }
	    else
	    {
		pasmvars->state = 5;
	        zflag = (result == 0);
	        cflag = 0;
	    }
	    break;

	    case 5: // waitpne - result, zflag and cflag not validated
            result = (pin_val & value2) ^ value1;
	    if (!result)
	    {
		pasmvars->state = 6;
    		pasmvars->pc = (pasmvars->pc - 1) & 511;
		return 0;
	    }
	    else
	    {
		pasmvars->state = 5;
	        zflag = (result == 0);
	        cflag = zflag;
	    }
	    break;

	    case 6: // waitcnt
	    result = GetCnt() - value1;
	    if (result < 0 || result > 20000000)
	    {
		pasmvars->state = 6;
    		pasmvars->pc = (pasmvars->pc - 1) & 511;
		return 0;
	    }
	    else
	    {
		pasmvars->state = 5;
	        result = value1 + value2;
	        zflag = (result == 0);
	        cflag = (((value1 & value2) | ((value1 | value2) & (~result))) >> 31) & 1;
	    }
	    break;

	    case 7: // waitvid
	    break;
	}
	break;
    }

    // Conditionally update flags and write result
    if (zcri & 8) pasmvars->zflag = zflag;
    if (zcri & 4) pasmvars->cflag = cflag;
    if (zcri & 2)
    {
	//if (dstaddr == 0x1f4) printf("outa = %8.8x\n", result);
	pasmvars->mem[dstaddr] = result;
	// Check if we need to update the pins
	if (dstaddr == 0x1f4 || dstaddr == 0x1f6) UpdatePins();
    }
    //CheckSerialOut(pasmvars);
    if (pasmvars->waitflag)
    {
      fprintf(tracefile, "XXXXXXXXXX BAD XXXXXXXXXXXXXXX\n");
	pasmvars->waitflag--;
    }
    return result;
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
