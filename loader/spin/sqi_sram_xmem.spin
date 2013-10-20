{
  SQI SRAM External Memory Driver
  Copyright (c) 2013 by David Betz
  
  Based on code from VMCOG - virtual memory server for the Propeller
  Copyright (c) February 3, 2010 by William Henning

  and on code from SdramCache
  Copyright (c) 2010 by John Steven Denson (jazzed)

  and on code from Chip Gracey's Propeller II SDRAM Driver
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

CON

PUB image
  return @init_xmem

DAT
        org   $0

' xmem_param1: $ssxxccee - ss=sio0 cc=sck ee=cs

init_xmem
        jmp     #init_continue
        
xmem_param1
        long    0
xmem_param2
        long    0
xmem_param3
        long    0
xmem_param4
        long    0
        
init_continue

        ' cmdbase is the base of an array of mailboxes
        mov     cmdbase, par
        
        ' setup pin masks
        
        ' 31:24 is the SIO0 PIN
        mov     sio_shift, xmem_param1
        shr     sio_shift, #24
        mov     mosi_mask, #1
        shl     mosi_mask, sio_shift
        mov     miso_mask, mosi_mask
        shl     miso_mask, #1
        mov     sio_mask, #$f
        shl     sio_mask, sio_shift
        
        ' 15:8 is the SCK pin
        mov     t1, xmem_param1
        shr     t1, #8
        and     t1, #$ff
        mov     sck_mask, #1
        shl     sck_mask, t1
        
        ' 7:0 is the CS pin
        mov     t1, xmem_param1
        and     t1, #$ff
        mov     cs_mask, #1
        shl     cs_mask, t1
        
        ' make the pin settings
        or      pindir, mosi_mask
        or      pindir, sck_mask
        or      pindir, cs_mask
        or      pinout, cs_mask
        mov     t1, mosi_mask       ' setup /WE and /HOLD
        shl     t1, #2
        or      pindir, t1
        or      pinout, t1
        shl     t1, #1
        or      pindir, t1
        or      pinout, t1

        ' set the pin directions
        call    #release
        
        ' select sequential access mode for the SRAM chip
        andn    outa, cs_mask
        mov     data, sram_ramseq
        mov     bits, #16
        call    #spiSend
        call    #release
        
        ' switch to quad mode
        andn    outa, cs_mask
        mov     data, sram_eqio
        mov     bits, #8
        call    #spiSend
        
        ' now that we're in quad mode we need to reset the pin defaults
        or      pindir, sio_mask
        andn    pinout, sio_mask
        call    #release

        ' start the command loop
waitcmd mov     dira, #0                ' release the pins for other SPI clients
:reset  mov     cmdptr, cmdbase
:loop   rdlong  t1, cmdptr wz
  if_z  jmp     #:next                  ' skip this mailbox if it's zero
        cmp     t1, #$8 wz              ' check for the end of list marker
  if_z  jmp     #:reset
        mov     hubaddr, t1             ' get the hub address
        andn    hubaddr, #$f
        mov     t2, cmdptr              ' get the external address
        add     t2, #4
        rdlong  extaddr, t2
        mov     t2, t1                  ' get the byte count
        and     t2, #7
        mov     count, #8
        shl     count, t2
        mov     dira, pindir            ' setup the pins so we can use them
        test    t1, #$8 wz              ' check the write flag
  if_z  jmp     #:read                  ' do read if the flag is zero
        call    #write_bytes            ' do write if the flag is one
        jmp     #:done
:read   call    #read_bytes
:done   mov     dira, #0                ' release the pins for other SPI clients
        wrlong  zero, cmdptr
:next   add     cmdptr, #8
        jmp     #:loop

' pointers to mailbox array
cmdbase long    0       ' base of the array of mailboxes
cmdptr  long    0       ' pointer to the current mailbox

' input parameters to read_bytes and write_bytes
extaddr long    0       ' external address
hubaddr long    0       ' hub address
count   long    0

zero    long    0       ' zero constant
t1      long    0       ' temporary variable
t2      long    0       ' temporary variable

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
        call    #read_write_init 
        andn    dira, sio_mask
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
        call    #read_write_init
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
' read_write_init
'
' select the chip and send the address for the read/write operation
'
' on input:
'   fn is the read/write code
'   vmaddr is the sram transfer address
'   hubaddr is the hub address
'
'----------------------------------------------------------------------------------------------------

read_write_init
        mov     cmd, extaddr
        and     cmd, sram_mask
        or      cmd, fn
        or      dira, sio_mask
        andn    outa, cs_mask
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
        call    #sqiSendByte
        mov     ptr, hubaddr
read_write_init_ret
        ret

' variables used by the spi send/receive functions
fn      long    0
cmd     long    0
ptr     long    0

'----------------------------------------------------------------------------------------------------
' SPI routines
'----------------------------------------------------------------------------------------------------

release
        mov     outa, pinout
        mov     dira, pindir
release_ret
        ret

spiSendByte
        shl     data, #24
        mov     bits, #8
spiSend rol     data, #1 wc
        muxc    outa, mosi_mask
        or      outa, sck_mask
        andn    outa, sck_mask
        djnz    bits, #spiSend
spiSendByte_ret
spiSend_ret
        ret

spiRecvByte
        mov     data, #0
        mov     bits, #8
spiRecv or      outa, sck_mask
        test    miso_mask, ina wc
        rcl     data, #1
        andn    outa, sck_mask
        djnz    bits, #spiRecv
spiRecvByte_ret
spiRecv_ret
        ret

sqiSendByte
        mov     bits, data
        ror     bits, #4
        rol     bits, sio_shift
        and     bits, sio_mask
        andn    outa, sio_mask
        or      outa, bits
        or      outa, sck_mask
        andn    outa, sck_mask
        rol     data, sio_shift
        and     data, sio_mask
        andn    outa, sio_mask
        or      outa, data
        or      outa, sck_mask
        ' the caller will need to clear the clock pin
        ' andn    outa, sck_mask
sqiSendByte_ret
        ret

sqiRecvByte
        or      outa, sck_mask
        mov     data, ina
        and     data, sio_mask
        rol     data, #4
        andn    outa, sck_mask
        or      outa, sck_mask
        mov     bits, ina
        and     bits, sio_mask
        or      data, bits
        ror     data, sio_shift
        andn    outa, sck_mask
sqiRecvByte_ret
        ret

data        long    0
bits        long    0

pindir      long    0
pinout      long    0

cs_mask     long    0
sio_shift   long    0
sio_mask    long    0
mosi_mask   long    0
miso_mask   long    0
sck_mask    long    0

' spi commands
sram_read   long    $03000000       ' read command
sram_write  long    $02000000       ' write command
sram_eqio   long    $38000000       ' enter quad I/O mode
sram_ramseq long    $01400000       ' %00000001_01000000 << 16 ' set sequential mode

sram_mask   long    $00ffffff       ' mask to isolate the sram offset bits

        fit     496
