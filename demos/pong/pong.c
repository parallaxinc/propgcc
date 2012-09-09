//
// simple pong game for VGA
// This is a very basic translation of Kwabena W. Agyeman's
// VGA64 6 Bits Per Pixel demo, plus a basic version of a
// ping-pong arcade game.
//
// The pong game is at the first part of the file. It displays output
// on VGA, and expects input on pins defined below: for example, if
// LPINUP is high, the left paddle moves up, similarly if LPINDN is high
// then the left paddle moves down (if both are high only one will take
// effect). Similarly for the right paddle. If pins are not defined for
// one side then the computer will run it.
//
// The game continues until one player gets a score of 20 (controlled by
// the MAX_SCORE macro) and then restarts.
//
// The graphics portion is near the end of the file.
// Interested readers should get the original video object; it is very
// well written and commented (unlike this code, unfortunately, which
// is just a proof-of-concept).
// The original is Copyright (c) 2010 Kwabena W. Agyeman
// C version is Copyright (c) 2011 Eric R. Smith
// Terms of use (MIT license) at end of file.
//

//
// very simple graphics demo, intended to run all in one cog
//

#include <stdint.h>
#include "vga.h"
#include "cog.h"

// utility functions
#define min(x, y) ((x<y) ? x : y)
#define max(x, y) ((x>y) ? x : y)

#define CENTERX (COLS/2)
#define CENTERY (ROWS/2)

//////////////////////////////////////////////////////////////////
// video definitions
//////////////////////////////////////////////////////////////////
/* OUTPUT PINS */
/* GROUP 0 == 0x000000ff */
/* GROUP 1 == 0x0000ff00 */
/* GROUP 2 == 0x00ff0000 */
/* GROUP 3 == 0xff000000 */
/* FOR C3, this is 0x00ff0000 */
#define PINGROUP 2  /* which group of pins controls VGA; for C3, group 2 */

/* pin to enable VGA output */
/* you can leave this undefined if your board does not need it */
#define VGA_ENABLE_PIN 15

unsigned char framebuffer[ROWS*COLS];

//////////////////////////////////////////////////////////////////
// definitions for the game
//////////////////////////////////////////////////////////////////

// INPUT PINS
#define LPINUP 7   /* if high, move left paddle up */
#define LPINDN 6   /* if high, move left paddle down */
/* if right pins are not defined, computer controls it */
//#define RPINUP 5
//#define RPINDN 4


#define BALLW 2
#define BALLH 2
#define BALLCOLOR White
#define BGCOLOR Black
//#define BGCOLOR Grey

#define SCOREX (CENTERX-40)
#define SCOREY 8
#define SCORECOLOR Blue

#define PADDLEW 2
#define PADDLEH 8
#define LPADDLEX 8
#define RPADDLEX (COLS-LPADDLEX)
#define PADDLECOLOR Green

_COGMEM int ballx = CENTERX, bally = CENTERY;
_COGMEM int vx = 1, vy = 1;

_COGMEM int lscore, rscore;
_COGMEM int lpaddley = (ROWS-PADDLEH)/2;
_COGMEM int rpaddley = (ROWS-PADDLEH)/2;

_COGMEM int pauseframes = 0;
_COGMEM int needreset = 0;

/* maximum score */
#define MAX_SCORE 20

/* draw a vertical line */
static _NATIVE void
vline(unsigned char *frame, int x, int y, int h, unsigned char color)
{
    int i;
    frame += (y*COLS) + x;
    if (y + h >= ROWS)
        h = ROWS - y;
    for (i = 0; i < h; i++) {
        *frame = color;
        frame += COLS;
    }
}

/* draw a box */
/* does this using vertical rather than horizontal lines because most of
 * our shapes are higher than wide
 */
static _NATIVE void
box(unsigned char *frame, int x, int y, int w, int h, unsigned char color)
{
    int i;
    if (x + w >= COLS)
        w = COLS - x;
    for (i = 0; i < w; i++) {
        vline(frame, x, y, h, color);
        x++;
    }
}

/* print a single digit n (0-9) at position x,y, using color "color" */
/* note: no error checking here */
static _NATIVE void
printdigit(unsigned char *frame, int n, int x, int y, unsigned char color)
{
    static unsigned char digits[] = {
        0xf, 0x9, 0x9, 0x9, 0xf,
        0x1, 0x1, 0x1, 0x1, 0x1,
        0xf, 0x1, 0xf, 0x8, 0xf,
        0xf, 0x1, 0xf, 0x1, 0xf,
        0x9, 0x9, 0xf, 0x1, 0x1,

        0xf, 0x8, 0xf, 0x1, 0xf, 
        0xf, 0x8, 0xf, 0x9, 0xf, 
        0xf, 0x1, 0x1, 0x1, 0x1, 
        0xf, 0x9, 0xf, 0x9, 0xf, 
        0xf, 0x9, 0xf, 0x1, 0xf, 

    };
    unsigned char *ptr = digits + n*5;
    int i, j;
    unsigned c;

    frame += COLS*y + x;
    for (j = 0; j < 5; j++) {
        c = *ptr++;
        if (c & 0x8) frame[0] = color;
        if (c & 0x4) frame[1] = color;
        if (c & 0x2) frame[2] = color;
        if (c & 0x1) frame[3] = color;
        frame += COLS;
    }
}

