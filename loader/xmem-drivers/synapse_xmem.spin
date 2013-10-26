{
  RamPage2 External Memory Driver
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

DAT

init
        ' set the pin directions
        mov     outa, pinout
        mov     dira, pindir
init_ret
        ret

'----------------------------------------------------------------------------------------------------
'
' read_bytes
'
' on input:
'   extaddr is the virtual memory address to read
'   hubaddr is the hub memory address to write
'   count is the number of longs to read
'
'----------------------------------------------------------------------------------------------------

read_bytes
        mov     t1, extaddr
        add     t1, #256
        andn    t1, #255
        sub     t1, extaddr
        max     t1, count
rloop   mov     t2, t1
        call    #read_write_start
        andn    dira, data_mask         ' set data bus to input
        mov     outa, cmd_rd
:loop   mov     data, ina
        or      outa, cmd_rd_count
        andn    outa, cmd_rd_count
        wrbyte  data, hubaddr
        add     hubaddr, #1
        djnz    t2, #:loop
        andn    outa, cmd_mask
        sub     count, t1 wz
read_bytes_ret
 if_z   ret
        add     extaddr, t1
        mov     t1, count
        max     t1, #256
        jmp     #rloop
        
data    long    0

'----------------------------------------------------------------------------------------------------
'
' write_bytes
'
' on input:
'   extaddr is the virtual memory address to write
'   hubaddr is the hub memory address to read
'   count is the number of longs to write
'
'----------------------------------------------------------------------------------------------------

write_bytes
        mov     t1, extaddr
        add     t1, #256
        andn    t1, #255
        sub     t1, extaddr
        max     t1, count
wloop   mov     t2, t1
        call    #read_write_start
:loop   rdbyte  outa, hubaddr
        or      outa, cmd_wr
        or      outa, cmd_wr_count
        add     hubaddr, #1
        djnz    t2, #:loop
        andn    outa, cmd_mask
        sub     count, t1 wz
write_bytes_ret
 if_z   ret
        add     extaddr, t1
        mov     t1, count
        max     t1, #256
        jmp     #wloop

'----------------------------------------------------------------------------------------------------
'
' read_write_start
'
' select the chip and send the address for the read/write operation
'
' on input:
'   extaddr is the sram transfer address
'
'----------------------------------------------------------------------------------------------------

read_write_start
        or      dira, data_mask         ' set data bus to output
        mov     data, extaddr
        movs    outa, data           ' load A7:0
        or      outa, cmd_byte0_latch
        andn    outa, cmd_mask
        shr     data, #8             ' load A15:8
        movs    outa, data
        or      outa, cmd_byte1_latch
        andn    outa, cmd_mask
        shr     data, #8             ' load A18:16
        movs    outa, data
        or      outa, cmd_byte2_latch
        andn    outa, cmd_mask
read_write_start_ret
        ret

' command codes
cmd_wr          long    $00010000
cmd_rd          long    $00020000
cmd_wr_count    long    $00020000
cmd_rd_count    long    $00010000
cmd_byte1_latch long    $00040000
cmd_byte2_latch long    $00050000
cmd_byte0_latch long    $00060000
cmd_status_led  long    $00070000

cmd_mask        long    $00070000
data_mask       long    $000000ff

pindir          long    $00070000
pinout          long    $00000000

        fit     496
