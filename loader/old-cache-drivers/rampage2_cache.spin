{
  RamPage2 flash JCACHE driver
  Copyright (c) November 21, 2012 by David Betz

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

  ' default cache dimensions
  DEFAULT_INDEX_WIDTH   = 7
  DEFAULT_OFFSET_WIDTH  = 6
  DEFAULT_CACHE_SIZE    = 1<<(DEFAULT_INDEX_WIDTH+DEFAULT_OFFSET_WIDTH)

  ' cache line tag flags
  EMPTY_BIT             = 30
  DIRTY_BIT             = 31
  
  ' SPI pins
  SIO0_PIN              = 0
  SCK_PIN               = 8
  CS_PIN                = 9
  
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
        rdlong  t2, t1 wz
  if_nz mov     index_width, t2     ' override the index_width default value
        add     t1, #4
        rdlong  t2, t1 wz
  if_nz mov     offset_width, t2    ' override the offset_width default value

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
        call    #release
                
        ' initialize the flash
        call    #sst_read_jedec_id
        cmp     t1, jedec_id_1 wz
  if_nz jmp     #:next
        cmp     t2, jedec_id_2 wz
  if_nz jmp     #:next
        cmp     t3, jedec_id_3 wz
  if_z  jmp     #:unprot
:next   call    #read_jedec_id
        cmp     t1, jedec_id wz
  if_nz jmp     #halt
        cmp     t2, jedec_id wz
  if_nz jmp     #halt
        call    #select
        mov     data, sst_quadmode
        call    #spiSendByte
        call    #release
        call    #sst_read_jedec_id
        cmp     t1, jedec_id_1 wz
  if_nz jmp     #halt
        cmp     t2, jedec_id_2 wz
  if_nz jmp     #halt
        cmp     t3, jedec_id_3 wz
  if_nz jmp     #halt
:unprot call    #sst_write_enable
        mov     cmd, sst_wrblkprot
        call    #sst_start_sqi_cmd_1
        andn    outa, sck_mask
        mov     data, #0
        call    #sst_sqi_write_word
        call    #sst_sqi_write_word
        call    #sst_sqi_write_word
        call    #sst_sqi_write_word
        call    #sst_sqi_write_word
        call    #sst_sqi_write_word
        call    #release
                
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

halt    jmp     #halt

extend  mov     vmaddr, vmpage
        shr     vmaddr, #8
        shr     vmpage, #2
        and     vmpage, #7
        add     vmpage, #dispatch
        mov     dira, spidir            ' set the pins back so we can use them
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
        call    #erase_4k_block
        wrlong  data, pvmaddr
        jmp     #waitcmd

write_data_handler
        rdlong  hubaddr, vmaddr         ' get the buffer pointer
        add     vmaddr, #4
        rdlong  remaining, vmaddr wz    ' get the byte count
  if_z  mov     data, #0
  if_z  jmp     #:done
        add     vmaddr, #4
        rdlong  vmaddr, vmaddr          ' get the flash address (zero based)
        
        ' round to an even number of words
        add     remaining, #1
        andn    remaining, #1
        
        ' compute the count to the end of this 256 byte block
        mov     t1, vmaddr
        add     t1, #256
        andn    t1, #255
        sub     t1, vmaddr
        max     t1, remaining
        
:loop   call    #sst_write_enable
        mov     cmd, vmaddr
        and     cmd, flashmask
        shr     cmd, #1
        or      cmd, sst_program
        mov     bytes, #4
        mov     count, t1
        call    #sst_sqi_write
        call    #wait_until_done
        sub     remaining, t1 wz
 if_z   jmp     #:done
        add     vmaddr, t1
        mov     t1, remaining
        max     t1, #256
        jmp     #:loop
        
:done   wrlong  data, pvmaddr
        jmp     #waitcmd

' spi commands

mask_4k long    $00000fff

erase_4k_block
        shr     vmaddr, #1
        test    vmaddr, mask_4k wz
 if_nz  jmp     erase_4k_block_ret
        call    #sst_write_enable
        mov     cmd, vmaddr
        and     cmd, flashmask
        or      cmd, ferase4kblk
        mov     bytes, #4
        call    #sst_start_sqi_cmd
        call    #release
        call    #wait_until_done
erase_4k_block_ret
        ret

read_jedec_id
        call    #select
        mov     data, frdjedecid
        call    #spiSendByte
        call    #spiRecvByte
        mov     t1, data_hi
        shl     t1, #8
        mov     t2, data_lo
        shl     t2, #8
        call    #spiRecvByte
        or      t1, data_hi
        shl     t1, #8
        or      t2, data_lo
        shl     t2, #8
        call    #spiRecvByte
        or      t1, data_hi
        or      t2, data_lo
        call    #release
read_jedec_id_ret
        ret
        
sst_read_jedec_id
        mov     cmd, sst_rdjedecid
        call    #sst_start_sqi_cmd_1
        andn    dira, sio_mask
        andn    outa, sck_mask
        call    #sst_sqi_read_word
        mov     t1, data
        call    #sst_sqi_read_word
        mov     t2, data
        call    #sst_sqi_read_word
        mov     t3, data
        call    #release
sst_read_jedec_id_ret
        ret
        
sst_write_enable
        mov     cmd, fwrenable
        call    #sst_start_sqi_cmd_1
        call    #release
sst_write_enable_ret
        ret

sst_sqi_write_word
        mov     sio_t1, data
        shr     sio_t1, #8
        shl     sio_t1, sio_shift
        and     sio_t1, sio_mask
        andn    outa, sio_mask
        or      outa, sio_t1
        or      outa, sck_mask
        andn    outa, sck_mask
        shl     data, sio_shift
        and     data, sio_mask
        andn    outa, sio_mask
        or      outa, data
        or      outa, sck_mask
        andn    outa, sck_mask
sst_sqi_write_word_ret
        ret

sst_sqi_write
        call    #sst_start_sqi_cmd
        andn    outa, sck_mask
        tjz     count, #:done
:loop   rdbyte  data, hubaddr
        add     hubaddr, #1
        shl     data, sio_shift
        andn    outa, sio_mask
        or      outa, data
        or      outa, sck_mask
        andn    outa, sck_mask
        djnz    count, #:loop
:done   call    #release
sst_sqi_write_ret
        ret

sst_sqi_read_word
        or      outa, sck_mask
        mov     data, ina
        andn    outa, sck_mask
        and     data, sio_mask
        shr     data, sio_shift
        shl     data, #8
        or      outa, sck_mask
        mov     sio_t1, ina
        andn    outa, sck_mask
        and     sio_t1, sio_mask
        shr     sio_t1, sio_shift
        or      data, sio_t1
sst_sqi_read_word_ret
        ret

sst_sqi_read
        call    #sst_start_sqi_cmd
        andn    outa, sck_mask
        andn    outa, sio_mask
        or      outa, sck_mask  ' hi dummy nibble
        andn    outa, sck_mask
        or      outa, sck_mask  ' lo dummy nibble
        andn    dira, sio_mask
        andn    outa, sck_mask
        tjz     count, #:done
:loop   or      outa, sck_mask
        mov     data, ina
        andn    outa, sck_mask
        shr     data, sio_shift
        wrbyte  data, hubaddr
        add     hubaddr, #1
        djnz    count, #:loop
:done   call    #release
sst_sqi_read_ret
        ret

sst_start_sqi_cmd_1
        mov     bytes, #1
        
sst_start_sqi_cmd
        or      dira, sio_mask      ' set data pins to outputs
        call    #select             ' select the chip
:loop   rol     cmd, #8
        mov     sio_t1, cmd         ' send the high nibble
        and     sio_t1, #$f0
        shl     sio_t1, sio_shift
        mov     sio_t2, sio_t1
        shr     sio_t2, #4
        or      sio_t1, sio_t2
        andn    outa, sio_mask
        or      outa, sio_t1
        or      outa, sck_mask
        andn    outa, sck_mask
        mov     sio_t1, cmd         ' send the low nibble
        and     sio_t1, #$0f
        shl     sio_t1, sio_shift
        mov     sio_t2, sio_t1
        shl     sio_t2, #4
        or      sio_t1, sio_t2
        andn    outa, sio_mask
        or      outa, sio_t1
        or      outa, sck_mask
        cmp     bytes, #1 wz
 if_nz  andn    outa, sck_mask
        djnz    bytes, #:loop
sst_start_sqi_cmd_1_ret
sst_start_sqi_cmd_ret
        ret
        
wait_until_done
        mov     cmd, frdstatus
        call    #sst_start_sqi_cmd_1
        andn    dira, sio_mask
        andn    outa, sck_mask
:wait   call    #sst_sqi_read_word
        test    data, busy_bits wz
  if_nz jmp     #:wait
        call    #release
wait_until_done_ret
        ret

sio_t1          long    0
sio_t2          long    0
busy_bits       long    $8800

jedec_id            long    $00bf2601    ' SST26VF016
jedec_id_1          long    $bbff
jedec_id_2          long    $2266
jedec_id_3          long    $0011

sst_rdjedecid       long    $af000000    ' read the manufacturers id, device type and device id
sst_quadmode        long    $38          ' enable quad mode
sst_wrblkprot       long    $42000000    ' write block protect register
sst_program         long    $02000000    ' flash program byte/page
sst_read            long    $0b000000    ' flash read command

' **********************************
' END OF CHIP SPECIFIC SPI FUNCTIONS
' **********************************

frdjedecid          long    $9f          ' read the manufacturers id, device type and device id
ferase4kblk         long    $20000000    ' flash erase a 4k block
frdstatus           long    $05000000    ' flash read status
fwrenable           long    $06000000    ' flash write enable

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
'
'----------------------------------------------------------------------------------------------------

BREAD
        mov     cmd, vmaddr
        and     cmd, flashmask
        shr     cmd, #1
        or      cmd, sst_read
        mov     bytes, #4
        mov     count, line_size
        call    #sst_sqi_read
BREAD_RET
        ret

'----------------------------------------------------------------------------------------------------
'
' BWRITE
'
' on input:
'   vmaddr is the virtual memory address to write
'   hubaddr is the hub memory address to read
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
        andn    outa, cs_mask
select_ret
        ret

release
        mov     outa, spiout
        mov     dira, spidir
release_ret
        ret
        
spiSendByte
        shl     data, #24
        mov     sio_t1, #8
:loop   rol     data, #1 wc
        muxc    outa, mosi_lo_mask
        muxc    outa, mosi_hi_mask
        or      outa, sck_mask
        andn    outa, sck_mask
        djnz    sio_t1, #:loop
spiSendByte_ret
        ret

spiRecvByte
        mov     data_lo, #0
        mov     data_hi, #0
        mov     sio_t1, #8
:loop   or      outa, sck_mask
        test    miso_lo_mask, ina wc
        rcl     data_lo, #1
        test    miso_hi_mask, ina wc
        rcl     data_hi, #1
        andn    outa, sck_mask
        djnz    sio_t1, #:loop
spiRecvByte_ret
        ret

' mosi_lo, mosi_hi, sck, cs
spidir          long    (1 << SIO0_PIN) | (1 << (SIO0_PIN + 4)) | (1 << SCK_PIN) | (1 << CS_PIN)

' mosi_lo, mosi_hi, cs
spiout          long    (1 << SIO0_PIN) | (1 << (SIO0_PIN + 4)) | (1 << CS_PIN)

mosi_lo_mask    long    1 << SIO0_PIN
miso_lo_mask    long    2 << SIO0_PIN
mosi_hi_mask    long    1 << (SIO0_PIN + 4)
miso_hi_mask    long    2 << (SIO0_PIN + 4)
sck_mask        long    1 << SCK_PIN
cs_mask         long    1 << CS_PIN
sio_mask        long    $ff << SIO0_PIN
sio_shift       long    SIO0_PIN

' variables used by the spi send/receive functions
cmd         long    0
bytes       long    0
data        long    0
data_lo     long    0
data_hi     long    0

' input parameters to BREAD and BWRITE
vmaddr      long    0       ' virtual address
hubaddr     long    0       ' hub memory address to read from or write to

count       long    0
remaining   long    0

flashmask   long    $00ffffff       ' mask to isolate the flash offset bits

            FIT     496             ' out of 496
