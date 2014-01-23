#undef FLASH
#define RW
#include "cache_common.spin"

init
        ' set the pin directions
        mov     outa, pinout
        mov     dira, pindir
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
'
' on output:
'   ptr is the hub address for the next read/write
'   count is the number of bytes to transfer
'
' trashes vmaddr
'
'----------------------------------------------------------------------------------------------------

BSTART
        or      dira, data_mask         ' set data bus to output
        movs    outa, vmaddr            ' load A7:0
        or      outa, cmd_byte0_latch
        andn    outa, cmd_mask
        shr     vmaddr, #8              ' load A15:8
        movs    outa, vmaddr
        or      outa, cmd_byte1_latch
        andn    outa, cmd_mask
        shr     vmaddr, #8              ' load A18:16
        movs    outa, vmaddr
        or      outa, cmd_byte2_latch
        andn    outa, cmd_mask
        mov     ptr, hubaddr            ' hubaddr = hub cache line address
BSTART_RET
        ret

'----------------------------------------------------------------------------------------------------
'
' BREAD
'
' on input:
'   vmaddr is the virtual memory address to read
'   hubaddr is the hub memory address to write
'   count is the number of longs to read
'
' trashes vmaddr, count, data, ptr
'
'----------------------------------------------------------------------------------------------------

BREAD
        call    #BSTART
        andn    dira, data_mask         ' set data bus to input
        mov     outa, cmd_rd
:loop   mov     data, ina
        or      outa, cmd_rd_count
        andn    outa, cmd_rd_count
        wrbyte  data, ptr
        add     ptr, #1
        djnz    count, #:loop
        andn    outa, cmd_mask
BREAD_RET
        ret

'----------------------------------------------------------------------------------------------------
'
' BWRITE
'
' on input:
'   vmaddr is the virtual memory address to write
'   hubaddr is the hub memory address to read
'   count is the number of longs to write
'
' trashes vmaddr, count, data, ptr
'
'----------------------------------------------------------------------------------------------------

BWRITE
        call    #BSTART
        or      dira, data_mask         ' set data bus to output
:loop   rdbyte  outa, ptr
        or      outa, cmd_wr
        or      outa, cmd_wr_count
        add     ptr, #1
        djnz    count, #:loop
        andn    outa, cmd_mask
BWRITE_RET
        ret

' temporaries used by BREAD and BWRITE
ptr             long    0
data            long    0

' command codes
cmd_wr          long    $00010000
cmd_rd          long    $00020000
cmd_wr_count    long    $00020000
cmd_rd_count    long    $00010000
cmd_byte1_latch long    $00040000
cmd_byte2_latch long    $00050000
cmd_byte0_latch long    $00060000
cmd_status_led  long    $00070000

cmd_mask        long    $00070000
data_mask       long    $000000ff

pindir          long    $00070000
pinout          long    $00000000
