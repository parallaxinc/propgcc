/**
 * @file tv_text.c
 * TV_Text native device driver interface.
 *
 * Copyright (c) 2008-2011, Steve Denson
 * See end of file for terms of use.
 */
// Some headers are not available in the tool chain yet
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for memcpy
#include <propeller.h>

#include "TvText.h"

// dprintf is defined only for debug output
#define dprintf
//#define dprintf printf

#define TV_TEXT_OUT

/**
 * tvText mailbox and any data shared with PASM must be in hub memory.
 */
HUBDATA static TvText_t tvText;
HUBDATA static uint32_t colorTable[TV_TEXT_COLORTABLE_SIZE];
HUBDATA static uint16_t screen[TV_TEXT_SCREENSIZE];

/**
 * This is the main global tv text control/status structure.
 */
HUBDATA volatile TvText_t *tvPtr;

/*
 * In the case of __PROPELLER_XMM__ we must copy the PASM to
 * a temporary HUB buffer for cog start. Define buffer here.
 */
#if defined(__PROPELLER_XMM__)
#define PASMLEN 496
HUBDATA static uint32_t pasm[PASMLEN];
#endif

/**
 * These are variables to keep up with display;
 */
static int col, row, flag;

static uint16_t blank = 0x220;

/**
 * This is the TV palette.
 */
static char gpalette[TV_TEXT_COLORTABLE_SIZE] =     
{// fgRGB bgRGB
    0x07, 0x0a,     // 0    white / dark blue
    0x07, 0xbb,     // 1   yellow / brown
    0x9e, 0x9b,     // 2  magenta / black
    0x04, 0x07,     // 3     grey / white
    0x3d, 0x3b,     // 4     cyan / dark cyan
    0x6b, 0x6e,     // 5    green / gray-green
    0xbb, 0xce,     // 6      red / pink
    0x3e, 0x0a      // 7     cyan / blue
};

/*
 * This should set the character foreground and screen background.
 * API are available to get/set this.
 */
static int color = 0;

/*
 * global var to keep cogid so we don't have to pass parm to stop
 */
static int gTvTextCog = 0;

HUBTEXT static void wordfill(uint16_t *dst, uint16_t val, int len)
{
    while(--len > -1) {
        *dst = val;
        dst++;
    }
}

HUBTEXT static void wordmove(uint16_t *dst, uint16_t *src, int len)
{
    while(--len > -1) {
        *dst = *src;
        dst++;
        src++;
    }
}

/*
 * TV_Text start function starts TV on a cog
 * See header file for more details.
 */
