/* port.c - serial port utility functions

Copyright (c) 2012 David Michael Betz

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "port.h"
#include "PLoadLib.h"
#include "osint.h"

typedef struct {
    int baud;
    int verbose;
    char *actualport;
    int noreset;
} CheckPortInfo;

static int CheckPort(const char *port, void *data)
{
    CheckPortInfo* info = (CheckPortInfo *)data;
    int rc;
    if (info->verbose) {
        printf("Trying %s                    \r", port);
        fflush(stdout);
    }
    if ((rc = popenport(port, info->baud, info->noreset)) != PLOAD_STATUS_OK)
        return rc;
    if (info->actualport) {
        strncpy(info->actualport, port, PATH_MAX - 1);
        info->actualport[PATH_MAX - 1] = '\0';
    }
    return 0;
}

static int ShowPort(const char *port, void *data)
{
    if (data)
        CheckPort(port, data);
    else
        printf("%s\n", port);
    return 1;
}

void ShowPorts(char *prefix)
{
    serial_find(prefix, ShowPort, NULL);
}

void ShowConnectedPorts(char *prefix, int baud, int flags)
{
    CheckPortInfo info;
    int noreset = PLOAD_RESET_DEVICE;
    int verbose = 0;

    if (flags & IFLAG_VERBOSE)
      verbose = 1;
    if (flags & IFLAG_NORESET)
      noreset = PLOAD_NORESET;

    info.baud = baud;
    info.verbose = verbose;
    info.actualport = NULL;
    info.noreset = noreset;
    
    serial_find(prefix, ShowPort, &info);
}

int InitPort(char *prefix, char *port, int baud, int flags, char *actualport)
{
    int rc;
    int noreset = PLOAD_RESET_DEVICE;
    int verbose = 0;

    if (flags & IFLAG_VERBOSE)
      verbose = 1;
    if (flags & IFLAG_NORESET)
      noreset = PLOAD_NORESET;

    if (port) {
        if (actualport) {
            strncpy(actualport, port, PATH_MAX - 1);
            actualport[PATH_MAX - 1] = '\0';
        }
        rc = popenport(port, baud, noreset);
    }
    else {
        CheckPortInfo info;
        info.baud = baud;
        info.verbose = verbose;
        info.actualport = actualport;
	    info.noreset = noreset;
        if (serial_find(prefix, CheckPort, &info) == 0)
            rc = PLOAD_STATUS_OK;
        else
            rc = PLOAD_STATUS_NO_PROPELLER;
    }
        
    return rc;
}
