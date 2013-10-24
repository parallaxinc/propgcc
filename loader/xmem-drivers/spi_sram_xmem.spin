{
  SPI SRAM External Memory Driver (16 bit addresses)
  Copyright (c) 2013 by David Betz
  
  Based on code from Chip Gracey's Propeller II SDRAM Driver
  Copyright (c) 2013 by Chip Gracey

  TERMS OF USE: MIT License

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
}

#include "xmem_common.spin"
#include "xmem_spi_pins.spin"
#include "xmem_spi.spin"

DAT

init
        ' build the pin masks
        call    #get_spi_pins

        ' set the pin directions
        mov     outa, pinout
        mov     dira, pindir
        call    #release
        
        ' select sequential access mode for the SRAM chip
        call    #select
        mov     data, ramseq
        mov     bits, #16
        call    #spiSend
        
init_ret
        ret
        
'----------------------------------------------------------------------------------------------------
'
' read_write_init
'
' select the chip and send the address for the read/write operation
'
' on input:
'   extaddr is the sram transfer address
'
'----------------------------------------------------------------------------------------------------

read_write_init
        call    #select          ' select SRAM chip
        mov     data, extaddr
        and     data, sram_mask
        shl     data, #8
        or      data, fn
        mov     bits, #24
        call    #spiSend
read_write_init_ret
        ret

'----------------------------------------------------------------------------------------------------
'
' read_bytes
'
' on input:
'   extaddr is the external memory address to read
'   hubaddr is the hub memory address to write
'   count is the number of bytes to read
'
'----------------------------------------------------------------------------------------------------

read_bytes
        mov     fn, read
        call    #read_write_init 
:loop   call    #spiRecvByte
        wrbyte  data, hubaddr
        add     hubaddr, #1
        djnz    count, #:loop
        call    #release
read_bytes_ret
        ret

'----------------------------------------------------------------------------------------------------
'
' write_bytes
'
' on input:
'   extaddr is the external memory address to write
'   hubaddr is the hub memory address to read
'   count is the number of bytes to write
'
'----------------------------------------------------------------------------------------------------

write_bytes
        mov     fn, write
        call    #read_write_init
:loop   rdbyte  data, hubaddr
        call    #spiSendByte
        add     hubaddr, #1
        djnz    count, #:loop
        call    #release
write_bytes_ret
        ret
        
fn      long    0

pindir      long    0
pinout      long    0

' variables used by the spi send/receive functions
cmd         long    0
bytes       long    0

' spi commands
read        long    $03000000       ' read command
write       long    $02000000       ' write command
ramseq      long    $01400000       ' %00000001_01000000 << 16 ' set sequential mode

' mask off the base address of external memory
sram_mask   long    $0000ffff

            fit     496
