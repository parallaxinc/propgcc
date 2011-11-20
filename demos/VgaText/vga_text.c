/**
 * @file vga_text.c
 * VGA_Text native device driver interface.
 *
 * Copyright (c) 2008, Steve Denson
 * See end of file for terms of use.
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <propeller.h>
#include <propeller.h>

#include "vga_text.h"

#define VGA_TEXT_OUT

/**
 * This is the main global vga text control/status structure.
 */
volatile vgaText_t gVgaText;

/**
 * This is the VGA text screen area.
 */
static short gscreen[VGA_TEXT_SCREENSIZE];

/**
 * This is the VGA color palette area.
 */
static int gcolors[VGA_TEXT_COLORTABLE_SIZE];

/**
 * These are variables to keep up with display;
 */
static int col, row, flag;

static int blank = 0x220;

/**
 * This is the VGA palette.
 */
static char gpalette[VGA_TEXT_COLORTABLE_SIZE] =     
{                           // fgRGB  bgRGB    '
    0b111111, 0b000001,     // %%333, %%001    '0    white / dark blue
    0b111100, 0b010100,     // %%330, %%110    '1   yellow / brown
    0b100010, 0b000000,     // %%202, %%000    '2  magenta / black
    0b010101, 0b111111,     // %%111, %%333    '3     grey / white
    0b001111, 0b000101,     // %%033, %%011    '4     cyan / dark cyan
    0b001000, 0b101110,     // %%020, %%232    '5    green / gray-green
    0b010000, 0b110101,     // %%100, %%311    '6      red / pink
    0b001111, 0b000001      // %%033, %%003    '7     cyan / blue
};

char greypalette[VGA_TEXT_COLORTABLE_SIZE] =     
{                           // fgRGB  bgRGB    '
    0b111111, 0b000001,     // %%333, %%001    '0    white / dark blue
    0b000001, 0b111111,     // %%333, %%001    '0 dark blue/ white
    0b111111, 0b000000,     // %%333, %%001    '0    white / black
    0b000000, 0b111111,     // %%333, %%001    '1    black / white
    0b010101, 0b000000,     // %%330, %%110    '2     grey / black
    0b000000, 0b010101,     // %%202, %%000    '3    black / grey
    0b111111, 0b010101,     // %%111, %%333    '4    white / grey
    0b010101, 0b111111      // %%111, %%333    '5     grey / white
};

/*
 * This should set the character foreground and screen background.
 * API are available to get/set this.
 */
static int color = 0;

static void wordfill(uint16_t *dst, uint16_t val, int len);
static void wordmove(uint16_t *dst, uint16_t *src, int len);
static void wait(int ms);

/*
 * VGA_Text start function starts VGA on a cog
 * See header file for more details.
 */
int     vgaText_start(int basepin)
{
    extern uint32_t binary_VGA_dat_start[];
    int id = 0;

    col   = 0; // init vars
    row   = 0;
    flag  = 0;

    gVgaText.status = 0;
    gVgaText.enable = 1;
    gVgaText.pins   = basepin | 0x7;
    gVgaText.mode   = 0b1000;
    gVgaText.screen = (long) gscreen;
    gVgaText.colors = (long) gcolors;
    gVgaText.ht = VGA_TEXT_COLS;
    gVgaText.vt = VGA_TEXT_ROWS;
    gVgaText.hx = 1;
    gVgaText.vx = 1;
    gVgaText.ho = 1;
    gVgaText.vo = 1;
    gVgaText.hd = 512;
    gVgaText.hf = 10;
    gVgaText.hs = 75;
    gVgaText.hb = 43;
    gVgaText.vd = 480;
    gVgaText.vf = 11;
    gVgaText.vs = 2;
    gVgaText.vb = 31;
    gVgaText.rate = 80000000 >> 2;
      
    id = cognew((void*)binary_VGA_dat_start, (void*)&gVgaText);
    
    // set main fg/bg color here
    vgaText_setColorPalette(&gpalette[VGA_TEXT_PAL_WHITE_BLUE]);
    wordfill(&gscreen[0], blank, VGA_TEXT_SCREENSIZE);
    
    wait(1);
    return id;
}

/*
 * VGA_Text stop function stops VGA cog
 * See header file for more details.
 */
void    vgaText_stop(int id)
{
    if(id) {
        cogstop(id);
    }
}

/*
 * VGA_Text setcolors function sets the palette to that defined by pointer.
 * See header file for more details.
 */
