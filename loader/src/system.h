/* system.h - an elf and spin binary loader for the Parallax Propeller microcontroller

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

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <stdarg.h>

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

#if defined(WIN32)
#define PATH_SEP    ';'
#define DIR_SEP     '\\'
#define DIR_SEP_STR "\\"
#else
#define PATH_SEP    ':'
#define DIR_SEP     '/'
#define DIR_SEP_STR "/"
#endif

/* forward typedefs */
typedef struct System System;

/* system operations table */
typedef struct {
    void (*info)(System *sys, const char *fmt, va_list ap);
    void (*error)(System *sys, const char *fmt, va_list ap);
} SystemOps;

/* system interface */
struct System {
    SystemOps   *ops;
};

#define xbInfoV(sys, fmt, args)     ((*(sys)->ops->info)((sys), (fmt), (args)))
#define xbErrorV(sys, fmt, args)    ((*(sys)->ops->error)((sys), (fmt), (args)))

void xbInfo(System *sys, const char *fmt, ...);
void xbError(System *sys, const char *fmt, ...);
int xbAddPath(const char *path);
int xbAddFilePath(const char *name);
int xbAddEnvironmentPath(char *name);
int xbAddProgramPath(char *argv[]);
void *xbOpenFileInPath(System *sys, const char *name, const char *mode);
void *xbOpenFile(System *sys, const char *name, const char *mode);
int xbCloseFile(void *file);
char *xbGetLine(void *file, char *buf, int size);
size_t xbReadFile(void *file, void *buf, size_t size);
size_t xbWriteFile(void *file, const void *buf, size_t size);
int xbSeekFile(void *file, long offset, int whence);
void *xbCreateTmpFile(System *sys, const char *name, const char *mode);
int xbRemoveTmpFile(System *sys, const char *name);
void *xbGlobalAlloc(System *sys, size_t size);
void *xbLocalAlloc(System *sys, size_t size);
void xbLocalFreeAll(System *sys);

#endif
