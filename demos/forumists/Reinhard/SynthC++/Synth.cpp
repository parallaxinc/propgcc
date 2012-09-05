/*****************************************
* Frequency Synthesizer demo v1.1       *
* Author: Beau Schwabe                  *
* Copyright (c) 2007 Parallax           *
* See end of file for terms of use.     *
*****************************************
  Original Author: Chip Gracey
  Modified by Beau Schwabe
  
  port to C++ Reinhard
*****************************************/


#include "Synth.h"



Synth::Synth (char CTR_AB,unsigned Pin,unsigned Freq)
{
    unsigned frq;
    unsigned ctr;
    unsigned pin;
    unsigned s;
    unsigned short d;

    if(Freq <= 0) Freq = 1;
    if(Freq >= 128000000) Freq = 127999999;
    
    if(Freq<500000)
    {
        ctr = (4 << 26);
        s = 1;
    }
    else
    {
        ctr = (2 << 26);
        d = findLeadingBitPosition((Freq-1)/1000000);
        s = 4 - d;                              
        ctr |= (d << 23);
    }
    
    frq = fraction(Freq, CLKFREQ, s);
    ctr |= Pin;

    pin = 1<<Pin;
    
    if(CTR_AB == 'A')
    {
        CTRA = ctr;
        FRQA = frq;
        DIRA |= pin;
    }
    else
    {
        CTRB = ctr;
        FRQB = frq;
        DIRA |= pin;
    }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

unsigned Synth::fraction (unsigned a, unsigned b, int shift)
{
    unsigned f = 0;
    int i;

    if(shift > 0) a <<= shift;
    if(shift < 0) b <<= -shift;

    for(i=0; i<32; i++)
    {
        f <<= 1;
        if(a >= b)
        {
            a -= b;
            f++;
        }
        a <<= 1;
    }
    return f;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

unsigned short Synth::findLeadingBitPosition(unsigned aNumber)
{
    short pos;

    if(aNumber == 0)
        return 0;

    for(pos = 31; pos >= 0; pos--)
    {
        if( aNumber & (1 << pos))
        {
            return (unsigned short)pos;             
        }
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*
+------------------------------------------------------------------------------------------------------------------------------+
�                                                   TERMS OF USE: MIT License                                                  �
+------------------------------------------------------------------------------------------------------------------------------�
�Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation    �
�files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,    �
�modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software�
�is furnished to do so, subject to the following conditions:                                                                   �
�                                                                                                                              �
�The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.�
�                                                                                                                              �
�THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE          �
�WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR         �
�COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,   �
�ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                         �
+------------------------------------------------------------------------------------------------------------------------------+
*/



