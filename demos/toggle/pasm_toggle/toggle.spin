{{
toggle.spin
Propgcc - PASM toggle demo

Simple PASM routine to demonstrate interaction between PASM subroutines an PROPGCC main program.
The code running in the C address space to talk to exchange data values with PASM code running
in a COG.

The C program has visibility/access to the mailbox variables as normal C variables.
The PASM program has visibility/access to the mailbox variables through the PAR register
initialized by the COGNEW and hte RDLONG/RDWORD/RDCHAR and WRLONG/WRWORD/WRCHAR instructions.  

C program starts the PASM routine via cognew() function, passing strat address and PAR register value.
PAR register should be the address of a STATIC data area of LONGs in the C program.

For this example:

PAR ->  static unsigned int delay;
        static unsigned int pins;
        static unsigned int loop_cnt;
        static unsigned int pasm_done;

The first three variables are used as input to the PASM routine, the last is used to act as a semaphore
back to the C routine.

This could have been written as more efficient PASM code but for the examples, I was going for maximum clarity at this point.

Copyright (c) 2011, Steve Denson, Rick Post
MIT Licensed. Terms of use below.

}}

pub start(pinptr)                                                  
    cognew(@pasm, pinptr)

dat             org 0

pasm
                mov      mailbox_ptr,par         ' save the pointer to the STATIC parameter (HUB) memory area
                                                 ' the PAR register is initialized by the cognew() function and is a pointer to
                                                 ' the first STATIC int declared in the C code as the mailbox
                                                 ' mailbox_ptr will be changed as the code executes. You can reload
                                                 ' the initial pointer from PAR if you ever need it to point to
                                                 ' the start of the mailbox again
                rdlong   waitdelay, mailbox_ptr  ' read the wait delay from HUB - it is initialized by the C program
                                                 ' in C program: delay = CLKFREQ>>1;
                add      mailbox_ptr,#4          ' point to the next LONG in HUB
                rdlong   pins,mailbox_ptr        ' the caller's PIN mask  as initialized in the C program
                                                 ' in C program: pins = 0x3fffffff;
                add      mailbox_ptr, #4         ' point to the next LONG (4 bytes each)
                rdlong   loopcounter,mailbox_ptr ' set the loop count as provided by the C program
                                                 ' in C program: loop_cnt = 20;
                add      mailbox_ptr, #4         ' point to the next LONG which is the semaphore we are setting when done

                mov      dira, pins              ' set pins provided by C program to OUTPUT
                mov      nextcnt, waitdelay
                add      nextcnt, cnt            ' best to add cnt last
:loop
                xor      outa, pins              ' toggle pins
                waitcnt  nextcnt, waitdelay      ' wait for user specified delay
                djnz     loopcounter,#:loop      ' loop until the C provided counter hits zero
                
                mov      done_flag,#1            ' set the semaphore to one
                wrlong   done_flag, mailbox_ptr  ' and save it back into hub memory via the ptr provided by the C program
                                                 ' in C program: while(!pasm_done) to test for update from PASM
                jmp     #$                       ' to infinity and BEYOND!!

' these do not need to be in any particular order or have particular names. There is no relationship between these
' local copies of the C variable except when you create via the PAR register and HUB instructions
' there is no address resolution or linkage done by propgcc or the loader
'
mailbox_ptr     long    0                       ' working ptr into the HUB area - reload from PAR if needed
pins            long    0                       ' local copy of the user's PIN mask
waitdelay       long    0                       ' local copy of the user's delay
loopcounter     long    0                       ' local copy of the user's loop counter
done_flag       long    0                       ' local copy of the semaphore to return to the C program
nextcnt         long    0                       ' local variable to save target value from waitcnt

{{

MIT Licensed.

+--------------------------------------------------------------------
 TERMS OF USE: MIT License
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
}}