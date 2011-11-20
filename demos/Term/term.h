#ifndef __TERM_H__
#define __TERM_H__

#ifdef __cplusplus
extern "C" 
{
#endif

#include <stdint.h>

typedef struct TERM TERM;

typedef struct {
	int (*vblank)(TERM *term);
} TERM_OPS;

struct TERM {
	TERM_OPS *ops;
	uint16_t *screen;
	uint32_t *colors;
	int screensize;	// cols * rows
	int lastrow;	// screensize - cols
	int rows;
	int cols;
	int row;
	int col;
	int flag;
	int color;
};

#define TERM_BLANK				0x220
#define TERM_COLORS				8
#define TERM_COLORTABLE_SIZE	(TERM_COLORS*2)

/*
 * Term clearScreen function clears the screen.
 * See header file for more details.
 */
void Term_clearScreen(TERM *term);

/**
 * Term str function prints a string at current position
 * @param sptr is string to print
 */
void Term_str(TERM *term, char* sptr);

/**
 * Term dec function prints a decimal number at current position
 * @param value is number to print
 */
void Term_dec(TERM *term, int value);

/**
 * Term hex function prints a hexadecimal number at current position
 * @param value is number to print
 * @param digits is number of digits in value to print
 */
void Term_hex(TERM *term, int value, int digits);

/**
 * Term bin function prints a binary number at current position
 * @param value is number to print
 * @param digits is number of digits in value to print
 */
void Term_bin(TERM *term, int value, int digits);

/**
 * Term out function prints a character at current position or performs
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
void Term_out(TERM *term, int c);

/**
 * Term setcolors function sets the palette to that defined by pointer.
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
void Term_setColorPalette(TERM *term, char* palette);

/**
 * Term setTileColor sets tile data color at x,y position
 * @param x is current x screen position
 * @param y is current y screen position
 */
int Term_getTileColor(TERM *term, int x, int y);

/**
 * Term setTileColor sets tile data color at x,y position
 * @param x is current x screen position
 * @param y is current y screen position
 * @param color is color to set
 */
void Term_setTileColor(TERM *term, int x, int y, int color);

/**
 * Term setCurPositon function sets position to x,y.
 * @param x is column counted from left.
 * @param y is row counted from top.
 */
void Term_setCurPosition(TERM *term, int x, int y);

/**
 * Term setCoordPosition function sets position to cartesian x,y.
 * @param x is column counted from left.
 * @param y is row counted from bottom.
 */
void Term_setCoordPosition(TERM *term, int x, int y);

/**
 * Term setXY function sets position to x,y.
 * @param x is column counted from left.
 * @param y is row counted from top.
 */
void Term_setXY(TERM *term, int x, int y);

/**
 * Term setX function sets column position value
 * @param value is new column position
 */
void Term_setX(TERM *term, int value);

/**
 * Term setY function sets row position value
 * @param value is new row position
 */
void Term_setY(TERM *term, int value);

/**
 * Term getX function gets column position
 * @returns column position
 */
int Term_getX(TERM *term);

/**
 * Term getY function gets row position
 * @returns row position
 */
int Term_getY(TERM *term);

/**
 * Term setColors function sets palette color set index
 * @param value is a color set index number 0 .. 7
 */
void Term_setColors(TERM *term, int value);

/**
 * Term getColors function gets palette color set index
 * @returns number representing color set index
 */
int Term_getColors(TERM *term);

/**
 * Term getWidth function gets screen width.
 * @returns screen column count.
 */
int Term_getColumns(TERM *term);

/**
 * Term getHeight function gets screen height.
 * @returns screen row count.
 */
int Term_getRows(TERM *term);

/**
 * Term print null terminated char* to screen with normal stdio definitions
 * @param s is null terminated string to print using putchar
 */
void Term_print(TERM *term, char* s);

/**
 * Term putchar print char to screen with normal stdio definitions
 * @param c is character to print
 */
int Term_putchar(TERM *term, char c);

#ifdef __cplusplus
}
#endif

#endif
