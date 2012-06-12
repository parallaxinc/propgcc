#ifndef __termH__
#define __termH__

#ifdef __cplusplus
extern "C" 
{
#endif

#include <stdint.h>

typedef struct TERM TERM;

typedef struct {
	int (*putch)(TERM *term, int ch);
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
 * termClearScreen function clears the screen.
 * See header file for more details.
 */
void termClearScreen(TERM *term);

/**
 * termStr function prints a string at current position
 * @param sptr is string to print
 */
void termStr(TERM *term, const char* sptr);

/**
 * Term Dec function prints a decimal number at current position
 * @param value is number to print
 */
void termDec(TERM *term, int value);

/**
 * Term Hex function prints a hexadecimal number at current position
 * @param value is number to print
 * @param digits is number of digits in value to print
 */
void termHex(TERM *term, int value, int digits);

/**
 * Term Bin function prints a binary number at current position
 * @param value is number to print
 * @param digits is number of digits in value to print
 */
void termBin(TERM *term, int value, int digits);

/**
 * Term Out function prints a character at current position or performs
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
int termOut(TERM *term, int c);

/**
 * Term Setcolors function sets the palette to that defined by pointer.
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
void termSetColorPalette(TERM *term, const char* palette);

/**
 * Term SetTileColor sets tile data color at x,y position
 * @param x is current x screen position
 * @param y is current y screen position
 */
int termGetTileColor(TERM *term, int x, int y);

/**
 * Term SetTileColor sets tile data color at x,y position
 * @param x is current x screen position
 * @param y is current y screen position
 * @param color is color to set
 */
void termSetTileColor(TERM *term, int x, int y, int color);

/**
 * Term SetCurPositon function sets position to x,y.
 * @param x is column counted from left.
 * @param y is row counted from top.
 */
void termSetCurPosition(TERM *term, int x, int y);

/**
 * Term SetCoordPosition function sets position to cartesian x,y.
 * @param x is column counted from left.
 * @param y is row counted from bottom.
 */
void termSetCoordPosition(TERM *term, int x, int y);

/**
 * Term SetXY function sets position to x,y.
 * @param x is column counted from left.
 * @param y is row counted from top.
 */
void termSetXY(TERM *term, int x, int y);

/**
 * Term SetX function sets column position value
 * @param value is new column position
 */
void termSetX(TERM *term, int value);

/**
 * Term SetY function sets row position value
 * @param value is new row position
 */
void termSetY(TERM *term, int value);

/**
 * Term GetX function gets column position
 * @returns column position
 */
int termGetX(TERM *term);

/**
 * Term GetY function gets row position
 * @returns row position
 */
int termGetY(TERM *term);

/**
 * Term SetColors function sets palette color set index
 * @param value is a color set index number 0 .. 7
 */
void termSetColors(TERM *term, int value);

/**
 * Term GetColors function gets palette color set index
 * @returns number representing color set index
 */
int termGetColors(TERM *term);

/**
 * Term GetColumns function gets screen width.
 * @returns screen column count.
 */
int termGetColumns(TERM *term);

/**
 * Term GetRows function gets screen height.
 * @returns screen row count.
 */
int termGetRows(TERM *term);

/**
 * Term Print null terminated char* to screen with normal stdio definitions
 * @param s is null terminated string to print using putchar
 */
void termPrint(TERM *term, const char* s);

/**
 * Term PutChar print char to screen with normal stdio definitions
 * @param c is character to print
 */
int termPutChar(TERM *term, int c);

/**
 * Term NewLine go to the start of a new line
 */
void termNewLine(TERM *term);

#ifdef __cplusplus
}
#endif

#endif
