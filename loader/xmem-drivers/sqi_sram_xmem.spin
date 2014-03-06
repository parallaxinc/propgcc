{
  SQI SRAM External Memory Driver
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
#include "xmem_sqi_pins.spin"
#include "xmem_sqi.spin"

' xmem_param1: $ssxxccee - ss=sio0 cc=sck ee=cs

DAT

init

        ' setup pin masks
        call    #get_sqi_pins
                
        ' set the pin directions
        mov     outa, pinout
        mov     dira, pindir
        call    #release
        
        ' select sequential access mode for the SRAM chip
        call    #select
        mov     data, sram_ramseq
        mov     bits, #16
        call    #spiSend
        call    #release
        
        ' switch to quad mode
        call    #select
        mov     data, sram_eqio
        mov     bits, #8
        call    #spiSend
        
        ' now that we're in quad mode we need to reset the pin defaults
        or      pindir, sio_mask
        andn    pinout, sio_mask
        call    #release
                
init_ret
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
        mov     fn, sram_read
        call    #read_write_start 
        andn    dira, sio_mask  ' switch to input
        andn    outa, sck_mask
        call    #sqiRecvByte    ' dummy byte
:loop   call    #sqiRecvByte
        wrbyte  data, ptr
        add     ptr, #1
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
        mov     fn, sram_write
        call    #read_write_start
        andn    outa, sck_mask
:loop   rdbyte  data, ptr
        call    #sqiSendByte
        andn    outa, sck_mask
        add     ptr, #1
        djnz    count, #:loop
        call    #release
write_bytes_ret
        ret
        
'----------------------------------------------------------------------------------------------------
'
' read_write_start
'
' select the chip and send the address for the read/write operation
'
' on input:
'   fn is the read/write code
'   vmaddr is the sram transfer address
'   hubaddr is the hub address
'
'----------------------------------------------------------------------------------------------------

read_write_start
        mov     cmd, extaddr
        and     cmd, sram_mask
        or      cmd, fn
        or      dira, sio_mask
        call    #select
        rol     cmd, #8
        mov     data, cmd
        call    #sqiSendByte
        andn    outa, sck_mask
        rol     cmd, #8
        mov     data, cmd
        call    #sqiSendByte
        andn    outa, sck_mask
        rol     cmd, #8
        mov     data, cmd
        call    #sqiSendByte
        andn    outa, sck_mask
        rol     cmd, #8
        mov     data, cmd
        call    #sqiSendByteX
        mov     ptr, hubaddr
read_write_start_ret
        ret

' variables used by the spi send/receive functions
fn          long    0
cmd         long    0
ptr         long    0

pindir      long    0
pinout      long    0

' spi commands
sram_read   long    $03000000       ' read command
sram_write  long    $02000000       ' write command
sram_eqio   long    $38000000       ' enter quad I/O mode
sram_ramseq long    $01400000       ' %00000001_01000000 << 16 ' set sequential mode

sram_mask   long    $00ffffff       ' mask to isolate the sram offset bits

        fit     496
