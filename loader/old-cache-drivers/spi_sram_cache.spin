{
  SPI SRAM JCACHE driver
  Copyright (c) April 17, 2012 by David Betz

  Based on code from VMCOG - virtual memory server for the Propeller
  Copyright (c) February 3, 2010 by William Henning

  and on code from SdramCache
  Copyright (c) 2010 by John Steven Denson (jazzed)

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
 
  ' default cache dimensions
  DEFAULT_INDEX_WIDTH   = 7
  DEFAULT_OFFSET_WIDTH  = 6
  DEFAULT_CACHE_SIZE    = 1<<(DEFAULT_INDEX_WIDTH+DEFAULT_OFFSET_WIDTH)

  ' cache line tag flags
  EMPTY_BIT             = 30
  DIRTY_BIT             = 31

OBJ
  int: "cache_interface"

PUB image
  return @init_vm

DAT
        org   $0

' initialization structure offsets
' $0: pointer to a two word mailbox
' $4: pointer to where to store the cache lines in hub ram
' $8: 0xooiiccee - ss=mosi ii=miso cc=sck pp=protocol
' $a: 0xaabbccdd - aa=cs-or-clr bb=inc-or-start cc=width dd=addr
' note that $4 must be at least 2^($8+$a)*2 bytes in size
' the protocol byte is a bit mask with the bits defined above
'   if CS_CLR_PIN_MASK ($01) is set, then byte aa contains the CS or C3-style CLR pin number
'   if INC_PIN_MASK ($02) is set, then byte bb contains the C3-style INC pin number
'   if MUX_START_BIT_MASK ($04) is set, then byte bb contains the starting bit number of the mux field
'   if MUX_WIDTH_MASK ($08) is set, then byte cc contains the width of the mux field
'   if ADDR_MASK ($10) is set, then byte dd contains either the C3-style address or the value to write to the mux field
'   if QUAD_SPI_HACK_MASK ($20) is set, assume that pins miso+1 and miso+2 are /WP and /HOLD and assert them
' example:
'   for a simple single pin CS you should set the protocol byte to $01 and place the CS pin number in byte aa.
' the cache line mask is returned in $0

init_vm mov     t1, par             ' get the address of the initialization structure
        rdlong  pvmcmd, t1          ' pvmcmd is a pointer to the virtual address and read/write bit
        mov     pvmaddr, pvmcmd     ' pvmaddr is a pointer into the cache line on return
        add     pvmaddr, #4
        add     t1, #4
        rdlong  cacheptr, t1        ' cacheptr is the base address in hub ram of the cache
        add     t1, #4

        ' get the pin definitions (cache-param1)
        rdlong  t2, t1
        add     t1, #4

        ' build the mosi mask
        mov     t3, t2
        shr     t3, #24
        mov     mosi_mask, #1
        shl     mosi_mask, t3
        or      spidir, mosi_mask
        
        ' build the miso mask
        mov     t3, t2
        shr     t3, #16
        and     t3, #$ff
        mov     miso_mask, #1
        shl     miso_mask, t3
        
        ' build the sck mask
        mov     t3, t2
        shr     t3, #8
        and     t3, #$ff
        mov     sck_mask, #1
        shl     sck_mask, t3
        or      spidir, sck_mask
        
        ' get the cs protocol selector bits (cache-param2)
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
        
        mov     index_count, #1
        shl     index_count, index_width
        mov     index_mask, index_count
        sub     index_mask, #1

        mov     line_size, #1
        shl     line_size, offset_width
        mov     t1, line_size
        sub     t1, #1
        wrlong  t1, par

        ' set the pin directions
        mov     outa, spiout
        mov     dira, spidir
        call    #release
        
        ' select sequential access mode for the SRAM chip
        call    #select
        mov     data, ramseq
        mov     bits, #16
        call    #send

        jmp     #vmflush

fillme  long    0[128-fillme]           ' first 128 cog locations are used for a direct mapped page table

        fit   128

        ' initialize the cache lines
vmflush movd    :flush, #0
        mov     t1, index_count
:flush  mov     0-0, empty_mask
        add     :flush, dstinc
        djnz    t1, #:flush

        ' start the command loop
waitcmd mov     dira, #0                ' release the pins for other SPI clients
        wrlong  zero, pvmcmd
:wait   rdlong  vmpage, pvmcmd wz
  if_z  jmp     #:wait

        test    vmpage, #int#EXTEND_MASK wz ' test for an extended command
  if_z  jmp     #extend

        shr     vmpage, offset_width wc ' carry is now one for read and zero for write
        mov     set_dirty_bit, #0       ' make mask to set dirty bit on writes
        muxnc   set_dirty_bit, dirty_mask
        mov     line, vmpage            ' get the cache line index
        and     line, index_mask
        mov     hubaddr, line
        shl     hubaddr, offset_width
        add     hubaddr, cacheptr       ' get the address of the cache line
        wrlong  hubaddr, pvmaddr        ' return the address of the cache line
        movs    :ld, line
        movd    :st, line
:ld     mov     vmcurrent, 0-0          ' get the cache line tag
        and     vmcurrent, tag_mask
        cmp     vmcurrent, vmpage wz    ' z set means there was a cache hit
  if_nz call    #miss                   ' handle a cache miss
:st     or      0-0, set_dirty_bit      ' set the dirty bit on writes
        jmp     #waitcmd                ' wait for a new command

' line is the cache line index
' vmcurrent is current page
' vmpage is new page
' hubaddr is the address of the cache line
miss    movd    mtest, line
        movd    mst, line
lck_spi test    $, #0 wc                ' lock no-op: clear the carry bit
   if_c jmp     #lck_spi
        mov     dira, spidir            ' set the pins back so we can use them
mtest   test    0-0, dirty_mask wz
  if_z  jmp     #:rd                    ' current page is clean, just read new page
        mov     vmaddr, vmcurrent
        shl     vmaddr, offset_width
        call    #BWRITE                 ' write current page
:rd     mov     vmaddr, vmpage
        shl     vmaddr, offset_width
        call    #BREAD                  ' read new page
        mov     dira, #0                ' release the pins for other SPI clients
nlk_spi nop        
mst     mov     0-0, vmpage
miss_ret ret

extend  mov     vmaddr, vmpage
        shr     vmaddr, #8
        shr     vmpage, #2
        and     vmpage, #7
        add     vmpage, #dispatch
        mov     dira, spidir            ' set the pins back so we can use them
        jmp     vmpage

dispatch
        jmp     #waitcmd
        jmp     #waitcmd
        jmp     #waitcmd
        jmp     #waitcmd
        jmp     #waitcmd
        jmp     #waitcmd
        jmp     #waitcmd
'       jmp     #lock_set_handler - This is the next instruction - no need to waste a long

' Note that we only provide SD locks for the cache operations - the other
' operations are specific to the sd_cache_loader's use of the cache driver, and
' there's no need to provide SPI bus-locking services there.

lock_set_handler
        mov     lock_id, vmaddr
        mov     lck_spi, lock_set
        mov     nlk_spi, lock_clr
        jmp     #waitcmd
lock_set
        lockset lock_id wc
lock_clr
        lockclr lock_id
lock_id long    0               ' lock id for optional bus interlock

' spi commands

' pointers to mailbox entries
pvmcmd          long    0       ' on call this is the virtual address and read/write bit
pvmaddr         long    0       ' on return this is the address of the cache line containing the virtual address

cacheptr        long    0       ' address in hub ram where cache lines are stored
vmpage          long    0       ' page containing the virtual address
vmcurrent       long    0       ' current page in selected cache line (same as vmpage on a cache hit)
line            long    0       ' current cache line index
set_dirty_bit   long    0       ' DIRTY_BIT set on writes, clear on reads

zero            long    0       ' zero constant
dstinc          long    1<<9    ' increment for the destination field of an instruction
t1              long    0       ' temporary variable
t2              long    0       ' temporary variable
t3              long    0       ' temporary variable
t4              long    0       ' temporary variable

tag_mask        long    (1<<DIRTY_BIT)-1        ' includes EMPTY_BIT
index_width     long    DEFAULT_INDEX_WIDTH
index_mask      long    0
index_count     long    0
offset_width    long    DEFAULT_OFFSET_WIDTH
line_size       long    0                       ' line size in bytes
empty_mask      long    (1<<EMPTY_BIT)
dirty_mask      long    (1<<DIRTY_BIT)

'----------------------------------------------------------------------------------------------------
'
' BSTART
'
' select the chip and send the address for the read/write operation
'
' on input:
'   vmaddr is the sram transfer address
'
' on output:
'   ptr is the hub address for the next read/write
'   count is the number of bytes to transfer
'
' trashes t1
'
'----------------------------------------------------------------------------------------------------

BSTART
        call    #select          ' select SRAM chip

        mov     data, vmaddr
        shl     data, #8          ' move it into position for transmission
        or      data, fn
        mov     bits, #24
        call    #send

        mov     ptr, hubaddr      ' hubaddr = hub page address
        mov     count, line_size
BSTART_RET
        ret

'----------------------------------------------------------------------------------------------------
'
' BREAD
'
' on input:
'   vmaddr is the virtual memory address to read
'   hubaddr is the hub memory address to write
'   count is the number of longs to read
'
' trashes fn, count, bits, data, ptr, count, t1, c and z flags
'
'----------------------------------------------------------------------------------------------------

BREAD
        mov     fn, read
        call    #BSTART 

:loop   call    #spiRecvByte
        wrbyte  data, ptr
        add     ptr, #1
        djnz    count, #:loop

        call    #release
BREAD_RET
        ret

'----------------------------------------------------------------------------------------------------
'
' BWRITE
'
' on input:
'   vmaddr is the virtual memory address to write
'   hubaddr is the hub memory address to read
'   count is the number of longs to write
'
' trashes fn, count, bits, data, ptr, count, z flag
'
'----------------------------------------------------------------------------------------------------

BWRITE
        mov     fn, write
        call    #BSTART

:loop   rdbyte  data, ptr
        call    #spiSendByte
        add     ptr, #1
        djnz    count, #:loop

        call    #release
BWRITE_RET
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

spidir          long    0
spiout          long    0

mosi_mask       long    0
miso_mask       long    0
sck_mask        long    0

cs_clr          long    0
mask_inc        long    0
select_addr     long    0

' variables used by the spi send/receive functions
cmd         long    0
bytes       long    0
data        long    0
bits        long    0

' input parameters to BREAD and BWRITE
vmaddr      long    0       ' virtual address
hubaddr     long    0       ' hub memory address to read from or write to

' temporaries used by BREAD and BWRITE
ptr         long    0
count       long    0

' spi commands

read        long    $03000000       ' read command
write       long    $02000000       ' write command
ramseq      long    $01400000       ' %00000001_01000000 << 16 ' set sequential mode

            FIT     496             ' out of 496
