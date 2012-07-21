/**
 * @file tv_text.c
 * TV_Text native device driver interface.
 *
 * Copyright (c) 2008-2011, Steve Denson
 * See end of file for terms of use.
 */
#include <propeller.h>
#include "TvText.h"

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
HUBDATA uint32_t pasm[PASMLEN];
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

    // set main fg/bg color
    tvText_setColorPalette(&gpalette[TV_TEXT_PAL_WHITE_BLUE]);

    // blank the screen
    wordfill(tvPtr->screen, blank, TV_TEXT_SCREENSIZE);

#if defined(__PROPELLER_XMM__)
    // start new cog from external memory using pasm and tvPtr
    memcpy((char*)pasm,(char*)binary_TV_dat_start,PASMLEN<<2);
    gTvTextCog = cognew(pasm, (uint32_t)tvPtr) + 1;
#else
    gTvTextCog = cognew((uint32_t)binary_TV_dat_start, (uint32_t)tvPtr) + 1;
#endif
    waitcnt((CLKFREQ>>4)+CNT); // 100us is all we need really

    // clear screen
    wordfill(tvPtr->screen, color << 11 | blank, TV_TEXT_SCREENSIZE);
    col = 0;
    row = 0;

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

    for(ii = 0; ii < TV_TEXT_COLORTABLE_SIZE; ii += 2)
    {
        fg = (uint8_t) ptr[ii];
        bg = (uint8_t) ptr[ii+1];
        colors[ii]     = fg << 24 | bg << 16 | fg << 8 | bg;
        colors[ii+1]   = fg << 24 | fg << 16 | bg << 8 | bg;
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
 * tvText_ClearScreen function clears the screen.
 * See header file for more details.
 */
void tvText_ClearScreen(void)
{
    wordfill(screen, blank, TV_TEXT_SCREENSIZE);
}

/*
 * tvText_GetWidth function gets screen width.
 * See header file for more details.
 */
int tvText_GetWidth(void)
{
    return TV_TEXT_COLS;
}

/*
 * tvText_GetHeight function gets screen height.
 * See header file for more details.
 */
int tvText_GetHeight(void)
{
    return TV_TEXT_ROWS;
}

/*
 * tvText_SetCursor function sets position to x,y.
 * See header file for more details.
 */
void tvText_SetCursor(int c, int r)
{
    col = c;
    row = r;
}

/*
 * tvText_GetCursor function gets the cursor position.
 * See header file for more details.
 */
void tvText_GetCursor(int *pc, int *pr)
{
    *pc = col;
    *pr = row;
}

/*
 * tvText_Print null terminated char* to screen
 * See header file for more details.
 */
void tvText_Print(const char* s)
{
    while(*s) {
        printc(*(s++));
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

