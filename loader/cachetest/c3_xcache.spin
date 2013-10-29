CON

  INC_PIN               = 8
  MOSI_PIN              = 9
  MISO_PIN              = 10
  CLK_PIN               = 11
  CLR_PIN               = 25

#define FLASH
#define RW
#define BLOCK_IO

#include "cache_common.spin"

init
        ' set the pin directions
        mov     outa, pinout
        mov     dira, pindir
        call    #deselect

        ' select sequential access mode for the first SRAM chip
        call    #sram_chip1
        mov     data, ramseq
        mov     bits, #16
        call    #send
        call    #deselect

        ' select sequential access mode for the second SRAM chip
        call    #sram_chip2
        mov     data, ramseq
        mov     bits, #16
        call    #send
        call    #deselect
		
        ' unprotect the entire flash chip
        call    #write_enable
        call    #flash_chip
        mov     data, fwrstatus ' write zero to the status register
        mov     bits, #16
        call    #send
        call    #deselect
init_ret
		ret

erase_4k_block
        call    #write_enable
        call    #flash_chip
        mov     data, vmaddr
        or      data, ferase4kblk
        mov     bits, #32
        call    #send
        call    #deselect
        call    #wait_until_done
erase_4k_block_ret
		ret

wloop   test    vmaddr, #$ff wz
  if_nz jmp     #wdata
        call    #deselect
        call    #wait_until_done
write_block
        call    #write_enable
        call    #flash_chip
        mov     data, vmaddr
        or      data, fprogram
        mov     bits, #32
        call    #send
wdata   rdbyte  data, hubaddr
        shl     data, #24
        mov     bits, #8
        call    #send
        add     hubaddr, #1
        add     vmaddr, #1
        djnz    count, #wloop
        call    #deselect
        call    #wait_until_done
write_block_ret
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
'   hubaddr is the hub address for the next read/write
'   count is the number of bytes to transfer
'
' trashes t1
'
'----------------------------------------------------------------------------------------------------

BSTART
        test    vmaddr, bit16 wz
  if_z  call    #sram_chip1      ' select first SRAM chip
  if_nz call    #sram_chip2      ' select second SRAM chip
        mov     data, vmaddr
        shl     data, #8          ' move it into position for transmission
        or      data, fn
        mov     bits, #24
        call    #send
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
' trashes fn, count, bits, data, ptr, t1, c and z flags
'
'----------------------------------------------------------------------------------------------------

BREAD
        mov     ptr, hubaddr
        cmp     vmaddr, flash_base wc
  if_b  jmp     #BREAD_sram
        call    #flash_chip       ' select flash chip
        mov     data, vmaddr
        and     data, offset_bits
        or      data, fread
        mov     bits, #40         ' includes 8 dummy bits
        call    #send

BREAD_DATA
:rloop  call    #spiRecvByte
        wrbyte  data, ptr
        add     ptr, #1
        djnz    count, #:rloop
        call    #deselect
BREAD_RET
        ret

BREAD_sram
        mov     fn, read
        call    #BSTART
        jmp     #BREAD_DATA

'----------------------------------------------------------------------------------------------------
'
' BWRITE
'
' on input:
'   vmaddr is the virtual memory address to write
'   hubaddr is the hub memory address to read
'   count is the number of longs to write
'
' trashes fn, count, bits, data, ptr, z flag
'
'----------------------------------------------------------------------------------------------------

BWRITE
        mov     ptr, hubaddr
        cmp     vmaddr, flash_base wc
  if_ae jmp     BWRITE_RET

BWRITE_sram
        mov     fn, write
        call    #BSTART
:wloop  rdbyte  data, ptr
        call    #spiSendByte
        add     ptr, #1
        djnz    count, #:wloop
        call    #deselect
BWRITE_RET
        ret

fn      long    0
ptr     long    0

'----------------------------------------------------------------------------------------------------
' SPI routines
'----------------------------------------------------------------------------------------------------

spiSendByte
        shl     data, #24
        mov     bits, #8
        jmp     #send

send0   andn    outa, TCLK

' CLK should be low coming into this function
send    rol     data, #1 wc
        muxc    outa, TMOSI
        or      outa, TCLK
        djnz    bits, #send0
        andn    outa, TCLK
        or      outa, TMOSI
spiSendByte_ret
send_ret
        ret

spiRecvByte
        mov     data, #0
        mov     bits, #8
        
' CLK was set H-L and data should be ready before this function starts
receive or      outa, TCLK
        test    TMISO, ina wc
        rcl     data, #1
        andn    outa, TCLK
        djnz    bits, #receive
spiRecvByte_ret
receive_ret
        ret

' spi select functions
' all trash t1

sram_chip1
        mov     t1, #1
        jmp     #select

sram_chip2
        mov     t1, #2
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
flash_chip_ret
        ret

deselect
        andn    outa, TCLR
        or      outa, TCLR
deselect_ret
        ret

write_enable
        call    #flash_chip
        mov     data, fwrenable
        mov     bits, #8
        call    #send
        call    #deselect
write_enable_ret
        ret

write_disable
        call    #flash_chip
        mov     data, fwrdisable
        mov     bits, #8
        call    #send
        call    #deselect
write_disable_ret
        ret

wait_until_done
        call    #flash_chip
        mov     data, frdstatus
        mov     bits, #8
        call    #send
:wait   mov     bits, #8
        call    #receive
        test    data, #1 wz
  if_nz jmp     #:wait
        call    #deselect
        and     data, #$ff
wait_until_done_ret
        ret

pindir      long    (1<<CLR_PIN)|(1<<INC_PIN)|(1<<CLK_PIN)|(1<<MOSI_PIN)
pinout      long    (1<<CLR_PIN)|(0<<INC_PIN)|(0<<CLK_PIN)|(1<<MOSI_PIN)

tclr        long    1<<CLR_PIN
tinc        long    1<<INC_PIN
tclk        long    1<<CLK_PIN
tmosi       long    1<<MOSI_PIN
tmiso       long    1<<MISO_PIN

' temporaries used by send
bits        long    0
data        long    0

read        long    $03000000       ' read command
write       long    $02000000       ' write command
ramseq      long    $01400000       ' %00000001_01000000 << 16 ' set sequential mode
readstat    long    $05000000       ' read status

fread       long    $0b000000       ' flash read command
ferasechip  long    $60000000       ' flash erase chip
ferase4kblk long    $20000000       ' flash erase a 4k block
fprogram    long    $02000000       ' flash program byte/page
fwrenable   long    $06000000       ' flash write enable
fwrdisable  long    $04000000       ' flash write disable
frdstatus   long    $05000000       ' flash read status
fwrstatus   long    $01000000       ' flash write status

bit16       long    $00008000       ' mask to select the SRAM chip
flash_base  long    $30000000       ' base address of flash memory
offset_bits long    $00ffffff       ' mask to isolate the offset bits

            fit     496
