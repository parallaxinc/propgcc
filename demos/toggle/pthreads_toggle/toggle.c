/**
 * @file toggle.c
 * This program demonstrates starting threads to
 * toggle pins, using the pthreads interface
 * The cog makes all IO except 30/31 toggle.
 *
 * Copyright (c) 2011 Parallax, Inc.
 * MIT Licensed (see at end of file for exact terms)
 */

#include <stdio.h>
#include <propeller.h>
#include <pthread.h>

/*
 * here's the toggle code that runs in another cog
 */

void *
do_toggle(void *arg)
{
  unsigned int pins = (unsigned int)arg;

  /* pin the thread to this particular cog
     this is necessary because we rely on the 
     DIRA hardware register, which is per cog
     pthread_set_affinity_thiscog_np is non-portable
     (hence the _np suffix) and is a Propeller
     extension
  */
  pthread_set_affinity_thiscog_np();

  /* nobody will be waiting for us to finish */
  pthread_detach(pthread_self());

  /* set up the hardware registers */
  _DIRA |= pins;
  _OUTA |= pins;

  for(;;) {
    _OUTA ^= pins; /* update the pins */
    printf("toggled %x on cog %d\n", pins, __builtin_propeller_cogid());
    sleep(1);      /* nap for 1 second */
  }
}

/*
 * main code
 * This is the main thread
 * It launches othre threads to actually toggle
 * the pins (one thread per pin).
 */

pthread_t thr[32];

void main (int argc,  char* argv[])
{
    int i;
    int result;
    unsigned int pins;

    printf("hello, world!\n");

    for (i = 0; i < 30; i++) {
      /* set up the parameters for the C cog */
      pins = (1U<<i);

      pthread_create(&thr[i], NULL, do_toggle, (void *)pins);
    }
    printf("done launching threads!\n");
    for(;;) {
      sleep(30);
    }
}

/* +--------------------------------------------------------------------
 * ¦  TERMS OF USE: MIT License
 * +--------------------------------------------------------------------
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * +--------------------------------------------------------------------
 */
