/*******************************************************************************
' Author: Dave Hein
' Version 0.21
' Copyright (c) 2010, 2011
' See end of file for terms of use.
'******************************************************************************/
#include <stdio.h>
#include "interp.h"

#define IGNORE_WZ_WC
#define PRINT_RAM_ACCESS

#if 1
#define REG_INDA 0x1f6
#define REG_INDB 0x1f7
#define REG_DIRA 0x1fc
#define REG_OUTA 0x1f8
#define REG_INA  0x1f8
#else
#define REG_INA  0x1f2
#define REG_OUTA 0x1f4
#define REG_DIRA 0x1f6
#endif

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

static int32_t seuss(int32_t value, int32_t forward)
{
    uint32_t a, c, x, y;

    x = value;
    if (!x) x = 1;
    y = 32;
    a = 0x17;
    if (!forward) a = (a >> 1) | (a << 31);
    while (y--)
    {
        c = x & a;
        while (c & 0xfffffffe) c = (c >> 1) ^ (c & 1);
        if (forward)
            x = (x >> 1) | (c << 31);
        else
            x = (x << 1) | c;
    }
    return x;
}

static int32_t CheckWaitFlag(PasmVarsT *pasmvars, int mode)
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
        //if (waitflag)
            //pasmvars->pc = (pasmvars->pc - 1) & 511;
	pasmvars->waitflag = waitflag;
    }

    return waitflag;
}

// Compute the hub RAM address from the pointer instruction - 1SUPIIIII
int32_t GetPointer(PasmVarsT *pasmvars, int32_t ptrinst, int32_t size)
{
    int32_t address;
    int32_t offset = (ptrinst << 27) >> (27 - size);

    switch ((ptrinst >> 5) & 7)
    {
        case 0: // ptra[offset]
        address = pasmvars->ptra + offset;
        break;

        case 1: // ptra
        address = pasmvars->ptra;
        break;

        case 2: // ptra[++offset]
        address = pasmvars->ptra + offset;
        pasmvars->ptra = address;
        break;

        case 3: // ptra[offset++]
        address = pasmvars->ptra;
        pasmvars->ptra += offset;
        break;

        case 4: // ptrb[offset]
        address = pasmvars->ptrb + offset;
        break;

        case 5: // ptrb
        address = pasmvars->ptrb;
        break;

        case 6: // ptrb[++offset]
        address = pasmvars->ptrb + offset;
        pasmvars->ptrb = address;
        break;

        case 7: // ptrb[offset++]
        address = pasmvars->ptrb;
        pasmvars->ptrb += offset;
        break;
    }

    return address;
}

void UpdatePins2(void)
{
    int32_t i;
    int32_t mask1;
    int32_t val = 0;
    int32_t mask = 0;

    for (i = 0; i < 8; i++)
    {
	if (PasmVars[i].state)
	{
	    mask1 = PasmVars[i].mem[REG_DIRA]; // dira
	    val |= mask1 & PasmVars[i].mem[REG_OUTA]; // outa
	    mask |= mask1;
	}
    }
    pin_val = (~mask) | val;
}

void CheckIndRegs(PasmVarsT *pasmvars, int32_t instruct, int32_t *pdstaddr, int32_t *psrcaddr)
{
    int32_t incra = 0;
    int32_t incrb = 0;
    int32_t dstaddr = *pdstaddr;
    int32_t srcaddr = *psrcaddr;

    // Check srcaddr if not immediate
    if (!(instruct & 0x00400000)) // Check the i-bit
    {
        if (srcaddr == REG_INDA)
        {
	    //printf("CheckIndRegs: Change srcaddr to %3.3x\n", pasmvars->inda);
	    incra = 1;
            srcaddr = pasmvars->inda;
        }
        else if (srcaddr == REG_INDB)
        {
	    //printf("CheckIndRegs: Change srcaddr to %3.3x\n", pasmvars->indb);
	    incrb = 1;
            srcaddr = pasmvars->indb;
        }
    }

    // Check for (opcode == 3 && (zcri & 3) == 3 && (srcaddr >> 7) == 1)
    // Check dstaddr if not immediate
    if ((instruct & 0xfcc00180) != 0x0cc00080)
    {
        if (dstaddr == REG_INDA)
        {
	    //printf("CheckIndRegs: Change dstaddr to %3.3x\n", pasmvars->inda);
	    incra = 1;
            dstaddr = pasmvars->inda;
        }
        else if (dstaddr == REG_INDB)
        {
	    //printf("CheckIndRegs: Change dstaddr to %3.3x\n", pasmvars->indb);
	    incrb = 1;
            dstaddr = pasmvars->indb;
        }
    }

    // Increment INDA and INDB if needed and not waiting
    if (!pasmvars->waitflag)
    {
	if (incra)
	{
            if (pasmvars->inda == pasmvars->indatop)
                pasmvars->inda = pasmvars->indabot;
            else
                pasmvars->inda = (pasmvars->inda + 1) & 0x1ff;
	}
	if (incrb)
	{
            if (pasmvars->indb == pasmvars->indbtop)
                pasmvars->indb = pasmvars->indbbot;
            else
                pasmvars->indb = (pasmvars->indb + 1) & 0x1ff;
	}
    }

    // Update values of srcaddr and dstaddr
    *psrcaddr = srcaddr;
    *pdstaddr = dstaddr;
}

