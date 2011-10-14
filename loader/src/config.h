/* config.h - an elf and spin binary loader for the Parallax Propeller microcontroller

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

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdio.h>
#include "system.h"

/* support for windows */
#if defined(WIN32)
#define NEED_STRCASECMP
#endif

/* internal memory map */
#define HUB_BASE        0x00000000
#define HUB_SIZE        (32 * 1024)

/* external memory map */
#define EXTERNAL_BASE   0x20000000
#define RAM_BASE        0x20000000
#define FLASH_BASE      0x30000000

#ifdef NO_STDINT
typedef long int32_t;
typedef unsigned long uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
#else
#include <stdint.h>
#endif

#define RCFAST      0x00
#define RCSLOW      0x01
#define XINPUT      0x22
#define XTAL1       0x2a
#define XTAL2       0x32
#define XTAL3       0x3a
#define PLL1X       0x41
#define PLL2X       0x42
#define PLL4X       0x43
#define PLL8X       0x44
#define PLL16X      0x45

/* default board configuration */
#define DEF_NAME            "default"
#define DEF_CLKFREQ         80000000
#define DEF_CLKMODE         XTAL1+PLL16X
#define DEF_BAUDRATE        115200
#define DEF_RXPIN           31
#define DEF_TXPIN           30
#define DEF_TVPIN           12

typedef struct BoardConfig BoardConfig;
struct BoardConfig {
    uint32_t clkfreq;
    uint8_t clkmode;
    uint32_t baudrate;
    uint8_t rxpin;
    uint8_t txpin;
    uint8_t tvpin;
    char *cacheDriver;
    uint32_t cacheSize;
    uint32_t cacheParam1;
    uint32_t cacheParam2;
    BoardConfig *next;
    char name[1];
};

void ParseConfigurationFile(System *sys, const char *path);
void SetConfigField(BoardConfig *config, char *tag, char *value);
BoardConfig *GetBoardConfig(const char *name);
#ifdef NEED_STRCASECMP
int strcasecmp(const char *s1, const char *s2);
#endif

#endif
