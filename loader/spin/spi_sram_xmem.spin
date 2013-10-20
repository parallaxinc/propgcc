{
  SPI SRAM External Memory Driver (16 bit addresses)
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

  ' protocol bits
  CS_CLR_PIN_MASK       = $01
  INC_PIN_MASK          = $02   ' for C3-style CS
  MUX_START_BIT_MASK    = $04   ' low order bit of mux field
  MUX_WIDTH_MASK        = $08   ' width of mux field
  ADDR_MASK             = $10   ' device number for C3-style CS or value to write to the mux
  QUAD_SPI_HACK_MASK    = $20   ' set /WE and /HOLD for testing on Quad SPI chips
 
PUB image
  return @init_xmem

DAT
        org   $0

' xmem_param1: $ooiiccee - oo=mosi ii=miso cc=sck pp=protocol
' xmem_param2: $aabbccdd - aa=cs-or-clr bb=inc-or-start cc=width dd=addr
' the protocol byte is a bit mask with the bits defined above
'   if CS_CLR_PIN_MASK ($01) is set, then byte aa contains the CS or C3-style CLR pin number
'   if INC_PIN_MASK ($02) is set, then byte bb contains the C3-style INC pin number
'   if MUX_START_BIT_MASK ($04) is set, then byte bb contains the starting bit number of the mux field
'   if MUX_WIDTH_MASK ($08) is set, then byte cc contains the width of the mux field
'   if ADDR_MASK ($10) is set, then byte dd contains either the C3-style address or the value to write to the mux field
'   if QUAD_SPI_HACK_MASK ($20) is set, assume that pins miso+1 and miso+2 are /WP and /HOLD and assert them
' example:
'   for a simple single pin CS you should set the protocol byte to $01 and place the CS pin number in byte aa.

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
        
        ' build the mosi mask
        mov     mosi_pin, xmem_param1
        shr     mosi_pin, #24
        mov     mosi_mask, #1
        shl     mosi_mask, mosi_pin
        or      spidir, mosi_mask
        
        ' build the miso mask
        mov     t1, xmem_param1
        shr     t1, #16
        and     t1, #$ff
        mov     miso_mask, #1
        shl     miso_mask, t1
        
        ' make the sio2 and sio3 pins outputs in single spi mode to assert /WP and /HOLD
        test    xmem_param1, #QUAD_SPI_HACK_MASK wz
  if_nz mov     t1, #$0c
  if_nz shl     t1, mosi_pin
  if_nz or      spidir, t1
  if_nz or      spiout, t1
        
        ' build the sck mask
        mov     t1, xmem_param1
        shr     t1, #8
        and     t1, #$ff
        mov     sck_mask, #1
        shl     sck_mask, t1
        or      spidir, sck_mask
        
        ' handle the CS or C3-style CLR pins
        test    xmem_param1, #CS_CLR_PIN_MASK wz
  if_nz mov     t2, xmem_param2
  if_nz shr     t2, #24
  if_nz mov     cs_clr, #1
  if_nz shl     cs_clr, t2
  if_nz or      spidir, cs_clr
  if_nz or      spiout, cs_clr
  
        ' handle the mux width
        test    xmem_param1, #MUX_WIDTH_MASK wz
  if_nz mov     t2, xmem_param2
  if_nz shr     t2, #8
  if_nz and     t2, #$ff
  if_nz mov     mask_inc, #1
  if_nz shl     mask_inc, t2
  if_nz sub     mask_inc, #1
  if_nz or      spidir, mask_inc
  
        ' handle the C3-style address or mux value
        test    xmem_param1, #ADDR_MASK wz
  if_nz mov     select_addr, xmem_param2
  if_nz and     select_addr, #$ff

        ' handle the C3-style INC pin
        mov     t2, xmem_param2
        shr     t2, #16
        and     t2, #$ff
        test    xmem_param1, #INC_PIN_MASK wz
  if_nz mov     mask_inc, #1
  if_nz shl     mask_inc, t2
  if_nz mov     select, c3_select_jmp       ' We're in C3 mode, so replace select/release
  if_nz mov     release, c3_release_jmp     ' with the C3-aware routines
  if_nz or      spidir, mask_inc
 
        ' handle the mux start bit (must follow setting of select_addr and mask_inc)
        test    xmem_param1, #MUX_START_BIT_MASK wz
  if_nz shl     select_addr, t2
  if_nz shl     mask_inc, t2
  if_nz or      spidir, mask_inc
  
        ' set the pin directions
        mov     outa, spiout
        mov     dira, spidir
        call    #release
        
        ' select sequential access mode for the SRAM chip
        call    #select
        mov     data, ramseq
        mov     bits, #16
        call    #send
        
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
        mov     dira, spidir            ' setup the pins so we can use them
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
cmdbase         long    0       ' base of the array of mailboxes
cmdptr          long    0       ' pointer to the current mailbox

zero            long    0       ' zero constant
t1              long    0       ' temporary variable
t2              long    0       ' temporary variable
t3              long    0       ' temporary variable

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
        call    #send
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

'----------------------------------------------------------------------------------------------------
' SPI routines
'----------------------------------------------------------------------------------------------------

select                              ' Single-SPI and Parallel-DeMUX
        andn    outa, mask_inc
        or      outa, select_addr
        andn    outa, cs_clr
select_ret
        ret

release                             ' Single-SPI and Parallel-DeMUX
        or      outa, cs_clr
        andn    outa, mask_inc
release_ret
        ret

c3_select_jmp                       ' Serial-DeMUX Jumps
        jmp     #c3_select          ' Initialization copies these jumps
c3_release_jmp                      '   over the select and release
        jmp     #c3_release         '   when in C3 mode.

c3_select                           ' Serial-DeMUX
        andn    outa, cs_clr
        or      outa, cs_clr
        mov     c3tmp, select_addr
:loop   or      outa, mask_inc
        andn    outa, mask_inc
        djnz    c3tmp, #:loop
        jmp     select_ret

c3_release                          ' Serial-DeMUX
        andn    outa, cs_clr
        or      outa, cs_clr
        jmp     release_ret

c3tmp   long    0

spiSendByte
        shl     data, #24
        mov     bits, #8
send    rol     data, #1 wc
        muxc    outa, mosi_mask
        or      outa, sck_mask
        andn    outa, sck_mask
        djnz    bits, #send
        or      outa, mosi_mask
spiSendByte_ret
send_ret
        ret

spiRecvByte
        mov     data, #0
        mov     bits, #8
recv    or      outa, sck_mask
        test    miso_mask, ina wc
        rcl     data, #1
        andn    outa, sck_mask
        djnz    bits, #recv
spiRecvByte_ret
        ret

spidir      long    0
spiout      long    0

' spi pins
mosi_pin    long    0
mosi_mask   long    0
miso_mask   long    0
sck_mask    long    0

' chip select variables
cs_clr      long    0
mask_inc    long    0
select_addr long    0

' variables used by the spi send/receive functions
cmd         long    0
bytes       long    0
data        long    0
bits        long    0

' input parameters to read_bytes and write_bytes
extaddr     long    0               ' external address
hubaddr     long    0               ' hub address
count       long    0

' spi commands
read        long    $03000000       ' read command
write       long    $02000000       ' write command
ramseq      long    $01400000       ' %00000001_01000000 << 16 ' set sequential mode

' mask off the base address of external memory
sram_mask   long    $0000ffff

            FIT     496             ' out of 496
