{
  Quad SPI flash JCACHE driver
  Copyright (c) April 17, 2012 by David Betz

  Based on code from VMCOG - virtual memory server for the Propeller
  Copyright (c) February 3, 2010 by William Henning

  and on code from SdramCache
  Copyright (c) 2010 by John Steven Denson (jazzed)

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

'#define WINBOND
'#define SST
#define DEBUG

CON

  ' protocol bits
  CS_CLR_PIN_MASK       = $01
  INC_PIN_MASK          = $02   ' for C3-style CS
  MUX_START_BIT_MASK    = $04   ' low order bit of mux field
  MUX_WIDTH_MASK        = $08   ' width of mux field
  ADDR_MASK             = $10   ' device number for C3-style CS or value to write to the mux
 
  ' default cache dimensions
  DEFAULT_WAY_WIDTH	= 1	' number of bits in the way offset (way count is 2^n)
  DEFAULT_INDEX_WIDTH   = 6	' number of bits in the index offset (index size is 2^n)
  DEFAULT_OFFSET_WIDTH  = 6	' number of bits in the line offset (line size is 2^n)
  DEFAULT_CACHE_SIZE    = 1<<(DEFAULT_WAY_WIDTH+DEFAULT_INDEX_WIDTH+DEFAULT_OFFSET_WIDTH)

  ' cache line tag flags
  EMPTY_BIT             = 30
  DIRTY_BIT             = 31

OBJ
  int: "cache_interface"

PUB image
  return @init_vm

DAT
        org   $0

' initialization structure offsets
' $00: pointer to a two word mailbox
' $04: pointer to where to store the cache lines in hub ram
' $08: 0xssxxccee - ss=sio0 xx=unused cc=sck pp=protocol
' $0c: 0xaabbccdd - aa=cs-or-clr bb=inc-or-start cc=width dd=addr
' $10: 0xwwiiooxx - ww=way-width ii=index-width oo=offset-width xx=unused
' $14: pointer to debug area
' note that $4 must be at least 2^(DEFAULT_INDEX_WIDTH+DEFAULT_OFFSET_WIDTH) bytes in size
' sio0 must be the lowest pin number of a group of four adjacent pins to be used for sio0-3
' the protocol byte is a bit mask with the bits defined above
'   if CS_CLR_PIN_MASK ($01) is set, then byte aa contains the CS or C3-style CLR pin number
'   if INC_PIN_MASK ($02) is set, then byte bb contains the C3-style INC pin number
'   if MUX_START_BIT_MASK ($04) is set, then byte bb contains the starting bit number of the mux field
'   if MUX_WIDTH_MASK ($08) is set, then byte cc contains the width of the mux field
'   if ADDR_MASK ($10) is set, then byte dd contains either the C3-style address or the value to write to the mux field
' example:
'   for a simple single pin CS you should set the protocol byte to $01 and place the CS pin number in byte aa.
' the cache line mask is returned in $0

init_vm mov     t1, par             ' get the address of the initialization structure
        rdlong  pvmcmd, t1          ' pvmcmd is a pointer to the virtual address and read/write bit
        mov     pvmaddr, pvmcmd     ' pvmaddr is a pointer into the cache line on return
        add     pvmaddr, #4
        add     t1, #4
        rdlong  cacheptr, t1        ' cacheptr is the base address in hub ram of the cache
        add     t1, #4

        ' get the pin definitions (cache-param1)
        rdlong  t2, t1
        add     t1, #4

        ' get the sio_shift and build the mosi, miso, and sio masks
        mov     sio_shift, t2
        shr     sio_shift, #24
        mov     mosi_mask, #1
        shl     mosi_mask, sio_shift
        mov     miso_mask, mosi_mask
        shl     miso_mask, #1
        mov     sio_mask, #$f
        shl     sio_mask, sio_shift
        or      spidir, mosi_mask
        or      spiout, mosi_mask
        
        ' make the sio2 and sio3 pins outputs in single spi mode to assert /WE and /HOLD
        mov     t3, #$0c
        shl     t3, sio_shift
        or      spidir, t3
        or      spiout, t3
        
        ' build the sck mask
        mov     t3, t2
        shr     t3, #8
        and     t3, #$ff
        mov     sck_mask, #1
        shl     sck_mask, t3
        or      spidir, sck_mask
        
        ' get the cs protocol selector bits (cache-param2)
        rdlong  t3, t1
        add	t1, #4
        
        ' handle the CS or C3-style CLR pins
        test    t2, #CS_CLR_PIN_MASK wz
  if_nz mov     t4, t3
  if_nz shr     t4, #24
  if_nz mov     cs_clr, #1
  if_nz shl     cs_clr, t4
  if_nz or      spidir, cs_clr
  if_nz or      spiout, cs_clr
  
        ' handle the mux width
        test    t2, #MUX_WIDTH_MASK wz
  if_nz mov     t4, t3
  if_nz shr     t4, #8
  if_nz and     t4, #$ff
  if_nz mov     mask_inc, #1
  if_nz shl     mask_inc, t4
  if_nz sub     mask_inc, #1
  if_nz or      spidir, mask_inc
  
        ' handle the C3-style address or mux value
        test    t2, #ADDR_MASK wz
  if_nz mov     select_addr, t3
  if_nz and     select_addr, #$ff

        ' handle the C3-style INC pin
        mov     t4, t3
        shr     t4, #16
        and     t4, #$ff
        test    t2, #INC_PIN_MASK wz
  if_nz mov     mask_inc, #1
  if_nz shl     mask_inc, t4
  if_nz mov     select, c3_select_jmp       ' We're in C3 mode, so replace select/release
  if_nz mov     release, c3_release_jmp     ' with the C3-aware routines
  if_nz or      spidir, mask_inc
 
        ' handle the mux start bit (must follow setting of select_addr and mask_inc)
        test    t2, #MUX_START_BIT_MASK wz
  if_nz shl     select_addr, t4
  if_nz shl     mask_inc, t4
  if_nz or      spidir, mask_inc
  
  	' get the cache geometry (cache-param3)
  	rdlong	t2, t1 wz
  if_z  jmp	#:next
  	mov	way_width, t2
  	shr	way_width, #24
  	mov	index_width, t2
  	shr	index_width, #16
  	and	index_width, #$ff
  	mov	offset_width, t2
  	shr	offset_width, #8
  	and	offset_width, #$ff
:next
        
        mov     index_count, #1
        shl     index_count, index_width
        mov     index_mask, index_count
        sub     index_mask, #1

        mov     line_size, #1
        shl     line_size, offset_width
        mov     t1, line_size
        sub     t1, #1
        wrlong  t1, par
        
        mov	way_count, #1
        shl	way_count, way_width
        mov	way_mask, way_count
        sub	way_mask, #1
        shl	way_mask, index_width
        
        ' set the pin directions
        mov     outa, spiout
        mov     dira, spidir
        call    #release
                
        call    #flash_init
                
        jmp     #vmflush

fillme  long    0[128-fillme]           ' first 128 cog locations are used for a direct mapped page table

        fit   128

        ' initialize the cache lines
vmflush movd    :flush, #0
        mov     t1, index_count
        shl	t1, way_width
:flush  mov     0-0, empty_mask
        add     :flush, dstinc
        djnz    t1, #:flush

        ' start the command loop
waitcmd mov     dira, #0                ' release the pins for other SPI clients
        wrlong  zero, pvmcmd
:wait   rdlong  vmpage, pvmcmd wz
  if_z  jmp     #:wait

        test    vmpage, #int#EXTEND_MASK wz ' test for an extended command
  if_z  jmp     #extend

        shr     vmpage, offset_width wc ' carry is now one for read and zero for write
        mov     set_dirty_bit, #0       ' make mask to set dirty bit on writes
        muxnc   set_dirty_bit, dirty_mask
        mov     line, vmpage            ' get the cache line index
        and     line, index_mask
        
        mov	t1, way_count		' try each way
loop	movs    :ld, line		' get ready to check the current line address
	movd	dset, line		' get ready to store the new line address
:ld     mov     vmcurrent, 0-0          ' get the cache line tag
        and     vmcurrent, tag_mask
        cmp     vmcurrent, vmpage wz    ' z set means there was a cache hit
  if_z  jmp	#hit			' handle a cache hit
  	add	line, index_count	' try the next way
  	djnz	t1, #loop
  	
miss    and	line, index_mask	' mask out the way bits
	mov	t1, CNT			' use low bits of CNT to choose a random way
	shl	t1, index_count
	and	t1, way_mask
	or	line, t1
	mov     hubaddr, line       	' get the address of the cache line
        shl     hubaddr, offset_width
        add     hubaddr, cacheptr
        movd    mtest, line
        movd    mst, line
lck_spi test    $, #0 wc                ' lock no-op: clear the carry bit
   if_c jmp     #lck_spi
        mov     dira, spidir            ' set the pins back so we can use them
mtest   test    0-0, dirty_mask wz
  if_z  jmp     #:rd                    ' current page is clean, just read new page
        mov     vmaddr, vmcurrent
        shl     vmaddr, offset_width
        call    #BWRITE                 ' write current page
:rd     mov     vmaddr, vmpage
        shl     vmaddr, offset_width
        call    #BREAD                  ' read new page
        mov     dira, #0                ' release the pins for other SPI clients
nlk_spi nop        
mst     mov     0-0, vmpage

#ifdef DEBUG
	mov	t1, pvmcmd
	add	t1, #(int#_MBOX_SIZE * 4 + 4)
	rdlong	t2, t1
	add	t2, #1
	wrlong	t2, t1
	add	t1, #4
	shl	line, #2
	add	t1, line
	rdlong	t2, t1
	add	t2, #1
	wrlong	t2, t1
#endif        

	jmp	#done

hit     mov     hubaddr, line       	' get the address of the cache line
        shl     hubaddr, offset_width
        add     hubaddr, cacheptr
        
#ifdef DEBUG
	mov	t1, pvmcmd
	add	t1, #(int#_MBOX_SIZE * 4)
	rdlong	t2, t1
	add	t2, #1
	wrlong	t2, t1
#endif        

done    wrlong  hubaddr, pvmaddr        ' return the address of the cache line
dset    or      0-0, set_dirty_bit      ' set the dirty bit on writes
        jmp     #waitcmd                ' wait for a new command

halt    jmp     #halt

extend  mov     vmaddr, vmpage
        shr     vmaddr, #8
        shr     vmpage, #2
        and     vmpage, #7
        add     vmpage, #dispatch
        mov     dira, spidir            ' set the pins back so we can use them
        jmp     vmpage

dispatch
        jmp     #waitcmd
        jmp     #erase_4k_block_handler
        jmp     #write_data_handler
        jmp     #waitcmd
        jmp     #waitcmd
        jmp     #waitcmd
        jmp     #waitcmd
'       jmp     #lock_set_handler - This is the next instruction - no need to waste a long

' Note that we only provide SD locks for the cache operations - the other
' operations are specific to the sd_cache_loader's use of the cache driver, and
' there's no need to provide SPI bus-locking services there.

lock_set_handler
        mov     lock_id, vmaddr
        mov     lck_spi, lock_set
        mov     nlk_spi, lock_clr
        jmp     #waitcmd
lock_set
        lockset lock_id wc
lock_clr
        lockclr lock_id
lock_id long    0               ' lock id for optional bus interlock

' pointers to mailbox entries
pvmcmd          long    0       ' on call this is the virtual address and read/write bit
pvmaddr         long    0       ' on return this is the address of the cache line containing the virtual address

cacheptr        long    0       ' address in hub ram where cache lines are stored
vmpage          long    0       ' page containing the virtual address
vmcurrent       long    0       ' current page in selected cache line (same as vmpage on a cache hit)
line            long    0       ' current cache line index
set_dirty_bit   long    0       ' DIRTY_BIT set on writes, clear on reads

zero            long    0       ' zero constant
dstinc          long    1<<9    ' increment for the destination field of an instruction
t1              long    0       ' temporary variable
t2              long    0       ' temporary variable
t3              long    0       ' temporary variable
t4              long    0       ' temporary variable

tag_mask        long    (1<<DIRTY_BIT)-1        ' includes EMPTY_BIT
index_width     long    DEFAULT_INDEX_WIDTH
index_mask      long    0
index_count     long    0
offset_width    long    DEFAULT_OFFSET_WIDTH
line_size       long    0                       ' line size in bytes
way_width	long	DEFAULT_WAY_WIDTH
way_count	long	0
way_mask	long	0
empty_mask      long    (1<<EMPTY_BIT)
dirty_mask      long    (1<<DIRTY_BIT)

' input parameters to BREAD and BWRITE
vmaddr      long    0       ' virtual address
hubaddr     long    0       ' hub memory address to read from or write to
count       long    0

erase_4k_block_handler
        call    #erase_4k_block
        wrlong  data, pvmaddr
        jmp     #waitcmd

write_data_handler
        rdlong  ptr, vmaddr     ' get the buffer pointer
        add     vmaddr, #4
        rdlong  count, vmaddr wz' get the byte count
  if_z  mov     data, #0
  if_z  jmp     #:done
        add     vmaddr, #4
        rdlong  vmaddr, vmaddr  ' get the flash address (zero based)
        jmp     #:addr
:loop   test    vmaddr, #$ff wz
  if_nz jmp     #:data
        call    #release
        call    #wait_until_done
:addr   call    #start_write
:data   rdbyte  data, ptr
        call    #sqiSendByte
        add     ptr, #1
        add     vmaddr, #1
        djnz    count, #:loop
        call    #release
        call    #wait_until_done
:done   wrlong  data, pvmaddr
        jmp     #waitcmd

' spi commands

read_jedec_id
        call    #select
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
        call    #select
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

erase_4k_block
        call    #sst_write_enable
        mov     cmd, vmaddr
        and     cmd, flashmask
        or      cmd, ferase4kblk
        mov     bytes, #4
        call    #sst_start_quad_spi_cmd
        call    #release
        call    #wait_until_done
erase_4k_block_ret
        ret

start_write
        call    #sst_write_enable
        mov     cmd, vmaddr
        and     cmd, flashmask
        or      cmd, sst_program
        mov     bytes, #4
        call    #sst_start_quad_spi_cmd
start_write_ret
        ret
        
start_read
        mov     cmd, vmaddr
        and     cmd, flashmask
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
        call    #select
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
  if_nz jmp     #halt
        call    #winbond_write_enable
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

erase_4k_block
        call    #winbond_write_enable
        mov     cmd, vmaddr
        and     cmd, flashmask
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
        
start_write
        call    #winbond_write_enable
        mov     cmd, vmaddr
        and     cmd, flashmask
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
        and     cmd, flashmask
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
        call    #select
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
        
jedec_id            long    $000040ef    ' value of t1 after read_jedec_id routine (W25Q80BV)

winbond_wrstatus    long    $01000000    ' write status
winbond_program     long    $32000000    ' flash program byte/page
winbond_read        long    $e3000000    ' flash read command

#endif

' **********************************
' END OF CHIP SPECIFIC SPI FUNCTIONS
' **********************************

frdjedecid          long    $9f          ' read the manufacturers id, device type and device id
ferase4kblk         long    $20000000    ' flash erase a 4k block
frdstatus           long    $05000000    ' flash read status
fwrenable           long    $06000000    ' flash write enable

'----------------------------------------------------------------------------------------------------
'
' BREAD
'
' on input:
'   vmaddr is the virtual memory address to read
'   hubaddr is the hub memory address to write
'   count is the number of longs to read
'
'----------------------------------------------------------------------------------------------------

BREAD
        call    #start_read
        mov     ptr, hubaddr      ' hubaddr = hub page address
        mov     count, line_size
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
'   count is the number of longs to write
'
'----------------------------------------------------------------------------------------------------

BWRITE
        ' no writes to flash
BWRITE_RET
        ret

'----------------------------------------------------------------------------------------------------
' SPI routines
'----------------------------------------------------------------------------------------------------

select                              ' Single-SPI and Parallel-DeMUX
        andn    outa, mask_inc
        or      outa, select_addr
        andn    outa, cs_clr
select_ret
        ret

release                             ' Single-SPI and Parallel-DeMUX
        mov     outa, spiout
        mov     dira, spidir
release_ret
        ret

c3_select_jmp                       ' Serial-DeMUX Jumps
        jmp     #c3_select          ' Initialization copies these jumps
c3_release_jmp                      '   over the select and release
        jmp     #c3_release         '   when in C3 mode.

c3_select                           ' Serial-DeMUX
        andn    outa, cs_clr
        or      outa, cs_clr
        mov     c3tmp, select_addr
:loop   or      outa, mask_inc
        andn    outa, mask_inc
        djnz    c3tmp, #:loop
        jmp     select_ret

c3_release                          ' Serial-DeMUX
        andn    outa, cs_clr
        mov     outa, spiout
        mov     dira, spidir
        jmp     release_ret

c3tmp   long    0
        
spiSendByte
        shl     data, #24
        mov     bits, #8
:loop   rol     data, #1 wc
        muxc    outa, mosi_mask
        or      outa, sck_mask
        andn    outa, sck_mask
        djnz    bits, #:loop
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

spidir          long    0
spiout          long    0

mosi_mask       long    0
miso_mask       long    0
sck_mask        long    0
sio_shift       long    0
sio_mask        long    0

cs_clr          long    0
mask_inc        long    0
select_addr     long    0

' variables used by the spi send/receive functions
cmd         long    0
bytes       long    0
data        long    0
bits        long    0

' temporaries used by BREAD and BWRITE
ptr         long    0

flashmask   long    $00ffffff       ' mask to isolate the flash offset bits

            FIT     496             ' out of 496
