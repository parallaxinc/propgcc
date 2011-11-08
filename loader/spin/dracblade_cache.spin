{
  SRAM JCACHE driver for the DracBlade
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

CON

  ' default cache dimensions
  DEFAULT_INDEX_WIDTH   = 5
  DEFAULT_OFFSET_WIDTH  = 7
  DEFAULT_CACHE_SIZE    = 1<<(DEFAULT_INDEX_WIDTH+DEFAULT_OFFSET_WIDTH+1)

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
' $0: pointer to a two word mailbox
' $4: pointer to where to store the cache lines in hub ram
' $8: number of bits in the cache line index if non-zero (default is DEFAULT_INDEX_WIDTH)
' $a: number of bits in the cache line offset if non-zero (default is DEFAULT_OFFSET_WIDTH)
' note that $4 must be at least 2^($8+$a) bytes in size
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

        jmp     #vmflush

fillme  long    0[128-fillme]           ' first 128 cog locations are used for a direct mapped page table

        fit   128

        ' initialize the cache lines
vmflush movd    :flush, #0
        mov     t1, index_count
:flush  mov     0-0, empty_mask
        add     :flush, dstinc
        djnz    t1, #:flush

        ' start the command loop
waitcmd mov     dira, #0                ' release the pins for other SPI clients
        wrlong  zero, pvmcmd
:wait   rdlong  vmpage, pvmcmd wz
  if_z  jmp     #:wait

        shr     vmpage, offset_width wc ' carry is now one for read and zero for write
        mov     set_dirty_bit, #0       ' make mask to set dirty bit on writes
        muxnc   set_dirty_bit, dirty_mask
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
miss    movd    :test, line
        movd    :st, line
:test   test    0-0, dirty_mask wz
  if_z  jmp     #:rd                    ' current page is clean, just read new page
        mov     vmaddr, vmcurrent
        shl     vmaddr, offset_width
        call    #BWRITE                 ' write current page
:rd     mov     vmaddr, vmpage
        shl     vmaddr, offset_width
        call    #BREAD                  ' read new page
:st     mov     0-0, vmpage
miss_ret ret

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

tag_mask        long    !(1<<DIRTY_BIT) ' includes EMPTY_BIT
index_width     long    DEFAULT_INDEX_WIDTH
index_mask      long    0
index_count     long    0
offset_width    long    DEFAULT_OFFSET_WIDTH
line_size       long    0                       ' line size in longs
empty_mask      long    (1<<EMPTY_BIT)
dirty_mask      long    (1<<DIRTY_BIT)

'----------------------------------------------------------------------------------------------------
'
' BSTART
'
' setup the high order address byte
'
'----------------------------------------------------------------------------------------------------

BSTART
        mov     address,vmaddr          ' get the high address byte
        shr     address,#16             ' shift right by 16 places
        and     address,#$FF            ' ensure rest of bits zero
        mov     HighLatch,address       ' put value into HighLatch
        mov     dira,LatchDirection     ' setup active pins 138 and bus
        mov     outa,HighLatch          ' send out HighLatch
        or      outa,HighAddress        ' or with the high address
        andn    outa,GateHigh           ' set gate low
        or      outa,GateHigh           ' set the gate high again
        mov     ptr, hubaddr            ' hubaddr = hub page address
        mov     address, vmaddr
        mov     count, line_size
BSTART_RET
        ret

'----------------------------------------------------------------------------------------------------
'
' BREAD
'
' vmaddr is the virtual memory address to read
' hubaddr is the hub memory address to write
' count is the number of longs to read
'
' trashes count, ptr
'
'----------------------------------------------------------------------------------------------------

BREAD
        call    #BSTART
rdloop  call    #read_memory_byte       ' read byte from address into data_8
        wrbyte  data_8,ptr              ' write data_8 to hubaddr ie copy byte to hub
        add     ptr,#1                  ' add 1 to hub address
        add     address,#1              ' add 1 to ram address
        djnz    count,#rdloop           ' loop until done
BREAD_RET
        ret

'----------------------------------------------------------------------------------------------------
'
' BWRITE
'
' vmaddr is the virtual memory address to write
' hubaddr is the hub memory address to read
' count is the number of longs to write
'
' trashes count, ptr, count
'
'----------------------------------------------------------------------------------------------------

BWRITE
        call    #BSTART
wrloop  rdbyte  data_8, ptr             ' copy byte from hub
        call    #write_memory_byte      ' write byte from data_8 to address
        add     ptr,#1                  ' add 1 to hub address
        add     address,#1              ' add 1 to ram address
        djnz    count,#wrloop           ' loop until done
BWRITE_RET
        ret

' input parameters to BREAD and BWRITE
vmaddr      long    0       ' virtual address
hubaddr     long    0       ' hub memory address to read from or write to

' temporaries used by BREAD and BWRITE
ptr         long    0
count       long    0

''From Dracblade driver for talking to a ram chip via three latches
'' Modified code from Cluso's triblade

'---------------------------------------------------------------------------------------------------------
'Memory Access Functions

read_memory_byte        call #RamAddress                ' sets up the latches with the correct ram address
                        mov dira,LatchDirection2        ' for reads so P0-P7 tristate till do read
                        mov outa,GateHigh               ' actually ReadEnable but they are the same
                        andn outa,GateHigh              ' set gate low
                        nop                             ' short delay to stabilise
                        nop
                        mov data_8, ina                 ' read SRAM
                        and data_8, #$FF                ' extract 8 bits
                        or  outa,GateHigh               ' set the gate high again
read_memory_byte_ret    ret

write_memory_byte       call #RamAddress                ' sets up the latches with the correct ram address
                        mov outx,data_8                 ' get the byte to output
                        and outx, #$FF                  ' ensure upper bytes=0
                        or outx,WriteEnable             ' or with correct 138 address
                        mov outa,outx                   ' send it out
                        andn outa,GateHigh              ' set gate low
                        nop                             ' no nop doesn't work, one does, so put in two to be sure
                        nop                             ' another NOP
                        or outa,GateHigh                ' set it high again
write_memory_byte_ret   ret

RamAddress ' sets up the ram latches. Assumes high latch A16-A18 low so only accesses 64k of ram
                        mov dira,LatchDirection         ' set up the pins for programming latch chips
                        mov outx,address                ' get the address into a temp variable
                        and outx,#$FF                   ' mask the low byte
                        or  outx,LowAddress             ' or with 138 low address
                        mov outa,outx                   ' send it out
                        andn outa,GateHigh              ' set gate low
                        or outa,GateHigh                ' set it high again
                                                        ' now repeat for the middle byte
                        mov outx,address                ' get the address into a temp variable
                        shr outx,#8                     ' shift right by 8 places
                        and outx,#$FF                   ' mask the low byte
                        or  outx,MiddleAddress          ' or with 138 middle address
                        mov outa,outx                   ' send it out
                        andn outa,GateHigh              ' set gate low
                        or outa,GateHigh                ' set it high again
RamAddress_ret          ret

GateHigh                long    %00000000_00000000_00000001_00000000  ' HC138 gate high, all others must be low - also used as ReadEnable
outx                    long    0                                     ' for temp use, same as n in the spin code
LatchDirection          long    %00000000_00000000_00001111_11111111 ' 138 active, gate active and 8 data lines active
LatchDirection2         long    %00000000_00000000_00001111_00000000 ' for reads so data lines are tristate till the read
LowAddress              long    %00000000_00000000_00000101_00000000 ' low address latch = xxxx010x and gate high xxxxxxx1
MiddleAddress           long    %00000000_00000000_00000111_00000000 ' middle address latch = xxxx011x and gate high xxxxxxx1
HighAddress             long    %00000000_00000000_00001001_00000000 ' high address latch = xxxx100x and gate high xxxxxxx1
WriteEnable             long    %00000000_00000000_00000011_00000000 ' /WE = xxxx001x and gate high xxxxxxx1
data_8                  long    %00000000_00000000_00000000_00000000 ' so code compatability with zicog driver
address                 long    %00000000_00000000_00000000_00000000 ' address for ram chip
HighLatch               long    %00000000_00000000_00000000_00000000 ' static value for the 374 latch that does the led, hA16-A19 and the other 4 outputs

                        fit     496
