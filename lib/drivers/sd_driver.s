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
	.equ CMD55_APP_CMD,           $40 | 55
	.equ CMD17_READ_SINGLE_BLOCK, $40 | 17
	.equ CMD24_WRITE_BLOCK,       $40 | 24
	.equ ACMD41_SD_APP_OP_COND,   $40 | 41
  
	.section .data
	.global  _sd_driver_array
_sd_driver_array
	long __load_start_cogsys1

	.section .cogsys1, "ax"

        org	0

init
        jmp     #init2

' these four values get patched by the loader
tmiso   long    1<<MISO_PIN
tclk    long    1<<CLK_PIN
tmosi   long    1<<MOSI_PIN
tcs_clr long    1<<CS_CLR_PIN

' this value is used by the C3 card
tinc	long	1<<INC_PIN

init2   mov     pvmcmd, par             ' get the address of the mailbox
        mov     pvmaddr, pvmcmd         ' pvmaddr is a pointer into the cache line on return
        add     pvmaddr, #4

	' Check if C3 mode or normal CS mode
	cmp	tinc, #0 wz
 if_nz	jmp	#c3_mode
	'movs	select, #cs_sel
	mov	select, jmp_cs_sel
	'movs	deselect, #cs_des
	mov	deselect, jmp_cs_des
        
        ' build composite masks
c3_mode mov     tclk_mosi, tmosi
        or      tclk_mosi, tclk
        mov     spidir, tcs_clr
        or      spidir, tclk
        or      spidir, tinc
        or      spidir, tmosi
        
        ' disable the chip select
	'call	#deselect
	jmpret	deselect_ret, #deselect

        ' get the clock frequency
        rdlong  sdFreq, #CLKFREQ_ADDR

        ' start the command loop
waitcmd mov     dira, #0                    ' release the pins for other SPI clients
        wrlong  zero, pvmcmd
_wait1  rdlong  vmpage, pvmcmd wz
  if_z  jmp     #_wait1
        mov     dira, spidir                ' set the pins back so we can use them

'       test    vmpage, #int#EXTEND_MASK wz ' test for an extended command
        test    vmpage, #EXTEND_MASK wz     ' test for an extended command
  if_z  jmp     #extend
        jmp     #waitcmd

extend  mov     vmaddr, vmpage
        shr     vmaddr, #8
        shr     vmpage, #2
        and     vmpage, #7
	'add    vmpage, #dispatch
        mov     t1, #dispatch
	shr	t1, #2
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

sd_init_handler
        mov     sdError, #0             ' assume no errors
	'call	#deselect
	jmpret	deselect_ret, #deselect
        mov     t1, sdInitCnt
_init   'call    #spiRecvByte            ' Output a stream of 32K clocks
	jmpret	spiRecvByte_ret, #spiRecvByte
        djnz    t1, #_init              '  in case SD card left in some
	'call	#select
	jmpret	select_ret, #select
        mov     sdOp, #CMD0_GO_IDLE_STATE
        mov     sdParam, #0
        'call    #sdSendCmd              ' Send a reset command and deselect
	jmpret	sdSendCmd_ret, #sdSendCmd
_wait2  mov     sdOp, #CMD55_APP_CMD
        'call    #sdSendCmd
	jmpret	sdSendCmd_ret, #sdSendCmd
        mov     sdOp, #ACMD41_SD_APP_OP_COND
        'call    #sdSendCmd
	jmpret	sdSendCmd_ret, #sdSendCmd
        cmp     data, #1 wz             ' Wait until response not In Idle
  if_e  jmp     #_wait2
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
	'call	#select                 ' Read from specified block
	jmpret	select_ret, #select
        mov     sdOp, #CMD17_READ_SINGLE_BLOCK
_readRepeat
        mov     sdParam, vmaddr
        'call    #sdSendCmd              ' Read from specified block
	jmpret	sdSendCmd_ret, #sdSendCmd
        'call    #sdResponse
	jmpret	sdResponse_ret, #sdResponse
        mov     sdBlkCnt, sdBlkSize     ' Transfer a block at a time
_getRead
        'call    #spiRecvByte
	jmpret	spiRecvByte_ret, #spiRecvByte
        tjz     count, #_skipStore      ' Check for count exhausted
        wrbyte  data, ptr
        add     ptr, #1
        sub     count, #1
_skipStore
        djnz    sdBlkCnt, #_getRead     ' Are we done with the block?
        'call    #spiRecvByte
	jmpret	spiRecvByte_ret, #spiRecvByte
        'call    #spiRecvByte            ' Yes, finish with 16 clocks
	jmpret	spiRecvByte_ret, #spiRecvByte
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
	'call	#select                ' Write to specified block
	jmpret	select_ret, #select
        mov     sdOp, #CMD24_WRITE_BLOCK
