{
  Skeleton JCACHE external RAM driver
  Copyright (c) 2011 by David Betz

  Based on code by Steve Denson (jazzed)
  Copyright (c) 2010 by John Steven Denson

  Inspired by VMCOG - virtual memory server for the Propeller
  Copyright (c) February 3, 2010 by William Henning

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
  DEFAULT_INDEX_WIDTH   = 6
  DEFAULT_OFFSET_WIDTH  = 7

  ' cache line tag flags
  EMPTY_BIT             = 30
  DIRTY_BIT             = 31

PUB image
  return @init_vm

DAT
        org   $0

' initialization structure offsets
' $0: pointer to a two word mailbox
' $4: pointer to where to store the cache lines in hub ram
' $8: number of bits in the cache line index if non-zero (default is DEFAULT_INDEX_WIDTH)
' $a: number of bits in the cache line offset if non-zero (default is DEFAULT_OFFSET_WIDTH)
' note that $4 must be at least 2^(index_width+offset_width) bytes in size
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

        ' put external memory initialization here

        jmp     #vmflush

fillme  long    0[128-fillme]           ' first 128 cog locations are used for a direct mapped cache table

        fit   128

        ' initialize the cache lines
vmflush movd    :flush, #0
        mov     t1, index_count
:flush  mov     0-0, empty_mask
        add     :flush, dstinc
        djnz    t1, #:flush

        ' start the command loop
waitcmd wrlong  zero, pvmcmd
:wait   rdlong  vmline, pvmcmd wz
  if_z  jmp     #:wait

        shr     vmline, offset_width wc ' carry is now one for read and zero for write
        mov     set_dirty_bit, #0       ' make mask to set dirty bit on writes
        muxnc   set_dirty_bit, dirty_mask
        mov     line, vmline            ' get the cache line index
        and     line, index_mask
        mov     hubaddr, line
        shl     hubaddr, offset_width
        add     hubaddr, cacheptr       ' get the address of the cache line
        wrlong  hubaddr, pvmaddr        ' return the address of the cache line
        movs    :ld, line
        movd    :st, line
:ld     mov     vmcurrent, 0-0          ' get the cache line tag
        and     vmcurrent, tag_mask
        cmp     vmcurrent, vmline wz    ' z set means there was a cache hit
  if_nz call    #miss                   ' handle a cache miss
:st     or      0-0, set_dirty_bit      ' set the dirty bit on writes
        jmp     #waitcmd                ' wait for a new command

' line is the cache line index
' vmcurrent is current cache line
' vmline is new cache line
' hubaddr is the address of the cache line
miss    movd    :test, line
        movd    :st, line
:test   test    0-0, dirty_mask wz
  if_z  jmp     #:rd                    ' current cache line is clean, just read new one
        mov     vmaddr, vmcurrent
        shl     vmaddr, offset_width
        call    #wr_cache_line          ' write current cache line
:rd     mov     vmaddr, vmline
        shl     vmaddr, offset_width
        call    #rd_cache_line          ' read new cache line
:st     mov     0-0, vmline
miss_ret ret

' pointers to mailbox entries
pvmcmd          long    0       ' on call this is the virtual address and read/write bit
pvmaddr         long    0       ' on return this is the address of the cache line containing the virtual address

cacheptr        long    0       ' address in hub ram where cache lines are stored
vmline          long    0       ' cache line containing the virtual address
vmcurrent       long    0       ' current selected cache line (same as vmline on a cache hit)
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
line_size       long    0                       ' line size in bytes
empty_mask      long    (1<<EMPTY_BIT)
dirty_mask      long    (1<<DIRTY_BIT)

' input parameters to rd_cache_line and wr_cache_line
vmaddr          long    0       ' external address
hubaddr         long    0       ' hub memory address

'----------------------------------------------------------------------------------------------------
'
' rd_cache_line - read a cache line from external memory
'
' vmaddr is the external memory address to read
' hubaddr is the hub memory address to write
' line_size is the number of bytes to read
' 
' this function can modify vmaddr but most not modify hubaddr or line_size
'
'----------------------------------------------------------------------------------------------------

rd_cache_line
rd_cache_line_ret
        ret

'----------------------------------------------------------------------------------------------------
'
' wr_cache_line - write a cache line to external memory
'
' vmaddr is the external memory address to write
' hubaddr is the hub memory address to read
' line_size is the number of bytes to write
' 
' this function can modify vmaddr but most not modify hubaddr or line_size
'
'----------------------------------------------------------------------------------------------------

wr_cache_line
wr_cache_line_ret
        ret

        fit     496
