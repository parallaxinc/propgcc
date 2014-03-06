{
  SQI Flash/SRAM External Memory Driver
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

' param2: 0xssccffrr - ss=sio0 cc=sck ff=flash-cs rr=sram-cs
' param3: 0xssxxxxxx - ss=sd-cs

'#define WINBOND
'#define SST

'#define SD_INIT        

CON

  ' SD commands
  CMD0_GO_IDLE_STATE      = $40|0

#include "xmem_common.spin"
#include "xmem_sqi.spin"

init
        ' get the pin definitions
        ' xmem_param1 - 0xssccffrr where ss=sio0, cc=clk, ff=flash cs, rr=sram cs
        ' xmem_param2 - $ssxxxxff where ss=sd cs, ff=1 to initialize sd card

        ' get the sio_shift and build the mosi, miso, and sio masks
        mov     sio_shift, xmem_param1
        shr     sio_shift, #24
        mov     mosi_mask, #1
        shl     mosi_mask, sio_shift
        mov     miso_mask, mosi_mask
        shl     miso_mask, #1
        mov     sio_mask, #$f
        shl     sio_mask, sio_shift
        or      pindir, mosi_mask
        
        ' make the sio2 and sio3 pins outputs in single spi mode to assert /WE and /HOLD
        mov     t1, #$0c
        shl     t1, sio_shift
        or      pindir, t1
        or      pinout, t1
                
        ' build the sck mask
        mov     t1, xmem_param1
        shr     t1, #16
        and     t1, #$ff
        mov     sck_mask, #1
        shl     sck_mask, t1
        or      pindir, sck_mask
        
        ' get the chip selects
        mov     t1, xmem_param1
        shr     t1, #8
        and     t1, #$ff
        mov     flash_cs_mask, #1
        shl     flash_cs_mask, t1
        or      pinout, flash_cs_mask
        or      pindir, flash_cs_mask
        mov     t1, xmem_param1
        and     t1, #$ff
        mov     sram_cs_mask, #1
        shl     sram_cs_mask, t1
        or      pinout, sram_cs_mask
        or      pindir, sram_cs_mask

#ifdef SD_INIT
        ' get the sd chip select (cache-param3)
        mov     t1, xmem_param2
        shr     t1, #24
        mov     sd_cs_mask, #1
        shl     sd_cs_mask, t1
        test    xmem_param2, #1 wz
  if_z  mov     sd_cs_mask, #0
  if_nz or      pinout, sd_cs_mask
  if_nz or      pindir, sd_cs_mask
#endif

        ' set the pin directions
        call    #release
        
#ifdef SD_INIT
        ' put the SD card in SPI mode
        tjz     sd_cs_mask, #:skip
        mov     t1, #10             ' output 80 clocks
:init   mov     data, #$ff
        call    #spiSendByte
        djnz    t1, #:init
        andn    outa, sd_cs_mask
        mov     data, #CMD0_GO_IDLE_STATE
        call    #spiSendByte
        mov     data, #0
        call    #spiSendByte
        mov     data, #0
        call    #spiSendByte
        mov     data, #0
        call    #spiSendByte
        mov     data, #0
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
:done  or      outa, sd_cs_mask
:skip
#endif
                
        ' initialize the flash chip
        call    #flash_init
        
        ' select sequential access mode for the SRAM chip
        andn    outa, sram_cs_mask
        mov     data, sram_seq
        mov     bits, #16
        call    #spiSend
        call    #release
        
        ' switch to quad mode for the SRAM chip
        andn    outa, sram_cs_mask
        mov     data, sram_eqio
        mov     bits, #8
        call    #spiSend
        call    #release

init_ret
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
  if_b  jmp     #read_bytes_sram
  
read_bytes_flash
        call    #start_read
:loop   call    #sqiRecvByte
        wrbyte  data, hubaddr
        add     hubaddr, #1
        djnz    count, #:loop
read_bytes_done
        call    #release
read_bytes_ret
        ret

read_bytes_sram
        mov     fn, sram_read
        call    #read_write_start 
        andn    dira, sio_mask  ' switch to input
        andn    outa, sck_mask
        call    #sqiRecvByte
:loop   call    #sqiRecvByte
        wrbyte  data, hubaddr
        add     hubaddr, #1
        djnz    count, #:loop
        jmp     #read_bytes_done

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
  if_b  jmp     #write_bytes_sram
  
write_bytes_flash
        tjz     wrenable, write_bytes_ret
        tjz     extaddr, #disable_writes
        test    extaddr, block_mask wz
  if_z  call    #erase_4k_block
        jmp     #:addr
:loop   test    extaddr, #$ff wz
  if_nz jmp     #:data
        call    #release
        call    #wait_until_done
:addr   call    #start_write
:data   rdbyte  data, hubaddr
        call    #sqiSendByte
        add     hubaddr, #1
        add     extaddr, #1
        djnz    count, #:loop
        call    #release
        call    #wait_until_done
        jmp     write_bytes_ret

write_bytes_sram
        mov     fn, sram_write
        call    #read_write_start
        andn    outa, sck_mask
:loop   rdbyte  data, hubaddr
        call    #sqiSendByte
        add     hubaddr, #1
        djnz    count, #:loop
        call    #release
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
'   hubaddr is the hub address
'
'----------------------------------------------------------------------------------------------------

read_write_start
        mov     cmd, extaddr
        and     cmd, offset_bits
        or      cmd, fn
        or      dira, sio_mask
        andn    outa, sram_cs_mask
        rol     cmd, #8
        mov     data, cmd
        call    #sqiSendByte
        rol     cmd, #8
        mov     data, cmd
        call    #sqiSendByte
        rol     cmd, #8
        mov     data, cmd
        call    #sqiSendByte
        rol     cmd, #8
        mov     data, cmd
        call    #sqiSendByteX
        ' let the caller release the clock so that the bus
        ' can be switched to input before the clock goes low
        ' andn    outa, sck_mask
read_write_start_ret
        ret

' spi commands

read_jedec_id
        andn    outa, flash_cs_mask
        mov     data, frdjedecid
        call    #spiSendByte
        call    #spiRecvByte       ' manufacturer's id
        mov     t1, data
        ror     t1, #8             ' save mfg id
        call    #spiRecvByte       ' memory type
        movs    t1, data
        rol     t1, #8             ' shift to the correct bits
        call    #release
read_jedec_id_ret
        ret

halt    cogid   t1
        cogstop t1
        
' ****************************
' SST SST26VF016 SPI FUNCTIONS
' ****************************

#ifdef SST

flash_init
        call    #sst_read_jedec_id
        cmp     t1, jedec_id wz
  if_z  jmp     #:unprot
        call    #read_jedec_id
        cmp     t1, jedec_id wz
  if_nz jmp     #halt
        andn    outa, flash_cs_mask
        mov     data, sst_quadmode
        call    #spiSendByte
        call    #release
        call    #sst_read_jedec_id
        cmp     t1, jedec_id wz
  if_nz jmp     #halt
:unprot call    #sst_write_enable
        mov     cmd, sst_wrblkprot
        mov     bytes, #4
        call    #sst_start_quad_spi_cmd
        mov     data, #0
        call    #sqiSendByte    ' byte 4
        call    #sqiSendByte    ' byte 5
        call    #sqiSendByte    ' byte 6
        ' BUG: 4 more bytes are necessary for the SST26VF032 chip
        call    #release
flash_init_ret
        ret

erase_4k_block
        call    #sst_write_enable
        mov     cmd, extaddr
        and     cmd, offset_bits
        or      cmd, ferase4kblk
        mov     bytes, #4
        call    #sst_start_quad_spi_cmd
        call    #release
        call    #wait_until_done
erase_4k_block_ret
        ret

start_write
        call    #sst_write_enable
        mov     cmd, extaddr
        and     cmd, offset_bits
        or      cmd, sst_program
        mov     bytes, #4
        call    #sst_start_quad_spi_cmd
start_write_ret
        ret
        
start_read
        mov     cmd, extaddr
        and     cmd, offset_bits
        or      cmd, sst_read
        mov     bytes, #4
        call    #sst_start_quad_spi_cmd
        mov     data, #0
        call    #sqiSendByte
        andn    dira, sio_mask
start_read_ret
        ret
        
wait_until_done
        mov     cmd, frdstatus
        call    #sst_start_quad_spi_cmd_1
        andn    dira, sio_mask
:wait   call    #sqiRecvByte
        test    data, #$80 wz
  if_nz jmp     #:wait
        call    #release
wait_until_done_ret
        ret

sst_read_jedec_id
        mov     cmd, sst_rdjedecid
        call    #sst_start_quad_spi_cmd_1
        andn    dira, sio_mask
        call    #sqiRecvByte       ' manufacturer's id
        mov     t1, data
        ror     t1, #8             ' save mfg id
        call    #sqiRecvByte       ' memory type
        movs    t1, data
        rol     t1, #8             ' merge with mfg id
        call    #release
sst_read_jedec_id_ret
        ret
        
sst_write_enable
        mov     cmd, fwrenable
        call    #sst_start_quad_spi_cmd_1
        call    #release
sst_write_enable_ret
        ret

sst_start_quad_spi_cmd_1
        mov     bytes, #1
sst_start_quad_spi_cmd
        or      dira, sio_mask
        andn    outa, flash_cs_mask
:loop   rol     cmd, #8
        mov     data, cmd
        call    #sqiSendByte
        djnz    bytes, #:loop
sst_start_quad_spi_cmd_1_ret
sst_start_quad_spi_cmd_ret
        ret
        
jedec_id            long    $000026bf    ' value of t1 after read_jedec_id routine (SST26VF016)

sst_rdjedecid       long    $af000000    ' read the manufacturers id, device type and device id
sst_quadmode        long    $38          ' enable quad mode
sst_wrblkprot       long    $42000000    ' write block protect register
sst_program         long    $02000000    ' flash program byte/page
sst_read            long    $0b000000    ' flash read command

#endif

' ******************************
' WINBOND W25Q80BV SPI FUNCTIONS
' ******************************

#ifdef WINBOND

flash_init
        call    #read_jedec_id
        cmp     t1, jedec_id wz
  if_z  jmp     #id_ok
        cmp     t1, jedec_id2 wz
  if_nz jmp     #halt
id_ok   call    #winbond_write_enable
        mov     cmd, winbond_wrstatus
        call    #winbond_start_quad_spi_cmd_1
        mov     data, #$00
        call    #spiSendByte
        mov     data, #$02
        call    #spiSendByte
        call    #release
        call    #wait_until_done
flash_init_ret
        ret

erase_4k_block
        call    #winbond_write_enable
        mov     cmd, extaddr
        and     cmd, offset_bits
        or      cmd, ferase4kblk
        call    #winbond_start_quad_spi_cmd_1
        rol     cmd, #8
        mov     data, cmd
        call    #spiSendByte
        rol     cmd, #8
        mov     data, cmd
        call    #spiSendByte
        rol     cmd, #8
        mov     data, cmd
        call    #spiSendByte
        call    #release
        call    #wait_until_done
erase_4k_block_ret
        ret
        
start_write
        call    #winbond_write_enable
        mov     cmd, extaddr
        and     cmd, offset_bits
        or      cmd, winbond_program
        call    #winbond_start_quad_spi_cmd_1
        rol     cmd, #8
        mov     data, cmd
        call    #spiSendByte
        rol     cmd, #8
        mov     data, cmd
        call    #spiSendByte
        rol     cmd, #8
        mov     data, cmd
        call    #spiSendByte
        or      dira, sio_mask
start_write_ret
        ret
        
start_read
        mov     cmd, extaddr
        and     cmd, offset_bits
        or      cmd, winbond_read
        mov     bytes, #4
        call    #winbond_start_quad_spi_cmd
        mov     data, #0
        call    #sqiSendByte
        andn    dira, sio_mask
start_read_ret
        ret
        
wait_until_done
        mov     cmd, frdstatus
        call    #winbond_start_quad_spi_cmd_1
        andn    dira, sio_mask
:wait   call    #spiRecvByte
        test    data, #1 wz
  if_nz jmp     #:wait
        call    #release
wait_until_done_ret
        ret

winbond_write_enable
        mov     cmd, fwrenable
        call    #winbond_start_quad_spi_cmd_1
        call    #release
winbond_write_enable_ret
        ret
        
winbond_start_quad_spi_cmd_1
        mov     bytes, #1
winbond_start_quad_spi_cmd
        andn    outa, flash_cs_mask
        rol     cmd, #8
        mov     data, cmd
        call    #spiSendByte
        sub     bytes, #1 wz
  if_z  jmp     winbond_start_quad_spi_cmd_ret
        or      dira, sio_mask
:loop   rol     cmd, #8
        mov     data, cmd
        call    #sqiSendByte
        djnz    bytes, #:loop
winbond_start_quad_spi_cmd_1_ret
winbond_start_quad_spi_cmd_ret
        ret
        
jedec_id            long    $000040ef    ' value of t1 after read_jedec_id routine (W25QxxBV, W25QxxFV)
jedec_id2           long    $000060ef    ' value of t1 after read_jedec_id routine (W25Q80DW)

winbond_wrstatus    long    $01000000    ' write status
winbond_program     long    $32000000    ' flash program byte/page
winbond_read        long    $e3000000    ' flash read command

#endif

' **********************************
' END OF CHIP SPECIFIC SPI FUNCTIONS
' **********************************

release
        mov     outa, pinout
        mov     dira, pindir
release_ret
        ret

' flash commands
frdjedecid      long    $9f          ' read the manufacturers id, device type and device id
ferase4kblk     long    $20000000    ' flash erase a 4k block
frdstatus       long    $05000000    ' flash read status
fwrenable       long    $06000000    ' flash write enable

pindir          long    0
pinout          long    0

flash_cs_mask   long    0
sram_cs_mask    long    0
#ifdef SD_INIT        
sd_cs_mask      long    0
#endif

' variables used by the spi send/receive functions
fn              long    0
cmd             long    0
bytes           long    0

' sram commands
sram_read       long    $03000000       ' read command
sram_write      long    $02000000       ' write command
sram_eqio       long    $38000000       ' enter quad I/O mode
sram_seq        long    $01400000       ' %00000001_01000000 << 16 ' set sequential mode

flash_base      long    $30000000       ' base address of flash memory
offset_bits     long    $00ffffff       ' mask to isolate the offset bits
block_mask      long    $00000fff       ' 4k flash blocks

wrenable        long    1

        fit     496
