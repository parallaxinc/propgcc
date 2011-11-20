/**
 * #########################################################
 * @file propeller.c
 * This is a propeller library file. It is a work in progress.
 * 
 * Copyright (c) 2011 Steve Denson
 * MIT Licensed
 * #########################################################
*/

#include "propeller.h"

/**
 * this function lets us start a new PASM cog on Propeller.
 * @param code - PASM code address
 * @param par  - parameter address
 * @returns COG ID allocated by coginit
 */
int cognew(void *code_p, void *par_p)
{
    unsigned int value = 8;
    int cog = 0;
    unsigned code = (unsigned)(code_p);
    unsigned par = (unsigned)(par_p);

    par &= ~3;
    code &= ~3;
    value |= (code << 2);
    value |= (par << 16);
    cog = coginit(value);
    return cog;
}


