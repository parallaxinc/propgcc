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

typedef struct BoardConfig BoardConfig;

#define DEF_BOARD   "default"
#define DEF_SUBTYPE "default"

BoardConfig *NewBoardConfig(BoardConfig *parent, const char *name);
BoardConfig *ParseConfigurationFile(System *sys, const char *path);
void DumpBoardConfiguration(BoardConfig *config);
BoardConfig *GetConfigSubtype(BoardConfig *config, const char *name);
BoardConfig *MergeConfigs(BoardConfig *parent, BoardConfig *child);
void SetConfigField(BoardConfig *config, const char *tag, const char *value);
char *GetConfigField(BoardConfig *config, const char *tag);
int GetNumericConfigField(BoardConfig *config, const char *tag, int *pValue);

#endif
