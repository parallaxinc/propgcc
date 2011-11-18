/**
 * @file vga_text.h
 * VGA_Text native device driver interface
 *
 * Copyright (c) 2008, Steve Denson
 * See end of file for terms of use.
 */
 
/**
 * This module uses the VGA.spin driver defined as an array of 
 * longs which was generated using the included px.bat file.
 * The px.bat file uses cygwin/linux applications od, sed, and cut.
 *
 * Currently setting individual character foreground/background
 * colors do not work. Setting a screen palette is ok though i.e.
 * setColorPalette(&gpalette[VGA_TEXT_PAL_MAGENTA_BLACK]);
 */
 
#ifndef __VGA_TEXT__
#define __VGA_TEXT__

/**
 * VGA_Text color indicies
 */
#define VGA_TEXT_WHITE_BLUE     0
#define VGA_TEXT_YELLOW_BROWN   1
#define VGA_TEXT_MAGENTA_BLACK  2
#define VGA_TEXT_GREY_WHITE     3
#define VGA_TEXT_CYAN_DARKCYAN  4
#define VGA_TEXT_GREEN_WHITE    5
#define VGA_TEST_RED_PINK       6
#define VGA_TEXT_CYAN_BLUE      7

#define VGA_TEXT_COLORS 8

/**
 * VGA_Text palette color indicies
 */
#define VGA_TEXT_PAL_WHITE_BLUE     0
#define VGA_TEXT_PAL_YELLOW_BROWN   2
#define VGA_TEXT_PAL_MAGENTA_BLACK  4
#define VGA_TEXT_PAL_GREY_WHITE     6
#define VGA_TEXT_PAL_CYAN_DARKCYAN  8
#define VGA_TEXT_PAL_GREEN_WHITE    10
#define VGA_TEST_PAL_RED_PINK       12
#define VGA_TEXT_PAL_CYAN_BLUE      14


/**
 * VGA_Text color table size.
 * Table holds foreground and background info, so size is 2 x table colors.
 */
#define VGA_TEXT_COLORTABLE_SIZE 8*2

/**
 * VGA_Text column count
 */
#define  VGA_TEXT_COLS 32

/**
 * VGA_Text row count
 */
#define  VGA_TEXT_ROWS 15

/**
 * VGA_Text screensize count
 */
#define  VGA_TEXT_SCREENSIZE (VGA_TEXT_COLS * VGA_TEXT_ROWS)

/**
 * VGA_Text lastrow position count
 */
#define  VGA_TEXT_LASTROW (VGA_TEXT_SCREENSIZE-VGA_TEXT_COLS)

/**
 * VGA_Text status enum
 */
typedef enum {
    VGA_TEXT_STAT_DISABLED,
    VGA_TEXT_STAT_INVISIBLE,
    VGA_TEXT_STAT_VISIBLE
} vgaTextStat_t;

/**
 * VGA_Text control struct
 */
typedef struct _vga_text_struct
{
    long status    ; // 0/1/2 = off/visible/invisible      read-only   (21 longs)
    long enable    ; // 0/non-0 = off/on                   write-only
    long pins      ; // %pppttt = pins                     write-only
    long mode      ; // %tihv = tile,interlace,hpol,vpol   write-only
    long screen    ; // pointer to screen (words)          write-only
    long colors    ; // pointer to colors (longs)          write-only            
    long ht        ; // horizontal tiles                   write-only
    long vt        ; // vertical tiles                     write-only
    long hx        ; // horizontal tile expansion          write-only
    long vx        ; // vertical tile expansion            write-only
    long ho        ; // horizontal offset                  write-only
    long vo        ; // vertical offset                    write-only
    long hd        ; // horizontal display ticks           write-only
    long hf        ; // horizontal front porch ticks       write-only
    long hs        ; // horizontal sync ticks              write-only
    long hb        ; // horizontal back porch ticks        write-only
    long vd        ; // vertical display lines             write-only
    long vf        ; // vertical front porch lines         write-only
    long vs        ; // vertical sync lines                write-only
    long vb        ; // vertical back porch lines          write-only
    long rate      ; // tick rate (Hz)                     write-only
} vgaText_t;