int tvText_start(int basepin)
{
    extern uint32_t binary_TV_dat_start[];
    extern uint32_t binary_TV_dat_end[];
    int size = binary_TV_dat_end - binary_TV_dat_start;
    int n;

#if defined(__PROPELLER_XMM__)
    dprintf("__PROPELLER_XMM__\n");
    copy_from_xmm((uint32_t*)pasm,(uint32_t*)binary_TV_dat_start,PASMLEN);
    dprintf("pasm size %d\r\n", size);
    for(n = 1; n <= size; n++) {
        dprintf("%08x ", pasm[n-1]);
        if((n & 7) == 0)
            dprintf("\r\n");
    }
    dprintf("\r\npasm %08x\r\n", pasm);
#else
    dprintf("binary_TV_dat size %d\r\n", size);
    for(n = 1; n <= size; n++) {
        dprintf("%08x ", binary_TV_dat_start[n-1]);
        if((n & 7) == 0)
            dprintf("\r\n");
    }
    dprintf("\r\nbinary_TV_dat_start %08x\r\n", binary_TV_dat_start);
#endif

    col = 0;
    row = 0;
    flag = 0;

    tvPtr = &tvText;
    tvPtr->status = 0;
    tvPtr->enable = 1;
    tvPtr->pins   = ((basepin & 0x38) << 1) | (((basepin & 4) == 4) ? 0x5 : 0);
    tvPtr->mode   = 0x12;
    tvPtr->colors = colorTable;
    tvPtr->screen = screen;
    tvPtr->ht = TV_TEXT_COLS;
    tvPtr->vt = TV_TEXT_ROWS;
    tvPtr->hx = 4;
    tvPtr->vx = 1;
    tvPtr->ho = 0;
    tvPtr->vo = -2; 
    tvPtr->broadcast = 0;
    tvPtr->auralcog  = 0;

    dprintf("%08x\n", tvPtr);
    dprintf("%08x\n", tvPtr->status );
    dprintf("%08x\n", tvPtr->enable );
    dprintf("%08x\n", tvPtr->pins   );
    dprintf("%08x\n", tvPtr->mode   );
    dprintf("%08x\n", tvPtr->colors );
    dprintf("%08x\n", tvPtr->screen );
    dprintf("%08x\n", tvPtr->ht );
    dprintf("%08x\n", tvPtr->vt );
    dprintf("%08x\n", tvPtr->hx );
    dprintf("%08x\n", tvPtr->vx );
    dprintf("%08x\n", tvPtr->ho );
    dprintf("%08x\n", tvPtr->vo );
    dprintf("%08x\n", tvPtr->broadcast );
    dprintf("%08x\n\n", tvPtr->auralcog  );

    // set main fg/bg color
    tvText_setColorPalette(&gpalette[TV_TEXT_PAL_WHITE_BLUE]);

    // blank the screen
    wordfill(tvPtr->screen, blank, TV_TEXT_SCREENSIZE);

    for(n = 1; 0 && n <= TV_TEXT_SCREENSIZE; n++) {
        dprintf("%04x ", screen[n-1]);
        if((n & 15) == 0)
            dprintf("\r\n");
    }

    // start new cog from external memory using pasm and tvPtr
#if defined(__PROPELLER_XMM__)
    gTvTextCog = cognew(pasm, (uint32_t)tvPtr) + 1;
#else
    gTvTextCog = cognew((uint32_t)binary_TV_dat_start, (uint32_t)tvPtr) + 1;
#endif
    waitcnt(CLKFREQ+CNT); // 100us is all we need really

    // clear screen
    tvText_out(0);

    dprintf("tvText_start() returns\n");

    return gTvTextCog;
}

/**
 * stop stops the cog running the native assembly driver 
 */
void tvText_stop(void)
{
    int id = gTvTextCog - 1;
    if(gTvTextCog > 0) {
        cogstop(id);
    }
}
/*
 * TV_Text setcolors function sets the palette to that defined by pointer.
 * See header file for more details.
 */
void tvText_setColorPalette(char* ptr)
{
    int  ii = 0;
    uint8_t  fg = 0;
    uint8_t  bg = 0;

    uint32_t* colors = tvPtr->colors;

    dprintf("tvText_setColorPalette()\r\n");

    for(ii = 0; ii < TV_TEXT_COLORTABLE_SIZE; ii += 2)
    {
        fg = (uint8_t) ptr[ii];
        bg = (uint8_t) ptr[ii+1];
        colors[ii]     = fg << 24 | bg << 16 | fg << 8 | bg;
        colors[ii+1]   = fg << 24 | fg << 16 | bg << 8 | bg;
        dprintf("%08x %08x\n",colors[ii],colors[ii+1]);
   }        
}

/*
 * print a new line
 */
static void newline(void)
{
    uint16_t* sp = (uint16_t*)tvPtr->screen;
    col = 0;
    if (++row == TV_TEXT_ROWS) {
        row--;
        wordmove(sp, &sp[TV_TEXT_COLS], TV_TEXT_LASTROW); // scroll
        wordfill(&sp[TV_TEXT_LASTROW], blank, TV_TEXT_COLS); // clear new line
    }
}

/*
 * print a character
 */
