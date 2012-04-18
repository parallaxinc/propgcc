{
  SPI flash JCACHE driver
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

  ' default pins
  MOSI_PIN              = 0
  MISO_PIN              = 1
  SCK_PIN               = 4
  CE_PIN                = 5
 
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
' $8: number of bits in the cache line index if non-zero (default is DEFAULT_INDEX_WIDTH)
' $a: number of bits in the cache line offset if non-zero (default is DEFAULT_OFFSET_WIDTH)
' note that $4 must be at least 2^($8+$a)*2 bytes in size
' the cache line mask is returned in $0

init_vm mov     t1, par             ' get the address of the initialization structure
        rdlong  pvmcmd, t1          ' pvmcmd is a pointer to the virtual address and read/write bit
        mov     pvmaddr, pvmcmd     ' pvmaddr is a pointer into the cache line on return
        add     pvmaddr, #4
        add     t1, #4
        rdlong  cacheptr, t1        ' cacheptr is the base address in hub ram of the cache
        add     t1, #4

        ' get the pin definitions (cache-param1)
        rdlong  t2, t1 wz
  if_z  jmp     #skip_pins

        ' build the mosi mask
        mov     t3, t2
        shr     t3, #24
        mov     mosi_mask, #1
        shl     mosi_mask, t3
        
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
        
        ' build the ce mask
        and     t2, #$ff
        mov     ce_mask, #1
        shl     ce_mask, t2
        
        ' build the outa and dira masks
        mov     spiout, ce_mask
        or      spiout, mosi_mask
        mov     spidir, spiout
        or      spidir, sck_mask
skip_pins
        or      spiout, #$0c  ' BUG: debugging code
        or      spidir, #$0c  ' BUG: debugging code

        ' get the jedec id (cache-param2)
        rdlong  t2, t1 wz
  if_z  mov     jedec_id, t2

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
        call    #deselect
        
        ' check the jedec id
        call    #read_jedec_id
        cmp     t1, jedec_id wz
halt
  if_nz jmp     #halt
        
        ' clear the status register
        call    #clear_status_reg
        
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
waitcmd 'mov     dira, #0                ' release the pins for other SPI clients
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
        'mov     dira, spidir            ' set the pins back so we can use them
mtest   test    0-0, dirty_mask wz
  if_z  jmp     #:rd                    ' current page is clean, just read new page
        mov     vmaddr, vmcurrent
        shl     vmaddr, offset_width
        call    #BWRITE                 ' write current page
:rd     mov     vmaddr, vmpage
        shl     vmaddr, offset_width
        call    #BREAD                  ' read new page
        'mov     dira, #0                ' release the pins for other SPI clients
nlk_spi nop        
mst     mov     0-0, vmpage
miss_ret ret

extend  mov     vmaddr, vmpage
        shr     vmaddr, #8
        shr     vmpage, #2
        and     vmpage, #7
        add     vmpage, #dispatch
        'mov     dira, spidir            ' set the pins back so we can use them
        jmp     vmpage

dispatch
        jmp     #waitcmd
        jmp     #erase_4k_block_handler
        jmp     #write_data_handler
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

erase_4k_block_handler
        call    #write_enable
        mov     cmd, vmaddr
        and     cmd, flashmask
        or      cmd, ferase4kblk
        mov     bytes, #4
        call    #start_spi_cmd
        call    #deselect
        call    #wait_until_done
        wrlong  data, pvmaddr
        jmp     #waitcmd

write_data_handler
        rdlong  ptr, vmaddr     ' get the buffer pointer
        add     vmaddr, #4
        rdlong  count, vmaddr wz' get the byte count
  if_z  jmp     #:done
        add     vmaddr, #4
        rdlong  vmaddr, vmaddr  ' get the flash address (zero based)
        jmp     #:addr

:loop   test    vmaddr, #$ff wz
  if_nz jmp     #:data
        call    #deselect
        call    #wait_until_done
:addr   call    #write_enable
        mov     cmd, vmaddr
        and     cmd, flashmask
        or      cmd, fprogram
        mov     bytes, #4
        call    #start_spi_cmd
:data   rdbyte  data, ptr
        call    #spiSendByte
        add     ptr, #1
        add     vmaddr, #1
        djnz    count, #:loop
        call    #deselect

:done   call    #wait_until_done
        wrlong  data, pvmaddr
        jmp     #waitcmd

' spi commands

read_jedec_id
        mov     cmd, frdjedecid
        call    #start_spi_cmd_1
        call    #spiRecvByte       ' manufacturer's id
        mov     t1, data
        ror     t1, #8             ' save mfg id
        call    #spiRecvByte       ' memory type
        movs    t1, data
        ror     t1, #8             ' save dev type
        call    #spiRecvByte       ' device capacity
        movs    t1, data           ' save dev type
        rol     t1, #16            ' data 00ccttmm c=capacity, t=type, m=mfgid
        call    #deselect
read_jedec_id_ret
        ret

clear_status_reg
        call    #write_enable
        mov     cmd, fwrstatus
        mov     bytes, #3
        call    #start_spi_cmd
        call    #deselect
        call    #wait_until_done
clear_status_reg_ret
        ret

write_enable
        mov     cmd, fwrenable
        call    #start_spi_cmd_1
        call    #deselect
write_enable_ret
        ret

wait_until_done
        mov     cmd, frdstatus
        call    #start_spi_cmd_1
:wait   call    #spiRecvByte
'        test    data, #$80 wz
        test    data, #1 wz
  if_nz jmp     #:wait
        call    #deselect
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
' BREAD
'
' on input:
'   vmaddr is the virtual memory address to read
'   hubaddr is the hub memory address to write
'   count is the number of longs to read
'
'----------------------------------------------------------------------------------------------------

BREAD
        mov     cmd, vmaddr
        and     cmd, flashmask
        or      cmd, fread
        mov     bytes, #5
        call    #start_spi_cmd

        mov     ptr, hubaddr      ' hubaddr = hub page address
        mov     count, line_size

read0   call    #spiRecvByte
        wrbyte  data, ptr
        add     ptr, #1
        djnz    count, #read0

        call    #deselect
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
'----------------------------------------------------------------------------------------------------

BWRITE
        ' no writes to flash
BWRITE_RET
        ret

'----------------------------------------------------------------------------------------------------
' SPI routines
'----------------------------------------------------------------------------------------------------

select
        andn    outa, ce_mask
select_ret
        ret

deselect
        or      outa, ce_mask
deselect_ret
        ret
        
spiSendByte
        shl     data, #24
        mov     bits, #8
        jmp     #send

send0   andn    outa, sck_mask

' CLK should be low coming into this function
send    rol     data, #1 wc
        muxc    outa, mosi_mask
        or      outa, sck_mask
        djnz    bits, #send0
        andn    outa, sck_mask
        or      outa, mosi_mask
spiSendByte_ret
send_ret
        ret

spiRecvByte
        mov     data, #0
        mov     bits, #8
        
' CLK was set H-L and data should be ready before this function starts
receive or      outa, sck_mask
        test    miso_mask, ina wc
        rcl     data, #1
        andn    outa, sck_mask
        djnz    bits, #receive
spiRecvByte_ret
receive_ret
        ret

spidir      long    (1<<CE_PIN)|(1<<SCK_PIN)|(1<<MOSI_PIN)
spiout      long    (1<<CE_PIN)|(0<<SCK_PIN)|(1<<MOSI_PIN)

mosi_mask   long    1<<MOSI_PIN
miso_mask   long    1<<MISO_PIN
sck_mask    long    1<<SCK_PIN
ce_mask     long    1<<CE_MASK

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

jedec_id    long    $001440ef       ' value of t1 after read_jedec_id routine (W25Q80BV)

' spi commands

' quad spi commands
fprogram    long    $02000000       ' flash program byte/page
fread       long    $0b000000       ' flash read command
frdjedecid  long    $9f000000       ' read the manufacturers id, device type and device id
ferase4kblk long    $20000000       ' flash erase a 4k block
frdstatus   long    $05000000       ' flash read status
fwrstatus   long    $01000000       ' flash write status
fwrenable   long    $06000000       ' flash write enable

flashmask   long    $00ffffff       ' mask to isolate the flash offset bits

ledmask     long    1<<15           ' for debugging

            FIT     496             ' out of 496
