{
  SPI SD JCACHE external memory driver
  Copyright (c) 2011 by David Betz

  Based on code by Steve Denson (jazzed)
  Copyright (c) 2010 by John Steven Denson

  Inspired by VMCOG - virtual memory server for the Propeller
  Copyright (c) February 3, 2010 by William Henning

  SDHC Initialization added by Ted Stefanik, 3/15/2012,
  based on fsrw's safe_spi.spin by Jonathan "lonesock" Dummer
  Copyright 2009  Tomas Rokicki and Jonathan Dummer

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

'----------------------------------------------------------------------------------------------------
' Constants
'----------------------------------------------------------------------------------------------------

CON

  ' SD card sector dimensions
  SECTOR_WIDTH              = 9   ' 512 byte sectors
  SECTOR_SIZE               = 1<<SECTOR_WIDTH

  ' cache line tag flags
  EMPTY_BIT                 = 30
  DIRTY_BIT                 = 31

  ' address of CLKFREQ in hub RAM
  CLKFREQ_ADDR            = $0000

  ' SD commands
  CMD0_GO_IDLE_STATE      = $40 | 0
  CMD1_SEND_OP_COND       = $40 | 1
  CMD8_SEND_IF_COND_CMD   = $40 | 8
  CMD16_SET_BLOCKLEN      = $40 | 16
  CMD17_READ_SINGLE_BLOCK = $40 | 17
  CMD24_WRITE_BLOCK       = $40 | 24
  CMD55_APP_CMD           = $40 | 55
  ACMD41_SD_APP_OP_COND   = $40 | 41
  CMD58_READ_OCR          = $40 | 58
  CMD59_CRC_ON_OFF        = $40 | 59
  
  ' protocol bits
  CS_CLR_PIN_MASK       = $01
  INC_PIN_MASK          = $02   ' for C3-style CS
  MUX_START_BIT_MASK    = $04   ' low order bit of mux field
  MUX_WIDTH_MASK        = $08   ' width of mux field
  ADDR_MASK             = $10   ' device number for C3-style CS or value to write to the mux

OBJ
  int: "cache_interface"

PUB image
  return @init

DAT
        org   $0

'----------------------------------------------------------------------------------------------------
' Driver initialization
'----------------------------------------------------------------------------------------------------

init    jmp     #init2                  ' The mailbox address is passed in par
        long    @params - @init

init2   mov     pvmcmd, par             ' pvmcmd is a pointer to the virtual address and read/write bit
        mov     pvmaddr, pvmcmd         ' pvmaddr is a pointer into the cache line on return, or the error return (for SD operations)
        add     pvmaddr, #4

        ' build the mosi mask
        mov     t1, sdspi_config1
        shr     t1, #24
        mov     mosi_mask, #1
        shl     mosi_mask, t1
        or      spidir, mosi_mask
        
        ' build the miso mask
        mov     t1, sdspi_config1
        shr     t1, #16
        and     t1, #$ff
        mov     miso_mask, #1
        shl     miso_mask, t1
        
        ' build the sck mask
        mov     t1, sdspi_config1
        shr     t1, #8
        and     t1, #$ff
        mov     sck_mask, #1
        shl     sck_mask, t1
        or      spidir, sck_mask
        
        ' handle the CS or C3-style CLR pins
        test    sdspi_config1, #CS_CLR_PIN_MASK wz
  if_nz mov     t1, sdspi_config2
  if_nz shr     t1, #24
  if_nz mov     cs_clr, #1
  if_nz shl     cs_clr, t1
  if_nz or      spidir, cs_clr
  
        ' handle the mux width
        test    sdspi_config1, #MUX_WIDTH_MASK wz
  if_nz mov     t1, sdspi_config2
  if_nz shr     t1, #8
  if_nz and     t1, #$ff
  if_nz mov     mask_inc, #1
  if_nz shl     mask_inc, t1
  if_nz sub     mask_inc, #1
  if_nz or      spidir, mask_inc
  
        ' handle the C3-style address or mux value
        test    sdspi_config1, #ADDR_MASK wz
  if_nz mov     select_addr, sdspi_config2
  if_nz and     select_addr, #$ff

        ' get the INC pin or the mux start bit
        mov     t1, sdspi_config2
        shr     t1, #16
        and     t1, #$ff

        ' handle the C3-style INC pin
        test    sdspi_config1, #INC_PIN_MASK wz
  if_nz mov     mask_inc, #1
  if_nz shl     mask_inc, t1
  if_nz mov     sd_select, c3_sd_select_jmp     ' We're in C3 mode, so replace sd_select/sd_release
  if_nz mov     sd_release, c3_sd_release_jmp   ' with the C3-aware routines
  if_nz or      spidir, mask_inc
 
        ' handle the mux start bit (must follow setting of select_addr and mask_inc)
        test    sdspi_config1, #MUX_START_BIT_MASK wz
  if_nz shl     select_addr, t1
  if_nz shl     mask_inc, t1
  if_nz or      spidir, mask_inc
        
        ' set the pin directions
        mov     outa, cs_clr
        or      outa, mosi_mask                ' Need to set output high so reads work correctly
        mov     dira, spidir
        call    #sd_release

        rdlong  sdFreq, #CLKFREQ_ADDR   ' Get the clock frequency

        jmp     #waitcmd

fillme  long    0[128-fillme]           ' first 128 cog locations are used for a direct mapped cache table

        fit   128

' sdspi_config1: 0xiiooccpp - ii=mosi oo=miso cc=sck pp=protocol
' sdspi_config2: 0xaabbccdd - aa=cs-or-clr bb=inc-or-start cc=width dd=addr
' the protocol byte is a bit mask with the bits defined above
'   if CS_CLR_PIN_MASK ($01) is set, then byte aa contains the CS or C3-style CLR pin number
'   if INC_PIN_MASK ($02) is set, then byte bb contains the C3-style INC pin number
'   if MUX_START_BIT_MASK ($04) is set, then byte bb contains the starting bit number of the mux field
'   if MUX_WIDTH_MASK ($08) is set, then byte cc contains the width of the mux field
'   if ADDR_MASK ($10) is set, then byte dd contains either the C3-style address or the value to write to the mux field

' These two values get patched by the loader
params
sdspi_config1 long    $090a0b13 ' for the c3 (di=9, do=10, clk=11, protocol=CS_CLR_PIN_MASK|INC_PIN_MASK|ADDR_MASK)
sdspi_config2 long    $19080005 ' for the c3 (clr=25, inc=8, addr=5)


'----------------------------------------------------------------------------------------------------
' Command loop
'----------------------------------------------------------------------------------------------------

waitcmd mov     dira, #0                ' Release the pins for other SPI clients
        wrlong  zero, pvmcmd
_wait   rdlong  vmline, pvmcmd wz
  if_z  jmp     #_wait

        test    vmline, #int#EXTEND_MASK wz ' Test for an extended command
  if_nz jmp     #cache

'----------------------------------------------------------------------------------------------------
' Extended command
'----------------------------------------------------------------------------------------------------

extend  mov     vmaddr, vmline
        shr     vmaddr, #8
        shr     vmline, #2
        and     vmline, #7
        add     vmline, #dispatch
lck_spi test    $, #0 wc                ' Lock no-op: clear the carry bit
   if_c jmp     #lck_spi
        mov     dira, spidir            ' Set the pins back so we can use them
        jmp     vmline

dispatch
        jmp     #nlk_fin
        jmp     #nlk_fin
        jmp     #nlk_fin
        jmp     #sd_init_handler
        jmp     #sd_read_handler
        jmp     #sd_write_handler
        jmp     #cache_init_handler
'       jmp     #lock_set_handler - This is the next instruction - no need to waste a long

'------------------------------------------------------------------------------
' SPI Bus Lock
'------------------------------------------------------------------------------

lock_set_handler
        mov     dira, #0
nlk_ini nop                     ' Unlock previous lock
        mov     lock_id, vmaddr
        mov     lck_spi, lock_set
        mov     lck_sp1, lock_set
        mov     nlk_spi, lock_clr
        mov     nlk_sp1, lock_clr
        mov     nlk_ini, lock_clr
        jmp     #waitcmd
lock_set
        lockset lock_id wc
lock_clr
        lockclr lock_id
lock_id long    0               ' lock id for optional bus interlock

'------------------------------------------------------------------------------
' SD Card Initialization
'------------------------------------------------------------------------------

' The following initialization code conforms to the diagrams on pp114-115 of
' Part_1_Physical_Layer_Simplified_Specification_Ver_3.01_Final_100518.pdf
' fouund at sdcard.org.  We used fsrw's safe_spi.spin as a template of how to
' implement these diagrams in the following code, only this code does not
' duplicate the "the card said CMDo ('go idle') was invalid, so we're possibly
' stuck in read or write mode" section - this appears to be only applicable to
' multi-block read/write, and the PropGCC code uses and supports neither.

sd_init_handler
        mov     sdError, #0             ' Assume no errors
        call    #sd_release

        mov     t1, sdInitCnt
_init   call    #spiRecvByte            ' Output a stream of 32K clocks
        djnz    t1, #_init              '  in case SD card left in some

        call    #sd_select
        mov     count, #10

_cmd0   mov     sdOp, #CMD0_GO_IDLE_STATE
        mov     sdParam, #0
        mov     sdCRC, #$95
        call    #sdSendCmd
        cmp     data, #1 wz             ' Wait until response is In Idle
  if_e  jmp     #_iniOK
        djnz    count, #_cmd0
        mov     sdError, #3             ' Error: Reset failed after 10 attempts
        jmp     #sd_finish

_iniOK  mov     adrShift, #9
        mov     sdBlkCnt, cnt           ' We overload sdBlkCnt as part of master timer during init
        mov     count, sdFreq           ' We overload count as part of master timer during init
        shl     count, #2               ' All initialization must be done in 4 seconds
        
_cmd8   mov     sdOp, #CMD8_SEND_IF_COND_CMD
        mov     sdParam, sd3_3V
        mov     sdCRC, #$87
        call    #sdSendCmd
        cmp     data, #1 wz             ' Wait until response is In Idle
  if_ne jmp     #_type1

        call    #spiRecvLong
        cmp     data, sd3_3V
  if_ne mov     sdError, #4             ' Error: 3.3V Not Supported
  if_ne jmp     #sd_finish

_type2  mov     sdParam1, ccsbit        ' CMD41 is necessary for both type 1 and 2
        mov     sdCRC, #$77             ' but with different paramaters and CRC, so
        call    #_cmd41                 ' it's in a subroutine.

_cmd58  mov     sdOp, #CMD58_READ_OCR
        mov     sdParam, #0
        mov     sdCRC, #$FD
        call    #sdSendCmd
        cmp     data, #0 wz
  if_ne mov     sdError, #5             ' Error: READ_OCR Failed
  if_ne jmp     #sd_finish

        call    #spiRecvLong            ' Check the SDHC bit
        test    data, ccsbit wz
  if_nz mov     adrShift, #0
        jmp     #_ifini

_type1  mov     sdParam1, #0
        mov     sdCRC, #$E5
        call    #_cmd41i

        cmp     data, #1 wc,wz
   if_a jmp     #_typMMC

_initsd call    #_cmd41

_cmd16  mov     sdOp, #CMD16_SET_BLOCKLEN
        mov     sdParam, sdBlkSize
        mov     sdCRC, #$15
        call    #sdSendCmd
        jmp     #_ifini

_typMMC mov     sdOp, #CMD1_SEND_OP_COND
        mov     sdParam, sdBlkSize
        mov     sdCRC, #$F9
        call    #sdSendCmd
        jmp     #_cmd16

_cmd41  call    #_cmd41i
        tjnz    data, #_cmd41            ' Wait until we the card idles
_cmd41_ret
        ret

_cmd41i call    #check_time              ' This routine does not wait until idle -
        mov     sdOp, #CMD55_APP_CMD     ' it just does one ACMD41, then returns.
        mov     sdParam, #0
        mov     sdCRC, #$65
        call    #sdSendCmd
        cmp     data, #1 wc,wz
  if_a  jmp     #_cmd41
        mov     sdOp, #ACMD41_SD_APP_OP_COND
        mov     sdParam, sdParam1
        mov     sdCRC, sdCRC1
        call    #sdSendCmd
_cmd41i_ret
        ret

check_time
        mov     t1, cnt
        sub     t1, sdBlkCnt            ' Check for expired timeout (1 sec)
        cmp     t1, count wc
  if_nc mov     sdError, #6             ' Error: Didn't totally initialize in 4 secs
  if_nc jmp     #sd_finish
check_time_ret
        ret

_ifini  mov     sdOp, #CMD59_CRC_ON_OFF ' Sad, but we don't have the code space nor
        mov     sdParam, #0             ' bandwidth to protect read/writes with CRCs.
        mov     sdCRC, #$91
        call    #sdSendCmd

        call    #spiRecvLong            ' Drain the previous command
        jmp     #sd_finish

'------------------------------------------------------------------------------
' Block read/write
'------------------------------------------------------------------------------

sd_read_handler
        mov     sdError, #0             ' Assume no errors
        rdlong  hubaddr, vmaddr         ' Get the buffer pointer
        add     vmaddr, #4
        rdlong  count, vmaddr wz        ' Get the byte count
  if_z  jmp     #sd_finish
        add     vmaddr, #4
        rdlong  vmaddr, vmaddr          ' Get the sector address
        call    #sd_read

sd_finish
        call    #sd_release
        wrlong  sdError, pvmaddr        ' Return error status
nlk_fin mov     dira, #0                ' release the pins for other SPI clients
nlk_spi nop        
        jmp     #waitcmd

sd_read call    #sd_select
        mov     sdOp, #CMD17_READ_SINGLE_BLOCK
_readRepeat
        mov     sdParam, vmaddr
        call    #sdSectorCmd            ' Read from specified block
        call    #sdResponse
        mov     sdBlkCnt, sdBlkSize     ' Transfer a block at a time
_getRead
        call    #spiRecvByte
        tjz     count, #_skipStore      ' Check for count exhausted
        wrbyte  data, hubaddr
        add     hubaddr, #1
        sub     count, #1
_skipStore
        djnz    sdBlkCnt, #_getRead     ' Are we done with the block?
        call    #spiRecvByte
        call    #spiRecvByte            ' Yes, finish with 16 clocks
        add     vmaddr, #1
        tjnz    count, #_readRepeat     ' Check for more blocks to do
sd_read_ret
        ret

sd_write_handler
        mov     sdError, #0             ' Assume no errors
        rdlong  hubaddr, vmaddr         ' Get the buffer pointer
        add     vmaddr, #4
        rdlong  count, vmaddr wz        ' Get the byte count
  if_z  jmp     #sd_finish
        add     vmaddr, #4
        rdlong  vmaddr, vmaddr          ' Get the sector address
        call    #sd_select
        mov     sdOp, #CMD24_WRITE_BLOCK
_writeRepeat
        mov     sdParam, vmaddr
        call    #sdSectorCmd            ' Write to specified block
        mov     data, #$fe              ' Ask to start data transfer
        call    #spiSendByte
        mov     sdBlkCnt, sdBlkSize     ' Transfer a block at a time
_putWrite
        mov     data, #0                '  padding with zeroes if needed
        tjz     count, #_padWrite       ' Check for count exhausted
        rdbyte  data, hubaddr           ' If not, get the next data byte
        add     hubaddr, #1
        sub     count, #1
_padWrite
        call    #spiSendByte
        djnz    sdBlkCnt, #_putWrite    ' Are we done with the block?
        call    #spiRecvByte
        call    #spiRecvByte            ' Yes, finish with 16 clocks
        call    #sdResponse
        and     data, #$1f              ' Check the response status
        cmp     data, #5 wz             ' Must be Data Accepted
  if_ne mov     sdError, #1             ' Error: Write Error to SD Card
  if_ne jmp     #sd_finish
        movs    sdWaitData, #0          ' Wait until not busy
        call    #sdWaitBusy
        add     vmaddr, #1
        tjnz    count, #_writeRepeat    ' Check for more blocks to do
        jmp     #sd_finish

'------------------------------------------------------------------------------
' Send Sector Read/Write Command to SD Card
'------------------------------------------------------------------------------

sdSectorCmd
        shl     sdParam, adrShift       ' SD/MMC card uses byte address, SDHC uses sector address
sdSendCmd
        call    #spiRecvLong            ' Flush any previous command results
        mov     data, sdOp
        call    #spiSendByte
        mov     data, sdParam
        call    #spiSendLong
        mov     data, sdCRC             ' CRC code
        call    #spiSendByte
sdResponse
        movs    sdWaitData, #$ff        ' Wait for response from card
sdWaitBusy
        mov     sdTime, cnt             ' Set up a 1 second timeout
sdWaitLoop
        call    #spiRecvByte
        mov     t1, cnt
        sub     t1, sdTime              ' Check for expired timeout (1 sec)
        cmp     t1, sdFreq wc
  if_nc mov     sdError, #2             ' Error: SD Command timed out after 1 second
  if_nc jmp     #sd_finish              ' Note: this isn't quite appropriate when we're reading a cache line, but what else is?
sdWaitData
        cmp     data, #0-0 wz           ' Wait for some other response
  if_e  jmp     #sdWaitLoop             '  than that specified
sdSectorCmd_ret
sdSendCmd_ret
sdResponse_ret
sdWaitBusy_ret
        ret

'----------------------------------------------------------------------------------------------------
' SPI Bus Access
'----------------------------------------------------------------------------------------------------

sd_select                           ' Single-SPI and Parallel-DeMUX
        andn    outa, mask_inc
        or      outa, select_addr
        andn    outa, cs_clr
sd_select_ret
        ret

sd_release                          ' Single-SPI and Parallel-DeMUX
        or      outa, cs_clr
        andn    outa, mask_inc
sd_release_ret
        ret

c3_sd_select_jmp                    ' Serial-DeMUX Jumps
        jmp     #c3_sd_select       ' Initialization copies these jumps
c3_sd_release_jmp                   '   over sd_select and sd_release
        jmp     #c3_sd_release      '   when in C3 mode.

c3_sd_select                        ' Serial-DeMUX
        mov     t1, select_addr
        andn    outa, cs_clr
        or      outa, cs_clr
_loop   or      outa, mask_inc
        andn    outa, mask_inc
        djnz    t1, #_loop
        jmp     sd_select_ret

c3_sd_release                       ' Serial-DeMUX
        andn    outa, cs_clr
        or      outa, cs_clr
        jmp     sd_release_ret

'----------------------------------------------------------------------------------------------------
' Low-level SPI routines
'----------------------------------------------------------------------------------------------------

spiSendLong
        mov     bits, #32
        jmp     #spiSend
spiSendByte
        shl     data, #24
        mov     bits, #8
spiSend andn    outa, sck_mask
        rol     data, #1 wc
        muxc    outa, mosi_mask
        or      outa, sck_mask
        djnz    bits, #spiSend
        andn    outa, sck_mask
        or      outa, mosi_mask
spiSendLong_ret
spiSendByte_ret
        ret

spiRecvLong
        mov     bits, #32
        jmp     #spiRecv
spiRecvByte
        mov     bits, #8
spiRecv mov     data, #0
_rloop  or      outa, sck_mask
        test    miso_mask, ina wc
        rcl     data, #1
        andn    outa, sck_mask
        djnz    bits, #_rloop
spiRecvLong_ret
spiRecvByte_ret
        ret

'----------------------------------------------------------------------------------------------------
' Data for the SD Card Routines
'----------------------------------------------------------------------------------------------------

sdOp            long    0
sdParam         long    0
sdParam1        long    0
sdCRC           long    0
sdCRC1          long    0
sdFreq          long    0
sdTime          long    0
sdError         long    0
sdBlkCnt        long    0
sdInitCnt       long    32768 / 8      ' Initial SPI clocks produced
dstinc                                 ' 1<<9 = increment for the destination field of an instruction = same as sdBlkSize!
sdBlkSize       long    512            ' Number of bytes in an SD block

adrShift        long    9       ' Number of bits to left shift SD sector address (9 for SD/MMC, 0 for SDHC)
sd3_3V          long    $1AA    ' Tell card we want to work at 3.3V
ccsbit          long    (1<<30) ' Flag to indicates SDHC/SDXC card

' Pointers to mailbox entries
pvmcmd          long    0       ' The address of the call parameter:
                                '     the virtual address and read/write bit, or the extended command
pvmaddr         long    0       ' The address of the call return:
                                '     the address of the cache line containing the virtual address, or the extended command error
vmline          long    0       ' line containing the virtual address, or the extended comand

zero            long    0       ' zero constant
t1              long    0       ' Temporary variable

spidir          long    0       ' Saved DIRA for the SPI bus

mosi_mask       long    0
miso_mask       long    0
sck_mask        long    0

cs_clr          long    0
mask_inc        long    0
select_addr     long    0

' Input parameters to block read/write
vmaddr          long    0       ' Pointer to the read/write parameters: buffer, count, sector, then during the reads the sector

' Temporaries used by block read/write
hubaddr         long    0       ' The block read/write buffer pointer
count           long    0       ' The block count

' Temporaries used by send/recv
bits            long    0       ' # bits to send/receive
data            long    0       ' Current data being sent/received

'------------------------------------------------------------------------------
' Cache Initialization
'------------------------------------------------------------------------------

cache_init_handler
        mov     t1, vmaddr
        movd    :loop, #cache_params
        mov     count, #cache_params_end - cache_params
:loop   rdlong  0-0, t1
        add     t1, #4
        add     :loop, dstinc
        djnz    count, #:loop

        mov     index_count, #1
        shl     index_count, index_width
        mov     index_mask, index_count
        sub     index_mask, #1

        mov     line_size, #1
        shl     line_size, offset_width
        mov     t1, line_size
        sub     t1, #1
        wrlong  t1, vmaddr

        mov     cluster_mask, #1
        shl     cluster_mask, cluster_width
        sub     cluster_mask, #1

        ' initialize the cache lines
vmflush movd    :flush, #0
        mov     t1, index_count
:flush  mov     0-0, empty_mask
        add     :flush, dstinc
        djnz    t1, #:flush
        jmp     #waitcmd

'----------------------------------------------------------------------------------------------------
' Cache command
'----------------------------------------------------------------------------------------------------

cache   shr     vmline, offset_width wc ' carry is now one for read and zero for write
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
        andn    vmcurrent, dirty_mask   ' = &tag_mask (includes EMPTY_BIT)
        cmp     vmcurrent, vmline wz    ' z set means there was a cache hit
  if_nz call    #miss                   ' handle a cache miss
:st     or      0-0, set_dirty_bit      ' set the dirty bit on writes
        jmp     #waitcmd                ' wait for a new command

' line is the cache line index
' vmcurrent is current cache line
' vmline is new cache line
' hubaddr is the address of the cache line
miss    movd    mst, line
lck_sp1 test    $, #0 wc                ' lock no-op: clear the carry bit
   if_c jmp     #lck_sp1
        mov     dira, spidir            ' set the pins back so we can use them
        mov     vmaddr, vmline
        shl     vmaddr, offset_width
        call    #get_physical_sector
        mov     count, line_size
        call    #sd_read
        call    #sd_release
        mov     dira, #0                ' release the pins for other SPI clients
nlk_sp1 nop        
mst     mov     0-0, vmline
miss_ret ret

'----------------------------------------------------------------------------------------------------
'
' get_physical_sector - translate virtual address to sector
'                       using the cluster_map
'
' on entry:
'   vmaddr - external memory address
' on return:
'   vmaddr - physical sector number containing the address
'   t1 - clobbered
'----------------------------------------------------------------------------------------------------

addrmask long $0fffffff

get_physical_sector
        and     vmaddr, addrmask
        shr     vmaddr, #SECTOR_WIDTH
        add     vmaddr, #4				' size of the pex header
        mov     t1, vmaddr
        and     t1, cluster_mask
        andn    vmaddr, cluster_mask
        shl     vmaddr, #2              ' byte offset to a long
        shr     vmaddr, cluster_width
        add     vmaddr, cluster_map
        rdlong  vmaddr, vmaddr
        add     vmaddr, t1
get_physical_sector_ret
        ret

'----------------------------------------------------------------------------------------------------
' Data for the Cache
'----------------------------------------------------------------------------------------------------

' Parameters passed by the cache driver
cache_params
cacheptr          long  0       ' address in hub ram where cache lines are stored
index_width       long  0
offset_width      long  0
cluster_width     long  0
cluster_map       long  0       ' address in hub ram where the cluster map is stored
cache_params_end

' Computed values that could be passed as parameters
cluster_mask      long  0
index_mask        long  0
index_count       long  0
line_size         long  0       ' line size in bytes

vmcurrent         long  0       ' current selected cache line (same as vmline on a cache hit)
line              long  0       ' current cache line index
set_dirty_bit     long  0       ' DIRTY_BIT set on writes, clear on reads

empty_mask        long  (1<<EMPTY_BIT)
dirty_mask        long  (1<<DIRTY_BIT)

        fit     496