static void printc(int c)
{
    int ndx = row * TV_TEXT_COLS + col;
    uint32_t val = 0;
    
    val  = ((color << 1) | (c & 1)) << 10;
    val += 0x200 + (c & 0xFE);

    // Driver updates during invisible. Need some delay so screen updates right.
    // For flicker-free once per scan update, you can wait for status != invisible.
    // while(*tvPtr->status != TV_TEXT_STAT_INVISIBLE)    ;

    tvPtr->screen[ndx] = val;

    if (++col == TV_TEXT_COLS)
        newline();
}

/*
 * TV_Text setTileColor sets tile data color at x,y position
 * See header file for more details.
 */
uint16_t tvText_getTileColor(int x, int y)
{
    int shift  = 11;
    int   mask = ((TV_TEXT_COLORS-1) << shift);
    int   ndx  = y * TV_TEXT_COLS + x;
    int   color = 0;
    
    if(x >= TV_TEXT_COLS)
        return 0; 
    if(y >= TV_TEXT_ROWS)
        return 0;
    color = tvPtr->screen[ndx] & mask;
    color >>= shift;
    return color;
}


/*
 * TV_Text setTileColor sets tile data color at x,y position
 * See header file for more details.
 */
void tvText_setTileColor(int x, int y, uint16_t color)
{
    uint16_t tile = 0;
    int shift  = 11;
    int   mask = ((TV_TEXT_COLORS-1) << shift);
    int   ndx  = y * TV_TEXT_COLS + x;
    
    while(tvPtr->status != TV_TEXT_STAT_INVISIBLE)
        ;
    if(x >= TV_TEXT_COLS)
        return; 
    if(y >= TV_TEXT_ROWS)
        return;

    color <<= shift; 
    tile = tvPtr->screen[ndx];
    tile = tile & ~mask;
    tile = tile | color;
    tvPtr->screen[ndx] = tile;
}

/*
 * TV_Text str function prints a string at current position
 * See header file for more details.
 */
void    tvText_str(char* sptr)
{
    while(*sptr) {
#ifdef TV_TEXT_OUT
        tvText_out(*(sptr++));
#else
        tvText_outchar(*(sptr++));
#endif
    }
}

/*
 * TV_Text dec function prints a decimal number at current position
 * See header file for more details.
 */
void    tvText_dec(int value)
{
    int n = value;
    int len = 10;
    int result = 0;

    if(value < 0) {
        value = ~value;
        printc('-');
    }

    n = 1000000000;

    while(--len > -1) {
        if(value >= n) {
            printc(value / n + '0');
            value %= n;
            result++;
        }
        else if(result || n == 1) {
            printc('0');
        }
        n /= 10;
    }
}

/*
 * TV_Text hex function prints a hexadecimal number at current position
 * See header file for more details.
 */
void    tvText_hex(int value, int digits)
{
    int ndx;
#ifdef CHAR_ARRAY
    char hexlookup[] =
    {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };
#else
    char* hexlookup = "0123456789ABCDEF";
#endif
    while(digits-- > 0) {
        ndx = (value >> (digits<<2)) & 0xf;
        printc(hexlookup[ndx]);
    }   
}


/*
 * TV_Text bin function prints a binary number at current position
 * See header file for more details.
 */
void    tvText_bin(int value, int digits)
{
    int bit = 0;
    while(digits-- > 0) {
        bit = (value >> digits) & 1;
        printc(bit + '0');
    }   
}

#ifdef TV_TEXT_OUT
/*
 * TV_Text out function prints a character at current position or performs
 * a screen function.
 * See header file for more details.
 */
