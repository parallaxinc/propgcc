' param2: 0xssccffrr - ss=sio0 cc=sck ff=flash-cs rr=sram-cs

'#define WINBOND
'#define SST

#define FLASH
#define RW
#define BLOCK_IO

#include "cache_common.spin"
#include "cache_sqi.spin"

init
        ' get the pin definitions (cache-param2)
        ' 0xssccffrr where ss=sio0, cc=clk, ff=flash cs, ss=sram cs
        rdlong  t2, t1

        ' get the sio_shift and build the mosi, miso, and sio masks
        mov     sio_shift, t2
        shr     sio_shift, #24
        mov     mosi_mask, #1
        shl     mosi_mask, sio_shift
        mov     miso_mask, mosi_mask
        shl     miso_mask, #1
        mov     sio_mask, #$f
        shl     sio_mask, sio_shift
        or      pindir, mosi_mask
        or      pinout, mosi_mask
        
        ' make the sio2 and sio3 pins outputs in single spi mode to assert /WE and /HOLD
        mov     t3, #$0c
        shl     t3, sio_shift
        or      pindir, t3
        or      pinout, t3
                
        ' build the sck mask
        mov     t3, t2
        shr     t3, #16
        and     t3, #$ff
        mov     sck_mask, #1
        shl     sck_mask, t3
        or      pindir, sck_mask
        
        ' get the chip selects
        mov     t3, t2
        shr     t3, #8
        and     t3, #$ff
        mov     flash_cs_mask, #1
        shl     flash_cs_mask, t3
        and     t2, #$ff
        mov     sram_cs_mask, #1
        shl     sram_cs_mask, t2
        or      pinout, flash_cs_mask
        or      pindir, flash_cs_mask
        or      pinout, sram_cs_mask
        or      pindir, sram_cs_mask

        ' set the pin directions
        mov     outa, pinout
        mov     dira, pindir
        call    #release
                
        ' initialize the flash chip
        call    #flash_init
        
        ' select sequential access mode for the SRAM chip
        call    #select_sram
        mov     data, sram_seq
        mov     bits, #16
        call    #spiSend
        call    #release
        
        ' switch to quad mode for the SRAM chip
        call    #select_sram
        mov     data, sram_eqio
        mov     bits, #8
        call    #spiSend
        call    #release

init_ret
        ret
                
'----------------------------------------------------------------------------------------------------
'
' erase_4k_block
'
' on input:
'   vmaddr is the virtual memory address to erase
'
'----------------------------------------------------------------------------------------------------

#ifdef SST

erase_4k_block
        call    #sst_write_enable
        mov     cmd, vmaddr
        and     cmd, offset_bits
        or      cmd, ferase4kblk
        mov     bytes, #4
        call    #sst_start_quad_spi_cmd
        call    #release
        call    #wait_until_done
erase_4k_block_ret
        ret

#endif

#ifdef WINBOND

erase_4k_block
        call    #winbond_write_enable
        mov     cmd, vmaddr
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
        
#endif

'----------------------------------------------------------------------------------------------------
'
' write_block
'
' on input:
'   vmaddr is the virtual memory address to write
'   hubaddr is the hub memory address to read
'   count is the number of bytes to write
'
'----------------------------------------------------------------------------------------------------

_wloop  test    vmaddr, #$ff wz
  if_nz jmp     #_wdata
        call    #release
        call    #wait_until_done
write_block
        call    #start_write
_wdata  rdbyte  data, hubaddr
        call    #sqiSendByte
        add     hubaddr, #1
        add     vmaddr, #1
        djnz    count, #_wloop
        call    #release
        call    #wait_until_done
write_block_ret
        ret
        
'----------------------------------------------------------------------------------------------------
'
' BSTART_sram
'
' select the chip and send the address for the read/write operation
'
' on input:
'   vmaddr is the sram transfer address
'   hubaddr is the hub address
'
'----------------------------------------------------------------------------------------------------

BSTART_sram
        mov     cmd, vmaddr
        and     cmd, offset_bits
        or      cmd, fn
        or      dira, sio_mask
        call    #select_sram
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
BSTART_sram_RET
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
        cmp     vmaddr, flash_base wc
  if_b  jmp     #BREAD_sram
        call    #start_read
        mov     ptr, hubaddr      ' hubaddr = hub page address
        mov     count, line_size
:loop   call    #sqiRecvByte
        wrbyte  data, ptr
        add     ptr, #1
        djnz    count, #:loop
BREAD_done
        call    #release
BREAD_RET
        ret

BREAD_sram
        mov     fn, sram_read
        call    #BSTART_sram 
        mov     data, #0
        call    #sqiSendByte
        andn    dira, sio_mask
:loop   call    #sqiRecvByte
        wrbyte  data, ptr
        add     ptr, #1
        djnz    count, #:loop
        jmp     #BREAD_done

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
        cmp     vmaddr, flash_base wc
  if_ae jmp     BWRITE_RET  ' no writes to flash
        mov     fn, sram_write
        call    #BSTART_sram
:loop   rdbyte  data, ptr
        call    #sqiSendByte
        add     ptr, #1
        djnz    count, #:loop
        call    #release
BWRITE_RET
        ret

' spi commands

read_jedec_id
        call    #select_flash
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
        call    #select_flash
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

start_write
        call    #sst_write_enable
        mov     cmd, vmaddr
        and     cmd, offset_bits
        or      cmd, sst_program
        mov     bytes, #4
        call    #sst_start_quad_spi_cmd
start_write_ret
        ret
        
start_read
        mov     cmd, vmaddr
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
        call    #select_flash
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

start_write
        call    #winbond_write_enable
        mov     cmd, vmaddr
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
        mov     cmd, vmaddr
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
        call    #select_flash
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
jedec_id2            long   $000060ef    ' value of t1 after read_jedec_id routine (W25Q80DW)

winbond_wrstatus    long    $01000000    ' write status
winbond_program     long    $32000000    ' flash program byte/page
winbond_read        long    $e3000000    ' flash read command

#endif

' **********************************
' END OF CHIP SPECIFIC SPI FUNCTIONS
' **********************************

select_flash
        andn    outa, flash_cs_mask
select_flash_ret
        ret
                
select_sram
        andn    outa, sram_cs_mask
select_sram_ret
        ret
                
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

' variables used by the spi send/receive functions
fn          long    0
cmd         long    0
bytes       long    0
ptr         long    0

' sram commands
sram_read   long    $03000000       ' read command
sram_write  long    $02000000       ' write command
sram_eqio   long    $38000000       ' enter quad I/O mode
sram_seq    long    $01400000       ' %00000001_01000000 << 16 ' set sequential mode

flash_base  long    $30000000       ' base address of flash memory
offset_bits long    $00ffffff       ' mask to isolate the offset bits

            FIT     496
