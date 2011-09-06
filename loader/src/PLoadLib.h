/**
 * @file PLoadLib.h
 *
 * Downloads an image to Propeller using Windows32 API.
 * Ya, it's been done before, but some programs like Propellent and
 * PropTool do not recognize virtual serial ports. Hence, this program.
 *
 * Copyright (c) 2009 by John Steven Denson
 * Modified in 2011 by David Michael Betz
 *
 * MIT License                                                           
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 */
#ifndef __PLOADLIB_H__
#define __PLOADLIB_H__

#include <stdint.h>

#define SHUTDOWN_CMD            0
#define DOWNLOAD_RUN_BINARY     1
#define DOWNLOAD_EEPROM         2
#define DOWNLOAD_RUN_EEPROM     3

int pload(char* file, char* port, int type);
int ploadfp(char* file, FILE *fp, char* port, int type);
int ploadbuf(uint8_t* dlbuf, int count, char* port, int type);

#endif
