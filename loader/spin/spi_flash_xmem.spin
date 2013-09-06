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

#define SST

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

' initialization structure offsets
' $0: pointer to an array of mailboxes terminated by a long containing $00000008
' $4: $ooiiccee - oo=mosi ii=miso cc=sck pp=protocol
' $8: $aabbccdd - aa=cs-or-clr bb=inc-or-start cc=width dd=addr
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
        mov     t1, par             ' get the address of the initialization structure
        rdlong  cmdbase, t1         ' cmdbase is the base of an array of mailboxes
        add     t1, #4

        ' get the pin definitions
        rdlong  t2, t1
        add     t1, #4

        ' build the mosi mask
        mov     mosi_pin, t2
        shr     mosi_pin, #24
        mov     mosi_mask, #1
        shl     mosi_mask, mosi_pin
        or      spidir, mosi_mask
        
        ' build the miso mask
        mov     t3, t2
        shr     t3, #16
        and     t3, #$ff
        mov     miso_mask, #1
        shl     miso_mask, t3
        
        ' make the sio2 and sio3 pins outputs in single spi mode to assert /WP and /HOLD
        test    t2, #QUAD_SPI_HACK_MASK wz
  if_nz mov     t3, #$0c
  if_nz shl     t3, mosi_pin
  if_nz or      spidir, t3
  if_nz or      spiout, t3
        
        ' build the sck mask
        mov     t3, t2
        shr     t3, #8
        and     t3, #$ff
        mov     sck_mask, #1
        shl     sck_mask, t3
        or      spidir, sck_mask
        
        ' get the cs protocol selector bits
        rdlong  t3, t1
        
        ' handle the CS or C3-style CLR pins
        test    t2, #CS_CLR_PIN_MASK wz
  if_nz mov     t4, t3
  if_nz shr     t4, #24
  if_nz mov     cs_clr, #1
  if_nz shl     cs_clr, t4
  if_nz or      spidir, cs_clr
  if_nz or      spiout, cs_clr
  
        ' handle the mux width
        test    t2, #MUX_WIDTH_MASK wz
  if_nz mov     t4, t3
  if_nz shr     t4, #8
  if_nz and     t4, #$ff
  if_nz mov     mask_inc, #1
  if_nz shl     mask_inc, t4
  if_nz sub     mask_inc, #1
  if_nz or      spidir, mask_inc
  
        ' handle the C3-style address or mux value
        test    t2, #ADDR_MASK wz
  if_nz mov     select_addr, t3
  if_nz and     select_addr, #$ff

        ' handle the C3-style INC pin
        mov     t4, t3
        shr     t4, #16
        and     t4, #$ff
        test    t2, #INC_PIN_MASK wz
  if_nz mov     mask_inc, #1
  if_nz shl     mask_inc, t4
  if_nz mov     select, c3_select_jmp       ' We're in C3 mode, so replace select/release
  if_nz mov     release, c3_release_jmp     ' with the C3-aware routines
  if_nz or      spidir, mask_inc
 
        ' handle the mux start bit (must follow setting of select_addr and mask_inc)
        test    t2, #MUX_START_BIT_MASK wz
  if_nz shl     select_addr, t4
  if_nz shl     mask_inc, t4
  if_nz or      spidir, mask_inc
  
        ' signal that we're done with initialization
        wrlong  zero, PAR
        
        ' set the pin directions
        mov     outa, spiout
        mov     dira, spidir
        call    #release
        
        ' clear the status register
        call    #clear_status_reg
                
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
t4              long    0       ' temporary variable

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
        mov     cmd, extaddr
        and     cmd, flash_mask
        or      cmd, fread
        mov     bytes, #5
        call    #start_spi_cmd
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

#ifdef SST

write_bytes
        tjz     wrenable, write_bytes_ret
        tjz     extaddr, disable_writes
        test    extaddr, block_mask wz
  if_z  call    #erase_4k_block
:loop   call    #write_enable
        mov     cmd, extaddr
        and     cmd, flash_mask
        or      cmd, fprogram
        mov     bytes, #4
        call    #start_spi_cmd
:data   rdbyte  data, hubaddr
        call    #spiSendByte
        call    #release
        call    #wait_until_done
        add     extaddr, #1
        add     hubaddr, #1
        djnz    count, #:loop
write_bytes_ret
        ret
        
#else

write_bytes
        tjz     wrenable, write_bytes_ret
        tjz     extaddr, disable_writes
        test    extaddr, block_mask wz
  if_z  call    #erase_4k_block
        jmp     #:addr
:loop   test    extaddr, #$ff wz
  if_nz jmp     #:data
        call    #release
        call    #wait_until_done
:addr   call    #write_enable
        mov     cmd, extaddr
        and     cmd, flash_mask
        or      cmd, fprogram
        mov     bytes, #4
        call    #start_spi_cmd
:data   rdbyte  data, hubaddr
        call    #spiSendByte
        add     extaddr, #1
        add     hubaddr, #1
        djnz    count, #:loop
        call    #release
:done   call    #wait_until_done
write_bytes_ret
        ret
        
#endif

erase_4k_block
        call    #write_enable
        mov     cmd, extaddr
        and     cmd, flash_mask
        or      cmd, ferase4kblk
        mov     bytes, #4
        call    #start_spi_cmd
        call    #release
        call    #wait_until_done
erase_4k_block_ret
        ret
        
disable_writes
        mov     wrenable, #0
        jmp     write_bytes_ret

' spi commands

#ifdef SST

clear_status_reg
        mov     cmd, fwrsenable
        call    #start_spi_cmd_1
        call    #release
        mov     cmd, fwrstatus
        mov     bytes, #2
        call    #start_spi_cmd
        call    #release
clear_status_reg_ret
        ret

#else

clear_status_reg
        call    #write_enable
        mov     cmd, fwrstatus
        mov     bytes, #2
        call    #start_spi_cmd
        call    #release
clear_status_reg_ret
        ret

#endif

write_enable
        mov     cmd, fwrenable
        call    #start_spi_cmd_1
        call    #release
write_enable_ret
        ret

wait_until_done
        mov     cmd, frdstatus
        call    #start_spi_cmd_1
:wait   call    #spiRecvByte
        test    data, #1 wz
  if_nz jmp     #:wait
        call    #release
wait_until_done_ret
        ret

start_spi_cmd_1
        mov     bytes, #1
start_spi_cmd
        call    #select
:loop   rol     cmd, #8
        mov     data, cmd
        call    #spiSendByte
        djnz    bytes, #:loop
start_spi_cmd_1_ret
start_spi_cmd_ret
        ret
        
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
fprogram    long    $02000000       ' flash program byte/page
fread       long    $0b000000       ' flash read command
frdjedecid  long    $9f000000       ' read the manufacturers id, device type and device id
ferase4kblk long    $20000000       ' flash erase a 4k block
#ifdef SST
fwrsenable  long    $50000000       ' SST write status register enable
#endif
frdstatus   long    $05000000       ' flash read status
fwrstatus   long    $01000000       ' flash write status
fwrenable   long    $06000000       ' flash write enable

block_mask  long    $00000fff       ' 4k block address mask
flash_mask  long    $00ffffff       ' mask off the base address of external memory

wrenable    long    1

            FIT     496             ' out of 496
