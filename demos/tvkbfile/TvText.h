/**
 * @file tvText.h
 * TV_Text native device driver interface
 *
 * Copyright (c) 2008-2011 Steve Denson
 * See end of file for terms of use.
 */
 
/**
 * This module uses the TV.spin driver defined as an array of 
 * uint32_t which was generated using the included px.bat file.
 * The px.bat file uses cygwin/linux applications od, sed, and cut.
 *
 * Currently setting individual character foreground/background
 * colors do not work. Setting a screen palette is ok though i.e.
 * setColorPalette(&gpalette[TV_TEXT_PAL_MAGENTA_BLACK]);
 */
 
#ifndef __TV_TEXT__
#define __TV_TEXT__

#include <stdint.h>

/**
 * TV_Text color indicies
 */
#define TV_TEXT_WHITE_BLUE     0
#define TV_TEXT_WHITE_RED      1
#define TV_TEXT_YELLOW_BROWN   2
#define TV_TEXT_GREY_WHITE     3
#define TV_TEXT_CYAN_DARKCYAN  4
#define TV_TEXT_GREEN_WHITE    5
#define TV_TEST_RED_PINK       6
#define TV_TEXT_CYAN_BLUE      7

#define TV_TEXT_COLORS 8

/**
 * TV_Text palette color indicies
 */
#define TV_TEXT_PAL_WHITE_BLUE     0
#define TV_TEXT_PAL_WHITE_RED      2
#define TV_TEXT_PAL_YELLOW_BROWN   4
#define TV_TEXT_PAL_GREY_WHITE     6
#define TV_TEXT_PAL_CYAN_DARKCYAN  8
#define TV_TEXT_PAL_GREEN_WHITE    10
#define TV_TEST_PAL_RED_PINK       12
#define TV_TEXT_PAL_CYAN_BLUE      14


/**
 * TV_Text color table size.
 * Table holds foreground and background info, so size is 2 x table colors.
 */
#define TV_TEXT_COLORTABLE_SIZE 8*2

/**
 * TV_Text column count
 */
#define  TV_TEXT_COLS 43

/**
 * TV_Text row count
 */
#define  TV_TEXT_ROWS 14

/**
 * TV_Text screensize count
 */
#define  TV_TEXT_SCREENSIZE (TV_TEXT_COLS * TV_TEXT_ROWS)

/**
 * TV_Text lastrow position count
 */
#define  TV_TEXT_LASTROW (TV_TEXT_SCREENSIZE-TV_TEXT_COLS)

/**
 * TV_Text status enum
 */
typedef enum {
    TV_TEXT_STAT_DISABLED,
    TV_TEXT_STAT_INVISIBLE,
    TV_TEXT_STAT_VISIBLE
} tvTextStat_t;

/**
 * TV_Text control struct
 */
typedef volatile struct _tv_text_struct
{
    uint32_t status     ; //0/1/2 = off/invisible/visible              read-only
    uint32_t enable     ; //0/non-0 = off/on                           write-only
    uint32_t pins       ; //%pppmmmm = pin group, pin group mode       write-only
    uint32_t mode       ; //%tccip = tile,chroma,interlace,ntsc/pal    write-only
    uint16_t *screen    ; //pointer to screen (words)                  write-only      
    uint32_t *colors    ; //pointer to colors (longs)                  write-only
    uint32_t ht         ; //horizontal tiles                           write-only
    uint32_t vt         ; //vertical tiles                             write-only
    uint32_t hx         ; //horizontal tile expansion                  write-only
    uint32_t vx         ; //vertical tile expansion                    write-only
    uint32_t ho         ; //horizontal offset                          write-only
    uint32_t vo         ; //vertical offset                            write-only
    uint32_t broadcast  ; //broadcast frequency (Hz)                   write-only
    uint32_t auralcog   ; //aural fm cog                               write-only      
}   TvText_t;

/*
 * TV_Text public API
 */

/**
 * TV_Text start function starts TV on a cog
 * @param basepin is first pin number (out of 8) connected to TV
 * param clockrate is the clockrate defined for the platform.
 * @returns non-zero cogid on success
 */
int     tvText_start(int basepin);

/**
 * TV_Text stop function stops TV cog
 */
void    tvText_stop(void);


/**
 * TV_Text outchar prints a character at the current position
 * does the traditional C processing with \n == newline, \r == return,
 * \b == backspace
 */
int tvText_outchar(char c);

/**
 * TV_Text setcolors function sets the palette to that defined by pointer.
 *
 * Override default color palette
 * palette must point to a list of up to 8 colors
 * arranged as follows (where r, g, b are 0..3):
 *
 *               fore   back
 *               ------------
 * palette  byte %%rgb, %%rgb     'color 0
 *          byte %%rgb, %%rgb     'color 1
 *          byte %%rgb, %%rgb     'color 2
 *          ...
 *
 * @param palette is a char array[16].
 */
void    tvText_setColorPalette(char* palette);

/**
 * TV_Text print null terminated char* to screen with normal stdio definitions
 * @param s is null terminated string to print using putchar
 */
void    print(char* s);

#endif
//__TV_TEXT__

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