/* print the score (a 2 digit number) at position x,y */
static _NATIVE void
printscore(unsigned char *frame, int number, int x, int y, unsigned char color)
{
    int tens, ones;

    tens = 0;
    while (number >= 10) {
        tens++;
        number -= 10;
    }
    printdigit(frame, tens, x, y, color);
    printdigit(frame, number, x+5, y, color);
}

/*
 * the screen and game update functions, called during vertical retrace
 */

_NATIVE void
initball()
{
    ballx = CENTERX;
    bally = CENTERY;
    vx = 1;
    vy = 1;
    pauseframes = 60;
}

_NATIVE void
initgame()
{
    initball();
    lpaddley = rpaddley = CENTERY - (PADDLEH/2);
    lscore = rscore = 0;
    needreset = 0;
}

/*
 * update the paddle state
 * FIXME: should read some input pins here!
 */
_NATIVE static void
updatepaddles(void)
{
    int center;

#ifdef LPINUP
    center = _INA;
    if (center & (1<<LPINDN))
        lpaddley++;
    else if (center & (1<<LPINUP))
        lpaddley--;
#else
    center = lpaddley + PADDLEH/2;
    if (center < bally)
        lpaddley++;
    else
        lpaddley--;
#endif
    lpaddley = max(lpaddley, 0);
    lpaddley = min(lpaddley, ROWS - PADDLEH);

#ifdef RPINUP
    center = _INA;
    if (center & (1<<RPINDN))
        rpaddley++;
    else if (center & (1<<RPINUP))
        rpaddley--;
#else
    center = rpaddley + PADDLEH/2;
    if (center < bally)
        rpaddley++;
    else
        rpaddley--;
#endif
    rpaddley = max(rpaddley, 0);
    rpaddley = min(rpaddley, ROWS - PADDLEH);
}

/*
 * drawing function; draw the screen, either to erase (erase=1)
 * or to really draw (erase=0)
 * i may be:
 * 0 = draw the left score
 * 1 = draw the right score
 * 2 = draw the left paddle
 * 3 = draw the right paddle
 * 4 = draw the ball
 */
static _NATIVE void
draw(int i, int erase)
{
    static unsigned char fgcolor[] = {
        SCORECOLOR, SCORECOLOR,
        PADDLECOLOR, PADDLECOLOR,
        BALLCOLOR
    };
    unsigned char color;

    if (erase)
        color = BGCOLOR;
    else
        color = fgcolor[i];
    switch(i) {
    case 0:
        /* left score */
        printscore(framebuffer, lscore, SCOREX, SCOREY, color);
        break;
    case 1:
        /* right score */
       printscore(framebuffer, rscore, COLS-(SCOREX+16), SCOREY, color);
        break;
    case 2:
        /* left paddle */
        box(framebuffer, LPADDLEX, lpaddley, PADDLEW, PADDLEH, color);
        break;
    case 3:
        /* right paddle */
        box(framebuffer, RPADDLEX, rpaddley, PADDLEW, PADDLEH, color);
        break;
    case 4:
        /* draw ball, and update paddles if erase was true */
        box(framebuffer, ballx, bally, BALLW, BALLH, color);
        break;
    default:
        break;
    }
}

/*
 * update the game state
 */
static _NATIVE void
updategame(void)
{
    updatepaddles();

    if (needreset) {
        initgame();
        return;
    }
    /* update ball position */
    ballx += vx;
    bally += vy;
    if (bally < 0) {
        bally = 0;
        vy = -vy;
    } else if (bally + BALLH > ROWS) {
        bally = ROWS - BALLH;
        vy = -vy;
    }
    /* check for paddle collisions */
    if (ballx <= LPADDLEX+PADDLEW) {
        if (ballx >= LPADDLEX - BALLW
            && bally >= lpaddley && bally < lpaddley + PADDLEH)
        {
            vx = -vx;
        }
        else if (ballx < 0)
        {
            /* check for missing the ball */
            ballx = 0;
            rscore++;
            initball();
        }
    } else if (ballx >= RPADDLEX-BALLW) {
        if (ballx <= RPADDLEX
            && bally >= rpaddley && bally < rpaddley + PADDLEH)
        {
            vx = -vx;
        }
        else if (ballx + BALLW > COLS)
        {
            ballx = COLS - BALLW;
            lscore++;
            initball();
        }
    }
}

