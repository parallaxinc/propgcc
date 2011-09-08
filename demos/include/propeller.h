/**
 * @file propeller.h
 * This is part of the propeller library. ... this is a work in progress.
 *
 * Copyright (c) 2011, Steve Denson
 * MIT Licensed
 */

#ifndef _PROPELLER_H_
#define _PROPELLER_H_

#include "cog.h"

/* some defines for special purpose COG registers */
#define PAR     _PAR
#define CNT     _CNT
#define INA     _INA
#define INB     _INB
#define OUTA    _OUTA
#define OUTB    _OUTB
#define DIRA    _DIRA
#define DIRB    _DIRB
#define CTRA    _CTRA
#define CTRB    _CTRB
#define FRQA    _FRQA
#define FRQB    _FRQB
#define PHSA    _PHSA
#define PHSB    _PHSB
#define VCFG    _VCFG
#define VSCL    _VSCL

/* clock frequency define */
#define CLKFREQ _CLKFREQ

/*
 * wait until system counter reaches a value
 * @param a - target value
 */
#define waitcnt(a) __builtin_waitcnt((a),_CNT)

/*
 * start a cog with a parameter
 * the fields in parameter are:
 * 31:18   = 14-bit Long address for PAR Register
 * 17:4    = 14-bit Long address of code to load
 * 3       = New bit
 * 2:0     = Cog ID if New bit is 0
 * @param a - parameter value
 */
#define coginit(a) __builtin_coginit((a))

/*
 * stop a cog
 * @param a - cog value
 */
#define cogstop(a) __builtin_cogstop((a))

/*
 * start a new propeller cog 
 * @param code - address of PASM to load
 * @param par  - value of par parameter usually an address
 * @returns COG ID provided by __builtin_coginit(...)
 */
int cognew(unsigned int code, unsigned int par);

#endif
// _PROPELLER_H_