void    vgaText_setColorPalette(char* ptr)
{
    int  ii = 0;
    int  mm = 0;
    int  fg = 0;
    int  bg = 0;
    for(ii = 0; ii < VGA_TEXT_COLORTABLE_SIZE; ii += 2)
    {
        mm = ii + 1; // beta1 ICC has trouble with math in braces. use mm
        fg = ptr[ii] << 2;
        bg = ptr[mm] << 2;
        gcolors[ii]  = fg << 24 | bg << 16 | fg << 8 | bg;
        gcolors[mm]  = fg << 24 | fg << 16 | bg << 8 | bg;
   }        
}

/*
 * print a new line
 */
static void newline(void)
{
    col = 0;
    if (++row == VGA_TEXT_ROWS) {
        row--;
        wordmove(&gscreen[0], &gscreen[VGA_TEXT_COLS], VGA_TEXT_LASTROW); // scroll
        wordfill(&gscreen[VGA_TEXT_LASTROW], blank, VGA_TEXT_COLS); // clear new line
    }
}

/*
 * print a character
 */
static void printc(int c)
{
    int   ndx = row * VGA_TEXT_COLS + col;
    short val = 0;
    
    // I can't seem to get the palette colors to work correctly.
    // If you want just one screen fg/bg color, use setColorPalette(&gcolors[N]);
    // It seems the color would be wrong for every "even" char.
    // Fine but remove the (c & 1) and the character is wrong.
    
    val  = (color << 1 | c & 1) << 10;
    val += 0x200 + (c & 0xFE);

    // Driver updates during invisible. Need some delay so screen updates right.
    // For flicker-free once per scan update, you can wait for status != invisible.
    // while(gVgaText.status != VGA_TEXT_STAT_INVISIBLE)    ;
    
    // Use some delay so printing works correctly.
    //wait(50);
    
    gscreen[ndx] = val; // works
    //gscreen[row * VGA_TEXT_COLS + col] = val; // fails ... don't know why

    if (++col == VGA_TEXT_COLS) {
        newline();
    }
}

/*
 * VGA_Text getTile gets tile data from x,y position
 * See header file for more details.
 */
short   vgaText_getTile(int x, int y)
{
    if(x >= VGA_TEXT_COLS)
        return 0; 
    if(y >= VGA_TEXT_ROWS)
        return 0;
    return gscreen[y * VGA_TEXT_COLS + x];
}

/*
 * VGA_Text setTile sets tile data at x,y position
 * See header file for more details.
 */
void vgaText_setTile(int x, int y, short tile)
{
    if(x >= VGA_TEXT_COLS)
        return; 
    if(y >= VGA_TEXT_ROWS)
        return;
    gscreen[y * VGA_TEXT_COLS + x] = tile;
}

/*
 * VGA_Text setTileColor sets tile data color at x,y position
 * See header file for more details.
 */
int vgaText_getTileColor(int x, int y)
{
    short tile = 0;
    int shift  = 11;
    int   mask = ((VGA_TEXT_COLORS-1) << shift);
    int   ndx  = y * VGA_TEXT_COLS + x;
    int   color = 0;
    
    if(x >= VGA_TEXT_COLS)
        return 0; 
    if(y >= VGA_TEXT_ROWS)
        return 0;
    color = gscreen[ndx] & mask;
    color >>= shift;
    return color;
}


/*
 * VGA_Text setTileColor sets tile data color at x,y position
 * See header file for more details.
 */
void vgaText_setTileColor(int x, int y, int color)
{
    short tile = 0;
    int shift  = 11;
    int   mask = ((VGA_TEXT_COLORS-1) << shift);
    int   ndx  = y * VGA_TEXT_COLS + x;
    
    while(gVgaText.status != VGA_TEXT_STAT_INVISIBLE)
        ;
    if(x >= VGA_TEXT_COLS)
        return; 
    if(y >= VGA_TEXT_ROWS)
        return;

    color <<= shift; 
    tile = gscreen[ndx];
    tile = tile & ~mask;
    tile = tile | color;
    gscreen[ndx] = tile;
}

/*
 * VGA_Text str function prints a string at current position
 * See header file for more details.
 */
void    vgaText_str(char* sptr)
{
    while(*sptr) {
#ifdef VGA_TEXT_OUT
        vgaText_out(*(sptr++));
#else
        putchar(*(sptr++));
#endif
    }
}

/*
 * VGA_Text dec function prints a decimal number at current position
 * See header file for more details.
 */
void    vgaText_dec(int value)
{
#if 0
    char b[128];
    memset(b, 0, 127);
    itoa(b, value, 10);
    vgaText_str(b);
#else
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
#endif
}

/*
 * VGA_Text hex function prints a hexadecimal number at current position
 * See header file for more details.
 */
