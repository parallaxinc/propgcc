/*
# #########################################################
# This file demonstrates starting and running a PASM TV
# driver that interacts with the C stdio library.
#   
# Written by Eric R. Smith
# Copyright (c) 2011 Parallax, Inc.
# MIT Licensed
#
# Modified by Steve Denson to use _cfg_ variable patching.
#
# #########################################################
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/driver.h>
#include <compiler.h>
#include <errno.h>
#include "TvText.h"
#include "keyboard.h"

/* get tv and keybd pins from .cfg file */
int _cfg_tvpin = -1;
int _cfg_kbclk_pin = -1;
int _cfg_kbdat_pin = -1;

static int users = 0;

const char TvPrefix[] = "TV:";


/*
 * hook called by fopen while setting up the file descriptor fp
 * "str" is the file name (excluding driver prefix) passed to fopen
 * we could use that to control various parameters, like the pin
 * used
 */
int
Tv_fopen(FILE *fp, const char *str, const char *mode)
{
    if (users == 0) {
        /* need to start the cog */
        if (tvText_start(_cfg_tvpin) == 0) {
            errno = EACCES; /* indicate failure to run */
            return -1;
        }
        if(keybd_start(_cfg_kbdat_pin,_cfg_kbclk_pin) == 0) {
            errno = EACCES;
            return -1;
        }
    }
    /* keep track of how many open file handles use this cog */
    ++users;
    return 0;
}

/*
 * close a TV file
 * when there are no more TV files open, shut down the associated cog
 */
int
Tv_fclose(FILE *fp)
{
    if (users > 0) {
        --users;
        if (users == 0) {
            tvText_stop();
            keybd_stop();
        }
    }
    return 0;
}

int
Tv_read(FILE *fp, unsigned char *buf, int count)
{
    char c = (char) keybd_getkey();
    if(c == '\r') c = '\n';
    buf[0] = c;
    return 1;
}

int
Tv_write(FILE *fp, unsigned char *buf, int count)
{
    int i;
    int c;
    for (i = 0; i < count; i++) {
          c = buf[i];
          tvText_outchar(c);
    }
    return count;
}

_Driver TvDriver =
{
    TvPrefix,
    Tv_fopen,
    Tv_fclose,
    Tv_read,
    Tv_write,
    NULL,  /* seek; not applicable */
    NULL,  /* remove; not applicable */
};

/*
+--------------------------------------------------------------------
|  TERMS OF USE: MIT License
+--------------------------------------------------------------------
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
+------------------------------------------------------------------
*/
