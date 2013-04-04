CON

#undef FLASH
#define RW
#include "cache_common.spin"
#include "cache_spi_pins.spin"

init
        call    #get_spi_pins

        ' set the pin directions
        mov     outa, pinout
        mov     dira, pindir
        call    #release
        
        ' select sequential access mode for the SRAM chip
        call    #select
        mov     data, ramseq
        mov     bits, #16
        call    #send
init_ret
        ret
        
'----------------------------------------------------------------------------------------------------
'
' BSTART
'
' select the chip and send the address for the read/write operation
'
' on input:
'   vmaddr is the sram transfer address
'   hubaddr is the hub address
'
'----------------------------------------------------------------------------------------------------

BSTART
        call    #select
        mov     data, vmaddr
'        shl     data, #8          ' move it into position for transmission
        and     data, sram_mask
        or      data, fn
'        mov     bits, #24
        mov     bits, #32
        call    #send
        mov     ptr, hubaddr
BSTART_RET
        ret

'----------------------------------------------------------------------------------------------------
'
' BREAD
'
' on input:
'   vmaddr is the virtual memory address to read
'   hubaddr is the hub memory address to write
'   count is the number of bytes to read
'
'----------------------------------------------------------------------------------------------------

BREAD
        mov     fn, read
        call    #BSTART 
:loop   call    #spiRecvByte
        wrbyte  data, ptr
        add     ptr, #1
        djnz    count, #:loop
        call    #release
BREAD_RET
        ret

'----------------------------------------------------------------------------------------------------
'
' BWRITE
'
' on input:
'   vmaddr is the virtual memory address to write
'   hubaddr is the hub memory address to read
'   count is the number of bytes to write
'
'----------------------------------------------------------------------------------------------------

BWRITE
        mov     fn, write
        call    #BSTART
:loop   rdbyte  data, ptr
        call    #spiSendByte
        add     ptr, #1
        djnz    count, #:loop
        call    #release
BWRITE_RET
        ret

spiSendByte
        shl     data, #24
        mov     bits, #8
send    rol     data, #1 wc
        muxc    outa, mosi_mask
        or      outa, sck_mask
        andn    outa, sck_mask
        djnz    bits, #send
        or      outa, mosi_mask
spiSendByte_ret
send_ret
        ret

spiRecvByte
        mov     data, #0
        mov     bits, #8
recv    or      outa, sck_mask
        test    miso_mask, ina wc
        rcl     data, #1
        andn    outa, sck_mask
        djnz    bits, #recv
spiRecvByte_ret
recv_ret
        ret

pindir      long    0
pinout      long    0

mosi_pin    long    0
mosi_mask   long    0
miso_pin    long    0
miso_mask   long    0
sck_pin     long    0
sck_mask    long    0

' variables used by the spi send/receive functions
fn          long    0
cmd         long    0
data        long    0
bits        long    0
ptr         long    0

' spi commands
read        long    $03000000       ' read command
write       long    $02000000       ' write command
ramseq      long    $01400000       ' %00000001_01000000 << 16 ' set sequential mode

sram_mask   long    $00ffffff       ' mask to isolate the sram offset bits

            FIT     496
