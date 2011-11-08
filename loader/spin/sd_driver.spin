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

  ' these defaults are for the PropBOE
  MISO_PIN              = 11
  CLK_PIN               = 12
  MOSI_PIN              = 13
  CS_PIN                = 14

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

PUB image
  return @init

DAT
        org   $0

init
        jmp     #init2

' these four values get patched by the loader
tmiso   long    1<<MISO_PIN
tclk    long    1<<CLK_PIN
tmosi   long    1<<MOSI_PIN
tcs     long    1<<CS_PIN

init2   mov     pvmcmd, par             ' get the address of the mailbox
        mov     pvmaddr, pvmcmd         ' pvmaddr is a pointer into the cache line on return
        add     pvmaddr, #4
        
        ' build composite masks
        mov     tclk_mosi, tmosi
        or      tclk_mosi, tclk
        mov     spidir, tcs
        or      spidir, tclk
        or      spidir, tmosi
        
        ' disable the chip select
        or      outa, tcs

        ' get the clock frequency
        rdlong  sdFreq, #CLKFREQ_ADDR

        ' start the command loop
waitcmd mov     dira, #0                    ' release the pins for other SPI clients
        wrlong  zero, pvmcmd
:wait   rdlong  vmpage, pvmcmd wz
  if_z  jmp     #:wait
        mov     dira, spidir                ' set the pins back so we can use them

        test    vmpage, #int#EXTEND_MASK wz ' test for an extended command
  if_z  jmp     #extend
        jmp     #waitcmd

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

sd_read_handler
        mov     sdError, #0             ' assume no errors
        rdlong  ptr, vmaddr             ' get the buffer pointer
        add     vmaddr, #4
        rdlong  count, vmaddr wz        ' get the byte count
  if_z  jmp     #sd_finish
        add     vmaddr, #4
        rdlong  vmaddr, vmaddr          ' get the sector address
        andn    outa, tcs               ' Read from specified block
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
        andn    outa, tcs              ' Write to specified block
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
        or      outa, tcs
        wrlong  sdError, pvmaddr        ' return error status
        jmp     #waitcmd

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
spidir          long    (1<<CS_PIN)|(1<<CLK_PIN)|(1<<MOSI_PIN)

' input parameters to BREAD and BWRITE
vmaddr          long    0       ' virtual address

' temporaries used by BREAD and BWRITE
ptr             long    0
count           long    0

' temporaries used by send
bits            long    0
data            long    0

        fit     496
