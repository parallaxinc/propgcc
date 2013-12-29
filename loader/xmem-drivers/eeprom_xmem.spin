{
  EEPROM external code backstore cache driver
  Code by Steve Denson
  Copyright (c) 2011 by Parallax, Inc.
  
  Based on code from Chip Gracey's Propeller II SDRAM Driver
  Copyright (c) 2013 by Chip Gracey

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

CON

  ' EEPROM SUPPORT
  RDELAY        = 1
  WDELAY        = 4
  EEID          = $A0
  RBIT          = 1   ' devid byte read bit

DAT

init
        mov     pinout,  scl             ' scl 1
        or      pinout,  sda             ' scl 1
        mov     pindir,  scl             ' scl output
        or      pindir,  sda             ' sda output
init_ret 
        ret

'------------------------------------------------------------------------------
'
' read_bytes - read from external memory
'
' extaddr is the external memory address to read
' hubaddr is the hub memory address to write
' count is the number of bytes to read
'
'------------------------------------------------------------------------------

read_bytes
lck_i2c test    $, #0 wc             ' lock no-op: clear the carry bit
   if_c jmp     #lck_i2c
        mov     _delayCount, #RDELAY
        mov     devs,    #EEID
        mov     psiz,    count
        add     extaddr, eebase      ' add eeprom base to skip the loader and get to the proper EEPROM address
        mov     tp,      hubaddr
        call    #readPage
nlk_i2c nop        
read_bytes_ret
        ret

'------------------------------------------------------------------------------
'
' write_bytes - write to external memory
'
' extaddr is the external memory address to write
' hubaddr is the hub memory address to read
' count is the number of bytes to write
'
'------------------------------------------------------------------------------
write_bytes
        tjz     wrenable, write_bytes_ret
        tjz     extaddr, #disable_writes

        and     extaddr, eeprom_mask
        add     extaddr, eebase         ' add the eeprom image base address
        mov     tp, hubaddr

        mov     _delayCount, #WDELAY    ' use write delay
        mov     devs,   #EEID
:loop
        cmp     count,  page_size wc
        mov     psiz,   page_size
  if_b  mov     psiz,   count           ' if bytes < line_size, write remainder
        call    #writePage
        add     extaddr, page_size
        sub     count,  page_size wc,wz
  if_a  jmp     #:loop
write_bytes_ret
        ret

disable_writes
        mov     wrenable, #0
        jmp     write_bytes_ret

page_size   long    128

'-------------------------------------------------------------------------------
' read eeprom page
' @param       *dp - device number
' @param       *ap - read address
' @param       *bp - address of buffer for read
' @param       psiz - number of bytes to read
'
readPage
        call    #sendAddr
        or      devs,    #RBIT   ' send read command
        call    #sendCurrent
        mov     t1,      psiz    ' read psiz bytes
        sub     t1,      #1      ' first psiz-1 are NAK
        tjz     t1,      #:last
        andn    ack,     sda     ' not last byte     
:loop
        call    #recVal          ' read page data
        wrbyte  val,     tp
        add     tp,      #1
        djnz    t1,      #:loop
:last  
        or      ack,     sda     ' last byte has ACK     
        call    #recVal
        wrbyte  val,     tp
        add     tp,      #1      ' increment for next loop too
        call    #sendEnd
readPage_ret   ret

'-------------------------------------------------------------------------------
' read current byte
' @param       *dp - device number
' @param       *ap - read address
' @param       *sp - number of bytes to read
' @param       *bp - address of buffer for read
'
readCurrent
        or      devs,    #RBIT   ' read
        call    #sendCurrent
        or      ack,     sda     ' last byte     
        call    #recVal
        wrbyte  val,     tp
        add     tp,      #1
        call    #sendEnd
readCurrent_ret   ret

'-------------------------------------------------------------------------------
' write eeprom page
' @param       *dp - device number
' @param       *ap - write address
' @param       *bp - address of buffer for write
' @param       psiz - number of bytes to read
'
writePage
        call    #sendAddr        ' send page address
        mov     t1,      psiz
:loop
        rdbyte  val,     tp
        add     tp,      #1
        call    #sendVal         ' send data
        djnz    t1,      #:loop
        call    #writeEndPoll
writePage_ret   ret

'-------------------------------------------------------------------------------
' send end and poll for write complete
'
writeEndPoll
        call    #sendEnd        ' send stop signals eeprom program
        mov     t0, #1
        shl     t0, #20         ' delay for 10ms (Tw = 5ms for most devices)
        add     t0, cnt
        waitcnt t0, t0          ' write does not appear reliable without this wait
{
'
' TODO write complete polling doesn't seem to work ... fix later
' download performance is main impact.
'
        call    #sclDelay
:loop
        call    #sendCurrent     ' now start write polling
        or      outa,    scl
        andn    dira,    sda     ' looking for ACK
        test    sda,     ina wc  ' is ack 0 ?
        call    #sclDelay
  if_c  jmp     #:loop           ' if ack 1, repeat
        call    #sendEnd         ' send stop
}
writeEndPoll_ret  ret  

'-------------------------------------------------------------------------------
' write current eeprom byte
' @param       *dp - device number
' @param       *ap - write address
' @param       *sp - number of bytes to write
' @param       *bp - address of buffer for write
'
writeCurrent
        andn    devs,    #RBIT
        call    #sendStart
        rdbyte  val,     tp
        add     tp,      #1
        call    #sendVal
writeCurrent_ret  ret

'-------------------------------------------------------------------------------
' send eeprom address
' @param       devs - device number
' @param       extaddr - read address

'
sendAddr
        call    #sendStart
        mov     val,     extaddr         ' %00000000_00000111_11111111_11111111 ->
        ror     val,     #16             ' %11111111_11111111_00000000_00000111
        shl     val,     #1              ' %11111111_11111110_00000000_00001110 ->
        andn    devs,    #RBIT           ' %10100000 send addr/write needs bit 0 low 
        or      val,     devs            ' %11111111_11111110_00000000_10101110
        call    #sendVal                 ' send device id and upper address bits
        mov     val,     extaddr         ' %00000000_00000111_11111111_11111111 ->
        ror     val,     #8              ' %11111111_00000000_00000111_11111111
        call    #sendVal                 ' send middle address bits
        mov     val,     extaddr         ' %00000000_00000111_11111111_11111111
        call    #sendVal                 ' send lower address bits
sendAddr_ret    ret

'-------------------------------------------------------------------------------
' send current is used to initiate a transaction after sendAddr
' @param       devs - device number with bit 0 set for read or clear for write
' @param       extaddr - read address
'
sendCurrent
        call    #sendStart
        mov     val,     extaddr         ' %00000000_00000111_11111111_11111111 ->
        ror     val,     #16             ' %11111111_11111111_00000000_00000111
        shl     val,     #1              ' %11111111_11111110_00000000_00001110 ->
        or      val,     devs            ' %11111111_11111110_00000000_1010111r
        call    #sendVal                 ' send device id and upper address bits
sendCurrent_ret ret


'-------------------------------------------------------------------------------
' A start signal is when SDA goes 1 -> 0 while SCL is 1
'
sendStart
        or      outa,    scl             ' scl high
        or      dira,    scl             ' scl output ... but it is always output right?
        or      outa,    sda             ' sda high
        or      dira,    sda             ' sda output
        call    #sclDelayD2              ' device detects high
        andn    outa,    sda             ' sda low for start condition 
        call    #sclDelayD2              ' device detects start
        andn    outa,    scl             ' finish with scl low 
sendStart_ret   ret
  
'-------------------------------------------------------------------------------
' An end signal is when SDA goes 0 -> 1 while SCL is 1
'
sendEnd
        andn    outa,    sda             ' sda high
        or      outa,    scl             ' scl high
        call    #sclDelayD2
        or      outa,    sda             ' sda high
        call    #sclDelayD2              ' device detects stop
        andn    dira,    sda             
        andn    dira,    scl             ' leave sda & scl input so other devices can use the bus
sendEnd_ret     ret

'-------------------------------------------------------------------------------
' scl has limits on how fast it can go. use this routine to throttle scl
'
sclDelay
        call    #sclDelayD2
sclDelay_ret    ret

sclDelayD2      ' write reliability is slower than read
        mov     _delay, _delayCount
        djnz    _delay, #$
sclDelayD2_ret  ret

'-------------------------------------------------------------------------------
' send 8 bits of data
' @param ack
' @param val
'
sendVal
        mov     ack,     #0              ' %00000000_00000000_00000000_11111111 ->
        ror     val,     #8              ' %11111111_00000000_00000000_00000000
        andn    outa,    scl             ' start with scl 0
        mov     t0,      #8              ' Output val on sda
:loop
        shl     val,     #1 wc           ' get next bit state into carry              
        muxc    outa,    sda             ' set bit depending on carry
        call    #sclDelay
        or      outa,    scl             ' toggle clock
        call    #sclDelay
        andn    outa,    scl
        call    #sclDelayD2
        djnz    t0,      #:loop
:findack  
        andn    dira,    sda             ' looking for ACK
        mov     ack,     ina             ' get ack
        and     ack,     sda wz          ' keep only ack bit
        or      outa,    scl             ' scl high
        call    #sclDelay
        andn    outa,    scl             ' scl low
        andn    outa,    sda             ' sda low
        or      dira,    sda             ' sda back to output
sendVal_ret     ret


'-------------------------------------------------------------------------------
' receive 8 bits of data
' @param ack
' @param val
'
recVal
        mov     val,     #0
        andn    dira,    sda             ' sda input
        andn    outa,    scl             ' clock starts at 0
        call    #sclDelay
        mov     t0,      #8              ' input 8 bits of val from sda
:loop
        or      outa,    scl             ' toggle clock
        call    #sclDelayD2
        shl     val,     #1
        test    sda,     ina wz          ' get bit in z
        muxnz   val,     #1
        andn    outa,    scl             ' toggle clock
        call    #sclDelay
        djnz    t0,      #:loop
:sendack
        or      dira,    sda             ' set sda to out so device can interpret ACK
        or      outa,    ack             ' ack is always on sda, but not always set
        call    #sclDelayD2
        or      outa,    scl        
        call    #sclDelay
        andn    outa,    scl             ' final clock pulse
        andn    outa,    sda        
recVal_ret      ret

_delay          long 0
_delayCount     long WDELAY

scl             long 1 << 28   
sda             long 1 << 29

tp              long 0   ' temp pointer
t0              long 0   ' temp vars

cmd             long 0   ' command word
devs            long 0   ' device select
psiz            long 0   ' parameter size in bytes
buff            long 0   ' buffer pointer
ueack           long 0   ' user early ack
ack             long 0   ' ack bit
val             long 0   ' accumulated value
eebase          long $1300 ' eebase sb > eeprom loader size

pindir          long 0
pinout          long 0

eof             long $55555555           

eeprom_mask     long $0fffffff
wrenable        long 1

        fit     496

