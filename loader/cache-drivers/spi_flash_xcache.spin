'#define WINBOND
'#define SST

#define FLASH
#undef RW
#include "cache_common.spin"
#include "cache_spi_pins.spin"
#include "cache_spi.spin"

init
        call    #get_spi_pins

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

pindir          long    0
pinout          long    0

' variables used by the spi send/receive functions
cmd         long    0
bytes       long    0

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
