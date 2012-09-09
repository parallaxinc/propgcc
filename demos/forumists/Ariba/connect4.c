/*
 This shows a PropGCC version of the QuickStart Game Connect4.
 Made by Andy Schenk, under MIT license (see end of file)

 You and the Quickstart set alternating a LED.
 The one that has first 4 in a row wins. You always begin.
*/

#include "propeller.h"

  int rnd;                //global vars
  int ms;

int getButtons(void)
{
  DIRA |= 0xFF;                //output high to buttons
  DIRA ^= 0xFF;                //switch to input
  waitcnt(160000+CNT);                //wait 2ms
  return INA & 0xFF ^ 0xFF;        //return button states
}

int testFour(void)
{
  int n=0,p=0,i;               //local vars
  for(i=16;i<=23;i++) {            //for each LED
      if(OUTA & 1<<i) n++;        //count contiguous ONs
      else n=0;
      if(n>3) {p=i-3; break;}        //found 4
  }
  return p;
}

int random(void)
{
  rnd = rnd * 17 + 87;            //pseudo random number
  return rnd;
}

int main(int argc, char* argv[])
{
  int n,i,p,qs;                       //local variables
  ms = CLKFREQ/1000;
  DIRA = 0x00FF0000;            //LED pins = Outputs
  OUTA = 0x000000FF;            //prepare highs on button pins
  rnd = CNT;                //randomize
  for(;;) {                //repeat forever
      n=0;                //Your turn
      while(n==0) n=getButtons();      //wait for a button
      OUTA |= n<<16;            //set LED
      p = testFour();            //4 in a row?
      qs=0;
      if(p==0) {               //QuickStart's turn
          for(i=16;i<21;i++) {        //test for a Win pattern
              n = OUTA >> i & 0xF;
              switch(n) {
                  case 0xE: qs=i; break;
                  case 0xD: qs=i+1; break;
                  case 0xB: qs=i+2; break;
                  case 0x7: qs=i+3; break;
              }
              if(qs>0) {
                  p=i;
                  waitcnt(1000*ms+CNT);
                  break;
              }
          }
          if(p==0) {            //if not: choose random one
              waitcnt(1800*ms+CNT);    //Thinking time
              while(OUTA & 1<<qs) qs = random() & 7 + 16;  //random unused LED
          }
          OUTA |= 1<<qs;        //Quickstart's LED on
          p = testFour();        //test again
      }

      if(p>0) {                //4 reached
          waitcnt(500*ms+CNT);
          if(qs>0) for(i=0;i<6;i++) {     //Quickstart wins
              OUTA ^= 0xF<<p;        //flash all 4 LEDs
              waitcnt(160*ms+CNT);
          }
          if(qs==0) for(n=0;n<2;n++) {    //You win
              for(i=p+3;i>=p;i--) {    //sequential light the 4 LEDS
                  OUTA ^= 1<<i;
                  waitcnt(160*ms+CNT);
                  OUTA |= 1<<i;
              }
          }
          waitcnt(330*ms+CNT);        //clear LEDs and restart Game
          OUTA &= 0xFF00FFFF;
      }
  }
}
/*
+--------------------------------------------------------------------
�  TERMS OF USE: MIT License
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