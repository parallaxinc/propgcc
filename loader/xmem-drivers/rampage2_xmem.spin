{
  RamPage2 External Memory Driver
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

#include "xmem_common.spin"

CON

  ' SPI pins
  SIO0_PIN              = 0
  SCK_PIN               = 8
  FLASH_CS_PIN          = 9
  SRAM_CS_PIN           = 10
  
DAT

'----------------------------------------------------------------------------------------------------
'
' init - initialize the memory functions
'
'----------------------------------------------------------------------------------------------------

init
        ' set the pin directions
        call    #release

        ' initialize the flash
        call    #sst_read_jedec_id
        cmp     t1, jedec_id_1 wz
  if_nz jmp     #:next
        cmp     t2, jedec_id_2 wz
  if_nz jmp     #:next
        cmp     t3, jedec_id_3 wz
  if_z  jmp     #:inquad
:next   call    #read_jedec_id
        cmp     t1, jedec_id wz
  if_nz jmp     #halt
        cmp     t2, jedec_id wz
  if_nz jmp     #halt
        call    #flash_select
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

:inquad or      pindir, sio_mask ' we're now in quad mode

        ' unprotect the entire flash
        call    #sst_write_enable
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

        ' select sequential access mode for the SRAM chip
        call    #sram_select
        mov     data, sram_seq
        mov     bits, #16
        call    #spiSend
        call    #release
        
        ' switch to quad mode for the SRAM chip
        call    #sram_select
        mov     data, sram_eqio
        mov     bits, #8
        call    #spiSend
        call    #release
                
init_ret
        ret
        
halt    jmp     #halt

'----------------------------------------------------------------------------------------------------
'
' read_bytes - read data from external memory
'
' on input:
'   extaddr is the external memory address to read
'   hubaddr is the hub memory address to write
'   count is the number of bytes to read
'
'----------------------------------------------------------------------------------------------------

read_bytes
        cmp     extaddr, flash_base wc
  if_b  jmp     #read_bytes_sram
        mov     cmd, extaddr
        and     cmd, offset_bits
        shr     cmd, #1
        or      cmd, sst_read
        mov     bytes, #4
        mov     dbytes, count
        call    #sst_sqi_read
read_bytes_ret
        ret
        
read_bytes_sram
        mov     fn, sram_read
        call    #read_write_start
        mov     dbytes, count
        call    #sqi_read
        jmp     read_bytes_ret

'----------------------------------------------------------------------------------------------------
'
' write_bytes
'
' on input:
'   extaddr is the external memory address to write
'   hubaddr is the hub memory address to read
'   count is the number of bytes to write
'
'----------------------------------------------------------------------------------------------------

write_bytes
        cmp     extaddr, flash_base wc
  if_b  jmp     #write_bytes_sram
  
write_bytes_flash
        tjz     wrenable, write_bytes_ret
        tjz     extaddr, #disable_writes
        test    extaddr, block_mask wz
  if_z  call    #erase_4k_block
  
        mov     t1, extaddr
        add     t1, #256
        andn    t1, #255
        sub     t1, extaddr
        max     t1, count
:loop   call    #sst_write_enable
        mov     cmd, extaddr
        and     cmd, offset_bits
        shr     cmd, #1
        or      cmd, sst_program
        mov     bytes, #4
        mov     dbytes, t1
        call    #sst_sqi_write
        call    #wait_until_done
        sub     count, t1 wz
 if_z   jmp     write_bytes_ret
        add     extaddr, t1
        mov     t1, count
        max     t1, #256
        jmp     #:loop

write_bytes_sram
        mov     fn, sram_write
        call    #read_write_start
        mov     dbytes, count
        call    #sqi_write
write_bytes_ret
        ret

disable_writes
        mov     wrenable, #0
        jmp     write_bytes_ret

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
        mov     cmd, extaddr
        and     cmd, offset_bits
        shr     cmd, #1
        or      cmd, fn
        mov     bytes, #4
        call    #sram_start_sqi_cmd
read_write_start_ret
        ret

'----------------------------------------------------------------------------------------------------
'
' erase_4k_block
'
' on input:
'   extaddr is the virtual memory address to erase
'
'----------------------------------------------------------------------------------------------------

erase_4k_block
        call    #sst_write_enable
        mov     cmd, extaddr
        and     cmd, offset_bits
        shr     cmd, #1
        or      cmd, ferase4kblk
        mov     bytes, #4
        call    #sst_start_sqi_cmd
        call    #release
        call    #wait_until_done
erase_4k_block_ret
        ret

read_jedec_id
        call    #flash_select
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
sqi_write
        andn    outa, sck_mask
:loop   rdbyte  data, hubaddr
        add     hubaddr, #1
        shl     data, sio_shift
        andn    outa, sio_mask
        or      outa, data
        or      outa, sck_mask
        andn    outa, sck_mask
        djnz    dbytes, #:loop
        call    #release
sst_sqi_write_ret
sqi_write_ret
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
sqi_read
        andn    dira, sio_mask
        andn    outa, sck_mask
        or      outa, sck_mask  ' hi dummy nibble
        andn    outa, sck_mask
        or      outa, sck_mask  ' lo dummy nibble
        andn    outa, sck_mask
:loop   or      outa, sck_mask
        mov     data, ina
        andn    outa, sck_mask
        shr     data, sio_shift
        wrbyte  data, hubaddr
        add     hubaddr, #1
        djnz    dbytes, #:loop
        call    #release
sst_sqi_read_ret
sqi_read_ret
        ret

sram_start_sqi_cmd
        or      dira, sio_mask      ' set data pins to outputs
        call    #sram_select
        jmp     #sloop
        
sst_start_sqi_cmd_1
        mov     bytes, #1
        
sst_start_sqi_cmd
        or      dira, sio_mask      ' set data pins to outputs
        call    #flash_select       ' select the chip
sloop   rol     cmd, #8
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
        djnz    bytes, #sloop
sram_start_sqi_cmd_ret
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

sio_t1              long    0
sio_t2              long    0
busy_bits           long    $8800

jedec_id            long    $00bf2601    ' SST26VF016
jedec_id_1          long    $bbff
jedec_id_2          long    $2266
jedec_id_3          long    $0011

sst_rdjedecid       long    $af000000    ' read the manufacturers id, device type and device id
sst_quadmode        long    $38          ' enable quad mode
sst_wrblkprot       long    $42000000    ' write block protect register
sst_program         long    $02000000    ' flash program byte/page
sst_read            long    $0b000000    ' flash read command

frdjedecid          long    $9f          ' read the manufacturers id, device type and device id
ferase4kblk         long    $20000000    ' flash erase a 4k block
frdstatus           long    $05000000    ' flash read status
fwrenable           long    $06000000    ' flash write enable

'----------------------------------------------------------------------------------------------------
' SPI routines
'----------------------------------------------------------------------------------------------------

flash_select
        andn    outa, flash_cs_mask
flash_select_ret
        ret

sram_select
        andn    outa, sram_cs_mask
sram_select_ret
        ret

release
        mov     outa, pinout
        mov     dira, pindir
release_ret
        ret
        
spiSendByte
        shl     data, #24
        mov     bits, #8
spiSend rol     data, #1 wc
        muxc    outa, mosi_lo_mask
        muxc    outa, mosi_hi_mask
        or      outa, sck_mask
        andn    outa, sck_mask
        djnz    bits, #spiSend
spiSendByte_ret
spiSend_ret
        ret

spiRecvByte
        mov     data_lo, #0
        mov     data_hi, #0
        mov     bits, #8
:loop   or      outa, sck_mask
        test    miso_lo_mask, ina wc
        rcl     data_lo, #1
        test    miso_hi_mask, ina wc
        rcl     data_hi, #1
        andn    outa, sck_mask
        djnz    bits, #:loop
spiRecvByte_ret
        ret

' mosi_lo, mosi_hi, sck, cs
pindir          long    (%1101 << SIO0_PIN) | (%1101 << (SIO0_PIN + 4)) | (1 << SCK_PIN) | (1 << FLASH_CS_PIN) | (1 << SRAM_CS_PIN)

' mosi_lo, mosi_hi, cs
pinout          long    (%1101 << SIO0_PIN) | (%1101 << (SIO0_PIN + 4)) | (0 << SCK_PIN) | (1 << FLASH_CS_PIN) | (1 << SRAM_CS_PIN)

mosi_lo_mask    long    1 << SIO0_PIN
miso_lo_mask    long    2 << SIO0_PIN
mosi_hi_mask    long    1 << (SIO0_PIN + 4)
miso_hi_mask    long    2 << (SIO0_PIN + 4)
sck_mask        long    1 << SCK_PIN
flash_cs_mask   long    1 << FLASH_CS_PIN
sram_cs_mask    long    1 << SRAM_CS_PIN
sio_mask        long    $ff << SIO0_PIN
sio_shift       long    SIO0_PIN

' variables used by the spi send/receive functions
fn              long    0
cmd             long    0
bytes           long    0
dbytes          long    0
data            long    0
data_lo         long    0
data_hi         long    0
bits            long    0

' sram commands
sram_read       long    $03000000       ' read command
sram_write      long    $02000000       ' write command
sram_eqio       long    $38000000       ' enter quad I/O mode
sram_seq        long    $01400000       ' set sequential mode

flash_base      long    $30000000       ' base address of flash memory
offset_bits     long    $00ffffff       ' mask to isolate the flash/sram offset bits
block_mask      long    $00001fff       ' 2 * 4k flash blocks

wrenable        long    1

                fit     496
