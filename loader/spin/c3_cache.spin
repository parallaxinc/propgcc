{
  SPI SRAM and flash JCACHE driver for the Parallax C3
  by David Betz

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

#define BYTE_TRANSFERS

CON

  FLASH_MASK            = $10000000
  FLASH_OFFSET_MASK     = FLASH_MASK - 1

  CLR_PIN               = 25
  INC_PIN               = 8
  CLK_PIN               = 11
  MOSI_PIN              = 9
  MISO_PIN              = 10

  ' default cache dimensions
  DEFAULT_INDEX_WIDTH   = 5
  DEFAULT_OFFSET_WIDTH  = 7
  DEFAULT_CACHE_SIZE    = 1<<(DEFAULT_INDEX_WIDTH+DEFAULT_OFFSET_WIDTH+1)

  ' cache line tag flags
  EMPTY_BIT             = 30
  DIRTY_BIT             = 31

  ' address of CLKFREQ in hub RAM
  CLKFREQ_ADDR          = $0000

  ' SD commands
  CMD0_GO_IDLE_STATE      = $40|0
  CMD55_APP_CMD           = $40|55
  CMD17_READ_SINGLE_BLOCK = $40|17
  CMD24_WRITE_BLOCK       = $40|24
  ACMD41_SD_APP_OP_COND   = $40|41
  
OBJ
  int: "cache_interface"

PUB dummy

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
        mov     outa, spiout
        mov     dira, spidir

        ' select sequential access mode for the first SRAM chip
        call    #sram_chip1
        mov     data, ramseq
        mov     bits, #16
        call    #send

        ' select sequential access mode for the second SRAM chip
        call    #sram_chip2
        mov     data, ramseq
        mov     bits, #16
        call    #send

        call    #deselect

        ' unprotect the entire flash chip
        call    #write_enable
        call    #flash_chip
        mov     data, fwrstatus ' write zero to the status register
        mov     bits, #16
        call    #send
        call    #deselect

        ' get the clock frequency
        rdlong  sdFreq, #CLKFREQ_ADDR

        jmp     #vmflush

fillme  long    0[128-fillme]           ' first 128 cog locations are used for a direct mapped page table

        fit   128

        ' initialize the cache lines
vmflush movd    :flush, #0
        mov     t1, index_count
        shl     t1, #1                  ' clear both the flash and SRAM tags
:flush  mov     0-0, empty_mask
        add     :flush, dstinc
        djnz    t1, #:flush

        ' start the command loop
waitcmd mov     dira, #0                ' release the pins for other SPI clients
        wrlong  zero, pvmcmd
:wait   rdlong  vmpage, pvmcmd wz
  if_z  jmp     #:wait
        mov     dira, spidir            ' set the pins back so we can use them

        test    vmpage, #int#EXTEND_MASK wz ' test for an extended command
  if_z  jmp     #extend

        test    vmpage, flashbit wz     ' check for flash or SRAM access
        shr     vmpage, offset_width wc ' carry is now one for read and zero for write
        mov     set_dirty_bit, #0       ' make mask to set dirty bit on writes
        muxnc   set_dirty_bit, dirty_mask
        mov     line, vmpage            ' get the cache line index
        and     line, index_mask
  if_nz add     line, index_count       ' use upper entries for flash addresses
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
miss    movd    :test, line
        movd    :st, line
:test   test    0-0, dirty_mask wz
  if_z  jmp     #:rd                    ' current page is clean, just read new page
        mov     vmaddr, vmcurrent
        shl     vmaddr, offset_width
        call    #BWRITE                 ' write current page
:rd     mov     vmaddr, vmpage
        shl     vmaddr, offset_width
        call    #BREAD                  ' read new page
:st     mov     0-0, vmpage
miss_ret ret

extend  mov     vmaddr, vmpage
        shr     vmaddr, #8
        shr     vmpage, #2
        and     vmpage, #7
        add     vmpage, #dispatch
        jmp     vmpage

dispatch
        jmp     #erase_chip_handler
        jmp     #erase_4k_block_handler
        jmp     #write_data_handler
        jmp     #sd_init_handler
        jmp     #sd_read_handler
        jmp     #sd_write_handler
        jmp     #waitcmd
        jmp     #waitcmd

erase_chip_handler
        call    #write_enable
        call    #flash_chip
        mov     data, ferasechip
        mov     bits, #8
        call    #send
        call    #deselect
        call    #wait_until_done
        wrlong  data, pvmaddr
        jmp     #waitcmd

erase_4k_block_handler
        call    #write_enable
        call    #flash_chip
        mov     data, vmaddr
        or      data, ferase4kblk
        mov     bits, #32
        call    #send
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
        call    #flash_chip
        mov     data, vmaddr
        or      data, fprogram
        mov     bits, #32
        call    #send
:data   rdbyte  data, ptr
        shl     data, #24
        mov     bits, #8
        call    #send
        add     ptr, #1
        add     vmaddr, #1
        djnz    count, #:loop
        call    #deselect

:done   call    #wait_until_done
        wrlong  data, pvmaddr
        jmp     #waitcmd

sd_init_handler
        mov     sdError, #0             ' assume no errors
        call    #deselect
        mov     t1, sdInitCnt
:init   call    #spiRecvByte            ' Output a stream of 32K clocks
        djnz    t1, #:init              '  in case SD card left in some
        call    #sd_card
        mov     sdOp, #CMD0_GO_IDLE_STATE
        mov     sdParam, #0
        call    #sdSendCmd              ' Send a reset command and deselect
:wait   mov     sdOp, #CMD55_APP_CMD
        call    #sdSendCmd
        mov     sdOp, #ACMD41_SD_APP_OP_COND
        call    #sdSendCmd
        cmp     data, #1 wz             ' Wait until response not In Idle
  if_e  jmp     #:wait
        tjz     data, #sd_finish        ' Initialization complete
        mov     sdError, data
        jmp     #sd_finish

sd_read_handler
        mov     sdError, #0             ' assume no errors
        rdlong  ptr, vmaddr             ' get the buffer pointer
        add     vmaddr, #4
        rdlong  count, vmaddr wz        ' get the byte count
  if_z  jmp     #sd_finish
        add     vmaddr, #4
        rdlong  vmaddr, vmaddr          ' get the sector address
        call    #sd_card                ' Read from specified block
        mov     sdOp, #CMD17_READ_SINGLE_BLOCK
:readRepeat
        mov     sdParam, vmaddr
        call    #sdSendCmd              ' Read from specified block
        call    #sdResponse
        mov     sdBlkCnt, sdBlkSize     ' Transfer a block at a time
:getRead
        call    #spiRecvByte
        tjz     count, #:skipStore      ' Check for count exhausted
        wrbyte  data, ptr
        add     ptr, #1
        sub     count, #1
:skipStore
        djnz    sdBlkCnt, #:getRead     ' Are we done with the block?
        call    #spiRecvByte
        call    #spiRecvByte            ' Yes, finish with 16 clocks
        add     vmaddr, #1
        tjnz    count, #:readRepeat     '  and check for more blocks to do
        jmp     #sd_finish

sd_write_handler
        mov     sdError, #0             ' assume no errors
        rdlong  ptr, vmaddr             ' get the buffer pointer
        add     vmaddr, #4
        rdlong  count, vmaddr wz        ' get the byte count
  if_z  jmp     #sd_finish
        add     vmaddr, #4
        rdlong  vmaddr, vmaddr         ' get the sector address
        call    #sd_card                ' Write to specified block
        mov     sdOp, #CMD24_WRITE_BLOCK
:writeRepeat
        mov     sdParam, vmaddr
        call    #sdSendCmd              ' Write to specified block
        mov     data, #$fe              ' Ask to start data transfer
        call    #spiSendByte
        mov     sdBlkCnt, sdBlkSize     ' Transfer a block at a time
:putWrite
        mov     data, #0                '  padding with zeroes if needed
        tjz     count, #:padWrite       ' Check for count exhausted
        rdbyte  data, ptr               ' If not, get the next data byte
        add     ptr, #1
        sub     count, #1
:padWrite
        call    #spiSendByte
        djnz    sdBlkCnt, #:putWrite    ' Are we done with the block?
        call    #spiRecvByte
        call    #spiRecvByte            ' Yes, finish with 16 clocks
        call    #sdResponse
        and     data, #$1f              ' Check the response status
        cmp     data, #5 wz
  if_ne mov     sdError, #1             ' Must be Data Accepted
  if_ne jmp     #sd_finish
        movs    sdWaitData, #0          ' Wait until not busy
        call    #sdWaitBusy
        add     vmaddr, #1
        tjnz    count, #:writeRepeat    '  to next if more data remains
sd_finish
        call    #deselect
        wrlong  sdError, pvmaddr        ' return error status
        jmp     #waitcmd

sdOp          long    0
sdParam       long    0
sdFreq        long    0
sdTime        long    0
sdError       long    0
sdBlkCnt      long    0
sdInitCnt     long    32768 / 8      ' Initial SPI clocks produced
sdBlkSize     long    512            ' Number of bytes in an SD block

sdSendCmd
        call    #spiRecvByte         ' ?? selecting card and clocking
        mov     data, sdOp
        call    #spiSendByte
        mov     data, sdParam
        shr     data, #15            ' Supplied address is sector number
        call    #spiSendByte
        mov     data, sdParam        ' Send to SD card as byte address,
        shr     data, #7             '  in multiples of 512 bytes
        call    #spiSendByte
        mov     data, sdParam        ' Total length of this address is
        shl     data, #1             '  four bytes
        call    #spiSendByte
        mov     data, #0
        call    #spiSendByte
        mov     data, #$95           ' CRC code (for 1st command only)
        call    #spiSendByte
sdResponse
        movs    sdWaitData, #$ff     ' Wait for response from card
sdWaitBusy
        mov     sdTime, cnt          ' Set up a 1 second timeout
sdWaitLoop
        call    #spiRecvByte
        mov     t1, cnt
        sub     t1, sdTime           ' Check for expired timeout (1 sec)
        cmp     t1, sdFreq wc
  if_nc mov     sdError, #1
  if_nc jmp     #sd_finish
sdWaitData
        cmp     data, #0-0 wz        ' Wait for some other response
  if_e  jmp     #sdWaitLoop          '  than that specified
sdSendCmd_ret
sdResponse_ret
sdWaitBusy_ret
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
        test    vmaddr, bit16 wz
  if_z  call    #sram_chip1      ' select first SRAM chip
  if_nz call    #sram_chip2      ' select second SRAM chip

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
        test    vmaddr, flashbit wz
  if_nz jmp     #FLASH_READ

        mov     fn, read
        call    #BSTART

BREAD_DATA
#ifdef BYTE_TRANSFERS
read0   call    #spiRecvByte
        wrbyte  data, ptr
        add     ptr, #1
        djnz    count, #read0
#else
read0   mov     bits, #32
        call    #receive
        wrlong  data, ptr
        add     ptr, #4
        sub     count, #4
        tjnz    count, #read0
#endif

        call    #deselect
BREAD_RET
        ret

FLASH_READ
        call    #flash_chip       ' select flash chip

        mov     data, vmaddr
        and     data, flashmask
        or      data, fread
        mov     bits, #40         ' includes 8 dummy bits
        call    #send

        mov     ptr, hubaddr      ' hubaddr = hub page address
        mov     count, line_size
        jmp     #BREAD_DATA

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
        test    vmaddr, flashbit wz
  if_nz jmp     BWRITE_RET

        mov     fn, write
        call    #BSTART

#ifdef BYTE_TRANSFERS
pw      rdbyte  data, ptr
        call    #spiSendByte
        add     ptr, #1
        djnz    count, #pw
#else
pw      rdlong  data, ptr
        mov     bits, #32
        call    #send
        add     ptr, #4
        sub     count, #4
        tjnz    count, #pw
#endif

        call    #deselect
BWRITE_RET
        ret

fn      long    0

'----------------------------------------------------------------------------------------------------
' SPI routines
'----------------------------------------------------------------------------------------------------

spiSendByte
        shl     data, #24
        mov     bits, #8
        jmp     #send

send0   andn    outa, TCLK
send    rol     data, #1 wc
        muxc    outa, TMOSI
        or      outa, TCLK
        djnz    bits, #send0
        andn    outa, TCLK
        or      outa, TMOSI
spiSendByte_ret
send_ret
        ret

spiRecvByte
        mov     data, #0
        mov     bits, #8
receive
gloop   or      outa, TCLK
        test    TMISO, ina wc
        rcl     data, #1
        andn    outa, TCLK
        djnz    bits, #gloop
spiRecvByte_ret
receive_ret
        ret

' spi select functions
' all trash t1

sd_card
        mov     t1, #5
        jmp     #select

sram_chip1
        mov     t1, #1
        jmp     #select

sram_chip2
        mov     t1, #2
        jmp     #select

flash_chip
        mov     t1, #3

select  andn    outa, TCLR
        or      outa, TCLR
:loop   or      outa, TINC
        andn    outa, TINC
        djnz    t1, #:loop
sd_card_ret
sram_chip1_ret
sram_chip2_ret
flash_chip_ret
        ret

deselect
        andn    outa, TCLR
        or      outa, TCLR
deselect_ret
        ret

write_enable
        call    #flash_chip
        mov     data, fwrenable
        mov     bits, #8
        call    #send
        call    #deselect
write_enable_ret
        ret

write_disable
        call    #flash_chip
        mov     data, fwrdisable
        mov     bits, #8
        call    #send
        call    #deselect
write_disable_ret
        ret

wait_until_done
        call    #flash_chip
        mov     data, frdstatus
        mov     bits, #8
        call    #send
:wait   mov     bits, #8
        call    #receive
        test    data, #1 wz
  if_nz jmp     #:wait
        call    #deselect
        and     data, #$ff
wait_until_done_ret
        ret

spidir      long    (1<<CLR_PIN)|(1<<INC_PIN)|(1<<CLK_PIN)|(1<<MOSI_PIN)
spiout      long    (1<<CLR_PIN)|(0<<INC_PIN)|(0<<CLK_PIN)|(0<<MOSI_PIN)

tclr        long    1<<CLR_PIN
tinc        long    1<<INC_PIN
tclk        long    1<<CLK_PIN
tmosi       long    1<<MOSI_PIN
tmiso       long    1<<MISO_PIN
tclk_mosi   long    (1<<CLK_PIN)|(1<<MOSI_PIN)

' input parameters to BREAD and BWRITE
vmaddr      long    0       ' virtual address
hubaddr     long    0       ' hub memory address to read from or write to

' temporaries used by BREAD and BWRITE
ptr         long    0
count       long    0

' temporaries used by send
bits        long    0
data        long    0

read        long    $03000000       ' read command
write       long    $02000000       ' write command
ramseq      long    $01400000       ' %00000001_01000000 << 16 ' set sequential mode
readstat    long    $05000000       ' read status

fread       long    $0b000000       ' flash read command
ferasechip  long    $60000000       ' flash erase chip
ferase4kblk long    $20000000       ' flash erase a 4k block
fprogram    long    $02000000       ' flash program byte/page
fwrenable   long    $06000000       ' flash write enable
fwrdisable  long    $04000000       ' flash write disable
frdstatus   long    $05000000       ' flash read status
fwrstatus   long    $01000000       ' flash write status

bit16       long    $00008000       ' mask to select the SRAM chip
flashbit    long    FLASH_MASK      ' mask to select flash vs. SRAM
flashmask   long    FLASH_OFFSET_MASK ' mask to isolate the flash offset bits

            FIT     496             ' out of 496
