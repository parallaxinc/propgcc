/**
 * @file vga_text.c
 * VGA_Text native device driver interface.
 *
 * Copyright (c) 2008, Steve Denson
 * See end of file for terms of use.
 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "vga_text.h"
#include <propeller.h>

//#define C3

void main(void)
{
    int ii = 85;
    int jj = 0;
    
#ifdef C3
    vgaText_start(16);  // start VGA on C3
    DIRA |= 1<<15;
    OUTA &= ~(1<<15);
#else
    vgaText_start(8);  // start VGA on PropBOE
#endif
    
    vgaText_print("\nzoot ");
    vgaText_print("color");
    vgaText_setXY(0,0);
    vgaText_print("Hi\n");
    vgaText_print("zoot");
    vgaText_setCoordPosition(1,3);
    vgaText_print("abcd");
    vgaText_setCurPosition(5,5);
    
    vgaText_print("Hello World!\r");
    vgaText_setY(vgaText_getY()+1);
    vgaText_str("Decimal ");
    vgaText_dec(ii);
    vgaText_setY(vgaText_getY()+1);

    for(jj = 10; jj < 20; jj++) {
        vgaText_setColors((jj-10));
        vgaText_setXY(jj, jj % VGA_TEXT_ROWS);
        vgaText_str("0x");
        vgaText_hex(ii++, 4);
    }

    while(1);
}

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