_NATIVE static void
init(void)
{
    unsigned int frequency, i, testval;
    unsigned int clkfreq = _CLKFREQ;

    // fill the screen with the background color
    box(framebuffer, 0, 0, COLS, ROWS, BGCOLOR);

    // set up game variables
    initgame();

    // initialize video output
#if defined(VGA_ENABLE_PIN)
    _DIRA = (1<<VGA_ENABLE_PIN);
#else
    _DIRA = 0;
#endif
    _OUTA = 0;

    _VCFG = 0x300000FF | (PINGROUP<<9);
    testval = (25175000 + 1600) / 4;
    frequency = 1;
    for (i = 0; i < 32; i++) {
        testval = testval << 1;
        frequency = (frequency << 1) | (frequency >> 31);
        if (testval >= clkfreq) {
            testval -= clkfreq;
            frequency++;
        }
    }
    _FRQA = frequency;

}


///////////////////////////////////////////////////////////////////
// here is the graphics code
///////////////////////////////////////////////////////////////////

#define DISPLAY_WIDTH_LONGS (COLS/4)
#define LINEREPEAT (480/ROWS)

static const uint32_t invisibleScale = (16 << 12) + 160;
//static const uint32_t visibleScale = (4<<12) + 16;
static const uint32_t visibleScale = ((640/COLS)<<12) + (4*640/COLS);
static const uint32_t blankPixels = 640;
static const uint32_t syncPixels = 0x3FFC;
static const uint32_t HSyncColors = 0x01030103;
static const uint32_t VSyncColors = 0x00020002;
static const uint32_t HVSyncColors = 0x03030303;

static _COGMEM uint32_t *displayPtr;

/* %%3210 is 3210 in base 4, or 0 + 1*4 + 2*16 + 3*4*16 */
#define ConstPixels 0xE4

/* display one video line from the buffer pointed to by lineptr */

static _NATIVE
videoLine(uint32_t *lineptr)
{
    int i;
    uint32_t pixels;

    _VSCL = visibleScale;
    for (i = 0; i < DISPLAY_WIDTH_LONGS; i++)
    {
        pixels = *lineptr++;
        __builtin_propeller_waitvid(pixels, ConstPixels);
    }
    _VSCL = invisibleScale;
    __builtin_propeller_waitvid(HSyncColors, syncPixels);
}

//
// display N blank lines
inline _NATIVE void
blankLines(int n, uint32_t thisSyncColor)
{
    int i;
    for (i = 0; i < n; i++) {
        _VSCL = blankPixels;
        __builtin_propeller_waitvid(thisSyncColor, 0);
        _VSCL = invisibleScale;
        __builtin_propeller_waitvid(thisSyncColor, syncPixels);
    }
}

/*
 * indicate a win for one side or the other
 * side 0 == left side, side 1 == right side
 */
_NATIVE void
win(int side)
{
    draw(side, (pauseframes>>4)&1);
    needreset = 1;
}

/* the main Cog program; "par" is the value placed in the PAR register
 * by our caller.
 * Since this is a Cog program it is "unhosted" in the ANSI sense, and
 * so the parameters do not match the usual ANSI argc, argv
 */
_NAKED void
main(void)
{
    int counter;
    int i, j;
    int skip;

    init();
    _CTRA = (0xD << 23);

    for(;;) {
        displayPtr = (uint32_t *)framebuffer;
        // active video
        for (i = 0; i < ROWS; i++) {
            /* repeat each line 4 times */
            for (j = 0; j < LINEREPEAT; j++) {
                videoLine(displayPtr);
            }
            displayPtr += DISPLAY_WIDTH_LONGS;
        }

        // inactive video
        skip = 0;
        if (pauseframes > 0) {
            skip = 1;
            --pauseframes;
        }
        // front porch
        for (i = 0; i < 11; i++) {
            _VSCL = blankPixels;
            __builtin_propeller_waitvid(HSyncColors, 0);
            if (skip)
            {
                /* see if there is a winner; if so, flash the video */
                if (i == 0) {
                    if (lscore == MAX_SCORE) {
                        win (0);
                    } else if (rscore == MAX_SCORE) {
                        win(1);
                    }
                }
            } else if (i < 5) {
                draw(i, 1); /* erase old picture */
            } else if (i == 5) {
                updategame();
            } else {
                draw(i-6, 0);
            }
            _VSCL = invisibleScale;
            __builtin_propeller_waitvid(HSyncColors, syncPixels);
        }

        // vertical sync
        blankLines(4, VSyncColors);

        // back porch
        blankLines(32, HSyncColors);

        // update display settings
        _DIRA |= (0xff << (8*PINGROUP));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                  TERMS OF USE: MIT License
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
// Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
