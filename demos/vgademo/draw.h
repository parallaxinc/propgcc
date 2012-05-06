/*
############################################################################
# Written by Dave Hein
# Copyright (c) 2012 Parallax, Inc.
# MIT Licensed
############################################################################
*/
#ifndef DRAW_DEFINED
#define DRAW_DEFINED
#include <stdint.h>

#define uchar unsigned char

#define COLOR_BLUE    0
#define COLOR_BLACK   1
#define COLOR_CYAN    2
#define COLOR_WHITE   3
#define COLOR_RED     4
#define COLOR_GREEN   5
#define COLOR_YELLOW  6
#define COLOR_MAGENTA 7
#define COLOR_MAX     8

static unsigned char color_table[COLOR_MAX][3] = {
    255,   0,   0,
      0,   0,   0,
    255, 255,   0,
    255, 255, 255,
      0,   0, 255,
      0, 255,   0,
      0, 255, 255,
    255,   0, 255};

void drawdot(int col, int row, int color);

void drawhorzline(int col0, int col1, int row, int color);

void drawvertline(int col, int row0, int row1, int color);

void drawline(int col0, int row0, int col1, int row1, int color);

void inittiles(int val);

void drawobject(int col, int row, unsigned int *ptr, int width, int height);
#endif
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

