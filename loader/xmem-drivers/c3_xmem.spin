{
  C3 External Memory Driver
  Copyright (c) 2013 by David Betz
  
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

#define RELEASE_PINS

#include "xmem_common.spin"
#include "xmem_spi.spin"

CON

  INC_PIN               = 8
  MOSI_PIN              = 9
  MISO_PIN              = 10
  CLK_PIN               = 11
  CLR_PIN               = 25

  ' SD commands
  CMD0_GO_IDLE_STATE      = $40|0

DAT

init
        ' setup the spi pins
        mov     mosi_mask, tmosi
        mov     miso_mask, tmiso
        mov     sck_mask, tclk
        
        ' set the pin directions
        mov     outa, pinout
        mov     dira, pindir
        call    #deselect

        ' put the SD card in SPI mode
        mov     t1, #10             ' output 80 clocks
:init   mov     data, #$ff
        call    #spiSendByte
        djnz    t1, #:init
        call    #sd_card
        mov     data, #CMD0_GO_IDLE_STATE
        call    #spiSendByte
        mov     data, #0
        call    #spiSendByte
        call    #spiSendByte
        call    #spiSendByte
        call    #spiSendByte
        mov     data, #$95          ' CRC (for 1st command only)
        call    #spiSendByte
        mov     data, #$ff
        call    #spiSendByte
        mov     t1, #20             ' try 20 times
:loop   call    #spiRecvByte
        test    data, #$80 wz
  if_z  jmp     #:done
        djnz    t1, #:loop
:done   call    #deselect
                
        ' select sequential access mode for the first SRAM chip
        call    #sram_chip1
        mov     data, ramseq
        mov     bits, #16
        call    #spiSend
        call    #deselect

        ' select sequential access mode for the second SRAM chip
        call    #sram_chip2
        mov     data, ramseq
        mov     bits, #16
        call    #spiSend
        call    #deselect
		
        ' unprotect the entire flash chip
        call    #write_enable
        call    #flash_chip
        mov     data, fwrstatus ' write zero to the status register
        mov     bits, #16
        call    #spiSend
        call    #deselect
init_ret
		ret

'----------------------------------------------------------------------------------------------------
'
' read_write_start
'
' select the chip and send the address for the read/write operation
'
' on input:
'   extaddr is the sram transfer address
'
'----------------------------------------------------------------------------------------------------

read_write_start
        test    extaddr, bit16 wz
  if_z  call    #sram_chip1      ' select first SRAM chip
  if_nz call    #sram_chip2      ' select second SRAM chip
        mov     data, extaddr
        and     data, sram_mask
        shl     data, #8          ' move it into position for transmission
        or      data, fn
        mov     bits, #24
        call    #spiSend
read_write_start_ret
        ret
'----------------------------------------------------------------------------------------------------
'
' read_bytes
'
' on input:
'   extaddr is the virtual memory address to read
'   hubaddr is the hub memory address to write
'   count is the number of bytes to read
'
'----------------------------------------------------------------------------------------------------

read_bytes
        cmp     extaddr, flash_base wc
  if_b  jmp     #read_sram_bytes
        call    #flash_chip       ' select flash chip
        mov     data, extaddr
        and     data, flash_mask
        or      data, fread
        mov     bits, #40         ' includes 8 dummy bits
        call    #spiSend

read_data
:rloop  call    #spiRecvByte
        wrbyte  data, hubaddr
        add     hubaddr, #1
        djnz    count, #:rloop
        call    #deselect
read_bytes_ret
        ret

read_sram_bytes
        mov     fn, read
        call    #read_write_start
        jmp     #read_data

'----------------------------------------------------------------------------------------------------
'
' write_bytes
'
' on input:
'   extaddr is the virtual memory address to write
'   hubaddr is the hub memory address to read
'   count is the number of bytes to write
'
'----------------------------------------------------------------------------------------------------

write_bytes
        cmp     extaddr, flash_base wc
  if_b  jmp     #write_sram_bytes
        tjz     wrenable, write_bytes_ret
        tjz     extaddr, #disable_writes
        test    extaddr, block_mask wz
  if_z  call    #erase_4k_block
        jmp     #write_block
  
wloop   test    extaddr, #$ff wz
  if_nz jmp     #wdata
        call    #deselect
        call    #wait_until_done
write_block
        call    #write_enable
        call    #flash_chip
        mov     data, extaddr
        and     data, flash_mask
        or      data, fprogram
        mov     bits, #32
        call    #spiSend
wdata   rdbyte  data, hubaddr
        call    #spiSendByte
        add     hubaddr, #1
        add     extaddr, #1
        djnz    count, #wloop
        call    #deselect
        call    #wait_until_done
        jmp     write_bytes_ret
        
write_sram_bytes
        mov     fn, write
        call    #read_write_start
:wloop  rdbyte  data, hubaddr
        call    #spiSendByte
        add     hubaddr, #1
        djnz    count, #:wloop
        call    #deselect
write_bytes_ret
        ret

disable_writes
        mov     wrenable, #0
        jmp     write_bytes_ret

fn      long    0

erase_4k_block
        call    #write_enable
        call    #flash_chip
        mov     data, extaddr
        and     data, flash_mask
        or      data, ferase4kblk
        mov     bits, #32
        call    #spiSend
        call    #deselect
        call    #wait_until_done
erase_4k_block_ret
		ret

write_enable
        call    #flash_chip
        mov     data, fwrenable
        mov     bits, #8
        call    #spiSend
        call    #deselect
write_enable_ret
        ret

wait_until_done
        call    #flash_chip
        mov     data, frdstatus
        mov     bits, #8
        call    #spiSend
:wait   call    #spiRecvByte
        test    data, #1 wz
  if_nz jmp     #:wait
        call    #deselect
        and     data, #$ff
wait_until_done_ret
        ret

' spi select functions
' all trash t1

sram_chip1
        mov     t1, #1
        jmp     #select

sram_chip2
        mov     t1, #2
        jmp     #select

sd_card
        mov     t1, #5
        jmp     #select

flash_chip
        mov     t1, #3

select  andn    outa, TCLR
        or      outa, TCLR
:loop   or      outa, TINC
        andn    outa, TINC
        djnz    t1, #:loop
sram_chip1_ret
sram_chip2_ret
sd_card_ret
flash_chip_ret
        ret

deselect
        andn    outa, TCLR
        or      outa, TCLR
deselect_ret
        ret

pindir      long    (1<<CLR_PIN)|(1<<INC_PIN)|(1<<CLK_PIN)|(1<<MOSI_PIN)
pinout      long    (1<<CLR_PIN)|(0<<INC_PIN)|(0<<CLK_PIN)|(1<<MOSI_PIN)

tclr        long    1<<CLR_PIN
tinc        long    1<<INC_PIN
tclk        long    1<<CLK_PIN
tmosi       long    1<<MOSI_PIN
tmiso       long    1<<MISO_PIN

' sram function codes
read        long    $03000000       ' read command
write       long    $02000000       ' write command
ramseq      long    $01400000       ' %00000001_01000000 << 16 ' set sequential mode
readstat    long    $05000000       ' read status

' flash function codes
fread       long    $0b000000       ' flash read command
ferase4kblk long    $20000000       ' flash erase a 4k block
fprogram    long    $02000000       ' flash program byte/page
fwrenable   long    $06000000       ' flash write enable
frdstatus   long    $05000000       ' flash read status
fwrstatus   long    $01000000       ' flash write status

bit16       long    $00008000       ' mask to select the SRAM chip
flash_base  long    $30000000       ' base address of flash memory

block_mask  long    $00000fff       ' 4k block address mask
flash_mask  long    $00ffffff       ' flash offset mask
sram_mask   long    $00007fff       ' sram chip offset mask (32k chip)

wrenable    long    1

            fit     496