int32_t ExecutePasmInstruction2(PasmVarsT *pasmvars)
{
    int32_t cflag = pasmvars->cflag;
    int32_t zflag = pasmvars->zflag;
    int32_t instruct, pc, cond;
    int32_t opcode, value2, value1, zcri;
    int32_t srcaddr, dstaddr;
    int32_t result = 0;

    // Fetch a new instruction and update the pipeline
    if (!pasmvars->waitflag)
    {
	//printf("Fetch new instruction\n");
        pasmvars->instruct3 = pasmvars->instruct2;
        pasmvars->instruct2 = pasmvars->instruct1;

	// Check for REP mode
	if (pasmvars->repcnt && pasmvars->pc == pasmvars->reptop)
	{
	    if (--pasmvars->repcnt)
	        pasmvars->pc = pasmvars->repbot;
	}

	if ((pasmvars->pc & 0x1fc) == pasmvars->cachecogaddr)
            pasmvars->instruct1 = pasmvars->cache[pasmvars->pc & 3];
	else
            pasmvars->instruct1 = pasmvars->mem[pasmvars->pc];
        pasmvars->pc3 = pasmvars->pc2;
        pasmvars->pc2 = pasmvars->pc1;
        pasmvars->pc1 = pasmvars->pc;
        pasmvars->pc = (pasmvars->pc + 1) & 511;
    }

    // Get the instruction and pc at the end of the pipeline
    instruct = pasmvars->instruct3;
    pc = pasmvars->pc3;
    cond = (instruct >> 18) & 15;

    // Return if not executed
    if ((!((cond >> ((cflag << 1) | zflag)) & 1)) || (pc & 0xfffffe00))
    {
	return 0;
    }

    // Extract parameters from the instruction
    opcode = (instruct >> 26) & 63;
    srcaddr = instruct & 511;
    dstaddr = (instruct >> 9) & 511;
    zcri = (instruct >> 22) & 15;

    // Check if using INDA or INDB
    CheckIndRegs(pasmvars, instruct, &dstaddr, &srcaddr);

    // Get the two operands
    if (opcode == 3 && (zcri & 3) == 3 && (srcaddr >> 7) == 1)
    {
        zcri &= 0xd;
	value1 = dstaddr;
    }
    else
    {
	if ((dstaddr & 0x1fc) == pasmvars->cachecogaddr)
            value1 = pasmvars->cache[dstaddr & 3];
	else
            value1 = pasmvars->mem[dstaddr];
    }

    if (zcri & 1)
        value2 = srcaddr;
    else if (srcaddr == 0x1f1)
        value2 = GetCnt();
    else if (srcaddr == REG_INA)
        value2 = pin_val;
    else
    {
	if ((srcaddr & 0x1fc) == pasmvars->cachecogaddr)
            value2 = pasmvars->cache[srcaddr & 3];
	else
            value2 = pasmvars->mem[srcaddr];
    }

    // Check for a hub wait
    if (cycleaccurate && ((opcode <= 2) ||
        (instruct & 0xfc4001f8) == 0x0c400000 ||
        (instruct & 0xfc4001fe) == 0x0c4000b0 ||
        (instruct & 0xfc400000) == 0x0c000000))
    {
	// Exclude if rdxxxxc and cache hit
	if (opcode > 2 || (zcri & 6) != 6 || (value2 & 0xffffff0) != pasmvars->cachehubaddr)
	{
	    if (CheckWaitFlag(pasmvars, 1)) return 0;
	}
    }

    // Decode the three most significant bits of the opcode
    switch(opcode >> 3)
    {
	// Hub access opcodes
	case 0:
	switch (opcode & 7)
	{
	    case 0: // wrbyte, rdbyte, rdbytec
	    case 1: // wrword, rdword, rdwordc
	    case 2: // wrlong, rdlong, rdlongc

	    // Check if using a ptr register
	    if ((zcri & 1) && (srcaddr & 0x100))
	        value2 = GetPointer(pasmvars, srcaddr, opcode & 3);

	    if (zcri & 2) // read
	    {
		if (zcri & 4) // cache read
		{
		    int32_t hubaddr = value2 & 0xfffffff0;
		    if (hubaddr != pasmvars->cachehubaddr)
		    {
	                pasmvars->cache[0] = LONG(hubaddr);
	                pasmvars->cache[1] = LONG(hubaddr+4);
	                pasmvars->cache[2] = LONG(hubaddr+8);
	                pasmvars->cache[3] = LONG(hubaddr+12);
			pasmvars->cachehubaddr = hubaddr;
		    }
		    if ((opcode & 7) == 0)
		    {
			uint8_t *ptr = (uint8_t *)pasmvars->cache;
		        result = ptr[value2 & 15];
#ifdef PRINT_RAM_ACCESS
                        if (LONG(SYS_DEBUG) & (1 << pasmvars->cogid))
		            printf(", rdb[%x]", value2);
#endif
		    }
		    else if ((opcode & 7) == 1)
		    {
			uint16_t *ptr = (uint16_t *)pasmvars->cache;
		        result = ptr[(value2 >> 1) & 7];
#ifdef PRINT_RAM_ACCESS
                        if (LONG(SYS_DEBUG) & (1 << pasmvars->cogid))
		            printf(", rdw[%x]", value2);
#endif
		    }
		    else
		    {
		        result = pasmvars->cache[(value2 >> 2) & 3];
#ifdef PRINT_RAM_ACCESS
                        if (LONG(SYS_DEBUG) & (1 << pasmvars->cogid))
		            printf(", rdl[%x]", value2);
#endif
		    }
		}
		else // non-cache read
		{
		    if ((opcode & 7) == 0)
		    {
		        result = BYTE(value2);
#ifdef PRINT_RAM_ACCESS
                        if (LONG(SYS_DEBUG) & (1 << pasmvars->cogid))
		            printf(", rdb[%x]", value2);
#endif
		    }
		    else if ((opcode & 7) == 1)
		    {
		        result = WORD(value2);
#ifdef PRINT_RAM_ACCESS
                        if (LONG(SYS_DEBUG) & (1 << pasmvars->cogid))
		            printf(", rdw[%x]", value2);
#endif
		    }
		    else
		    {
		        result = LONG(value2);
#ifdef PRINT_RAM_ACCESS
                        if (LONG(SYS_DEBUG) & (1 << pasmvars->cogid))
		            printf(", rdl[%x]", value2);
#endif
		    }
		}
		zflag = (result == 0);
#ifdef IGNORE_WZ_WC
	        zcri &= 0xb; // Ignore the C write flag
#endif
	    }
	    else          // write
	    {
	        //if ((dstaddr & 0x1fc) == pasmvars->cachecogaddr)
                    //result = pasmvars->cache[dstaddr & 3];
		//else
		    //result = pasmvars->mem[dstaddr];
		if ((opcode & 7) == 0)
		{
		    BYTE(value2) = value1;
#ifdef PRINT_RAM_ACCESS
                    if (LONG(SYS_DEBUG) & (1 << pasmvars->cogid))
		        printf(", wrb[%x] = %x", value2, value1);
#endif
		}
		else if ((opcode & 7) == 1)
		{
		    WORD(value2) = value1;
#ifdef PRINT_RAM_ACCESS
                    if (LONG(SYS_DEBUG) & (1 << pasmvars->cogid))
		        printf(", wrw[%x] = %x", value2, value1);
#endif
		}
		else
		{
		    LONG(value2) = value1;
#ifdef PRINT_RAM_ACCESS
                    if (LONG(SYS_DEBUG) & (1 << pasmvars->cogid))
		        printf(", wrl[%x] = %x", value2, value1);
#endif
		}
	    }
	    break;

	    case 3: // misc hubops
	    // Check for new coginit.  If it is the new coginit
	    // save value2 and set it for the old coginit.
	    if ((zcri & 1) == 0)
	    {
		result = value2;
		value2 = 2;
	    }
	    switch (value2 & 0x1f8)
	    {
                case 0x000: // coginit, clkset, cogid, coginit_old,
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
		    break;

		    case 2: // coginit
		    if ((zcri & 1) == 0) // New coginit
		    {
			par = (result << 2) & 0x3fffc;
			addr = (result >> 14) & 0x3fffc;
		    }
		    else                 // Old coginit
		    {
		        par = (value1 >> 16) & 0xfffc;
		        addr = (value1 >> 2) & 0xfffc;
		    }
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

                    // Check for P1 Spin interpreter address, and change
		    // to interpreter at the beginning of the DAT section.
                    if (addr == 0xf004)
		    {
	                int pbase = WORD(6);
	                int nummeth = BYTE(pbase + 2);
	                int numobj = BYTE(pbase + 3);
	                addr = pbase + (nummeth + numobj) * 4;
		    }

		    StartPasmCog2(&PasmVars[result], par, addr, result);
	            UpdatePins2();

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
		    UpdatePins2();
		    //printf("COGSTOP %d\n", result);
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

                case 0x008:
	        switch (value2 & 7)
	        {
		    case 0: // rcvser, sndser
		    // TODO
		    break;

		    case 1: // swapzc
		    result = (value1 & 0xfffffffc) | (zflag << 1) | cflag;
		    zflag = (value1 >> 1) & 1;
		    cflag = value1 & 1;
		    break;

		    case 2: // pushzc
		    result = (value1 << 2) | (zflag << 1) | cflag;
		    zflag = (value1 >> 31) & 1;
		    cflag = (value1 >> 30) & 1;
		    break;

		    case 3: // popzc
		    result = ((value1 >> 2) & 0x3fffffff) | (zflag << 31) | (cflag << 30);
		    zflag = (value1 >> 1) & 1;
		    cflag = value1 & 1;
		    break;

		    case 4: // cachex
		    result = 0;
		    zflag = 1;
		    cflag = 0;
		    pasmvars->cachehubaddr = 0xffffffff;
		    break;

		    case 5: // clracc
		    pasmvars->acch = 0;
		    pasmvars->accl = 0;
		    break;

		    case 6: // getacc
		    result = pasmvars->accl;
		    zflag = (result == 0);
		    cflag = 0;
		    break;

		    case 7: // getcnt
		    result = GetCnt();
		    zflag = (result == 0);
		    cflag = 0;
		    break;
		}
		break;

                case 0x010:
	        switch (value2 & 7)
	        {
		    case 0: // getlfsr
		    // TODO
		    break;

		    case 1: // gettops
		    result = pasmvars->cache[3] & 0xff000000;
		    result |= (pasmvars->cache[2] >> 8) & 0xff0000;
		    result |= (pasmvars->cache[1] >> 16) & 0xff00;
		    result |= (pasmvars->cache[0] >> 24) & 0xff;
		    break;

		    case 2: // getptra
		    result = pasmvars->ptra;
		    break;

		    case 3: // getptrb
		    result = pasmvars->ptrb;
		    break;

		    case 4: // getpix
		    // TODO
		    break;

		    case 5: // getspd
		    result = (pasmvars->spa - pasmvars->spb) & 127;
		    break;

		    case 6: // getspa
		    result = pasmvars->spa;
		    break;

		    case 7: // getspb
		    result = pasmvars->spb;
		    break;
		}
		zflag = (result == 0);
		cflag = 0;
		break;

                case 0x018:
	        switch (value2 & 7)
	        {
		    case 0: // popupa
		    result = pasmvars->clut[pasmvars->spa];
		    pasmvars->spa = (pasmvars->spa + 1) & 127;
		    break;

		    case 1: // popupb
		    result = pasmvars->clut[pasmvars->spb];
		    pasmvars->spb = (pasmvars->spb + 1) & 127;
		    break;

		    case 2: // popa
		    pasmvars->spa = (pasmvars->spa - 1) & 127;
		    result = pasmvars->clut[pasmvars->spa];
		    break;

		    case 3: // popb
		    pasmvars->spb = (pasmvars->spb - 1) & 127;
		    result = pasmvars->clut[pasmvars->spb];
		    break;

		    case 4: // reta
		    zcri &= 0xd; // Disable the WR bit
		    pasmvars->spa = (pasmvars->spa - 1) & 127;
		    result = pasmvars->clut[pasmvars->spa];
		    pasmvars->pc = result & 0x1ff;
	            // Invalidate the instruction pipeline
                    pasmvars->pc1 |= 512;
                    pasmvars->pc2 |= 512;
		    break;

		    case 5: // retb
		    zcri &= 0xd; // Disable the WR bit
		    pasmvars->spb = (pasmvars->spb - 1) & 127;
		    result = pasmvars->clut[pasmvars->spb];
		    pasmvars->pc = result & 0x1ff;
	            // Invalidate the instruction pipeline
                    pasmvars->pc1 |= 512;
                    pasmvars->pc2 |= 512;
		    break;

		    case 6: // retad
		    zcri &= 0xd; // Disable the WR bit
		    pasmvars->spa = (pasmvars->spa - 1) & 127;
		    result = pasmvars->clut[pasmvars->spa];
		    pasmvars->pc = result & 0x1ff;
		    break;

		    case 7: // retbd
		    zcri &= 0xd; // Disable the WR bit
		    pasmvars->spb = (pasmvars->spb - 1) & 127;
		    result = pasmvars->clut[pasmvars->spb];
		    pasmvars->pc = result & 0x1ff;
		    break;
		}
		zflag = (result == 0);
		cflag = 0;
		break;

                case 0x020:
	        switch (value2 & 7)
	        {
		    case 0: // decod2
		    result = 1 << (value1 & 3);
		    result |= result << 4;
		    result |= result << 8;
		    result |= result << 16;
		    break;

		    case 1: // decod3
		    result = 1 << (value1 & 7);
		    result |= result << 8;
		    result |= result << 16;
		    break;

		    case 2: // decod4
		    result = 1 << (value1 & 0xf);
		    result |= result << 16;
		    break;

		    case 3: // decod5
		    result = 1 << (value1 & 0x1f);
		    break;

		    case 4: // blmask
		    value1 = (value1 & 0x1f) + 1;
		    if (value1 == 32)
		        result = 0xffffffff;
		    else
		        result = (1 << value1) - 1;
		    break;

		    case 5: // not
		    result = ~value1;
		    break;

		    case 6: // onecnt
		    for (result = 0; value1; value1 <<= 1)
		    {
			if (value1 & 0x80000000) result++;
		    }
		    break;

		    case 7: // zercnt
		    for (result = 32; value1; value1 <<= 1)
		    {
			if (value1 & 0x80000000) result--;
		    }
		    break;
		}
		zflag = (result == 0);
		cflag = 0;
		break;

                case 0x028:
	        switch (value2 & 7)
	        {
		    int i;
		    case 0: // incpat
		    // TODO
		    break;

		    case 1: // decpat
		    // TODO
		    break;

		    case 2: // bingry
		    result = value1 ^ ((value1 >> 1) & 0x7fffffff);
		    break;

		    case 3: // grybin
		    // TODO
		    break;

		    case 4: // mergew
		    for (i = 0; i < 16; i++)
		    {
			result = (result << 2) |
			    ((value1 >> 30) & 2) | ((value1 >> 15) & 1);
			value1 <<= 1;
		    }
		    break;

		    case 5: // splitw
		    for (i = 0; i < 16; i++)
		    {
			result = (result << 1) |
			    ((value1 >> 15) & 0x10000) | ((value1 >> 30) & 1);
			value1 <<= 1;
		    }
		    break;

		    case 6: // seussf
		    result = seuss(value1, 1);
		    break;

		    case 7: // seussr
		    result = seuss(value1, 0);
		    break;
		}
		break;

                case 0x030:
	        switch (value2 & 7)
	        {
		    case 0: // getmull
		    // TODO
		    break;

		    case 1: // getmulh
		    // TODO
		    break;

		    case 2: // divq
		    // TODO
		    break;

		    case 3: // divr
		    // TODO
		    break;

		    case 4: // sqrt
		    // TODO
		    break;

		    case 5: // corx
		    // TODO
		    break;

		    case 6: // cory
		    // TODO
		    break;

		    case 7: // corz
		    // TODO
		    break;
		}
		break;

                case 0x038:
	        switch (value2 & 7)
	        {
		    case 0: // getphsa
		    // TODO
		    break;

		    case 1: // getphza
		    // TODO
		    break;

		    case 2: // getcosa
		    // TODO
		    break;

		    case 3: // getsina
		    // TODO
		    break;

		    case 4: // getphsb
		    // TODO
		    break;

		    case 5: // getphzb
		    // TODO
		    break;

		    case 6: // getcosb
		    // TODO
		    break;

		    case 7: // getsinb
		    // TODO
		    break;
		}
		break;

                case 0x080:
                case 0x088:
                case 0x090:
                case 0x098: // rep
		pasmvars->repcnt = value1;
		pasmvars->repbot = (pasmvars->pc - 1) & 0x1ff;
		pasmvars->reptop = (pasmvars->pc + (srcaddr & 31) - 1) & 0x1ff;
		//printf("repcnt = %d, repbot = $%3.3x, reptop = $%3.3x\n",
		    //pasmvars->repcnt, pasmvars->repbot, pasmvars->reptop);
		break;

                case 0x0a0:
	        switch (value2 & 7)
	        {
		    case 0: // nopx
		    if (!pasmvars->waitflag)
		        pasmvars->waitflag = value1 - 1;
		    else
			pasmvars->waitflag--;
		    return 0;
		    break;

		    case 1: // setzc
		    zflag = (value1 >> 1) & 1;
		    cflag = value1 & 1;
		    break;

		    case 2: // setspa
		    result = value1 & 127;
		    pasmvars->spa = result;
		    break;

		    case 3: // setspb
		    result = value1 & 127;
		    pasmvars->spb = result;
		    break;

		    case 4: // addspa
		    result = (pasmvars->spa + value1) & 127;
		    pasmvars->spa = result;
		    break;

		    case 5: // addspb
		    result = (pasmvars->spb + value1) & 127;
		    pasmvars->spb = result;
		    break;

		    case 6: // subspa
		    result = (pasmvars->spa - value1) & 127;
		    pasmvars->spa = result;
		    break;

		    case 7: // subspb
		    result = (pasmvars->spb - value1) & 127;
		    pasmvars->spb = result;
		    break;
		}
		zflag = (result == 0);
		cflag = 0;
		break;

                case 0x0a8:
	        switch (value2 & 7)
	        {
		    case 0: // pushdna
		    result = value1;
		    pasmvars->spa = (pasmvars->spa - 1) & 127;
		    pasmvars->clut[pasmvars->spa] = result;
		    break;

		    case 1: // pushdnb
		    result = value1;
		    pasmvars->spb = (pasmvars->spb - 1) & 127;
		    pasmvars->clut[pasmvars->spb] = result;
		    break;

		    case 2: // pusha
		    result = value1;
		    pasmvars->clut[pasmvars->spa] = result;
		    pasmvars->spa = (pasmvars->spa + 1) & 127;
		    break;

		    case 3: // pushb
		    result = value1;
		    pasmvars->clut[pasmvars->spb] = result;
		    pasmvars->spb = (pasmvars->spb + 1) & 127;
		    break;

		    case 4: // calla
	            //result = pasmvars->pc;
	            result = (pc + 1) & 511;
		    pasmvars->clut[pasmvars->spa] = result;
		    pasmvars->spa = (pasmvars->spa + 1) & 127;
	            pasmvars->pc = value1 & 0x1ff;
	            // Invalidate the instruction pipeline
                    pasmvars->pc1 |= 512;
                    pasmvars->pc2 |= 512;
		    break;

		    case 5: // callb
	            //result = pasmvars->pc;
	            result = (pc + 1) & 511;
		    pasmvars->clut[pasmvars->spb] = result;
		    pasmvars->spb = (pasmvars->spb + 1) & 127;
	            pasmvars->pc = value1 & 0x1ff;
	            // Invalidate the instruction pipeline
                    pasmvars->pc1 |= 512;
                    pasmvars->pc2 |= 512;
		    break;

		    case 6: // callad
	            result = pasmvars->pc;
		    pasmvars->clut[pasmvars->spa] = result;
		    pasmvars->spa = (pasmvars->spa + 1) & 127;
	            pasmvars->pc = value1 & 0x1ff;
		    break;

		    case 7: // callbd
	            result = pasmvars->pc;
		    pasmvars->clut[pasmvars->spb] = result;
		    pasmvars->spb = (pasmvars->spb + 1) & 127;
	            pasmvars->pc = value1 & 0x1ff;
		    break;
		}
		zflag = (result == 0);
		cflag = 0;
		break;

                case 0x0b0:
	        switch (value2 & 7)
	        {
		    case 0: // wrquad
		    zflag = 1;
		    cflag = 0;

	            // Check if using a ptr register
	            //if ((zcri & 2) && (dstaddr & 0x100))
		    if (instruct & 0x00820000)
		    {
			//printf("Using ptr: Changing value1 from %8.8x to ", value1);
	                value1 = GetPointer(pasmvars, dstaddr, 2);
			//printf("%8.8x\n", value1);
		    }

                    value1 &= 0xfffffff0;
	            LONG(value1) = pasmvars->cache[0];
	            LONG(value1+4) = pasmvars->cache[1];
	            LONG(value1+8) = pasmvars->cache[2];
	            LONG(value1+12) = pasmvars->cache[3];
#ifdef  PRINT_RAM_ACCESS
		    printf("wrquad(%8.8x) %8.8x %8.8x %8.8x %8.8x\n",
		        value1, pasmvars->cache[0], pasmvars->cache[1],
		        pasmvars->cache[2], pasmvars->cache[3]);
#endif
		    break;

		    case 1: // rdquad
		    zflag = 1;
		    cflag = 0;

	            // Check if using a ptr register
	            //if ((zcri & 2) && (dstaddr & 0x100))
		    if (instruct & 0x00820000)
		    {
			//printf("Using ptr: Changing value1 from %8.8x to ", value1);
	                value1 = GetPointer(pasmvars, dstaddr, 2);
			//printf("%8.8x\n", value1);
		    }

                    value1 &= 0xfffffff0;
	            pasmvars->cache[0] = LONG(value1);
	            pasmvars->cache[1] = LONG(value1+4);
	            pasmvars->cache[2] = LONG(value1+8);
	            pasmvars->cache[3] = LONG(value1+12);
#ifdef  PRINT_RAM_ACCESS
		    printf("rdquad(%8.8x) %8.8x %8.8x %8.8x %8.8x\n",
		        value1, pasmvars->cache[0], pasmvars->cache[1],
		        pasmvars->cache[2], pasmvars->cache[3]);
#endif
		    break;

		    case 2: // setptra
		    result = value1;
		    pasmvars->ptra = value1;
		    //printf("setptra %8.8x\n", value1);
		    break;

		    case 3: // setptrb
		    result = value1;
		    pasmvars->ptrb = value1;
		    //printf("setptrb %8.8x\n", value1);
		    break;

		    case 4: // addptra
		    result = pasmvars->ptra + value1;
		    pasmvars->ptra = result;
		    break;

		    case 5: // subptra
		    result = pasmvars->ptra - value1;
		    pasmvars->ptra = result;
		    break;

		    case 6: // addptrb
		    result = pasmvars->ptrb + value1;
		    pasmvars->ptrb = result;
		    break;

		    case 7: // subptrb
		    result = pasmvars->ptrb - value1;
		    pasmvars->ptrb = result;
		    break;
		}
		cflag = 0;
		zflag = (result = 0);
		break;

                case 0x0b8:
	        switch (value2 & 7)
	        {
		    case 0: // setpix
		    // TODO
		    break;

		    case 1: // setpixu
		    // TODO
		    break;

		    case 2: // setpixv
		    // TODO
		    break;

		    case 3: // setpixz
		    // TODO
		    break;

		    case 4: // setpixr
		    // TODO
		    break;

		    case 5: // setpixg
		    // TODO
		    break;

		    case 6: // setpixb
		    // TODO
		    break;

		    case 7: // setpixa
		    // TODO
		    break;
		}
		break;

                case 0x0c0:
	        switch (value2 & 7)
	        {
		    case 0: // setmula
		    // TODO
		    break;

		    case 1: // setmulb
		    // TODO
		    break;

		    case 2: // setdiva
		    // TODO
		    break;

		    case 3: // setdivb
		    // TODO
		    break;

		    case 4: // setsqrt
		    // TODO
		    break;

		    case 5: // setcorx
		    // TODO
		    break;

		    case 6: // setcory
		    // TODO
		    break;

		    case 7: // setcorz
		    // TODO
		    break;
		}
		break;

                case 0x0c8:
	        switch (value2 & 7)
	        {
		    case 0: // cordrot
		    // TODO
		    break;

		    case 1: // cordatn
		    // TODO
		    break;

		    case 2: // cordexp
		    // TODO
		    break;

		    case 3: // cordlog
		    // TODO
		    break;

		    case 4: // cfgdac0
		    // TODO
		    break;

		    case 5: // cfgdac1
		    // TODO
		    break;

		    case 6: // cfgdac2
		    // TODO
		    break;

		    case 7: // cfgdac3
		    // TODO
		    break;
		}
		break;

                case 0x0d0:
	        switch (value2 & 7)
	        {
		    case 0: // setdac0
		    // TODO
		    break;

		    case 1: // setdac1
		    // TODO
		    break;

		    case 2: // setdac2
		    // TODO
		    break;

		    case 3: // setdac3
		    // TODO
		    break;

		    case 4: // cfgdacs
		    // TODO
		    break;

		    case 5: // setdacs
		    // TODO
		    break;

		    case 6: // getp
		    // TODO
		    break;

		    case 7: // getpn
		    // TODO
		    break;
		}
		break;

                case 0x0d8:
	        switch (value2 & 7)
	        {
		    case 0: // offp
		    // TODO
		    break;

		    case 1: // notp
		    // TODO
		    break;

		    case 2: // clrp
		    // TODO
		    break;

		    case 3: // setp
		    // TODO
		    break;

		    case 4: // setpc
		    // TODO
		    break;

		    case 5: // setpnc
		    // TODO
		    break;

		    case 6: // setpz
		    // TODO
		    break;

		    case 7: // setpnz
		    // TODO
		    break;
		}
		break;

                case 0x0e0:
	        switch (value2 & 7)
	        {
		    case 0: // setcog
		    // TODO
		    break;

		    case 1: // setmap
		    // TODO
		    break;

		    case 2: // setquad
		    cflag = 0;
		    result = value1 & 0x1fc;
		    zflag = (result == 0);
		    pasmvars->cachecogaddr = result;
		    break;

		    case 3: // setport
		    // TODO
		    break;

		    case 4: // setpora
		    // TODO
		    break;

		    case 5: // setporb
		    // TODO
		    break;

		    case 6: // setporc
		    // TODO
		    break;

		    case 7: // setpord
		    // TODO
		    break;
		}
		break;

                case 0x0e8:
	        switch (value2 & 7)
	        {
		    case 0: // setxch
		    // TODO
		    break;

		    case 1: // setpnz
		    // TODO
		    break;

		    case 2: // setser
		    // TODO
		    break;

		    case 3: // setvid
		    // TODO
		    break;

		    case 4: // setvidm
		    // TODO
		    break;

		    case 5: // setvidy
		    // TODO
		    break;

		    case 6: // setvidi
		    // TODO
		    break;

		    case 7: // setvidq
		    // TODO
		    break;
		}
		break;

                case 0x0f0:
	        switch (value2 & 7)
	        {
		    case 0: // setctra
		    // TODO
		    break;

		    case 1: // setwava
		    // TODO
		    break;

		    case 2: // setfrqa
		    // TODO
		    break;

		    case 3: // setphsa
		    // TODO
		    break;

		    case 4: // addphsa
		    // TODO
		    break;

		    case 5: // subphsa
		    // TODO
		    break;

		    case 6: // synctra
		    // TODO
		    break;

		    case 7: // capctra
		    // TODO
		    break;
		}
		break;

                case 0x0f8:
	        switch (value2 & 7)
	        {
		    case 0: // setctrb
		    // TODO
		    break;

		    case 1: // setwavb
		    // TODO
		    break;

		    case 2: // setfrqb
		    // TODO
		    break;

		    case 3: // setphsb
		    // TODO
		    break;

		    case 4: // addphsb
		    // TODO
		    break;

		    case 5: // subphsb
		    // TODO
		    break;

		    case 6: // synctrb
		    // TODO
		    break;

		    case 7: // capctrb
		    // TODO
		    break;
		}
		break;

                default:
		if ((value2 & 0x100) == 0) // Unknown
		{
		    // TODO - Handle unknown instruction
		}
		value2 = (value2 & 0x1f);
	        switch ((value2 >> 5) & 7)
	        {
		    case 0: // isob
		    result = (value1 >> value2) & 1;
		    break;

		    case 1: // notb
		    result = value1 ^ (1 << value2);
		    break;

		    case 2: // clrb
		    result = value1 & (~(1 << value2));
		    break;

		    case 3: // setb
		    result = value1 | (1 << value2);
		    break;

		    case 4: // setbc
		    result = value1 & (~(1 << value2));
		    result |= cflag << value2;
		    break;

		    case 5: // setbnc
		    result = value1 & (~(1 << value2));
		    result |= (cflag ^ 1) << value2;
		    break;

		    case 6: // setbz
		    result = value1 & (~(1 << value2));
		    result |= zflag << value2;
		    break;

		    case 7: // setbnz
		    result = value1 & (~(1 << value2));
		    result |= (zflag ^ 1) << value2;
		    break;
		}
	    }
	    break;

	    case 4: // mac, mul
	    result = (value1 & 0xffff) * (value2 & 0xffff);
	    if (!(zcri & 2))
	    {
		int ones = ((pasmvars->accl >> 31) & 1) + ((result >> 31) & 1);
		pasmvars->accl += result;
		if (ones == 2 || (ones == 1 && !(pasmvars->accl & 0x80000000)))
		    pasmvars->acch++;
	    }
	    break;

	    case 5: // macs, muls
	    result = ((value1 << 16) >> 16) * ((value2 << 16) >> 16);
	    if (!(zcri & 2))
	    {
		int ones = ((pasmvars->accl >> 31) & 1) + ((result >> 31) & 1);
		pasmvars->accl += result;
		if (result & 0x80000000)
		    pasmvars->acch--;
		if (ones == 2 || (ones == 1 && !(pasmvars->accl & 0x80000000)))
		    pasmvars->acch++;
	    }
	    break;

	    case 6: // enc
	    //printf("enc %8.8x = ", value2);
	    for (result = 32; result > 0; result--)
	    {
		if (value2 & 0x80000000) break;
		value2 <<= 1;
	    }
	    //printf("%d\n", result);
	    break;

	    case 7: // ret, jmp, jmpret, call
	    cflag = ((uint32_t)value1) < ((uint32_t)value2);
	    result = (value1 & 0xfffffe00) | ((pc + 1) & 511);
	    pasmvars->pc = value2 & 0x1ff;
	    zflag = (result == 0);
	    // Invalidate the instruction pipeline
            pasmvars->pc1 |= 512;
            pasmvars->pc2 |= 512;
	    //printf("jmp %3.3x, value2 = %8.8x\n", pasmvars->pc, value2);
	    //printf("zcri = %x, srcaddr = %8.8x\n", zcri, srcaddr);
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

	    case 7: // retd, jmpd, calld, jmpretd
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

	// subr, cmpsub, incmod, decmod ijxx, djxx, tjxx, waitxxxx
	case 7:
	switch (opcode & 7)
	{
            case 0: // subr
	    result = value2 - value1;
	    cflag = ((uint32_t)value2) < ((uint32_t)value1);
	    break;

	    case 1: // cmpsub
	    cflag = (((uint32_t)value1) >= ((uint32_t)value2));
	    result = cflag ? value1 - value2 : value1;
	    zflag = (result == 0);
	    break;

            case 2: // incmod
	    cflag = (value1 == value2);
	    if (cflag)
	        result = 0;
	    else
	        result = value1 + 1;
	    break;

            case 3: // decmod
	    cflag = (value1 == 0);
	    if (cflag)
	        result = value2;
	    else
	        result = value1 - 1;
	    break;

            case 4: // ijz, ijzd, ijnz, ijnzd
	    result = value1 - 1;
	    zflag = (result == 0);
	    cflag = (result == -1);
	    // Determine if we should jump
	    if (zflag != (zcri >> 3))
	    {
	        pasmvars->pc = srcaddr;
	        // Invalidate the instruction pipeline if non-delayed jump
	        if (!(zcri & 4))
	        {
                    pasmvars->pc1 |= 512;
                    pasmvars->pc2 |= 512;
	        }
	    }
#ifdef IGNORE_WZ_WC
	    zcri &= 3; // Ignore Z and C write flags
#endif
	    break;

	    case 5: // djz, djzd, djnz, djnzd
	    result = value1 - 1;
	    zflag = (result == 0);
	    cflag = (result == -1);
	    // Determine if we should jump
	    if (zflag != (zcri >> 3))
	    {
	        pasmvars->pc = srcaddr;
	        // Invalidate the instruction pipeline if non-delayed jump
	        if (!(zcri & 4))
	        {
                    pasmvars->pc1 |= 512;
                    pasmvars->pc2 |= 512;
	        }
	    }
#ifdef IGNORE_WZ_WC
	    zcri &= 3; // Ignore Z and C write flags
#endif
	    break;

	    case 6: // tjz, tjzd, tjnz, tjnzd, setinda, setindb, cfgpins, waitvid
	    if (zcri & 2) // setinda, setindb, cfgpins, waitvid
	    {
		zcri &= 0xd; // Ignore R write flag
		if ((zcri >> 2) == 0)      // setinda
		{
		    pasmvars->indatop = value1 & 0x1ff;
		    pasmvars->indabot = value2 & 0x1ff;
		    pasmvars->inda = pasmvars->indabot;
		}
		else if ((zcri >> 2) == 1) // setindb
		{
		    pasmvars->indbtop = value1 & 0x1ff;
		    pasmvars->indbbot = value2 & 0x1ff;
		    pasmvars->indb = pasmvars->indbbot;
		}
		else if ((zcri >> 3) == 1) // cfgpins
		{
		    // TODO - implement cfgpins
		}
		else                       // waitvid
		{
		    // TODO - implement waitvid
		}
	    }
	    else    // tjz, tjzd, tjnz, tjnzd
	    {
	        result = value1;
	        zflag = (result == 0);
	        cflag = 0;
	        // Determine if we should jump
	        if (zflag != (zcri >> 3))
	        {
	            pasmvars->pc = srcaddr;
	            // Invalidate the instruction pipeline if non-delayed jump
	            if (!(zcri & 4))
	            {
                        pasmvars->pc1 |= 512;
                        pasmvars->pc2 |= 512;
	            }
	        }
	    }
#ifdef IGNORE_WZ_WC
	    zcri &= 3; // Ignore Z and C write flags
#endif
	    break;

	    case 7: // waitcnt, waitpeq, waitpne
	    if ((zcri & 8) == 0) // waitcnt
	    {
		// Implement waitcnt with a precise count instead of a range
	        //result = GetCnt() - value1;
	        //if (result < 0 || result > 20000000)
		//printf("waitcnt %d, %d\n", value1, GetCnt());
		if (value1 != GetCnt())
	        {
		    //pasmvars->state = 6;
		    pasmvars->waitflag = 1;
    		    //pasmvars->pc = (pasmvars->pc - 1) & 511;
		    return 0;
	        }
	        else
	        {
		    //pasmvars->state = 5;
		    pasmvars->waitflag = 0;
	            result = value1 + value2;
	            zflag = (result == 0);
	            cflag = (((value1 & value2) | ((value1 | value2) & (~result))) >> 31) & 1;
	        }
	    }
	    else // waitpeq, waitpne
	    {
		// TODO - implement waitpeq and waitpne
	    }
#ifdef IGNORE_WZ_WC
	    zcri &= 3; // Ignore Z and C write flags
#endif
	    break;
	}
	break;
    }

    // Conditionally update flags and write result
    //if ((zcri & 8) && pasmvars->zflag != zflag) printf("Changing zflag to %d\n", zflag);
    if (zcri & 8)
    {
	pasmvars->zflag = zflag;
        if (LONG(SYS_DEBUG) & (1 << (8 + pasmvars->cogid)))
	    printf(", z = %d", zflag);
    }
    if (zcri & 4)
    {
	pasmvars->cflag = cflag;
        if (LONG(SYS_DEBUG) & (1 << (8 + pasmvars->cogid)))
	    printf(", c = %d", cflag);
    }
    if (zcri & 2)
    {
	//if (dstaddr == 0x1f4) printf("outa = %8.8x\n", result);
	if ((dstaddr & 0x1fc) == pasmvars->cachecogaddr)
	{
	    //printf("cache[%d] = %8.8x\n", dstaddr & 3, result);
	    pasmvars->cache[dstaddr & 3] = result;
            if (LONG(SYS_DEBUG) & (1 << (8 + pasmvars->cogid)))
	        printf(", cache[%d] = %x", dstaddr & 3, result);
	}
	else
	{
	    pasmvars->mem[dstaddr] = result;
            if (LONG(SYS_DEBUG) & (1 << (8 + pasmvars->cogid)))
	        printf(", cram[%x] = %x", dstaddr, result);
	}
	// Check if we need to update the pins
	//if (dstaddr == 0x1f4 || dstaddr == 0x1f6) UpdatePins2();
	if (dstaddr == REG_OUTA || dstaddr == REG_DIRA) UpdatePins2();
    }
    //CheckSerialOut(pasmvars);
    if (pasmvars->waitflag)
    {
	printf("XXXXXXXXXX BAD XXXXXXXXXXXXXXX\n");
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
