/*
############################################################################
# Written by Dave Hein
# Copyright (c) 2012 Parallax, Inc.
# MIT Licensed
############################################################################
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "propeller.h"
#include "cog.h"
#include "draw.h"
#include "text.h"
#include "text2.h"

#define bitsPerPixel      1
#define horizontalScaling 1
#define horizontalPixels  640

#define HOLE_TOP 192
#define HOLE_BOT 288

extern int8_t vga_array[];

uint8_t tilemap1[30*40];
uint8_t *tilemap0 = (uint8_t *)vga_array;
uint8_t *tilemap = (uint8_t *)vga_array;
uint32_t tiles[256*16];
extern int lastindex;
extern int lockindex;

volatile int syncIndicator = 0;
int pixelColors = 0xfc3c000c;

typedef struct VgaVarS {
    uint32_t directionState;
    uint32_t videoState;
    uint32_t frequencyState;
    uint32_t verticalScaling;
    uint32_t verticalPixels;
    uint32_t visibleScale;
    uint32_t invisibleScale;
    uint32_t horizontalLongs;
    uint32_t *colortable1;
    uint32_t *buffer1;
    uint32_t *pixelColorsAddress;
    uint32_t *syncIndicatorAddress;
} VgaVarT;

#if 0
void CheckStack(void)
{
    char *ptr = malloc(100);
    printf("%8.8x %8.8x %d\n", ptr, &ptr, ((int)&ptr) - (int)ptr);
    free(ptr);
}
#endif

void Step(void)
{
    char buffer[20];
    printf("Step\n");
    gets(buffer);
}

void VgaStart(int pinGroup, int verticalResolution, int *newDisplayPointer,
int *colortable, int *pixelColors, int *syncIndicator)
{
    int i, temp;
    VgaVarT vga;
    int frequencyState;

#if 0
    printf("%8.8x %8.8x %8.8x %8.8x %8.8x %8.8x\n",
        newDisplayPointer, colortable, pixelColors, syncIndicator, vga_array, &vga);
#endif

    vga.directionState = 0xff << (8 * pinGroup);
    vga.videoState = 0x200000ff | (pinGroup << 9) | (bitsPerPixel << 28);
    temp = (25175000 + 1600) / 4;
    frequencyState = 1;
    for (i = 0; i < 32; i++)
    {
        temp <<= 1;
        frequencyState <<= 1;
        if (i == 31) frequencyState++;
        if (temp >= CLKFREQ)
        {
            temp -= CLKFREQ;
            frequencyState++;
        }
    }
    vga.frequencyState = frequencyState;
    vga.verticalScaling = 480 / verticalResolution;
    vga.verticalPixels = 480 / vga.verticalScaling;
    vga.visibleScale = (horizontalScaling << 12) + ((640 * 32) >> bitsPerPixel) / horizontalPixels;
    vga.invisibleScale = ((8 << bitsPerPixel) << 12) + 160;
    vga.horizontalLongs = horizontalPixels / (32 >> bitsPerPixel);
    vga.buffer1 = newDisplayPointer;
    vga.colortable1 = colortable;
    vga.pixelColorsAddress = pixelColors;
    vga.syncIndicatorAddress = syncIndicator;

    memcpy(vga_array + 4, &vga, sizeof(VgaVarT));

    cognew(vga_array, 0);
}

typedef struct ObjectS {
    int col;
    int prevcol;
    int row;
    int delcol;
    int delrow;
    uint32_t *object;
    int width;
    int height;
    int mass;
} ObjectT;

void CheckInnerWallCollisions(ObjectT *obj, int leftcol, int rightcol, int hole)
{
    leftcol -= obj->width;

    if (hole && obj->row >= HOLE_TOP && obj->row + obj->height - 1 < HOLE_BOT) return;

    if (obj->col < rightcol && obj->prevcol >= rightcol)
    {
        obj->col = (rightcol << 1) - obj->col;
        obj->delcol = -obj->delcol;
    }
    else if (obj->col >= leftcol && obj->prevcol < leftcol)
    {
        obj->col = (leftcol << 1) - obj->col;
        obj->delcol = -obj->delcol;
        if (!hole)
        {
            obj->col--;
            obj->delcol--;
        }
    }
    else if (obj->col >= leftcol && obj->col <= rightcol)
    {
        int prevrow = obj->row - obj->delrow;

        if (hole && prevrow >= HOLE_TOP && prevrow < HOLE_BOT)
        {
            if (obj->row < HOLE_TOP)
                obj->row = (HOLE_TOP << 1) - obj->row;
            else
                obj->row = ((HOLE_BOT - obj->height) << 1) - obj->row;
            obj->delrow = -obj->delrow;
        }
        else
        {
            if (obj->prevcol < obj->col)
            {
                obj->col = (leftcol << 1) - obj->col;
                if (!hole)
                {
                    obj->col--;
                    obj->delcol--;
                }
                obj->delcol = -obj->delcol;
            }
            else
            {
                obj->col = (rightcol << 1) - obj->col;
                obj->delcol = -obj->delcol;
            }
        }
    }
}

void CheckWallCollisions(ObjectT *obj, int mincol, int maxcol, int minrow, int maxrow)
{
    maxcol -= obj->width;
    maxrow -= obj->height;

    if (obj->col < mincol)
    {
        obj->col = (mincol << 1) - obj->col;
        obj->delcol = -obj->delcol;
    }
    else if (obj->col >= maxcol)
    {
        obj->col = (maxcol << 1) - obj->col;
        obj->delcol = -obj->delcol;
    }

    if (obj->row < minrow)
    {
        obj->row = (minrow << 1) - obj->row;
        obj->delrow = -obj->delrow;
    }
    else if (obj->row >= maxrow)
    {
        obj->row = (maxrow << 1) - obj->row;
        obj->delrow = -obj->delrow;
    }
}

#if 0
void CheckCollision(ObjectT *obj1, ObjectT *obj2)
{
    int diff;
    int coldiff = obj1->col + (obj1->width>>1) - obj2->col - (obj2->width>>1);
    int rowdiff = obj1->row + (obj1->height>>1) - obj2->row - (obj2->height>>1);

    if (coldiff < 0) coldiff = -coldiff;
    if (rowdiff < 0) rowdiff = -rowdiff;

#if 0
    if ((coldiff << 1) > obj1->width + obj2->width ||
        (rowdiff << 1) > obj1->height + obj2->height)
        return;
#endif

    diff = (coldiff * coldiff) + (rowdiff * rowdiff);

    if (4 * diff > (obj1->width * obj1->height) + (obj2->width * obj2->height))
        return;

    obj1->delcol = -obj1->delcol;
    obj2->delcol = -obj2->delcol;
    obj1->delrow = -obj1->delrow;
    obj2->delrow = -obj2->delrow;
}
#endif

void Bounce(void)
{
    int i, j, k, l;
    int mode = 0;
    ObjectT *obj;
    int maxcol, maxrow;
    int syncIndicatorPrev;

    uint32_t object1[64] = {
        0xa0000000, 0x0000000a, 0xaaa00000, 0x00000aaa, 0xaaaa0000, 0x0000aaaa,
        0xaaaa8000, 0x0002aaaa, 0xaaaaa800, 0x002aaaaa, 0xaaaaaa00, 0x00aaaaaa,
        0xaaaaaa00, 0x00aaaaaa, 0xaaaaaa80, 0x02aaaaaa, 0xaaaaaaa0, 0x0aaaaaaa,
        0xaaaaaaa0, 0x0aaaaaaa, 0xaaaaaaa8, 0x2aaaaaaa, 0xaaaaaaa8, 0x2aaaaaaa,
        0xaaaaaaa8, 0x2aaaaaaa, 0xaaaaaaa8, 0x2aaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa,
        0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa,
        0xaaaaaaa8, 0x2aaaaaaa, 0xaaaaaaa8, 0x2aaaaaaa, 0xaaaaaaa8, 0x2aaaaaaa,
        0xaaaaaaa8, 0x2aaaaaaa, 0xaaaaaaa0, 0x0aaaaaaa, 0xaaaaaaa0, 0x0aaaaaaa,
        0xaaaaaa80, 0x02aaaaaa, 0xaaaaaa00, 0x00aaaaaa, 0xaaaaaa00, 0x00aaaaaa,
        0xaaaaa800, 0x002aaaaa, 0xaaaa8000, 0x0002aaaa, 0xaaaa0000, 0x0000aaaa,
        0xaaa00000, 0x00000aaa, 0xa0000000, 0x0000000a};

    uint32_t object2[64] = {
        0x05500000, 0x00000000, 0x55554000, 0x00000001, 0x55555400, 0x00000015,
        0x55555500, 0x00000055, 0x55555540, 0x00000155, 0x55555550, 0x00000555,
        0x55555550, 0x00000555, 0x55555554, 0x00001555, 0x55555554, 0x00001555,
        0x55555554, 0x00001555, 0x55555555, 0x00005555, 0x55555555, 0x00005555,
        0x55555555, 0x00005555, 0x55555555, 0x00005555, 0x55555554, 0x00001555,
        0x55555554, 0x00001555, 0x55555554, 0x00001555, 0x55555550, 0x00000555,
        0x55555550, 0x00000555, 0x55555540, 0x00000155, 0x55555500, 0x00000055,
        0x55555400, 0x00000015, 0x55554000, 0x00000001, 0x05500000, 0x00000000};

    uint32_t object3[64] = {
        0x000ff000, 0x00ffff00, 0x0ffffff0, 0x0ffffff0, 0x3ffffffc, 0x3ffffffc,
        0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x3ffffffc, 0x3ffffffc,
        0x0ffffff0, 0x0ffffff0, 0x00ffff00, 0x000ff000};

    ObjectT objects[] = {
        100, 100, 200,  1, 1, (uint32_t *)object1, 32, 32, 64,
        200, 200, 300,  3, 5, (uint32_t *)object2, 24, 24, 27,
        300, 300, 400, -2, 8, (uint32_t *)object3, 16, 16,  8,
        100, 100, 250,  7, 2, (uint32_t *)object1, 32, 32, 64,
        200, 200, 350,  2,-5, (uint32_t *)object2, 24, 24, 27,
        300, 300, 450, -5, 3, (uint32_t *)object3, 16, 16,  8,
        100, 100, 200,  1, 1, (uint32_t *)object1, 32, 32, 64,
        200, 200, 300,  3, 5, (uint32_t *)object2, 24, 24, 27,
        300, 300, 400, -2, 8, (uint32_t *)object3, 16, 16,  8,
        100, 100, 250,  7, 2, (uint32_t *)object1, 32, 32, 64,
        200, 200, 350,  2,-5, (uint32_t *)object2, 24, 24, 27,
        300, 300, 450, -5, 3, (uint32_t *)object3, 16, 16,  8,
        100, 100, 200,  1, 1, (uint32_t *)object1, 32, 32, 64,
        200, 200, 300,  3, 5, (uint32_t *)object2, 24, 24, 27,
        300, 300, 400, -2, 8, (uint32_t *)object3, 16, 16,  8,
        100, 100, 250,  7, 2, (uint32_t *)object1, 32, 32, 64,
        200, 200, 350,  2,-5, (uint32_t *)object2, 24, 24, 27,
        300, 300, 450, -5, 3, (uint32_t *)object3, 16, 16,  8,
        100, 100, 200,  1, 1, (uint32_t *)object1, 32, 32, 64,
        200, 200, 300,  3, 5, (uint32_t *)object2, 24, 24, 27,
        300, 300, 400, -2, 8, (uint32_t *)object3, 16, 16,  8,
        100, 100, 250,  7, 2, (uint32_t *)object1, 32, 32, 64,
        200, 200, 350,  2,-5, (uint32_t *)object2, 24, 24, 27,
        300, 300, 450, -5, 3, (uint32_t *)object3, 16, 16,  8};

    int numobj = 6;
    int leftwall = 640 - 16;
    int hole = 0;
    int rightwall = 639;
    int moveflag;
    int lockindex1;

    inittiles(0);
    pixelColors = 0x0c30c000;
    tilemap = tilemap1;
    memset(tilemap, 0, 40*30);
    memset(&tiles[16], 0xdd, 4);
    memset(&tiles[17], 0x77, 4);
    memcpy(&tiles[18], &tiles[16], 8);
    memcpy(&tiles[20], &tiles[16], 16);
    memcpy(&tiles[24], &tiles[16], 32);
    tilemap[0] = ++lastindex;
    drawline( 0, 0,  0, 15, 3);
    drawline(15, 0, 15, 15, 3);
    drawline(0, 0, 15, 0, 3);
    drawline(0, 15, 15, 15, 3);
    drawline(0, 7, 15, 7, 3);
    drawline(0, 8, 15, 8, 3);
    memcpy(&tiles[32], &tiles[16], 32);
    memset(&tiles[40], 0, 32);
    memset(&tiles[48], 0, 32);
    memcpy(&tiles[56], &tiles[16], 32);
    memset(&tiles[4<<4], 0, 64);
    memcpy(&tiles[5<<4], &tiles[1<<4], 64);
    lastindex = 3;
    PutStringImage(16, 3, "GRAVITY ON", 1, 0, 1);
    lockindex1 = lockindex = lastindex;
    
    for (i = 0; i < 2500; i++)
    {
        moveflag = 0;

        // Update the object positions
        for (j = 0; j < numobj; j++)
        {
            obj = &objects[j];
            obj->prevcol = obj->col;
            obj->col += obj->delcol;
            obj->row += obj->delrow;
        }

#if 0
        // Check for collisions with other objects
        for (j = 0; j < numobj-1; j++)
        {
            for (k = j + 1; k < numobj; k++)
                CheckCollision(&objects[j], &objects[k]);
        }
#endif

        if (i > 850)
        {
            moveflag = 1;
            leftwall--;
            if (leftwall < 320)
            {
                moveflag = 0;
                hole = 1;
                leftwall = 320;
                rightwall = leftwall + 15;
            }
            else
            {
                hole = 0;
                rightwall = 640;
            }
            for (j = 0; j < numobj; j++)
            {
                CheckInnerWallCollisions(&objects[j], leftwall, rightwall, hole);
            }
            
        }

        // Check for collisions with the walls
        for (j = 0; j < numobj; j++)
            CheckWallCollisions(&objects[j], 16, 623, 8, 471);

        // Add gravity after 400 loops
        if (i > 400 && i < 700)
        {
            for (j = 0; j < numobj; j++)
                objects[j].delrow += 1;
        }

        // Draw the inner wall
        drawobject(leftwall, 8, &tiles[1<<4], 16, 8);
        drawobject(leftwall, 480 - 16, &tiles[1<<4], 16, 8);
        drawobject(leftwall, 16, &tiles[1<<4], 16, 16);
        lockindex = lastindex;
        k = (leftwall >> 4) + (2 * 40);
        if (leftwall & 15)
        {
            for (j = 2; j < 29; j++, k += 40)
            {
                if (!hole || j < (HOLE_TOP >> 4) || j >= (HOLE_BOT >> 4))
                {
                    tilemap[k] = lastindex - 1;
                    tilemap[k+1] = lastindex;
                }
            }
        }
        else
        {
            for (j = 2; j < 29; j++, k += 40)
            {
                if (!hole || j < (HOLE_TOP >> 4) || j >= (HOLE_BOT >> 4))
                {
                    tilemap[k] = lastindex;
                }
            }
        }

        // Draw the objects
        for (j = 0; j < numobj; j++)
        {
            obj = &objects[j];
            drawobject(obj->col, obj->row, obj->object, obj->width, obj->height);
        }

        memcpy(tilemap0, tilemap, 30*40);
        memset(tilemap, 0, 30*40);
        memset(tilemap, 2, 40);
        memset(&tilemap[29*40], 3, 40);

        for (j = 0, l = 0; l < 30; j += 40, l++)
        {
            tilemap[j] = tilemap[j+39] = 1;
        }
        if (i > 400 && i < 700 && (i%25) < 13)
        {
            for (j = 0; j < 5; j++)
                tilemap[82 + j] = lockindex1 - 4 + j;
        }

        if ((mode ^= 1))
        {
            lastindex = (256 + lockindex1) >> 1;
        }
        else
        {
            lastindex = lockindex1;
        }
        if (i == 700) numobj = 12;
        if (i == 800) numobj = 18;
    }
    tilemap = tilemap0;
    lockindex = 0;
}

int sine(int angle)
{
    int cflag, zflag;

    cflag = angle & 0x0800;
    zflag = angle & 0x1000;
    if (cflag) angle = -angle;
    angle &= 0xfff;
    angle |= 0xe000 >> 1;
    angle <<= 1;
    angle = *(uint16_t *)angle;
    if (zflag) angle = -angle;
    return angle;
}

void PlotLog(void)
{
    int i;
    uint16_t *sptr;

    pixelColors = 0xfcc0000c;
    inittiles(255);
    pixelColors = 0x000c28a8;
    for (i = 0; i < 15; i++) tiles[i] = 0x40000000;
    tiles[15] = 0x55555555;

    PutStringImage2(250, 20, "Log Table", 1, 0, 2);
    sptr = (uint16_t *)0xc000;
    for (i = 0; i < 512; i++)
    {
        drawdot(i+64, 400 - (sptr[i*4]/200), 2);
    }

    drawline(64, 400, 576, 400, 3);
    drawline(64, 400 - 65536/200, 576, 400 - 65536/200, 3);
    drawline(64, 400, 64, 400 - 65536/200, 3);
    drawline(576, 400, 576, 400 - 65536/200, 3);
    PutStringImage(290, 420, "X-Axis", 1, 0, 2);
    PutStringImage(34, 250, "Y-Axis", 1, 1, 2);
    waitcnt(4*CLKFREQ+CNT);
}

void PlotAntiLog(void)
{
    int i;
    uint16_t *sptr;

    inittiles(255);
    PutStringImage2(220, 20, "Anti-log Table", 1, 0, 0);
    sptr = (uint16_t *)0xd000;
    for (i = 0; i < 512; i++)
    {
        drawdot(i+64, 400 - (sptr[i*4]/200), 2);
    }

    drawline(64, 400, 576, 400, 1);
    drawline(64, 400 - 65536/200, 576, 400 - 65536/200, 1);
    drawline(64, 400, 64, 400 - 65536/200, 1);
    drawline(576, 400, 576, 400 - 65536/200, 1);
    waitcnt(4*CLKFREQ+CNT);
}

void PlotSine(void)
{
    int i;
    uint16_t *sptr;

    inittiles(255);
    PutStringImage2(250, 20, "Sine Table", 1, 0, 0);
    pixelColors = 0x3cc0110c;
    sptr = (uint16_t *)0xe000;
    for (i = 0; i <= 512; i++)
    {
        drawdot(i+64, 400 - (sptr[i*4]/200), 2);
    }
    drawline(64, 400, 576, 400, 1);
    drawline(64, 400 - 65536/200, 576, 400 - 65536/200, 1);
    drawline(64, 400, 64, 400 - 65536/200, 1);
    drawline(576, 400, 576, 400 - 65536/200, 1);

    waitcnt(4*CLKFREQ+CNT);
}

void PlotSinewave()
{
    int i;
    int col1, col2, row1, row2;
    uint32_t hole[64] = {
        0x000ff000, 0x00ffff00, 0x0ffffff0, 0x0ffffff0, 0x3ffffffc, 0x3ffffffc,
        0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x3ffffffc, 0x3ffffffc,
        0x0ffffff0, 0x0ffffff0, 0x00ffff00, 0x000ff000};

    inittiles(0x55);
    pixelColors = 0x0028a00c;
    tiles[15] = 0xaaaaaaaa;
    drawobject(16, 64, hole, 16, 16);
    drawobject(16, 416, hole, 16, 16);
    drawline(64, 0, 64, 479, 3);
    PutStringImage2(250, 20, "Sinewave", 1, 0, 0);
    col1 = 64;
    row1 = 236;
    for (i = 0; i <= 512; i += 4)
    {
        col2 = i + 64;
        row2 = 236 - (sine(i*32) / 500);
        drawline(col1, row1, col2, row2, 0);
        col1 = col2;
        row1 = row2;
    }
    drawline(64, 236, 576, 236, 3);

    waitcnt(4*CLKFREQ+CNT);
}

void DrawText()
{
    int i;
    int row = 100;

    inittiles(255);
    pixelColors = 0xfcc0000c;
    PutStringImage(236, row, "This is the 8x11 font", 1, 0, 1);
    row += 12;
    PutStringImage(32, row, "This is the 8x11 font in double size", 2, 0, 2);
    row += 24;
    PutStringImage2(72, row, "This is the Parallax 16x32 font", 1, 0, 0);
    row += 60;
    PutStringImage(100, row, "Now is the time for all good men", 1, 0, 1);
    row += 12;
    PutStringImage(100, row, "to come to the aid of their party", 1, 0, 1);
    row += 24;
    PutStringImage(100, row, "The quick brown fox jumps over", 1, 0, 2);
    row += 12;
    PutStringImage(100, row, "the lazy dog", 1, 0, 2);
    row += 12;

    waitcnt(4*CLKFREQ+CNT);
}

void DisplayRawRomFont(void)
{
    int i, k;

    pixelColors = 0xfc3c000c;

    memcpy(tiles, (void *)0x8000, 256*16*4);
    k = 1;
    for (i = 0; i < 1200; i++)
    {
        if ((i%80) == 40) k = k - 80 + 1;
        else if ((i%80) == 0) k = k - 1;
        tilemap[i] = (k & 255);
        k += 2;
    }

    //waitcnt(2*CLKFREQ+CNT);
}

int vga_text_col = 0;
int vga_text_row = 0;

void vga_tx(int val)
{
    int i;
    int index = vga_text_col + 80 * vga_text_row;

    index %= 40 * 30;

    if (val < 32)
        vga_text_col = 40;
    else
    {
        val <<= 1;
        tilemap[index] = val;
        tilemap[index+40] = val+1;
        vga_text_col++;
    }
    if (vga_text_col >= 40)
    {
        vga_text_col = 0;
        vga_text_row++;
        if (vga_text_row >= 15)
        {
            for (i = 0; i < 40*28; i += 80)
                memcpy(&tilemap[i], &tilemap[i+80], 80);
            memset(&tilemap[40*28], ' ' << 1, 80);
            vga_text_row = 14;
        }
    }
}

void vga_str(char *str)
{
    while (*str)
        vga_tx(*str++);
}

void vga_tx2(int val)
{
    int i;
    int index = vga_text_col + 40 * vga_text_row;

    index %= 40 * 30;

    if (val < 32)
        vga_text_col = 40;
    else
    {
        tilemap[index] = val;
        vga_text_col++;
    }
    if (vga_text_col >= 40)
    {
        vga_text_col = 0;
        vga_text_row++;
        if (vga_text_row >= 30)
        {
            for (i = 0; i < 40*29; i += 40)
                memcpy(&tilemap[i], &tilemap[i+40], 40);
            memset(&tilemap[40*29], ' ', 40);
            vga_text_row = 29;
        }
    }
}

void vga_str2(char *str)
{
    while (*str)
        vga_tx2(*str++);
}

void DisplayRomFont(void)
{
    int i, j, temp;
    uint32_t *ptr1;
    uint32_t *ptr2;
    char outstr[80];

    pixelColors = 0xfc3c000c;
    memcpy(tiles, (void *)0x8000, 256*16*4);
    ptr1 = tiles;
    ptr2 = (uint32_t *)0x8000;
    for (i = 0; i < 64; i++)
    {
       for (j = 0; j < 32; j++)
       {
           temp = ptr2[j] & 0x55555555;
           *ptr1++ = temp | (temp << 1);
       }
       for (j = 0; j < 32; j++)
       {
           temp = ptr2[j] & 0xaaaaaaaa;
           *ptr1++ = temp | (temp >> 1);
       }
       ptr2 += 32;
    }
    waitcnt(2*CLKFREQ+CNT);
    memset(tilemap, ' ' << 1, 40*30);
    for (i = 0; i < 200; i++)
    {
        sprintf(outstr, "This is a test %3d\n", i);
        vga_str(outstr);
    }
    
}

void DisplayRomFont2(void)
{
    int i, j, temp;
    uint32_t *ptr1;
    uint32_t *ptr2;
    char outstr[80];

    pixelColors = 0xfc3c000c;
    memcpy(tiles, (void *)0x8000, 256*16*4);
    ptr1 = tiles;
    ptr2 = (uint32_t *)0x8000;
    for (i = 0; i < 128; i++)
    {
       for (j = 0; j < 32; j += 2)
       {
           temp = (ptr2[j] | ptr2[j+1]) & 0x55555555;
           *ptr1++ = temp | (temp << 1);
       }
       for (j = 0; j < 32; j += 2)
       {
           temp = (ptr2[j] | ptr2[j+1]) & 0xaaaaaaaa;
           *ptr1++ = temp | (temp >> 1);
       }
       ptr2 += 32;
    }
    for (i = 0; i < 1200; i++)
    {
        tilemap[i] = (i & 255);
    }
    waitcnt(2*CLKFREQ+CNT);
    memset(tilemap, ' ', 40*30);
    for (i = 0; i < 200; i++)
    {
        sprintf(outstr, "This is a test %3d\n", i);
        vga_str2(outstr);
    }
    
}

int main(void)
{
    OUTA = 0;
    DIRA |= 0x8000;

    VgaStart(2, 30, (int *)tilemap, tiles, &pixelColors, (int *)&syncIndicator);
    waitcnt(CLKFREQ + CNT);

    //CheckStack();


    DisplayRawRomFont();
    DisplayRomFont();

    while (1)
    {
        DisplayRawRomFont();
        DisplayRomFont();
        DisplayRomFont2();
        PlotLog();
        PlotAntiLog();
        PlotSine();
        PlotSinewave();
        DrawText();
        Bounce();
    }

    return 0;
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

