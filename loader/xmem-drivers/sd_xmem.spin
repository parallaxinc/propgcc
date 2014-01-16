{
  SPI SD external memory driver
  Copyright (c) 2011 by David Betz

  Based on code from Chip Gracey's Propeller II SDRAM Driver
  Copyright (c) 2013 by Chip Gracey

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

#include "xmem_common.spin"
#include "xmem_spi_pins.spin"
#include "xmem_spi.spin"

'----------------------------------------------------------------------------------------------------
' Constants
'----------------------------------------------------------------------------------------------------

CON

  ' SD card sector dimensions
  SECTOR_WIDTH              = 9   ' 512 byte sectors
  SECTOR_SIZE               = 1<<SECTOR_WIDTH

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
  
DAT

'----------------------------------------------------------------------------------------------------
' Driver initialization
'----------------------------------------------------------------------------------------------------

init
 
        ' get the pin masks
        call    #get_spi_pins

        ' set the pin directions
        mov     outa, pinout
        mov     dira, pindir
        call    #release
        
        rdlong  sdFreq, #CLKFREQ_ADDR   ' Get the clock frequency
        
        mov     cluster_map, xmem_param1
        mov     cluster_width, xmem_param2
        mov     cluster_mask, #1
        shl     cluster_mask, cluster_width
        sub     cluster_mask, #1

init_ret
        ret
        
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

sd_init
        call    #release

        mov     t1, sdInitCnt
_init   call    #spiRecvByte            ' Output a stream of 32K clocks
        djnz    t1, #_init              '  in case SD card left in some

        call    #select
        mov     count, #10

_cmd0   mov     sdOp, #CMD0_GO_IDLE_STATE
        mov     sdParam, #0
        mov     sdCRC, #$95
        call    #sdSendCmd
        cmp     data, #1 wz             ' Wait until response is In Idle
  if_e  jmp     #_iniOK
        djnz    count, #_cmd0
        mov     sdError, #3             ' Error: Reset failed after 10 attempts
        jmp     #_ifinish

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
  if_ne jmp     #_ifinish

_type2  mov     sdParam1, ccsbit        ' CMD41 is necessary for both type 1 and 2
        mov     sdCRC, #$77             ' but with different paramaters and CRC, so
        call    #_cmd41                 ' it's in a subroutine.

_cmd58  mov     sdOp, #CMD58_READ_OCR
        mov     sdParam, #0
        mov     sdCRC, #$FD
        call    #sdSendCmd
        cmp     data, #0 wz
  if_ne mov     sdError, #5             ' Error: READ_OCR Failed
  if_ne jmp     #_ifinish

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
  if_nc jmp     #_ifinish
check_time_ret
        ret

_ifini  mov     sdOp, #CMD59_CRC_ON_OFF ' Sad, but we don't have the code space nor
        mov     sdParam, #0             ' bandwidth to protect read/writes with CRCs.
        mov     sdCRC, #$91
        call    #sdSendCmd

        call    #spiRecvLong            ' Drain the previous command
        mov     sdInit, #1              ' Initialization done
_ifinish
        call    #release
        mov     t1, sdError             ' Return the error code
sd_init_ret
        ret
        
'------------------------------------------------------------------------------
' Block read/write
'------------------------------------------------------------------------------

read_bytes 
        mov     sdError, #0             ' Assume no errors
        mov     sdErrorHandler, #readFinish
        tjnz    sdInit, #_readSkipInit
        call    #sd_init
_readSkipInit
        call    #select
        mov     sdOp, #CMD17_READ_SINGLE_BLOCK
_readRepeat
        mov     sdParam, extaddr
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
        add     extaddr, #1
        tjnz    count, #_readRepeat     ' Check for more blocks to do
readFinish
        call    #release
        mov     t1, sdError
read_bytes_ret
        ret

write_bytes
        mov     sdError, #0             ' Assume no errors
        mov     sdErrorHandler, #writeFinish
        tjnz    sdInit, #_writeSkipInit
        call    #sd_init
_writeSkipInit
        rdlong  hubaddr, extaddr         ' Get the buffer pointer
        add     extaddr, #4
        rdlong  count, extaddr wz        ' Get the byte count
  if_z  jmp     #writeFinish
        add     extaddr, #4
        rdlong  extaddr, extaddr          ' Get the sector address
        call    #select
        mov     sdOp, #CMD24_WRITE_BLOCK
_writeRepeat
        mov     sdParam, extaddr
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
  if_ne jmp     #writeFinish
        movs    sdWaitData, #0          ' Wait until not busy
        call    #sdWaitBusy
        add     extaddr, #1
        tjnz    count, #_writeRepeat    ' Check for more blocks to do
writeFinish
        call    #release
        mov     t1, sdError             ' Return the error code
write_bytes_ret
        ret

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
  if_nc jmp     sdErrorHandler          ' return to the error handler
sdWaitData
        cmp     data, #0-0 wz           ' Wait for some other response
  if_e  jmp     #sdWaitLoop             '  than that specified
sdSectorCmd_ret
sdSendCmd_ret
sdResponse_ret
sdWaitBusy_ret
        ret

'----------------------------------------------------------------------------------------------------
' Data for the SD Card Routines
'----------------------------------------------------------------------------------------------------

sdInit          long    0
sdOp            long    0
sdParam         long    0
sdParam1        long    0
sdCRC           long    0
sdCRC1          long    0
sdFreq          long    0
sdTime          long    0
sdError         long    0
sdErrorHandler  jmp     #sdErrorHandler
sdBlkCnt        long    0
sdInitCnt       long    32768 / 8      ' Initial SPI clocks produced
dstinc                                 ' 1<<9 = increment for the destination field of an instruction = same as sdBlkSize!
sdBlkSize       long    512            ' Number of bytes in an SD block

adrShift        long    9       ' Number of bits to left shift SD sector address (9 for SD/MMC, 0 for SDHC)
sd3_3V          long    $1AA    ' Tell card we want to work at 3.3V
ccsbit          long    (1<<30) ' Flag to indicates SDHC/SDXC card

pindir          long    0       ' Saved DIRA for the SPI bus
pinout          long    0

'----------------------------------------------------------------------------------------------------
'
' get_physical_sector - translate virtual address to sector
'                       using the cluster_map
'
' on entry:
'   extaddr - external memory address
' on return:
'   extaddr - physical sector number containing the address
'   t1 - clobbered
'----------------------------------------------------------------------------------------------------

addrmask long $0fffffff

get_physical_sector
        and     extaddr, addrmask
        shr     extaddr, #SECTOR_WIDTH
        add     extaddr, #4				' size of the pex header
        mov     t1, extaddr
        and     t1, cluster_mask
        andn    extaddr, cluster_mask
        shl     extaddr, #2              ' byte offset to a long
        shr     extaddr, cluster_width
        add     extaddr, cluster_map
        rdlong  extaddr, extaddr
        add     extaddr, t1
get_physical_sector_ret
        ret

' Computed values that could be passed as parameters
cluster_mask      long  0
cluster_width     long  0
cluster_map       long  0       ' address in hub ram where the cluster map is stored

        fit     496
