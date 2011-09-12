#
# PASM toggle demo
#
# Copyright (c) 2011 Parallax, Inc.
# MIT Licensed.
#
# +--------------------------------------------------------------------
# ¦  TERMS OF USE: MIT License
# +--------------------------------------------------------------------
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files
# (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
# +--------------------------------------------------------------------

		''
		'' .coguser0 - .coguser7 are special sections for the default
		'' linker script: it knows that these need to be relocated to
		'' COG memory but loaded into hub.
		'' The "ax" flag says the section will contain executable
		'' code. The assembler should figure this out on its own,
		'' but it never hurts to be explicit!
		''
		.section .coguser0, "ax"
		.cog_ram
	
		'' load the pins and wait delay from the C
		'' variables "pins" and "wait_time" respectively
		'' note that C variables have an _ prepended
		'' also note that LMM code tends to be big,
		'' so a direct rdlong waitdelay, #_wait_time is
		'' probably not going to work (_wait_time is bigger than 511)

                rdlong  waitdelay, wait_addr ' read from hub to get
                rdlong  pins, pins_addr      ' the user's clkfreq delay and pins

                mov     dira, pins          ' set pins to output
                mov     nextcnt, waitdelay
                add     nextcnt, cnt        ' best to add cnt last
.loop
                xor     outa, pins          ' toggle pins
                waitcnt nextcnt, waitdelay  ' wait for half second
		rdlong  waitdelay, wait_addr ' update wait delay
                jmp     #.loop

pins            long    0
waitdelay       long    0                   ' read from hub to int
nextcnt         long    0

		'' addresses of C variables
wait_addr	long	_wait_time
pins_addr	long	_pins