_writeRepeat
        mov     sdParam, vmaddr
        'call    #sdSendCmd              ' Write to specified block
	jmpret	sdSendCmd_ret, #sdSendCmd
        mov     data, #$fe              ' Ask to start data transfer
        'call    #spiSendByte
	jmpret	spiSendByte_ret, #spiSendByte
        mov     sdBlkCnt, sdBlkSize     ' Transfer a block at a time
_putWrite
        mov     data, #0                '  padding with zeroes if needed
        tjz     count, #_padWrite       ' Check for count exhausted
        rdbyte  data, ptr               ' If not, get the next data byte
        add     ptr, #1
        sub     count, #1
_padWrite
        'call    #spiSendByte
	jmpret	spiSendByte_ret, #spiSendByte
        djnz    sdBlkCnt, #_putWrite    ' Are we done with the block?
        'call    #spiRecvByte
	jmpret	spiRecvByte_ret, #spiRecvByte
        'call    #spiRecvByte            ' Yes, finish with 16 clocks
	jmpret	spiRecvByte_ret, #spiRecvByte
        'call    #sdResponse
	jmpret	sdResponse_ret, #sdResponse
        and     data, #$1f              ' Check the response status
        cmp     data, #5 wz
  if_ne mov     sdError, #1             ' Must be Data Accepted
  if_ne jmp     #sd_finish
        movs    sdWaitData, #0          ' Wait until not busy
        'call    #sdWaitBusy
	jmpret	sdWaitBusy_ret, #sdWaitBusy
        add     vmaddr, #1
        tjnz    count, #_writeRepeat    '  to next if more data remains
sd_finish
	'call	#deselect
	jmpret	deselect_ret, #deselect
        wrlong  sdError, pvmaddr        ' return error status
        jmp     #waitcmd

sdSendCmd
        'call    #spiRecvByte         ' ?? selecting card and clocking
	jmpret	spiRecvByte_ret, #spiRecvByte
        mov     data, sdOp
        'call    #spiSendByte
	jmpret	spiSendByte_ret, #spiSendByte
        mov     data, sdParam
        shr     data, #15            ' Supplied address is sector number
        'call    #spiSendByte
	jmpret	spiSendByte_ret, #spiSendByte
        mov     data, sdParam        ' Send to SD card as byte address,
        shr     data, #7             '  in multiples of 512 bytes
        'call    #spiSendByte
	jmpret	spiSendByte_ret, #spiSendByte
        mov     data, sdParam        ' Total length of this address is
        shl     data, #1             '  four bytes
        'call    #spiSendByte
	jmpret	spiSendByte_ret, #spiSendByte
        mov     data, #0
        'call    #spiSendByte
	jmpret	spiSendByte_ret, #spiSendByte
        mov     data, #$95           ' CRC code (for 1st command only)
        'call    #spiSendByte
	jmpret	spiSendByte_ret, #spiSendByte
sdResponse
        movs    sdWaitData, #$ff     ' Wait for response from card
sdWaitBusy
        mov     sdTime, cnt          ' Set up a 1 second timeout
sdWaitLoop
        'call    #spiRecvByte
	jmpret	spiRecvByte_ret, #spiRecvByte
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

' Select the SD card
select	jmp	#c3_sel              ' This jump is modified for normal CS mode
c3_sel  mov     t1, #5
        andn    outa, tcs_clr
        or      outa, tcs_clr
_loop   or      outa, tinc
        andn    outa, tinc
        djnz    t1, #_loop
	jmp	#select_ret
cs_sel  andn    outa, tcs_clr
select_ret ret

' Deselect the SD card
deselect jmp	#c3_des              ' This jump is modified for normal CS mode
c3_des  andn    outa, tcs_clr
cs_des	or      outa, tcs_clr
deselect_ret ret

'----------------------------------------------------------------------------------------------------
' SPI routines
'----------------------------------------------------------------------------------------------------

spiSendByte
        shl     data, #24
        mov     bits, #8
        jmp     #send

send0   andn    outa, tclk
send    rol     data, #1 wc
        muxc    outa, tmosi
        or      outa, tclk
        djnz    bits, #send0
        andn    outa, tclk
        or      outa, tmosi
spiSendByte_ret
send_ret
        ret

spiRecvByte
        mov     data, #0
        mov     bits, #8
receive
gloop   or      outa, tclk
        test    tmiso, ina wc
        rcl     data, #1
        andn    outa, tclk
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
sdBlkSize       long    512            ' Number of bytes in an SD block

' pointers to mailbox entries
pvmcmd          long    0       ' on call this is the virtual address and read/write bit
pvmaddr         long    0       ' on return this is the address of the cache line containing the virtual address
vmpage          long    0       ' page containing the virtual address

zero            long    0       ' zero constant
t1              long    0       ' temporary variable

tclk_mosi       long    (1<<CLK_PIN)|(1<<MOSI_PIN)
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
jmp_cs_sel 	jmp	#cs_sel
jmp_cs_des 	jmp	#cs_des

