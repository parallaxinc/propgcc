{
  SPI Flash External Memory Driver
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
#include "xmem_spi_pins.spin"
#include "xmem_spi.spin"

DAT

init

        ' get the pin masks
        call    #get_spi_pins

        ' set the pin directions
        mov     outa, pinout
        mov     dira, pindir
        call    #release
        
        ' clear the status register
        call    #clear_status_reg
        
init_ret
        ret
                
'----------------------------------------------------------------------------------------------------
'
' read_bytes
'
' on input:
'   extaddr is the external memory address to read
'   hubaddr is the hub memory address to write
'   count is the number of bytes to read
'
'----------------------------------------------------------------------------------------------------

read_bytes
        mov     cmd, extaddr
        and     cmd, flash_mask
        or      cmd, fread
        mov     bytes, #5
        call    #start_spi_cmd
:loop   call    #spiRecvByte
        wrbyte  data, hubaddr
        add     hubaddr, #1
        djnz    count, #:loop
        call    #release
read_bytes_ret
        ret

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

#ifdef SST

write_bytes
        tjz     wrenable, write_bytes_ret
        tjz     extaddr, #disable_writes
        test    extaddr, block_mask wz
  if_z  call    #erase_4k_block
:loop   call    #write_enable
        mov     cmd, extaddr
        and     cmd, flash_mask
        or      cmd, fprogram
        mov     bytes, #4
        call    #start_spi_cmd
:data   rdbyte  data, hubaddr
        call    #spiSendByte
        call    #release
        call    #wait_until_done
        add     extaddr, #1
        add     hubaddr, #1
        djnz    count, #:loop
write_bytes_ret
        ret
        
#else

write_bytes
        tjz     wrenable, write_bytes_ret
        tjz     extaddr, #disable_writes
        test    extaddr, block_mask wz
  if_z  call    #erase_4k_block
        jmp     #:addr
:loop   test    extaddr, #$ff wz
  if_nz jmp     #:data
        call    #release
        call    #wait_until_done
:addr   call    #write_enable
        mov     cmd, extaddr
        and     cmd, flash_mask
        or      cmd, fprogram
        mov     bytes, #4
        call    #start_spi_cmd
:data   rdbyte  data, hubaddr
        call    #spiSendByte
        add     extaddr, #1
        add     hubaddr, #1
        djnz    count, #:loop
        call    #release
:done   call    #wait_until_done
write_bytes_ret
        ret
        
#endif

erase_4k_block
        call    #write_enable
        mov     cmd, extaddr
        and     cmd, flash_mask
        or      cmd, ferase4kblk
        mov     bytes, #4
        call    #start_spi_cmd
        call    #release
        call    #wait_until_done
erase_4k_block_ret
        ret
        
disable_writes
        mov     wrenable, #0
        jmp     write_bytes_ret

' spi commands

#ifdef SST

clear_status_reg
        mov     cmd, fwrsenable
        call    #start_spi_cmd_1
        call    #release
        mov     cmd, fwrstatus
        mov     bytes, #2
        call    #start_spi_cmd
        call    #release
clear_status_reg_ret
        ret

#else

clear_status_reg
        call    #write_enable
        mov     cmd, fwrstatus
        mov     bytes, #2
        call    #start_spi_cmd
        call    #release
clear_status_reg_ret
        ret

#endif

write_enable
        mov     cmd, fwrenable
        call    #start_spi_cmd_1
        call    #release
write_enable_ret
        ret

wait_until_done
        mov     cmd, frdstatus
        call    #start_spi_cmd_1
:wait   call    #spiRecvByte
        test    data, #1 wz
  if_nz jmp     #:wait
        call    #release
wait_until_done_ret
        ret

start_spi_cmd_1
        mov     bytes, #1
start_spi_cmd
        call    #select
:loop   rol     cmd, #8
        mov     data, cmd
        call    #spiSendByte
        djnz    bytes, #:loop
start_spi_cmd_1_ret
start_spi_cmd_ret
        ret
        
pindir      long    0
pinout      long    0

' variables used by the spi send/receive functions
cmd         long    0
bytes       long    0

' spi commands
fprogram    long    $02000000       ' flash program byte/page
fread       long    $0b000000       ' flash read command
frdjedecid  long    $9f000000       ' read the manufacturers id, device type and device id
ferase4kblk long    $20000000       ' flash erase a 4k block
#ifdef SST
fwrsenable  long    $50000000       ' SST write status register enable
#endif
frdstatus   long    $05000000       ' flash read status
fwrstatus   long    $01000000       ' flash write status
fwrenable   long    $06000000       ' flash write enable

block_mask  long    $00000fff       ' 4k block address mask
flash_mask  long    $00ffffff       ' mask off the base address of external memory

wrenable    long    1

            fit     496
