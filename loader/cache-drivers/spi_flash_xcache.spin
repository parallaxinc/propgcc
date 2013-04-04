CON

#define FLASH
#undef RW
#include "cache_common.spin"
#include "cache_spi_pins.spin"

init
        call    #get_spi_pins
        call    #spiInit

        ' get the jedec id (cache-param4)
        rdlong  t2, t1 wz
  if_nz mov     jedec_id, t2

        ' set the pin directions
        mov     outa, pinout
        mov     dira, pindir
        call    #release
        
        ' check the jedec id
'         call    #read_jedec_id
'         cmp     t1, jedec_id wz
'halt
'   if_nz jmp     #halt
        
        ' clear the status register
        call    #clear_status_reg
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

erase_4k_block
        call    #write_enable
        mov     cmd, vmaddr
        and     cmd, flash_mask
        or      cmd, ferase4kblk
        mov     bytes, #4
        call    #start_spi_cmd
        call    #release
        call    #wait_until_done
erase_4k_block_ret
		ret
		
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

#ifdef SST

write_block
:loop   call    #write_enable
        mov     cmd, vmaddr
        and     cmd, flash_mask
        or      cmd, fprogram
        mov     bytes, #4
        call    #start_spi_cmd
:data   rdbyte  data, hubaddr
        call    #spiSendByte
        call    #release
        call    #wait_until_done
        add     hubaddr, #1
        add     vmaddr, #1
        djnz    count, #:loop
write_block_ret
		ret

#else

wloop   test    vmaddr, #$ff wz
  if_nz jmp     #wdata
        call    #release
        call    #wait_until_done
write_block
        call    #write_enable
        mov     cmd, vmaddr
        and     cmd, flash_mask
        or      cmd, fprogram
        mov     bytes, #4
        call    #start_spi_cmd
wdata   rdbyte  data, hubaddr
        call    #spiSendByte
        add     hubaddr, #1
        add     vmaddr, #1
        djnz    count, #wloop
        call    #release
        call    #wait_until_done
write_block_ret
		ret

#endif

' spi commands

read_jedec_id
        mov     cmd, frdjedecid
        call    #start_spi_cmd_1
        call    #spiRecvByte       ' manufacturer's id
        mov     t1, data
        ror     t1, #8             ' save mfg id
        call    #spiRecvByte       ' memory type
        movs    t1, data
        ror     t1, #8             ' save dev type
        call    #spiRecvByte       ' device capacity
        movs    t1, data           ' save dev type
        rol     t1, #16            ' data 00ccttmm c=capacity, t=type, m=mfgid
        call    #release
read_jedec_id_ret
        ret

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
        
' t1-t3 are defined in cache_common.spin
t4              long    0       ' temporary variable

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
        mov     cmd, vmaddr
        and     cmd, flash_mask
        or      cmd, fread
        mov     bytes, #5
        call    #start_spi_cmd
:loop   call    #spiRecvByte
        wrbyte  data, hubaddr
        add     hubaddr, #1
        djnz    count, #:loop
        call    #release
BREAD_RET
        ret

#ifdef FAST

{{
  Counter-based SPI code based on code from:
  
  SPI interface routines for SD & SDHC & MMC cards

  Jonathan "lonesock" Dummer
  version 0.3.0  2009 July 19
}}

spiInit
        or writeMode,mosi_pin
        or clockLineMode,sck_pin
        mov ctra,writeMode      ' Counter A drives data out
        mov ctrb,clockLineMode  ' Counter B will always drive my clock line
spiInit_ret
        ret
        
clockLineMode   long    %00100 << 26
writeMode       long    %00100 << 26
        
spiSendByte
        mov phsa,data
        shl phsa,#24
        andn outa,mosi_mask 
        'movi phsb,#%11_0000000
        mov phsb,#0
        movi frqb,#%01_0000000        
        rol phsa,#1
        rol phsa,#1
        rol phsa,#1
        rol phsa,#1
        rol phsa,#1
        rol phsa,#1
        rol phsa,#1
        mov frqb,#0
        ' don't shift out the final bit...already sent, but be aware 
        ' of this when sending consecutive bytes (send_cmd, for e.g.) 
spiSendByte_ret
        ret

spiRecvByte
        neg phsa,#1' DI high
        mov data,#0
        ' set up my clock, and start it
        movi phsb,#%011_000000
        movi frqb,#%001_000000
        ' keep reading in my value
        test miso_mask,ina wc
        rcl data,#1
        test miso_mask,ina wc
        rcl data,#1
        test miso_mask,ina wc
        rcl data,#1
        test miso_mask,ina wc
        rcl data,#1
        test miso_mask,ina wc
        rcl data,#1
        test miso_mask,ina wc
        rcl data,#1
        test miso_mask,ina wc
        rcl data,#1
        test miso_mask,ina wc
        mov frqb,#0 ' stop the clock
        rcl data,#1
        mov phsa,#0 'DI low
spiRecvByte_ret
        ret

#else

spiInit
		nop	' why is this needed?
spiInit_ret
        ret

spiSendByte
        shl     data, #24
        mov     bits, #8
:loop   rol     data, #1 wc
        muxc    outa, mosi_mask
        or      outa, sck_mask
        andn    outa, sck_mask
        djnz    bits, #:loop
        or      outa, mosi_mask
spiSendByte_ret
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

bits    long    0

#endif

pindir          long    0
pinout          long    0

mosi_pin        long    0
mosi_mask       long    0
miso_pin        long    0
miso_mask       long    0
sck_pin         long    0
sck_mask        long    0

' variables used by the spi send/receive functions
cmd         long    0
bytes       long    0
data        long    0

#ifdef SST
jedec_id    long    $004a25bf       ' value of t1 after read_jedec_id routine (SST25VF032B)
#else
jedec_id    long    $001440ef       ' value of t1 after read_jedec_id routine (W25Q80BV)
#endif

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

flash_mask  long    $00ffffff       ' mask to isolate the flash offset bits

            fit     496
