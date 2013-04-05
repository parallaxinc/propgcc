#undef FLASH
#define RW
#include "cache_common.spin"
#include "cache_sqi_pins.spin"

init
        call    #get_sqi_pins

        ' set the pin directions
        mov     outa, pinout
        mov     dira, pindir
        call    #release
        
        ' select sequential access mode for the SRAM chip
        call    #select
        mov     data, ramseq
        mov     bits, #16
        call    #send
        call    #release
        
        ' switch to quad mode
        call    #select
        mov     data, eqio
        mov     bits, #8
        call    #send
        call    #release
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
        mov     cmd, vmaddr
        and     cmd, sram_mask
        or      cmd, fn
        or      dira, sio_mask
        call    #select
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
        call    #sqiSendByte
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
        mov     data, #0
        call    #sqiSendByte
        andn    dira, sio_mask
:loop   call    #sqiRecvByte
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
        call    #sqiSendByte
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
:loop   or      outa, sck_mask
        test    miso_mask, ina wc
        rcl     data, #1
        andn    outa, sck_mask
        djnz    bits, #:loop
spiRecvByte_ret
        ret

sqiSendByte
        mov     bits, data
        ror     bits, #4
        rol     bits, sio_shift
        and     bits, sio_mask
        andn    outa, sio_mask
        or      outa, bits
        or      outa, sck_mask
        andn    outa, sck_mask
        rol     data, sio_shift
        and     data, sio_mask
        andn    outa, sio_mask
        or      outa, data
        or      outa, sck_mask
        andn    outa, sck_mask
sqiSendByte_ret
        ret

sqiRecvByte
        or      outa, sck_mask
        mov     data, ina
        and     data, sio_mask
        rol     data, #4
        andn    outa, sck_mask
        or      outa, sck_mask
        mov     bits, ina
        and     bits, sio_mask
        or      data, bits
        ror     data, sio_shift
        andn    outa, sck_mask
sqiRecvByte_ret
        ret

pindir      long    0
pinout      long    0

mosi_mask   long    0
miso_mask   long    0
sck_pin     long    0
sck_mask    long    0
sio_shift   long    0
sio_mask    long    0

' variables used by the spi send/receive functions
fn          long    0
cmd         long    0
data        long    0
bits        long    0
ptr         long    0

' spi commands
read        long    $03000000       ' read command
write       long    $02000000       ' write command
eqio        long    $38000000       ' enter quad I/O mode
rstio       long    $ffffffff       ' reset quad I/O mode
ramseq      long    $01400000       ' %00000001_01000000 << 16 ' set sequential mode

sram_mask   long    $00ffffff       ' mask to isolate the sram offset bits

            FIT     496
