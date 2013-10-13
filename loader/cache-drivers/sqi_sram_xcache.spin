#undef FLASH
#define RW
#define BLOCK_IO

#include "cache_common.spin"
#include "cache_sqi_pins.spin"
#include "cache_sqi.spin"

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
        call    #spiSend
        call    #release
        
        ' switch to quad mode
        call    #select
        mov     data, eqio
        mov     bits, #8
        call    #spiSend
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
        andn    dira, sio_mask
        call    #sqiRecvByte    ' dummy byte
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

pindir      long    0
pinout      long    0

' variables used by the spi send/receive functions
fn          long    0
cmd         long    0
ptr         long    0

' spi commands
read        long    $03000000       ' read command
write       long    $02000000       ' write command
eqio        long    $38000000       ' enter quad I/O mode
ramseq      long    $01400000       ' %00000001_01000000 << 16 ' set sequential mode

sram_mask   long    $00ffffff       ' mask to isolate the sram offset bits

            FIT     496
