/*
############################################################################
# Written by Dave Hein
# Copyright (c) 2012 Parallax, Inc.
# MIT Licensed
############################################################################
*/
#include <stdio.h>
#include <string.h>
#include "draw.h"

#define ncol 640
#define nrow 480

extern unsigned char *tilemap;
extern int tiles[16*256];
volatile int lastindex = 0;
volatile int background = 0;
volatile int lockindex = 0;

void inittiles(int val)
{
    lastindex = 0;
    lockindex = 0;
    memset(tilemap, 0, 40*30);
    memset(tiles, val, 16*4);
    background = val;
}

inline int getindex(uint8_t *ptr)
{
    int index = *ptr;
    int index1 = index;
   
    if (index > lockindex)
        return index; 

    if (lastindex == 255)
    {
        return 0;
    }
    index = ++lastindex;
    memcpy(&tiles[index << 4], &tiles[index1 << 4], 64);
    *ptr = index;

    return index;
}

void drawobject(int col, int row, unsigned int *ptr, int width, int height)
{
    int mask;
    int i, j;
    int tilerow, tilecol;
    int lastrow, row15, shift;
    int numtilerow, numtilecol;
    int index1, index2, index3;
    unsigned int val1, val2, val3;
    uchar map[3][3] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

    numtilecol = (width + (col&15) + 15) >> 4;
    numtilerow = (height + (row&15) + 15) >> 4;

    // Get the tile indices
    for (i = 0, tilerow = (row >> 4); i < numtilerow; i++, tilerow++)
    {
        if (tilerow < 0 || tilerow >= 30) continue;
        for (j = 0, tilecol = (col >> 4); j < numtilecol; j++, tilecol++)
        {
            if (tilecol < 0 || tilecol >= 40) continue;
            map[i][j] = getindex(&tilemap[tilecol + tilerow * 40]);
        }
    }

#if 0
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++) printf("%4d", map[i][j]);
        printf("\n");
    }
#endif

    lastrow = row + height;
    if (lastrow > nrow) lastrow = nrow;

    // Initialize the shift and index values
    shift = (col & 15) << 1;
    index1 = map[0][0] << 4;
    index2 = map[0][1] << 4;
    index3 = map[0][2] << 4;

    // Copy the object
    for (i = 0; row < lastrow; row++)
    {
        // Get the object values
        val1 = *ptr++;
        if (width > 16)
            val2 = *ptr++;
        else
            val2 = 0;
        val3 = 0;

        // Shift the values
        val3 = val2 >> (30 - shift);
        val2 = (val1 >> (30 - shift)) | (val2 << shift);
        val1 <<= shift;

        // Update tile rows
        row15 = row & 15;
        if (index1)
        {
            mask = (val1 | (val1 >> 1)) & 0x55555555;
            mask = ~(mask | (mask << 1));
            tiles[index1 + row15] = (tiles[index1 + row15] & mask) | val1;
        }
        if (index2)
        {
            mask = (val2 | (val2 >> 1)) & 0x55555555;
            mask = ~(mask | (mask << 1));
            tiles[index2 + row15] = (tiles[index2 + row15] & mask) | val2;
        }
        if (index3)
        {
            mask = (val3 | (val3 >> 1)) & 0x55555555;
            mask = ~(mask | (mask << 1));
            tiles[index3 + row15] = (tiles[index3 + row15] & mask) | val3;
        }

        // Check for next row of tiles
        if (row15 == 15)
        {
            i++;
            index1 = map[i][0] << 4;
            index2 = map[i][1] << 4;
            index3 = map[i][2] << 4;
        }
    }
}

inline void drawdotfast(int col, int row, int color)
{
    int tilenum, index, shift;

    tilenum = (col >> 4) + (row >> 4) * (ncol >> 4);
    if (!(index = getindex(&tilemap[tilenum]))) return;
    index <<= 4;
    index += row & 15;
    shift = (col & 15) << 1;
    tiles[index] = (tiles[index] & ~(3 << shift)) | (color << shift);
}

inline void drawdot(int col, int row, int color)
{
    int tilenum, index, shift;

    if (col < 0 || col >= ncol || row < 0 || row >= nrow) return;

    if (color < 0 || color >= COLOR_MAX) return;

    color &= 3;

    tilenum = (col >> 4) + (row >> 4) * (ncol >> 4);
    if (!(index = getindex(&tilemap[tilenum]))) return;
    index *= 16;
    index += row & 15;
    shift = (col & 15) << 1;
    tiles[index] = (tiles[index] & ~(3 << shift)) | (color << shift);
}

void drawhorzline(int col0, int col1, int row, int color)
{
    int temp;

    if (row < 0 || row >= nrow) return;

    if (col0 > col1)
    {
	temp = col0;
	col0 = col1;
	col1 = temp;
    }

    if (col1 < 0) return;
    if (col0 >= ncol) return;
    if (col1 >= ncol) col1 = ncol - 1;
    if (col0 < 0) col0 = 0;

    for (;col0 <= col1; col0++)
    {
	drawdotfast(col0, row, color);
    }
}

void drawvertline(int col, int row0, int row1, int color)
{
    int temp;

    if (col < 0 || col >= ncol) return;

    if (row0 > row1)
    {
	temp = row0;
	row0 = row1;
	row1 = temp;
    }

    if (row1 < 0) return;
    if (row0 >= nrow) return;
    if (row1 >= nrow) row1 = nrow - 1;
    if (row0 < 0) row0 = 0;

    for (;row0 <= row1; row0++)
    {
	drawdotfast(col, row0, color);
    }
}

void drawline(int col0, int row0, int col1, int row1, int color)
{
    int col, row, incr, acc;
    int temp, cdiff, rdiff, cdiffa, rdiffa;

    cdiffa = cdiff = col1 - col0;
    rdiffa = rdiff = row1 - row0;

    if (cdiff == 0 && rdiff == 0)
    {
        drawdot(col0, row0, color);
        return;
    }
    else if (cdiff == 0)
    {
        drawvertline(col0, row0, row1, color);
        return;
    }
    else if (rdiff == 0)
    {
        drawhorzline(col0, col1, row1, color);
        return;
    }

    if (cdiffa < 0) cdiffa = -cdiffa;
    if (rdiffa < 0) rdiffa = -rdiffa;

    if (((cdiffa > rdiffa) && (cdiff < 0)) || ((rdiffa >= cdiffa) && (rdiff < 0)))
    {
        temp = col0;
        col0 = col1;
        col1 = temp;
        temp = row0;
        row0 = row1;
        row1 = temp;
        cdiff = -cdiff;
        rdiff = -rdiff;
    }

    acc = 0;
    if (cdiffa > rdiffa)
    {
        incr = (rdiff << 16) / cdiff;
        for (col = col0; col <= col1; col++)
        {
            row = row0 + (acc >> 16);
            acc += incr;
            drawdot(col, row, color);
        }
    }
    else
    {
        incr = (cdiff << 16) / rdiff;
        for (row = row0; row <= row1; row++)
        {
            col = col0 + (acc >> 16);
            acc += incr;
            drawdot(col, row, color);
        }
    }
}
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

