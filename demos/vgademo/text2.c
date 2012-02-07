/*
############################################################################
# Written by Dave Hein
# Copyright (c) 2012 Parallax, Inc.
# MIT Licensed
############################################################################
*/
#include "draw.h"
#include "text2.h"

void DrawHorzChar2(int col, int row, uint *ptr2, int scale, int color, int odd)
{
    int i, j, k, l;
    int pixel, temp;

    for (j = 0; j < CHAR_HEIGHT2; j++)
    {
	for (l = 0; l < scale; l++)
	{
            temp = *ptr2;
            temp >>= odd;
	    for (i = 0; i < CHAR_WIDTH2; i++)
	    {
                pixel = temp & 1;
                temp >>= 2;
	        for (k = 0; k < scale; k++)
	        {
	            if(pixel)
		    {
                        drawdot(col, row, color);
		    }
                    col++;
	        }
	    }
            row++;
            col -= CHAR_WIDTH2 * scale;
	}
	ptr2++;
    }
}

void DrawVertChar2(int col, int row, uint *ptr2, int scale, int color, int odd)
{
    int i, j, k, l;
    int pixel, temp;

    for (j = 0; j < CHAR_HEIGHT2; j++)
    {
	for (l = 0; l < scale; l++)
	{
            temp = *ptr2;
            temp >>= odd;
	    for (i = 0; i < CHAR_WIDTH2; i++)
	    {
                pixel = temp & 1;
                temp >>= 2;
	        for (k = 0; k < scale; k++)
	        {
	            if(pixel)
		    {
                        drawdot(col, row, color);
		    }
                    row--;
	        }
	    }
            col++;
            row += CHAR_WIDTH2 * scale;
	}
	ptr2++;
    }
}

void PutCharImage2(int col, int row, int value, int scale, int vert, int color)
{
    uchar *ptr2 = (uchar *)0x8000 + ((value >> 1) * 128);

    if (vert == 0)
        DrawHorzChar2(col, row, (uint *)ptr2, scale, color, value&1);
    else
        DrawVertChar2(col, row, (uint *)ptr2, scale, color, value&1);
}

void PutStringImage2(int col, int row, char *str, int scale, int vert, int color)
{
    while (*str)
    {
	PutCharImage2(col, row, str[0], scale, vert, color);
	str++;
	if (vert == 0)
	    col += CHAR_WIDTH2 * scale;
	else
	    row -= CHAR_WIDTH2 * scale;
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

