/* loader.c - an elf and spin binary loader for the Parallax Propeller microcontroller

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

#ifndef __LOADER_H__
#define __LOADER_H__

#include "config.h"
#include "system.h"

/* InitPort flags */
#define IFLAG_VERBOSE               (1 << 0)

/* LoadXXX flags */
#define LFLAG_WRITE_EEPROM          (1 << 0)
#define LFLAG_RUN                   (1 << 1)
#define LFLAG_WRITE_BINARY          (1 << 2)
#define LFLAG_WRITE_PEX             (1 << 3)
#define LFLAG_WRITE_SDLOADER        (1 << 4)
#define LFLAG_WRITE_SDCACHELOADER   (1 << 5)

int InitPort(char *prefix, char *port, int baud, int flags, char *actualport);
int LoadImage(System *sys, BoardConfig *config, char *path, int flags);
int LoadSDLoader(System *sys, BoardConfig *config, char *path, int flags);
int LoadSDCacheLoader(System *sys, BoardConfig *config, char *path, int flags);

#endif
