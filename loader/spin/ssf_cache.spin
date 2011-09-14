{{
===============================================================================
ssf_cache.spin

Defines a flash interface for SpinSocket Flash and compatible devices

Copyright (c) 2011 by John Steven Denson (jazzed)
All rights MIT licensed
===============================================================================
}}

{
  SPI SRAM and flash JCACHE driver for the Parallax C3
  by David Betz

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

#define BYTE_TRANSFERS

CON

  FLASH_CS_PIN          = 27
  CLK_PIN               = 26

  DINMASK               = $11
  OUTMASK               = $22
  DO0MASK               = $02
  DO1MASK               = $20
  DBMASK                = $ff
  WRPMASK               = $44
  HLDMASK               = $88

  ' default cache dimensions
  DEFAULT_INDEX_WIDTH   = 6
  DEFAULT_OFFSET_WIDTH  = 7
  DEFAULT_CACHE_SIZE    = 1<<(DEFAULT_INDEX_WIDTH+DEFAULT_OFFSET_WIDTH+1)

  ' cache line tag flags
  EMPTY_BIT             = 30
  DIRTY_BIT             = 31

OBJ
  int: "cache_interface"

PUB code
  return @init_vm

DAT
        org   $0

' initialization structure offsets
' $0: pointer to a two word mailbox
' $4: pointer to where to store the cache lines in hub ram
' $8: number of bits in the cache line index if non-zero (default is DEFAULT_INDEX_WIDTH)
' $a: number of bits in the cache line offset if non-zero (default is DEFAULT_OFFSET_WIDTH)
' note that $4 must be at least 2^($8+$a)*2 bytes in size
' the cache line mask is returned in $0

init_vm mov     t1, par             ' get the address of the initialization structure
        rdlong  pvmcmd, t1          ' pvmcmd is a pointer to the virtual address and read/write bit
        mov     pvmaddr, pvmcmd     ' pvmaddr is a pointer into the cache line on return
        add     pvmaddr, #4
        add     t1, #4
        rdlong  cacheptr, t1        ' cacheptr is the base address in hub ram of the cache
        add     t1, #4
        rdlong  t2, t1 wz
  if_nz mov     index_width, t2     ' override the index_width default value
        add     t1, #4
        rdlong  t2, t1 wz
  if_nz mov     offset_width, t2    ' override the offset_width default value

        mov     index_count, #1
        shl     index_count, index_width
        mov     index_mask, index_count
        sub     index_mask, #1

        mov     line_size, #1
        shl     line_size, offset_width
        mov     t1, line_size
        sub     t1, #1
        wrlong  t1, par

        ' set the pin directions
        mov     outa, spiout
        or      dira, spidir

        ' unprotect the entire flash chip
        call    #write_enable
        call    #flash_chip
        mov     data, fwrstatqen        ' write zero to the status register
        mov     bits, #24
        call    #send
        call    #deselect

        call    #wait_until_done        ' wait for status register write complete
        jmp     #vmflush

fillme  long    0[128-fillme]           ' first 128 cog locations are used for a direct mapped page table

        fit   128

        ' initialize the cache lines
vmflush movd    :flush, #0
        mov     t1, index_count
'       shl     t1, #1                  ' clear both the flash and SRAM tags
:flush  mov     0-0, empty_mask
        add     :flush, dstinc
        djnz    t1, #:flush

        ' start the command loop
waitcmd
        wrlong  zero, pvmcmd
:wait   rdlong  vmpage, pvmcmd wz
  if_z  jmp     #:wait
        or      dira, spidir            ' set the pins back so we can use them

        test    vmpage, #int#EXTEND_MASK wz ' test for an extended command
  if_z  jmp     #extend

        shr     vmpage, offset_width wc ' carry is now one for read and zero for write
        muxnc   set_dirty_bit,dirty_mask' make mask to set dirty bit on writes
        mov     line, vmpage            ' get the cache line index
        and     line, index_mask
        mov     hubaddr, line
        shl     hubaddr, offset_width
        add     hubaddr, cacheptr       ' get the address of the cache line
        wrlong  hubaddr, pvmaddr        ' return the address of the cache line
        movs    :ld, line
        movd    :st, line
:ld     mov     vmcurrent, 0-0          ' get the cache line tag
        and     vmcurrent, tag_mask
        cmp     vmcurrent, vmpage wz    ' z set means there was a cache hit
  if_nz call    #miss                   ' handle a cache miss
:st     or      0-0, set_dirty_bit      ' set the dirty bit on writes
        jmp     #waitcmd                ' wait for a new command

' line is the cache line index
' vmcurrent is current page
' vmpage is new page
' hubaddr is the address of the cache line
miss
        movd    :st, line
:rd     mov     vmaddr, vmpage
        shl     vmaddr, offset_width
        call    #BREAD                  ' read new page
:st     mov     0-0, vmpage
miss_ret ret

extend  mov     vmaddr, vmpage
        shr     vmaddr, #8
        shr     vmpage, #2
        and     vmpage, #7
        add     vmpage, #dispatch
        jmp     vmpage

dispatch
        jmp     #erase_chip_handler
        jmp     #erase_4k_block_handler
        jmp     #write_data_handler
        jmp     #waitcmd    ' jmp     #sd_init_handler ' no sdcard on SSF
        jmp     #waitcmd    ' jmp     #sd_read_handler ' no sdcard on SSF
        jmp     #waitcmd    ' jmp     #sd_write_handler ' no sdcard on SSF
        jmp     #waitcmd
        jmp     #waitcmd

erase_chip_handler
        call    #write_enable
        call    #flash_chip
        mov     data, ferasechip
        mov     bits, #8
        call    #send
        call    #deselect
        call    #wait_until_done
        wrlong  data, pvmaddr
        jmp     #waitcmd

erase_4k_block_handler
        call    #write_enable
        call    #flash_chip
        test    vmaddr, epage wz
  if_nz jmp     #waitcmd
        mov     data, vmaddr
        or      data, ferase4kblk
        mov     bits, #32
        call    #send
        call    #deselect
        call    #wait_until_done
        wrlong  data, pvmaddr
        jmp     #waitcmd

write_data_handler
        rdlong  ptr, vmaddr             ' get the buffer pointer
        add     vmaddr, #4
        rdlong  count, vmaddr wz        ' get the byte count
  if_z  jmp     #:done
        add     vmaddr, #4
        rdlong  vmaddr, vmaddr          ' get the flash address (zero based)

        test    count, #$1ff wz         ' count mod #$1ff > 0 ??
        shr     count, #9               ' count / 512 bytes should be 2 512 pages for 1K transfer
  if_nz add     count, #1               ' add 1 if count mod #$1ff > 0

        shr     vmaddr, #1              ' adjust for Dual QuadSpi flash pages
        andn    vmaddr, #$ff            ' lower 8 must be 0

:addr   
        or      dira, #DBMASK           ' enable data out. hld/wrp are part of DBMASK
        call    #write_enable
        mov     data, vmaddr
        or      data, fprogram
        mov     bits, #32               ' 8 instruction bits + 24 address bits
        call    #flash_chip
        call    #send
        mov     len, psize
:data
        rdbyte  outa, ptr
        or      outa, TCLK
        add     ptr, #1
        djnz    len, #:data

        call    #deselect               ' start program cycle ... wait before status check

        call    #wait_until_done        ' sets DO to input
        add     vmaddr, #$100
        djnz    count, #:addr

        call    #deselect               ' start program cycle
:done   call    #wait_until_done
        wrlong  data, pvmaddr
        jmp     #waitcmd

epage   long    $fff
len     long    0
psize   long    $200

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

tag_mask        long    (1<<DIRTY_BIT)-1        ' includes EMPTY_BIT
index_width     long    DEFAULT_INDEX_WIDTH
index_mask      long    0
index_count     long    0
offset_width    long    DEFAULT_OFFSET_WIDTH
line_size       long    0                       ' line size in bytes
empty_mask      long    (1<<EMPTY_BIT)
dirty_mask      long    (1<<DIRTY_BIT)

'--------------------------------------------------------------------
'
' BSTART
'
' Not used for flash
'
'--------------------------------------------------------------------

'--------------------------------------------------------------------
'
' BREAD
'
' on input:
'   vmaddr is the virtual memory address to read
'   hubaddr is the hub memory address to write
'
'--------------------------------------------------------------------

'{
'--------------------------------------------------------------------
' read a buffer
' conservative timing
'
BREAD     
        mov     data, fread
        mov     bits, #8        ' 8 instruction bits
        call    #flash_chip
        call    #send
        call    #sendQuadAddr
        mov     len,#6
:loop1  andn    outa,TCLK
        or      outa,TCLK
        djnz    len,#:loop1

        andn    dira,#$ff       ' ensure bits are set to input
        or      outa,TCLK
        mov     ptr, hubaddr    ' hubaddr = hub page address
        andn    outa,TCLK       ' high-low clock gets data from chip
        mov     len,line_size
:loop2
        mov     data,ina        ' read byte
        wrbyte  data,ptr        ' write to hub pointer
        or      outa,TCLK
        add     ptr,#1
        andn    outa,TCLK       ' high-low clock gets data from chip
        djnz    len,#:loop2     ' do until _len bytes are stored
        andn    outa,TCLK
        call    #deselect
BREAD_RET ret
'}

{
'--------------------------------------------------------------------
' read a buffer
' faster but very tight timing
'
BREAD     
        mov     data, fread
        mov     bits, #8                ' 8 instruction bits + 24 address bits
        call    #flash_chip
        call    #send
        call    #sendQuadAddr
        andn    outa,TCLK
        mov     frqa,_frqa4     ' phsa accumulate frequency
        mov     phsa,_phsa      ' phsa initial value
        mov     ctra,_ctra      ' start clock so it goes hi at shift instruction
        nop                     ' clock M7..0
        nop
        andn    dira,#$ff       ' ensure bits are set to input
        nop                     ' dummy read
        nop                     ' dummy read
        mov     ctra,#0

        mov     ptr, hubaddr    ' hubaddr = hub page address
        mov     len,line_size
        call    #doRead

        call    #deselect
BREAD_RET ret

'--------------------------------------------------------------------
' read buffer from Flash to HUB at highest propeller speed
' aggressive read
doRead
        mov     phsb,ptr        ' ptr is the HUB pointer start address
        mov     frqb,#1         ' phsb accumulate once per EDGE
        movs    ctrb,#CLK_PIN   ' detect the SPI clock edge
        movi    ctrb,#%01110<<3 ' Negative EDGE detect
        mov     frqa,_frqa1     ' phsa accumulate frequency 5MB/s
        mov     phsa,_phsa4     ' phsa intial value
        rdlong  t1, ptr         ' just a hub op to sync up for read
        mov     ctra,_ctra      ' start clock so it goes hi at wrbyte instruction
:loop
        mov     data,ina        ' read byte
        wrbyte  data,phsb       ' write to hub pointer
        djnz    len,#:loop      ' do until _len bytes are stored

        mov     ctra,#0         ' kill clock
doRead_ret      ret
'}


'--------------------------------------------------------------------
' send address
'
sendQuadAddr
        or      dira,#dbmask    ' ensure bits are set to output
        ror     vmaddr,#21      ' put A[20..23] on D[0..3] ... would be 20, but we need 1/2 addr
        mov     data,#0
        mov     len,#6          ' send 6 nibbles
:loop   andn    outa,TCLK
        movs    data,vmaddr     ' copy 0..3
        ror     data,#4         ' to 28..31
        movs    data,vmaddr     ' copy 0..3
        rol     data,#4         ' to 4..7 and 0..3
        movs    outa,data       ' set address nibble
        rol     vmaddr,#4       ' get next address nibble
        or      outa,TCLK       ' clock it
        djnz    len,#:loop
        andn    outa,TCLK
sendQuadAddr_ret    ret


_frqa           long    $1000_0000      ' clock update frequency for writing SPI bits
_frqa8          long    $8000_0000      ' clock update frequency for writing SPI bits
_frqa4          long    $4000_0000      ' clock update frequency for writing SPI bits
_frqa2          long    $2000_0000      ' clock update frequency for writing SPI bits
_frqa1          long    $1000_0000      ' clock update frequency for writing SPI bits
_frqa08         long    $0800_0000      ' clock update frequency for writing SPI bits
_frqa04         long    $0400_0000      ' clock update frequency for writing SPI bits
_phsa           long    $0000_0000      ' phsa offset for adjusting clock start
_phsa4          long    $4000_0000      ' phsa offset for adjusting clock start
_phsa8          long    $8000_0000      ' phsa offset for adjusting clock start
_ctra           long    1<<28 | CLK_PIN ' NCO mode

'--------------------------------------------------------------------
'
' BWRITE
'
' not used for flash
'
'--------------------------------------------------------------------

'--------------------------------------------------------------------
' SPI routines
'--------------------------------------------------------------------

send0   andn    outa, TCLK
send    rol     data, #1 wc
        muxc    outa, TMOSI
        or      outa, TCLK
        djnz    bits, #send0
        andn    outa, TCLK
        or      outa, TMOSI
send_ret
        ret

' spi select functions
'
flash_chip
        or      outa, #hldmask|wrpmask ' deassert hold/wprot
        or      dira, #hldmask|wrpmask
        or      dira, TMOSI 
        andn    outa, FLASH_CS
'       or      dira, FLASH_CS
        andn    outa, TCLK
flash_chip_ret
        ret

deselect
        andn    outa, TCLK
        or      outa, CS_MASK
        or      dira, CS_MASK   ' flash pin has pull up
'       andn    dira, FLASH_CS  ' flash pin has pull up
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
        or      dira, TMOSI             ' ensure output enabled to flash
        call    #send
        andn    dira, TMISO2
:wait   mov     bits, #8                ' read status continuously if necessary
:loop   andn    outa, TCLK
        or      outa, TCLK
        djnz    bits, #:loop
        test    TMISO2, ina wz          ' if any bit is high, clear z
  if_nz jmp     #:wait                  ' read again if nz bits high
        andn    outa, TCLK
        call    #deselect
        or      dira, TMISO2            ' restore
        and     data, #$ff
wait_until_done_ret
        ret

spidir      long    (1<<FLASH_CS_PIN)|(1<<CLK_PIN)|(DBMASK)
spiout      long    (1<<FLASH_CS_PIN)|(0<<CLK_PIN)

tclk        long    1<<CLK_PIN
tmosi       long    DINMASK
tmiso       long    DO0MASK
tmiso2      long    OUTMASK

CS_MASK     long    (1<<FLASH_CS_PIN)
FLASH_CS    long    (1<<FLASH_CS_PIN)

' input parameters to BREAD and BWRITE
vmaddr      long    0       ' virtual address
hubaddr     long    0       ' hub memory address to read from or write to

' temporaries used by BREAD and BWRITE
ptr         long    0
count       long    0

' temporaries used by send
bits        long    0
data        long    0

read        long    $03000000       ' read command
write       long    $02000000       ' write command
ramseq      long    $01400000       ' %00000001_01000000 << 16 ' set sequential mode
readstat    long    $05000000       ' read status

fread       long    $eb000000       ' flash read command FAST_QUAD
ferasechip  long    $c7000000       ' flash erase chip QUAD
ferase4kblk long    $20000000       ' flash erase a 4k block
fprogram    long    $32000000       ' flash program byte/page QUAD
fwrenable   long    $06000000       ' flash write enable
fwrdisable  long    $04000000       ' flash write disable
frdstatus   long    $05000000       ' flash read status
fwrstatqen  long    $01000200       ' flash write status + quad enable

            FIT     496             ' out of 496
