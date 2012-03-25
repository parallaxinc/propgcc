'
' SPI SRAM and flash JCACHE driver for the Parallax C3
' by David Betz
'
' Based on code from VMCOG - virtual memory server for the Propeller
' Copyright (c) February 3, 2010 by William Henning
'
' and on code from SdramCache
' Copyright (c) 2010 by John Steven Denson (jazzed)
'
' Modified to interface to the SD card on the C3 or other boards that use
' a single pin for the chip select - Dave Hein, 11/7/11
' Converted from Spin/PASM to GAS assembly - Dave Hein, 11/12/11
'
' SDHC Initialization added by Ted Stefanik, 3/15/2012,
' based on fsrw's safe_spi.spin by Jonathan "lonesock" Dummer
' Copyright 2009  Tomas Rokicki and Jonathan Dummer
'
' TERMS OF USE: MIT License
'
' Permission is hereby granted, free of charge, to any person obtaining a copy
' of this software and associated documentation files (the "Software"), to deal
' in the Software without restriction, including without limitation the rights
' to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
' copies of the Software, and to permit persons to whom the Software is
' furnished to do so, subject to the following conditions:
'
' The above copyright notice and this permission notice shall be included in
' all copies or substantial portions of the Software.
'
' THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
' IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
' FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
' AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
' LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,ARISING FROM,
' OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
' THE SOFTWARE.
'
'----------------------------------------------------------------------------------------------------
' Constants
'----------------------------------------------------------------------------------------------------

        .equ CS_CLR_PIN,              25
        .equ CLK_PIN,                 11
        .equ MOSI_PIN,                9
        .equ MISO_PIN,                10
        .equ INC_PIN,                 8

        .equ EXTEND_MASK,             %10
  
        ' address of CLKFREQ in hub RAM
        .equ CLKFREQ_ADDR,            $0000

        ' SD commands
        .equ CMD0_GO_IDLE_STATE,      $40 | 0
        .equ CMD1_SEND_OP_COND,       $40 | 1
        .equ CMD8_SEND_IF_COND_CMD,   $40 | 8
        .equ CMD55_APP_CMD,           $40 | 55
        .equ CMD16_SET_BLOCKLEN,      $40 | 16
        .equ CMD17_READ_SINGLE_BLOCK, $40 | 17
        .equ CMD24_WRITE_BLOCK,       $40 | 24
        .equ ACMD41_SD_APP_OP_COND,   $40 | 41
        .equ CMD58_READ_OCR,          $40 | 58
        .equ CMD59_CRC_ON_OFF,        $40 | 59
  
        .section .data
        .global  _sd_driver_array
_sd_driver_array
        long __load_start_cogsys1

        .section .cogsys1, "ax"

        org     0

'----------------------------------------------------------------------------------------------------
' Driver initialization
'----------------------------------------------------------------------------------------------------

init
        jmp     #init2

' these four values get patched by the loader
tmiso   long    1<<MISO_PIN
tclk    long    1<<CLK_PIN
tmosi   long    1<<MOSI_PIN
tcs_clr long    1<<CS_CLR_PIN

' this value is used by the C3 card
tinc    long    1<<INC_PIN

init2   mov     pvmcmd, par             ' get the address of the mailbox
        mov     pvmaddr, pvmcmd         ' pvmaddr is a pointer into the cache line on return
        add     pvmaddr, #4

        or      outa, tmosi             ' need to set output high so reads work correctly

        ' Check if C3 mode or normal CS mode
        cmp     tinc, #0 wz
 if_nz  jmp     #c3_mode
        mov     select, jmp_cs_sel
        mov     deselect, jmp_cs_des
        
        ' build composite masks
c3_mode mov     spidir, tcs_clr
        or      spidir, tclk
        or      spidir, tinc
        or      spidir, tmosi
        
        ' disable the chip select
        call    #deselect

        ' get the clock frequency
        rdlong  sdFreq, #CLKFREQ_ADDR

'----------------------------------------------------------------------------------------------------
' Command loop
'----------------------------------------------------------------------------------------------------

waitcmd mov     dira, #0                    ' release the pins for other SPI clients
        wrlong  zero, pvmcmd
_wait1  rdlong  vmpage, pvmcmd wz
  if_z  jmp     #_wait1
        mov     dira, spidir                ' set the pins back so we can use them

        test    vmpage, #EXTEND_MASK wz     ' test for an extended command
  if_z  jmp     #extend
        jmp     #waitcmd

extend  mov     vmaddr, vmpage
        shr     vmaddr, #8
        shr     vmpage, #2
        and     vmpage, #7
        mov     t1, #dispatch
        shr     t1, #2
        add     vmpage, t1
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
        mov     sdError, #0             ' assume no errors
        call    #deselect

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
        mov     sdParam, 0
        mov     sdCRC, #$FD
        call    #sdSendCmd
        cmp     data, #0 wz
  if_ne mov     sdError, #5             ' Error: READ_OCR Failed
  if_ne jmp     #sd_finish

        call    #spiRecvLong            ' Check the SDHC bit
        test    data, ccsbit wz
  if_nz mov     adrShift, #0
        jmp     #_ifini

_type1  mov     sdParam1, 0
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
        mov     sdParam, 0
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
        mov     sdParam, 0              ' bandwidth to protect read/writes with CRCs.
        mov     sdCRC, #$91
        call    #sdSendCmd

        call    #spiRecvLong            ' Drain the previous command
        jmp     #sd_finish