/*
 * VGA_Text public API
 */

/**
 * VGA_Text start function starts VGA on a cog
 * @param basepin is first pin number (out of 8) connected to VGA
 * param clockrate is the clockrate defined for the platform.
 * @returns non-zero cogid on success
 */
int     vgaText_start(int basepin);

/**
 * VGA_Text stop function stops VGA cog
 * @param id is cog id returned from start function.
 */
void    vgaText_stop(int id);

/**
 * VGA_Text str function prints a string at current position
 * @param sptr is string to print
 */
void    vgaText_str(char* sptr);

/**
 * VGA_Text dec function prints a decimal number at current position
 * @param value is number to print
 */
void    vgaText_dec(int value);

/**
 * VGA_Text hex function prints a hexadecimal number at current position
 * @param value is number to print
 * @param digits is number of digits in value to print
 */
void    vgaText_hex(int value, int digits);

/**
 * VGA_Text bin function prints a binary number at current position
 * @param value is number to print
 * @param digits is number of digits in value to print
 */
void    vgaText_bin(int value, int digits);

/**
 * VGA_Text out function prints a character at current position or performs
 * a screen function based on the following table:
 *
 *    $00 = clear screen
 *    $01 = home
 *    $08 = backspace
 *    $09 = tab (8 spaces per)
 *    $0A = set X position (X follows)
 *    $0B = set Y position (Y follows)
 *    $0C = set color (color follows)
 *    $0D = return
 * others = printable characters
 *
 * @param value is number to print
 * @param digits is number of digits in value to print
 */
void    vgaText_out(int c);

/**
 * VGA_Text setcolors function sets the palette to that defined by pointer.
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
void    vgaText_setColorPalette(char* palette);

/**
 * VGA_Text setTileColor sets tile data color at x,y position
 * @param x is current x screen position
 * @param y is current y screen position
 */
int     vgaText_getTileColor(int x, int y);

/**
 * VGA_Text setTileColor sets tile data color at x,y position
 * @param x is current x screen position
 * @param y is current y screen position
 * @param color is color to set
 */
void    vgaText_setTileColor(int x, int y, int color);

/**
 * VGA_Text setCurPositon function sets position to x,y.
 * @param x is column counted from left.
 * @param y is row counted from top.
 */
void    vgaText_setCurPosition(int x, int y);

/**
 * VGA_Text setCoordPosition function sets position to cartesian x,y.
 * @param x is column counted from left.
 * @param y is row counted from bottom.
 */
void    vgaText_setCoordPosition(int x, int y);

/**
 * VGA_Text setXY function sets position to x,y.
 * @param x is column counted from left.
 * @param y is row counted from top.
 */
void    vgaText_setXY(int x, int y);

/**
 * VGA_Text setX function sets column position value
 * @param value is new column position
 */
void    vgaText_setX(int value);

/**
 * VGA_Text setY function sets row position value
 * @param value is new row position
 */
void    vgaText_setY(int value);

/**
 * VGA_Text getX function gets column position
 * @returns column position
 */
int vgaText_getX(void);

/**
 * VGA_Text getY function gets row position
 * @returns row position
 */
int vgaText_getY(void);

/**
 * VGA_Text setColors function sets palette color set index
 * @param value is a color set index number 0 .. 7
 */
void vgaText_setColors(int value);

/**
 * VGA_Text getColors function gets palette color set index
 * @returns number representing color set index
 */
int vgaText_getColors(void);

/**
 * VGA_Text getWidth function gets screen width.
 * @returns screen column count.
 */
int vgaText_getColumns(void);

/**
 * VGA_Text getHeight function gets screen height.
 * @returns screen row count.
 */
int vgaText_getRows(void);

/**
 * VGA_Text print null terminated char* to screen with normal stdio definitions
 * @param s is null terminated string to print using putchar
 */
void    vgaText_print(char* s);

/**
 * VGA_Text putchar print char to screen with normal stdio definitions
 * @param c is character to print
 */
int    vgaText_putchar(char c);


#endif
//__VGA_TEXT__

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
