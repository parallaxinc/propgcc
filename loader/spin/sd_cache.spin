{
  SD JCACHE external RAM driver
  Copyright (c) 2011 by David Betz

  Based on code by Steve Denson (jazzed)
  Copyright (c) 2010 by John Steven Denson

  Inspired by VMCOG - virtual memory server for the Propeller
  Copyright (c) February 3, 2010 by William Henning

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

  ' these defaults are for the PropBOE
  MISO_PIN                = 11
  CLK_PIN                 = 12
  MOSI_PIN                = 13
  CS_PIN                  = 14

  ' SD card sector dimensions
  SECTOR_WIDTH            = 9   ' 512 byte sectors
  SECTOR_SIZE             = 1<<SECTOR_WIDTH

  ' default cache dimensions
  DEFAULT_INDEX_WIDTH     = 4
  DEFAULT_OFFSET_WIDTH    = SECTOR_WIDTH

  ' FAT cluster dimensions
  DEFAULT_CLUSTER_WIDTH   = 5   ' 32 sector clusters

  ' cache line tag flags
  EMPTY_BIT               = 30
  DIRTY_BIT               = 31
  
  ' cluster map dimensions
  CLUSTER_MAP_WIDTH       = 5
  CLUSTER_MAP_SIZE        = 1<<CLUSTER_MAP_WIDTH
  CLUSTER_MAP_MASK        = CLUSTER_MAP_SIZE - 1

  ' address of CLKFREQ in hub RAM
  CLKFREQ_ADDR            = $0000

  ' SD commands
  CMD0_GO_IDLE_STATE      = $40|0
  CMD55_APP_CMD           = $40|55
  CMD17_READ_SINGLE_BLOCK = $40|17
  CMD24_WRITE_BLOCK       = $40|24
  ACMD41_SD_APP_OP_COND   = $40|41

OBJ
  int: "cache_interface"

PUB image
  return @init

DAT
        org   $0

' initialization structure offsets
' $0: pointer to a two word mailbox
' $4: pointer to where to store the cache lines in hub ram
' $8: (<# bits in the cache line index><<8) | <# bits in the cache line offset> if non-zero (default is (DEFAULT_INDEX_WIDTH<<8)|DEFAULT_OFFSET_WIDTH)
' $a: number of bits in the cluster mask if non-zero (default is DEFAULT_CLUSTER_WIDTH)
' note that $4 must be at least 2^(index_width+offset_width) bytes in size
' the cache line mask is returned in $0

init    jmp     #init2
        long    @params - @init

init2   mov     t1, par             ' get the address of the initialization structure
        rdlong  pvmcmd, t1          ' pvmcmd is a pointer to the virtual address and read/write bit
        mov     pvmaddr, pvmcmd     ' pvmaddr is a pointer into the cache line on return
        add     pvmaddr, #4
        add     t1, #4
        rdlong  cacheptr, t1        ' cacheptr is the base address in hub ram of the cache
        add     t1, #4
        rdlong  t2, t1 wz
  if_nz mov     index_width, t2     ' override the index_width default value
  if_nz and     index_width, #$ff
  if_nz shr     t2, #8
  if_nz mov     offset_width, t2    ' override the offset_width default value
        add     t1, #4
        rdlong  t2, t1 wz
  if_nz mov     cluster_width, t2   ' override the cluster_width default value

        mov     index_count, #1
        shl     index_count, index_width
        mov     index_mask, index_count
        sub     index_mask, #1

        mov     line_size, #1
        shl     line_size, offset_width
        mov     t1, line_size
        sub     t1, #1
        wrlong  t1, par

        mov     cluster_mask, #1
        shl     cluster_mask, cluster_width
        sub     cluster_mask, #1

        ' build composite masks
        mov     tclk_mosi, tmosi
        or      tclk_mosi, tclk
        mov     spidir, tcs
        or      spidir, tclk
        or      spidir, tmosi

        ' disable the chip select
        call    #sd_release

        ' get the clock frequency
        rdlong  sdFreq, #CLKFREQ_ADDR

        jmp     #vmflush

fillme  long    0[128-fillme]           ' first 128 cog locations are used for a direct mapped cache table

        fit   128

' these values get patched by the loader
params
tmiso         long    1<<MISO_PIN
tclk          long    1<<CLK_PIN
tmosi         long    1<<MOSI_PIN
tcs           long    1<<CS_PIN
clusters      long    0[CLUSTER_MAP_SIZE]

        ' initialize the cache lines
vmflush movd    :flush, #0
        mov     t1, index_count
:flush  mov     0-0, empty_mask
        add     :flush, dstinc
        djnz    t1, #:flush

        ' start the command loop
waitcmd wrlong  zero, pvmcmd
:wait   rdlong  vmline, pvmcmd wz
  if_z  jmp     #:wait

        test    vmpage, #int#EXTEND_MASK wz ' test for an extended command
  if_z  jmp     #extend

        shr     vmline, offset_width wc ' carry is now one for read and zero for write
        mov     set_dirty_bit, #0       ' make mask to set dirty bit on writes
        muxnc   set_dirty_bit, dirty_mask
        mov     line, vmline            ' get the cache line index
        and     line, index_mask
        mov     hubaddr, line
        shl     hubaddr, offset_width
        add     hubaddr, cacheptr       ' get the address of the cache line
        wrlong  hubaddr, pvmaddr        ' return the address of the cache line
        movs    :ld, line
        movd    :st, line
:ld     mov     vmcurrent, 0-0          ' get the cache line tag
        and     vmcurrent, tag_mask
        cmp     vmcurrent, vmline wz    ' z set means there was a cache hit
  if_nz call    #miss                   ' handle a cache miss
:st     or      0-0, set_dirty_bit      ' set the dirty bit on writes
        jmp     #waitcmd                ' wait for a new command

' line is the cache line index
' vmcurrent is current cache line
' vmline is new cache line
' hubaddr is the address of the cache line
miss    movd    :test, line
        movd    :st, line
:test   test    0-0, dirty_mask wz
  if_z  jmp     #:rd                    ' current cache line is clean, just read new one
        mov     vmaddr, vmcurrent
        shl     vmaddr, offset_width
        call    #wr_cache_line          ' write current cache line
:rd     mov     vmaddr, vmline
        shl     vmaddr, offset_width
        call    #rd_cache_line          ' read new cache line
:st     mov     0-0, vmline
miss_ret ret

extend  mov     vmaddr, vmpage
        shr     vmaddr, #8
        shr     vmpage, #2
        and     vmpage, #7
        add     vmpage, #dispatch
        jmp     vmpage

dispatch
        jmp     #waitcmd
        jmp     #waitcmd
        jmp     #waitcmd
        jmp     #sd_init_handler
        jmp     #sd_read_handler
        jmp     #sd_write_handler
        jmp     #waitcmd
        jmp     #waitcmd

' pointers to mailbox entries
pvmcmd          long    0       ' on call this is the virtual address and read/write bit
pvmaddr         long    0       ' on return this is the address of the cache line containing the virtual address

cacheptr        long    0       ' address in hub ram where cache lines are stored
vmline          long    0       ' cache line containing the virtual address
vmcurrent       long    0       ' current selected cache line (same as vmline on a cache hit)
line            long    0       ' current cache line index
set_dirty_bit   long    0       ' DIRTY_BIT set on writes, clear on reads

zero            long    0       ' zero constant
dstinc          long    1<<9    ' increment for the destination field of an instruction
t1              long    0       ' temporary variable
t2              long    0       ' temporary variable
tag_mask        long    !(1<<DIRTY_BIT) ' includes EMPTY_BIT
index_width     long    DEFAULT_INDEX_WIDTH
index_mask      long    0
index_count     long    0
offset_width    long    DEFAULT_OFFSET_WIDTH
line_size       long    0                       ' line size in bytes
empty_mask      long    (1<<EMPTY_BIT)
dirty_mask      long    (1<<DIRTY_BIT)

' input parameters to rd_cache_line and wr_cache_line
vmaddr          long    0       ' external address
vmpage          long    0       ' page containing the external address
hubaddr         long    0       ' hub memory address

'----------------------------------------------------------------------------------------------------
'
' rd_cache_line - read a cache line from external memory
'
' vmaddr is the external memory address to read
' hubaddr is the hub memory address to write
' line_size is the number of bytes to read
'
'----------------------------------------------------------------------------------------------------

rd_cache_line
rd_cache_line_ret
        call    #get_physical_sector
        mov     count, line_size
        call    #sd_read
        ret

'----------------------------------------------------------------------------------------------------
'
' wr_cache_line - write a cache line to external memory
'
' vmaddr is the external memory address to write
' hubaddr is the hub memory address to read
' line_size is the number of bytes to write
'
'----------------------------------------------------------------------------------------------------

wr_cache_line
wr_cache_line_ret
        ret

cluster_width   long    DEFAULT_CLUSTER_WIDTH
cluster_mask    long    0

' on entry:
'   vmaddr - external memory address
' on return:
'   vmaddr - physical sector number containing the address
'   t1 - clobbered
get_physical_sector
        shr     vmaddr, #SECTOR_WIDTH
        mov     t1, vmaddr
        shr     vmaddr, cluster_width
        and     vmaddr, #CLUSTER_MAP_MASK
        add     vmaddr, #clusters
        movs    vmaddr, :rd
        and     t1, cluster_mask
:rd     mov     vmaddr, 0-0
        add     vmaddr, t1
get_physical_sector_ret
        ret

sd_init_handler
        mov     sdError, #0             ' assume no errors
        or      outa, tcs
        mov     t1, sdInitCnt
:init   call    #spiRecvByte            ' Output a stream of 32K clocks
        djnz    t1, #:init              '  in case SD card left in some
        andn    outa, tcs
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

sd_write_handler
        mov     sdError, #0             ' assume no errors
        rdlong  hubaddr, vmaddr         ' get the buffer pointer
        add     vmaddr, #4
        rdlong  count, vmaddr wz        ' get the byte count
  if_z  jmp     #sd_finish
        add     vmaddr, #4
        rdlong  vmaddr, vmaddr         ' get the sector address
        call    #sd_write
        jmp     #sd_finish

sd_read_handler
        mov     sdError, #0             ' assume no errors
        rdlong  hubaddr, vmaddr         ' get the buffer pointer
        add     vmaddr, #4
        rdlong  count, vmaddr wz        ' get the byte count
  if_z  jmp     #sd_finish
        add     vmaddr, #4
        rdlong  vmaddr, vmaddr          ' get the sector address
        call    #sd_read

sd_finish
        wrlong  sdError, pvmaddr        ' return error status
        jmp     #waitcmd

' hubaddr - buffer pointer
' count - number of bytes to read
' vmaddr - sector number
sd_read
        call    #sd_select
        mov     sdOp, #CMD17_READ_SINGLE_BLOCK
:readRepeat
        mov     sdParam, vmaddr
        call    #sdSendCmd              ' Read from specified block
        call    #sdResponse
        mov     sdBlkCnt, sdBlkSize     ' Transfer a block at a time
:getRead
        call    #spiRecvByte
        tjz     count, #:skipStore      ' Check for count exhausted
        wrbyte  data, hubaddr
        add     hubaddr, #1
        sub     count, #1
:skipStore
        djnz    sdBlkCnt, #:getRead     ' Are we done with the block?
        call    #spiRecvByte
        call    #spiRecvByte            ' Yes, finish with 16 clocks
        add     vmaddr, #1
        tjnz    count, #:readRepeat     '  and check for more blocks to do
        call    #sd_release
sd_read_ret
        ret

' hubaddr - buffer pointer
' count - number of bytes to read
' vmaddr - sector number
sd_write
        call    #sd_select
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
        rdbyte  data, hubaddr           ' If not, get the next data byte
        add     hubaddr, #1
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
  if_ne jmp     #:finish
        movs    sdWaitData, #0          ' Wait until not busy
        call    #sdWaitBusy
        add     vmaddr, #1
        tjnz    count, #:writeRepeat    '  to next if more data remains
:finish
        call    #sd_release
sd_write_ret
        ret

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
  if_nc jmp     #sdSendCmd_ret
sdWaitData
        cmp     data, #0-0 wz        ' Wait for some other response
  if_e  jmp     #sdWaitLoop          '  than that specified
sdSendCmd_ret
sdResponse_ret
sdWaitBusy_ret
        ret

'----------------------------------------------------------------------------------------------------
' SPI routines
'----------------------------------------------------------------------------------------------------

sd_select
sd_select_ret
        andn    outa, tcs
        ret

sd_release
sd_release_ret
        or      outa, tcs
        ret

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

sdOp            long    0
sdParam         long    0
sdFreq          long    0
sdTime          long    0
sdError         long    0
sdBlkCnt        long    0
sdInitCnt       long    32768 / 8      ' Initial SPI clocks produced
sdBlkSize       long    SECTOR_SIZE    ' Number of bytes in an SD block

tclk_mosi       long    (1<<CLK_PIN)|(1<<MOSI_PIN)
spidir          long    (1<<CS_PIN)|(1<<CLK_PIN)|(1<<MOSI_PIN)

' temporaries used by sd_read and sd_write
count           long    0

' temporaries used by send
bits            long    0
data            long    0

        fit     496