'------------------------------------------------------------------------------
' Block read/write
'------------------------------------------------------------------------------

sd_read_handler
        mov     sdError, #0             ' assume no errors
        rdlong  ptr, vmaddr             ' get the buffer pointer
        add     vmaddr, #4
        rdlong  count, vmaddr wz        ' get the byte count
  if_z  jmp     #sd_finish
        add     vmaddr, #4
        rdlong  vmaddr, vmaddr          ' get the sector address
        call    #select                 ' Read from specified block
        mov     sdOp, #CMD17_READ_SINGLE_BLOCK
_readRepeat
        mov     sdParam, vmaddr
        call    #sdSectorCmd            ' Read from specified block
        call    #sdResponse
        mov     sdBlkCnt, sdBlkSize     ' Transfer a block at a time
_getRead
        call    #spiRecvByte
        tjz     count, #_skipStore      ' Check for count exhausted
        wrbyte  data, ptr
        add     ptr, #1
        sub     count, #1
_skipStore
        djnz    sdBlkCnt, #_getRead     ' Are we done with the block?
        call    #spiRecvByte
        call    #spiRecvByte            ' Yes, finish with 16 clocks
        add     vmaddr, #1
        tjnz    count, #_readRepeat     '  and check for more blocks to do
        jmp     #sd_finish

sd_write_handler
        mov     sdError, #0             ' assume no errors
        rdlong  ptr, vmaddr             ' get the buffer pointer
        add     vmaddr, #4
        rdlong  count, vmaddr wz        ' get the byte count
  if_z  jmp     #sd_finish
        add     vmaddr, #4
        rdlong  vmaddr, vmaddr         ' get the sector address
        call    #select                ' Write to specified block
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
        rdbyte  data, ptr               ' If not, get the next data byte
        add     ptr, #1
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
        tjnz    count, #_writeRepeat    '  to next if more data remains
sd_finish
        call    #deselect
        wrlong  sdError, pvmaddr        ' return error status
        jmp     #waitcmd

sdSectorCmd
        shl     sdParam, adrShift    ' SD/MMC card uses byte address, SDHC uses sector address
sdSendCmd
        call    #spiRecvLong         ' ?? selecting card and clocking
        mov     data, sdOp
        call    #spiSendByte
        mov     data, sdParam
        call    #spiSendLong
        mov     data, sdCRC          ' CRC code
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
  if_nc mov     sdError, #2          ' Error: SD Command timed out after 1 second
  if_nc jmp     #sd_finish
sdWaitData
        cmp     data, #0-0 wz        ' Wait for some other response
  if_e  jmp     #sdWaitLoop          '  than that specified
sdSectorCmd_ret
sdSendCmd_ret
sdResponse_ret
sdWaitBusy_ret
        ret

'----------------------------------------------------------------------------------------------------
' SPI Bus Access
'----------------------------------------------------------------------------------------------------

' Select the SD card
select  jmp     #c3_sel              ' This jump is modified for normal CS mode
c3_sel  mov     t1, #5
        andn    outa, tcs_clr
        or      outa, tcs_clr
_loop   or      outa, tinc
        andn    outa, tinc
        djnz    t1, #_loop
        jmp     #select_ret
cs_sel  andn    outa, tcs_clr
select_ret ret

' Deselect the SD card
deselect jmp    #c3_des              ' This jump is modified for normal CS mode
c3_des  andn    outa, tcs_clr
cs_des  or      outa, tcs_clr
deselect_ret ret

'----------------------------------------------------------------------------------------------------
' Low-level SPI routines
'----------------------------------------------------------------------------------------------------

spiSendLong
        mov     bits, #32
        jmp     #send
spiSendByte
        shl     data, #24
        mov     bits, #8
send    andn    outa, tclk
        rol     data, #1 wc
        muxc    outa, tmosi
        or      outa, tclk
        djnz    bits, #send
        andn    outa, tclk
        or      outa, tmosi
spiSendLong_ret
spiSendByte_ret
        ret

spiRecvLong
        mov     bits, #32
        jmp     #spiRecv
spiRecvByte
        mov     bits, #8
spiRecv mov     data, #0
gloop   or      outa, tclk
        test    tmiso, ina wc
        rcl     data, #1
        andn    outa, tclk
        djnz    bits, #gloop
spiRecvLong_ret
spiRecvByte_ret
        ret

'----------------------------------------------------------------------------------------------------
' Data
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
sdBlkSize       long    512            ' Number of bytes in an SD block

adrShift        long    9       ' number of bits to left shift SD sector address (9 for SD/MMC, 0 for SDHC)
sd3_3V          long    $1AA    ' tell card we want to work at 3.3V
ccsbit          long    (1<<30) ' flag to indicates SDHC/SDXC card

' pointers to mailbox entries
pvmcmd          long    0       ' on call this is the virtual address and read/write bit
pvmaddr         long    0       ' on return this is the address of the cache line containing the virtual address
vmpage          long    0       ' page containing the virtual address

zero            long    0       ' zero constant
t1              long    0       ' temporary variable

spidir          long    (1<<CS_CLR_PIN)|(1<<CLK_PIN)|(1<<MOSI_PIN)

' input parameters to BREAD and BWRITE
vmaddr          long    0       ' virtual address

' temporaries used by BREAD and BWRITE
ptr             long    0
count           long    0

' temporaries used by send
bits            long    0
data            long    0

' jump instructions for the single CS mode
jmp_cs_sel      jmp     #cs_sel
jmp_cs_des      jmp     #cs_des
