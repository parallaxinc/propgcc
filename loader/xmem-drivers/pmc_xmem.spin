{
  Propeller Memory Card External Memory Driver
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

'#define SD_INIT        

CON

    FLASH_RDJEDECID     = $9f
    FLASH_ERASE4KBLK    = $20
    FLASH_RDSTATUS      = $05
    FLASH_WRSTATUS      = $01
    FLASH_WRSTATUS2     = $31
    FLASH_WRENABLE      = $06
    FLASH_ENTERQPI      = $38
    FLASH_EXITQPI       = $ff
    FLASH_PROGRAM       = $02
    FLASH_READ          = $0b
    FLASH_QE_MASK       = $02   ' in status2
    
    SRAM_WRSTATUS       = $01
    SRAM_SEQ_MASK       = $40
    SRAM_EQIO           = $38
    SRAM_READ           = $03
    SRAM_WRITE          = $02

    ' SD commands
    CMD0_GO_IDLE_STATE  = $40

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
:done   call    #release
:skip
#endif
                
        ' initialize the flash chip
        call    #flash_init
        
        ' select sequential access mode for the SRAM chip
        andn    outa, sram_cs_mask
        mov     data, #SRAM_WRSTATUS
        call    #spiSendByte
        mov     data, #SRAM_SEQ_MASK
        call    #spiSendByte
        call    #release
        
        ' switch to quad mode for the SRAM chip
        andn    outa, sram_cs_mask
        mov     data, #SRAM_EQIO
        call    #spiSendByte
        call    #release

        ' setup pins for qpi mode
        andn    pinout, sio_mask
        or      pindir, sio_mask
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
        call    #start_flash_read
:loop   call    #sqiRecvByte
        wrbyte  data, hubaddr
        add     hubaddr, #1
        djnz    count, #:loop
read_bytes_done
        call    #release
read_bytes_ret
        ret

read_bytes_sram
        mov     data, #SRAM_READ
        call    #start_sram_read_write 
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
        call    #qpi_wait_until_done
:addr   call    #start_flash_write
:data   rdbyte  data, hubaddr
        call    #sqiSendByte
        add     hubaddr, #1
        add     extaddr, #1
        djnz    count, #:loop
        call    #release
        call    #qpi_wait_until_done
        jmp     write_bytes_ret

write_bytes_sram
        mov     data, #SRAM_WRITE
        call    #start_sram_read_write
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
' start_sram_read_write
'
' select the chip and send the address for the read/write operation
'
' on input:
'   data is the sram command
'   extaddr is the sram transfer address
'
'----------------------------------------------------------------------------------------------------

start_sram_read_write
        andn    outa, sram_cs_mask
        call    #sqiSendByte
        mov     data, extaddr
        shr     data, #16
        call    #sqiSendByte
        mov     data, extaddr
        shr     data, #8
        call    #sqiSendByte
        mov     data, extaddr
        call    #sqiSendByteX
        ' let the caller release the clock so that the bus
        ' can be switched to input before the clock goes low
        ' andn    outa, sck_mask
start_sram_read_write_ret
        ret

' spi commands

flash_init
  
        ' write status registers 1 and 2
        call    #spi_write_enable
        andn    outa, flash_cs_mask
        mov     data, #FLASH_WRSTATUS
        call    #spiSendByte
        mov     data, #0                ' status register 1 - clear protect bits
        call    #spiSendByte
        mov     data, #FLASH_QE_MASK    ' status register 2 - set QE bit
        call    #spiSendByte
        call    #release
        call    #spi_wait_until_done
        
        ' enter qpi mode
        andn    outa, flash_cs_mask
        mov     data, #FLASH_ENTERQPI
        call    #spiSendByte
        call    #release

flash_init_ret
        ret

erase_4k_block
#ifdef NOTDEF
        call    #qpi_write_enable
        mov     data, #FLASH_ERASE4KBLK
        call    #start_flash_read_write
        call    #release
        call    #qpi_wait_until_done
#else
        andn    outa, flash_cs_mask
        mov     data, #FLASH_EXITQPI
        call    #sqiSendByte
        call    #release
        andn    dira, miso_mask     ' make this an input for SPI mode
        mov     data, #FLASH_ERASE4KBLK
        call    #spiSendByte
        mov     data, extaddr
        shr     data, #16
        call    #spiSendByte
        mov     data, extaddr
        shr     data, #8
        call    #spiSendByte
        mov     data, extaddr
        call    #spiSendByte
        or      outa, flash_cs_mask
        andn    outa, flash_cs_mask
        mov     data, #FLASH_RDSTATUS
        call    #spiSendByte
:wait   call    #spiRecvByte
        test    data, #1 wz
  if_nz jmp     #:wait
        or      outa, flash_cs_mask
        andn    outa, flash_cs_mask
        mov     data, #FLASH_ENTERQPI
        call    #spiSendByte
        call    #release
#endif
erase_4k_block_ret
        ret
        
start_flash_write
        call    #qpi_write_enable
        mov     data, #FLASH_PROGRAM
        call    #start_flash_read_write
start_flash_write_ret
        ret
        
start_flash_read
        mov     data, #FLASH_READ
        call    #start_flash_read_write
        mov     data, #0
        call    #sqiSendByteX
        andn    dira, sio_mask
        andn    outa, sck_mask
start_flash_read_ret
        ret
        
start_flash_read_write
        andn    outa, flash_cs_mask
        call    #sqiSendByte
        mov     data, extaddr
        shr     data, #16
        call    #sqiSendByte
        mov     data, extaddr
        shr     data, #8
        call    #sqiSendByte
        mov     data, extaddr
        call    #sqiSendByte
start_flash_read_write_ret
        ret

spi_write_enable
        andn    outa, flash_cs_mask
        mov     data, #FLASH_WRENABLE
        call    #spiSendByte
        call    #release
spi_write_enable_ret
        ret
        
spi_wait_until_done
        andn    outa, flash_cs_mask
        mov     data, #FLASH_RDSTATUS
        call    #spiSendByte
:wait   call    #spiRecvByte
        test    data, #1 wz
  if_nz jmp     #:wait
        call    #release
spi_wait_until_done_ret
        ret

qpi_write_enable
        andn    outa, flash_cs_mask
        mov     data, #FLASH_WRENABLE
        call    #sqiSendByte
        call    #release
qpi_write_enable_ret
        ret
        
qpi_wait_until_done
        andn    outa, flash_cs_mask
        mov     data, #FLASH_RDSTATUS
        call    #sqiSendByteX
        andn    dira, sio_mask
        andn    outa, sck_mask
:wait   call    #sqiRecvByte
        test    data, #1 wz
  if_nz jmp     #:wait
        call    #release
qpi_wait_until_done_ret
        ret

release
        mov     outa, pinout
        mov     dira, pindir
release_ret
        ret

pindir          long    0
pinout          long    0

flash_cs_mask   long    0
sram_cs_mask    long    0
#ifdef SD_INIT        
sd_cs_mask      long    0
#endif

flash_base      long    $30000000       ' base address of flash memory
block_mask      long    $00000fff       ' 4k flash blocks

wrenable        long    1

        fit     496