void    tvText_out(int c)
{
    if(flag == 0)
    {
        switch(c)
        {
            case 0:
                wordfill(tvPtr->screen, color << 11 | blank, TV_TEXT_SCREENSIZE);
                col = 0;
                row = 0;
                break;
            case 1:
                col = 0;
                row = 0;
                break;
            case 8:
                if (col)
                    col--;
                break;
            case 9:
                do {
                    printc(' ');
                } while(col & 7);
                break;
            case 0xA:   // fall though
            case 0xB:   // fall though
            case 0xC:   // fall though
                flag = c;
                return;
            case 0xD:
                newline();
                break;
            default:
                printc(c);
                break;
        }
    }
    else
    if (flag == 0xA) {
        col = c % TV_TEXT_COLS;
    }
    else
    if (flag == 0xB) {
        row = c % TV_TEXT_ROWS;
    }
    else
    if (flag == 0xC) {
        color = c & 0xf;
    }
    flag = 0;
}
#endif

/*
 * TV_Text print null terminated char* to screen with normal stdio definitions
 * See header file for more details.
 */
void    tvText_print(char* s)
{
    while(*s) {
        tvText_outchar(*(s++));
    }
}

/*
 * TV_Text outchar print char to screen with normal stdio definitions
 * See header file for more details.
 */
int     tvText_outchar(char c)
{
    switch(c)
    {
        case '\b':
            if (col)
                col--;
            break;
        case '\t':
            do {
                printc(' ');
            } while(col & 7);
            break;
        case '\n':
            newline();
            break;
        case '\r':
            col = 0;
            break;
        default:
            printc(c);
            break;
    }
    return (int)c;
}

/*
 * TV_Text getTile gets tile data from x,y position
 * See header file for more details.
 */
uint16_t   tvText_getTile(int x, int y)
{
    if(x >= TV_TEXT_COLS)
        return 0;
    if(y >= TV_TEXT_ROWS)
        return 0;
    return tvPtr->screen[y * TV_TEXT_COLS + x];
}

/*
 * TV_Text setTile sets tile data at x,y position
 * See header file for more details.
 */
void tvText_setTile(int x, int y, uint16_t tile)
{
    if(x >= TV_TEXT_COLS)
        return;
    if(y >= TV_TEXT_ROWS)
        return;
    tvPtr->screen[y * TV_TEXT_COLS + x] = tile;
}

/*
 * TV_Text setCurPosition function sets position to x,y.
 * See header file for more details.
 */
void    tvText_setCurPosition(int x, int y)
{
    col = x;
    row = y;
}

/*
 * TV_Text setCoordPosition function sets position to Cartesian x,y.
 * See header file for more details.
 */
void    tvText_setCoordPosition(int x, int y)
{
    col = x;
    row = TV_TEXT_ROWS-y-1;
}

/*
 * TV_Text setXY function sets position to x,y.
 * See header file for more details.
 */
void    tvText_setXY(int x, int y)
{
    col = x;
    row = y;
}

/*
 * TV_Text setX function sets column position value
 * See header file for more details.
 */
void    tvText_setX(int value)
{
    col = value;
}

/*
 * TV_Text setY function sets row position value
 * See header file for more details.
 */
void    tvText_setY(int value)
{
    row = value;
}

/*
 * TV_Text getX function gets column position
 * See header file for more details.
 */
int tvText_getX(void)
{
    return col;
}

/*
 * TV_Text getY function gets row position
 * See header file for more details.
 */
int tvText_getY(void)
{
    return row;
}

/*
 * TV_Text setColors function sets palette color set index
 * See header file for more details.
 */
void tvText_setColors(int value)
{
    color = value % TV_TEXT_COLORS;
}

/*
 * TV_Text getColors function gets palette color set index
 * See header file for more details.
 */
int tvText_getColors(void)
{
    return color % TV_TEXT_COLORS;
}

/*
 * TV_Text getWidth function gets screen width.
 * See header file for more details.
 */
int tvText_getColumns(void)
{
    return TV_TEXT_COLS;
}

/*
 * TV_Text getHeight function gets screen height.
 * See header file for more details.
 */
int tvText_getRows(void)
{
    return TV_TEXT_ROWS;
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