void    vgaText_hex(int value, int digits)
{
    int ndx;
    char hexlookup[] =
    {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };
    while(digits-- > 0) {
        ndx = (value >> (digits<<2)) & 0xf;
        printc(hexlookup[ndx]);
    }   
}


/*
 * VGA_Text bin function prints a binary number at current position
 * See header file for more details.
 */
void    vgaText_bin(int value, int digits)
{
    int bit = 0;
    while(digits-- > 0) {
        bit = (value >> digits) & 1;
        printc(bit + '0');
    }   
}

//#ifdef VGA_TEXT_OUT
/*
 * VGA_Text out function prints a character at current position or performs
 * a screen function.
 * See header file for more details.
 */
void    vgaText_out(int c)
{
    if(flag == 0)
    {
        switch(c)
        {
            case 0:
                wordfill(&gscreen[0], color << 11 | blank, VGA_TEXT_SCREENSIZE);
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
        col = c % VGA_TEXT_COLS;
    }
    else
    if (flag == 0xB) {
        row = c % VGA_TEXT_ROWS;
    }
    else
    if (flag == 0xC) {
        color = c & 0xf;
    }
    flag = 0;
}
//#endif

/*
 * VGA_Text vgaText_print null terminated char* to screen with normal stdio definitions
 * See header file for more details.
 */
void   vgaText_print(char* s)
{
    while(*s) {
        vgaText_putchar(*(s++));
    }
}

/*
 * VGA_Text vgaText_putchar print char to screen with normal stdio definitions
 * See header file for more details.
 */
int     vgaText_putchar(char c)
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
 * VGA_Text setCurPosition function sets position to x,y.
 * See header file for more details.
 */
void    vgaText_setCurPosition(int x, int y)
{
    col = x;
    row = y;
}

/*
 * VGA_Text setCoordPosition function sets position to Cartesian x,y.
 * See header file for more details.
 */
void    vgaText_setCoordPosition(int x, int y)
{
    col = x;
    row = VGA_TEXT_ROWS-y-1;
}

/*
 * VGA_Text setXY function sets position to x,y.
 * See header file for more details.
 */
void    vgaText_setXY(int x, int y)
{
    col = x;
    row = y;
}

/*
 * VGA_Text setX function sets column position value
 * See header file for more details.
 */
void    vgaText_setX(int value)
{
    col = value;
}

/*
 * VGA_Text setY function sets row position value
 * See header file for more details.
 */
void    vgaText_setY(int value)
{
    row = value;
}

/*
 * VGA_Text getX function gets column position
 * See header file for more details.
 */
int vgaText_getX(void)
{
    return col;
}

/*
 * VGA_Text getY function gets row position
 * See header file for more details.
 */
int vgaText_getY(void)
{
    return row;
}

/*
 * VGA_Text setColors function sets palette color set index
 * See header file for more details.
 */
void vgaText_setColors(int value)
{
    color = value % VGA_TEXT_COLORS;
}

/*
 * VGA_Text getColors function gets palette color set index
 * See header file for more details.
 */
int vgaText_getColors(void)
{
    return color % VGA_TEXT_COLORS;
}

/*
 * VGA_Text getWidth function gets screen width.
 * See header file for more details.
 */
int vgaText_getColumns(void)
{
    return VGA_TEXT_COLS;
}

/*
 * VGA_Text getHeight function gets screen height.
 * See header file for more details.
 */
int vgaText_getRows(void)
{
    return VGA_TEXT_ROWS;
}

static void wordfill(uint16_t *dst, uint16_t val, int len)
{
    while(--len > -1) {
        *dst = val;
        dst++;
    }
}

static void wordmove(uint16_t *dst, uint16_t *src, int len)
{
    while(--len > -1) {
        *dst = *src;
        dst++;
        src++;
    }
}

static void wait(int ms)
{
    int delay = (CLKFREQ / 1000) * ms;
    waitcnt(delay+CNT);
}

/*
+------------------------------------------------------------------------------------------------------------------------------+
¦                                                   TERMS OF USE: MIT License                                                  ¦                                                            
+------------------------------------------------------------------------------------------------------------------------------¦
¦Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation    ¦ 
¦files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,    ¦
¦modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software¦
¦is furnished to do so, subject to the following conditions:                                                                   ¦
¦                                                                                                                              ¦
¦The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.¦
¦                                                                                                                              ¦
¦THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE          ¦
¦WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR         ¦
¦COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,   ¦
¦ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                         ¦
+------------------------------------------------------------------------------------------------------------------------------+
*/
