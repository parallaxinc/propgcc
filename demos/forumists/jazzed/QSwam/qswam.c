/**
 * This is a QuickStart Whack-a-Mole game.
 *
 * Copyright 2011 by Steve Denson.
 * Rights MIT Licensed. See end of file.
 */
 
/**
 * The Blue LED moles are eating up your QuickStart yard!
 * You must get rid of them!
 *
 * Gameplay:
 *
 * All living moles will run around the QuickStart burrow during
 * the game and you'll know they're alive by the dimly lit LEDs.
 *
 * Sometimes a mole will pop up as brighly lit LED and you must
 * wack the mole pad to knock it out.
 *
 * When the moles are gone, your QuickStart yard will be serene.
 * That is: no more LED flickering. A rather boring place, no?
 *
 */
#include <stdlib.h>             // for rand() and seed()
#include <propeller.h>          // for propeller functions.

/* game utilities */
void    celebrate(void);        // no more moles
int     rotateLeft(int mask);
int     rotateRight(int mask);

/* simple utilities */
void    LEDOUT(int val);        // set leds to val
void    msleep(int t);          // sleep for t ms
int     getButton(int n);       // used to get button states
int     getButtons();           // used to get button states

/*
 * Main program entry point
 * Start with 8 moles and flick LEDs via rotating mask.
 * Choose a random mole to light for fraction of a second.
 * If the mole button is pushed while the LED is lit, kill the mole.
 * When all moles are dead celebrate - check button press for restart.
 */
int main(void)
{
    int molemask    = 0xff;     // live moles

    int ontime      = 1;        // flicker on time in ms
    int offtime     = 80;       // flicker off time

    int whacked     = 0;        // it's whacked?
    int direction   = 1;        // keep direction of LED mole run

    int popcount    = 6;        // initial popcount
    int countmask   = 0x1f00;   // allow up to 32 mole run steps
    int popmask     = 0x7;      // only 8 LEDs, choose from rand

    int popupspot   = 0;        // show popup position
    int popuptime   = 500;      // popup on time

    /* mole run forever loop */
    for(;;)
    {
        /* nothing whacked */
        whacked = 0;
        /* seed random function with Propeller CNT */
        srand(CNT);

        /* blank moles */
        LEDOUT(0);
        /* sleep a little */
        msleep(offtime);

        /* if all moles gone, celebrate */
        while(!molemask) {
            /* make LEDs dance */
            celebrate();
            /* if button press, restart game */
            if(getButtons()) {
                molemask = 0xff;
            }
        }

        /* get popup spot */
        if(popcount-- == 0) {
            /* get random number */
            popupspot  = rand();
            /* use upper bits for popup count */
            popcount = (popupspot & countmask) >> 8;
            /* use lower popmask bits for popup mole LED */
            popupspot &= popmask;

            /* show popup and check button */
            if(molemask & (1<<popupspot)) {
                /* set LED to show a mole to whack */
                LEDOUT((1<<popupspot) & molemask);
                /* single thread limit. sample twice */
                msleep(popuptime>>1);
                whacked = getButton(popupspot);
                msleep(popuptime>>1);
                whacked |= getButton(popupspot);
                /* set back to mole mask */
                LEDOUT(molemask);
            }
        }

        /* mole whacked ? */
        if(whacked) {
            molemask &= ~(1<<popupspot);
        }

        /* if a random bit is set switch direction */
        if(rand() & 0x20) {
            direction ^= 1;
        }

        /* set new mole run spots */
        if(direction) {
            molemask = rotateRight(molemask);
        }
        else {
            molemask = rotateLeft(molemask);
        }

        /* show mole run */
        LEDOUT(molemask);
        /* sleep a little so we can see the mole run */
        msleep(ontime);

    }
    return 0;
}

/*
 * make LEDs dance with a checker board pattern
 */
void celebrate(void)
{
    int n = 16;
    while(n-- > -1)
    {
        LEDOUT(0x55);
        msleep(50);
        LEDOUT(0xAA);
        msleep(50);
    }
    LEDOUT(0);
    msleep(400);
}

/*
 * rotate mask left
 */
int rotateLeft(int mask)
{
    int temp = mask << 1;
    mask = (temp & 0x100) ? 1 : 0;
    mask |= temp & 0xff;
    return mask;
}

/*
 * rotate mask right
 */
int rotateRight(int mask)
{
    int temp = mask >> 1;
    mask &= 0x01;
    mask <<= 7;
    mask |= temp;
    return mask;
}

/*
 * Set the LEDs to val
 */
void LEDOUT(int val)
{
    DIRA |= 0xff << 16;
    OUTA = (OUTA & ~(0xff<<16)) | (val<<16);
}

/*
 * msleep makes the cog wait for about t milliseconds.
 */
void msleep(int t)
{
    waitcnt((CLKFREQ/1000)*t+CNT);
}

/*
 * Get a specific button number.
 * buttons are enumerated from 0 to 7.
 */
int getButton(int n)
{
    return (getButtons() & (1<<n));
}

/*
 * Get all buttons at once
 * Run this function from HUB if compiled for LMM.
 */
int getButtons()
{
    int n = 16;
    int result  = -1;
    int reading =  0;
    int pins = 0xFF;
    OUTA |= pins;               //output high to buttons

    while(n--)
    {
        DIRA |= pins;           //output high to buttons
        DIRA ^= pins;           //switch to input
        reading = pins;
        msleep(2);              //wait for RC time
        reading &= INA;          //return button states
        result  &= reading;
    }
    result = ~result & 0xFF;
    return result;
}

/*
+--------------------------------------------------------------------
| TERMS OF USE: MIT License
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
+--------------------------------------------------------------------
*/

