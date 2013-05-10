#undef FLASH
#define RW
#include "cache_common.spin"
#include "cache_spi_pins.spin"
#include "cache_spi.spin"

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
        call    #spiSend
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
        shl     data, #8
        or      data, fn
        mov     bits, #24
        call    #spiSend
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

pindir      long    0
pinout      long    0

' variables used by the spi send/receive functions
fn          long    0
cmd         long    0
ptr         long    0

' spi commands
read        long    $03000000       ' read command
write       long    $02000000       ' write command
ramseq      long    $01400000       ' %00000001_01000000 << 16 ' set sequential mode

            FIT     496
