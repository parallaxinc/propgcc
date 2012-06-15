/**
 * @file toggle.c
 * This program demonstrates starting COG code with C in it
 * from C. The cog makes all IO except 30/31 toggle.
 *
 * Copyright (c) 2011 Parallax, Inc.
 * MIT Licensed (see at end of file for exact terms)
 */

#include <stdio.h>  // using temporary stdio
#include <propeller.h>
#include "toggle.h"
#include "i2c.h"
#include "cogload.h"

/*
 * This is the structure which we'll pass to the C cog.
 * It contains a small stack for working area, and the
 * mailbox which we use to communicate. See toggle.h
 * for the definition/
 */
struct par {
  unsigned stack[STACK_SIZE];
  struct toggle_mailbox m;
};

#define usefw(fw)           extern unsigned char _load_start_ ## fw ## _ecog[];        			\
                            extern unsigned char _load_stop_ ## fw ## _ecog[]

#define startcog_eeprom(fw, arg)	cognewFromBootEeprom(                                       	\
                                	  _load_start_ ## fw ## _ecog,                              	\
                                	  _load_stop_ ## fw ## _ecog - _load_start_ ## fw ## _ecog,	\
                                	  arg)

usefw(toggle_fw_0);
usefw(toggle_fw_1);
usefw(toggle_fw_2);
usefw(toggle_fw_3);
usefw(toggle_fw_4);
usefw(toggle_fw_5);
usefw(toggle_fw_6);
usefw(toggle_fw_7);

struct par par_0;
struct par par_1;
struct par par_2;
struct par par_3;
struct par par_4;
struct par par_5;
struct par par_6;
struct par par_7;

/*
 * main code
 * This is the code running in the LMM cog (cog 0).
 * It launches 7 more cogs to actually run the 
 * toggling code and then overwrites itself
 * with an 8th toggle program.
 */

int _cfg_basepin = -1;

void main (int argc,  char* argv[])
{
    uint32_t cogimage[496];
    void *buf;
    uint32_t off;
    size_t size;

    /* default the base pin to 0 to indicate LEDs on P0-6 */
    if (_cfg_basepin == -1)
    	_cfg_basepin = 0;
    	    
    /* set up the parameters for the C cogs */
    par_0.m.wait_time = _clkfreq;     /* start by waiting for 1 second */
    par_0.m.basepin = _cfg_basepin;
    par_0.m.token = 0;
    par_0.m.next = &par_1.m;
    
    par_1.m.wait_time = _clkfreq>>2;  /* start by waiting for 1/4 second */
    par_1.m.basepin = _cfg_basepin;
    par_1.m.token = 0;
    par_1.m.next = &par_2.m;
    
    par_2.m.wait_time = _clkfreq>>4;  /* start by waiting for 1/16 second */
    par_2.m.basepin = _cfg_basepin;
    par_2.m.token = 0;
    par_2.m.next = &par_3.m;
    
    par_3.m.wait_time = _clkfreq>>6;  /* start by waiting for 1/64 second */
    par_3.m.basepin = _cfg_basepin;
    par_3.m.token = 0;
    par_3.m.next = &par_4.m;
    
    par_4.m.wait_time = _clkfreq>>5;  /* start by waiting for 1/32 second */
    par_4.m.basepin = _cfg_basepin;
    par_4.m.token = 0;
    par_4.m.next = &par_5.m;
    
    par_5.m.wait_time = _clkfreq>>3;  /* start by waiting for 1/8 second */
    par_5.m.basepin = _cfg_basepin;
    par_5.m.token = 0;
    par_5.m.next = &par_6.m;
    
    par_6.m.wait_time = _clkfreq>>1;  /* start by waiting for 1/2 second */
    par_6.m.basepin = _cfg_basepin;
    par_6.m.token = 0;
    par_6.m.next = &par_7.m;
    
    par_7.m.wait_time = _clkfreq>>1;  /* start by waiting for 1/2 second */
    par_7.m.basepin = _cfg_basepin;
    par_7.m.token = 1;
    par_7.m.next = &par_0.m;
    
    /* start the new cogs */
    startcog_eeprom(toggle_fw_0, &par_0.m);
    startcog_eeprom(toggle_fw_1, &par_1.m);
    startcog_eeprom(toggle_fw_2, &par_2.m);
    startcog_eeprom(toggle_fw_3, &par_3.m);
    startcog_eeprom(toggle_fw_4, &par_4.m);
    startcog_eeprom(toggle_fw_5, &par_5.m);

    buf = i2cBootBuffer();
    off = COG_IMAGE_EEPROM_OFFSET(_load_start_toggle_fw_6_ecog);
    size = _load_stop_toggle_fw_6_ecog - _load_start_toggle_fw_6_ecog;
    readFromBootEeprom(off, buf, size);

    off = COG_IMAGE_EEPROM_OFFSET(_load_start_toggle_fw_7_ecog);
    size = _load_stop_toggle_fw_7_ecog - _load_start_toggle_fw_7_ecog;
    readFromBootEeprom(off, cogimage, size);

    i2cClose(i2cBootOpen()); // close the boot i2c driver to free up a cog
    
    cognew(buf, &par_6.m);
    coginit(cogid(), cogimage, &par_7.m);
}

/* +--------------------------------------------------------------------
 * ¦  TERMS OF USE: MIT License
 * +--------------------------------------------------------------------
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * +--------------------------------------------------------------------
 */
