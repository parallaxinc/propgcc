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
