{
  Cache Driver
  Copyright (c) February 23, 2013 by David Betz

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

  ' default cache geometry
  DEFAULT_WAY_WIDTH     = 2 ' number of bits in the way offset (way count is 2^n)
  DEFAULT_INDEX_WIDTH   = 5 ' number of bits in the index offset (index size is 2^n)
  DEFAULT_OFFSET_WIDTH  = 6 ' number of bits in the line offset (line size is 2^n)

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
' $04: pointer to where to store the tags and cache lines in hub ram
' $08: geometry: (way_width << 24) | (index_width << 16) | (offset_width << 8)
' $0c: driver-specific parameter
' $10: driver-specific parameter
' $14: driver-specific parameter

init_vm mov     t1, par                 ' get the address of the initialization structure
        rdlong  pvmcmd, t1              ' pvmcmd is a pointer to the virtual address and read/write bit
        mov     pvmaddr, pvmcmd         ' pvmaddr is a pointer into the cache line on return
        add     pvmaddr, #4
        add     t1, #4
        rdlong  tagsptr, t1             ' address of tags in hub memory
        add     t1, #4

        rdlong  t2, t1 wz               ' get the cache geometry
        add     t1, #4
  if_z  jmp     #:skip
        mov     way_width, t2           ' way width in bits 31:24
        shr     way_width, #24
        mov     index_width, t2         ' index width in bits 23:16
        shr     index_width, #16
        and     index_width, #$ff
        mov     offset_width, t2        ' offset width in bits 15:8
        shr     offset_width, #8
        and     offset_width, #$ff
:skip

        mov     index_count, #1
        shl     index_count, index_width
        mov     index_mask, index_count
        sub     index_mask, #1

        mov     line_size, #1
        shl     line_size, offset_width
        mov     t2, line_size
        sub     t2, #1
        wrlong  t2, par
        
        mov     offset_shift, offset_width
        sub     offset_shift, #2
        
        mov     way_count, #1
        shl     way_count, way_width
        mov     way_size, index_count
        shl     way_size, #2            ' multiply by tag size in bytes
        mov     way_mask, way_count
        sub     way_mask, #1
        shl     way_mask, index_width
        
        mov     cacheptr, way_count       ' cache lines follow the tags
        shl     cacheptr, index_width
        shl     cacheptr, #2
        add     cacheptr, tagsptr
        
        call    #init
        
        ' initialize the cache lines
vmflush mov     t1, way_count
        shl     t1, index_width
        mov     t2, tagsptr
:flush  wrlong  empty_mask, t2
        add     t2, #4
        djnz    t1, #:flush

        ' start the command loop
waitcmd mov     dira, #0                ' release the pins for other SPI clients
        wrlong  zero, pvmcmd
:wait   rdlong  vmpage, pvmcmd wz
  if_z  jmp     #:wait

        test    vmpage, #int#EXTEND_MASK wz ' test for an extended command
  if_z  jmp     #extend

        shr     vmpage, offset_width wc ' carry is now one for read and zero for write
#ifdef RW
        mov     set_dirty_bit, #0       ' make mask to set dirty bit on writes
        muxnc   set_dirty_bit, dirty_mask
#endif
        mov     tagptr, vmpage          ' get the tag address
        and     tagptr, index_mask
        shl     tagptr, #2
        add     tagptr, tagsptr
        
        mov     t1, way_count           ' try each way
:loop   rdlong  t2, tagptr              ' get the cache line tag
        and     t2, tag_mask
        cmp     t2, vmpage wz           ' z set means there was a cache hit
  if_z  jmp     #hit                    ' handle a cache hit
        add     tagptr, way_size        ' try the next way
        djnz    t1, #:loop
        
miss    mov     tagptr, vmpage          ' get the tag index into way
        and     tagptr, index_mask
        mov     t1, CNT                 ' use low bits of CNT to choose a random way
        shl     t1, index_count
        and     t1, way_mask wz
        or      tagptr, t1
        mov     hubaddr, tagptr  
        shl     hubaddr, offset_width   ' get the address of the cache line
        add     hubaddr, cacheptr
        wrlong  hubaddr, pvmaddr        ' return the address of the cache line
        shl     tagptr, #2              ' get the address of the tag
        add     tagptr, tagsptr
lck_spi test    $, #0 wc                ' lock no-op: clear the carry bit
   if_c jmp     #lck_spi
        mov     dira, pindir            ' set the pins back so we can use them
#ifdef RW
        rdlong	t1, tagptr              ' read the tag
        test    t1, dirty_mask wz
  if_z  jmp     #:rd                    ' current page is clean, just read new page
        mov     vmaddr, t1
        shl     vmaddr, offset_width
        mov     count, line_size
        call    #BWRITE                 ' write current page
#endif
:rd     mov     vmaddr, vmpage
        shl     vmaddr, offset_width
        mov     count, line_size
        call    #BREAD                  ' read new page
nlk_spi nop        
        wrlong  vmpage, tagptr
#ifdef RW
        jmp     #done
#else
        jmp     #waitcmd                ' wait for a new command
#endif
        
hit     mov		hubaddr, tagptr
		sub     hubaddr, tagsptr
        shl     hubaddr, offset_shift
        add     hubaddr, cacheptr
        wrlong  hubaddr, pvmaddr        ' return the address of the cache line
        
done    
#ifdef RW
        tjz		set_dirty_bit, #:clean
		rdlong	t1, tagptr
		or		t1, set_dirty_bit
		wrlong	t1, tagptr
:clean  
#endif
        jmp     #waitcmd                ' wait for a new command

extend  mov     vmaddr, vmpage
        shr     vmaddr, #8
        shr     vmpage, #2
        and     vmpage, #7
        add     vmpage, #dispatch
        mov     dira, pindir            ' set the pins back so we can use them
        jmp     vmpage

dispatch
        jmp     #waitcmd                ' 0 - erase_chip
#ifdef FLASH
        jmp     #erase_block_handler    ' 1 - erase_block
        jmp     #write_data_handler     ' 2 - write_data
#else
        jmp     #waitcmd                ' 1 - erase block
        jmp     #waitcmd                ' 2 - write_data
#endif
#ifdef BLOCK_IO
        jmp     #block_read_handler     ' 3 - block_read
#ifdef RW
        jmp     #block_write_handler    ' 4 - block_write
#else
        jmp     #waitcmd                ' 4 - block_write
#endif
#else
        jmp     #waitcmd                ' 3 - block_read
        jmp     #waitcmd                ' 4 - block_write
#endif
        jmp     #waitcmd                ' 5 - unused
        jmp     #lock_set_handler       ' 6 - lock_set
        jmp     #waitcmd                ' 7 - unused

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

#ifdef FLASH

#ifndef NEED_BLOCK_SETUP
#define NEED_BLOCK_SETUP
#endif

erase_block_handler
        call    #erase_4k_block
        jmp     #block_finish

write_data_handler
        call    #block_setup
        call    #write_block
        jmp     #block_finish

#endif

#ifdef BLOCK_IO

#ifndef NEED_BLOCK_SETUP
#define NEED_BLOCK_SETUP
#endif

block_read_handler
        call    #block_setup
        call    #BREAD
        jmp     #block_finish

#ifdef RW

block_write_handler
        call    #block_setup
        call    #BWRITE
        jmp     #block_finish

#endif

#endif

#ifdef NEED_BLOCK_SETUP

block_setup
        rdlong  hubaddr, vmaddr         ' get the hub buffer pointer
        add     vmaddr, #4
        rdlong  count, vmaddr           ' get the byte count
        add     vmaddr, #4
        rdlong  vmaddr, vmaddr          ' get the address or offset
block_setup_ret
        ret

block_finish
        wrlong  data, pvmaddr           ' return status
        jmp     #waitcmd

#endif
        
' pointers to mailbox entries and cache hub space
pvmcmd          long    0       ' on call this is the virtual address and read/write bit
pvmaddr         long    0       ' on return this is the address of the cache line containing the virtual address
tagsptr         long    0	    ' address of tags in hub memory
cacheptr        long    0       ' address of cache lines in hub memory
vmpage          long    0       ' page containing the virtual address
tagptr			long	0	    ' address of tag in hub memory
zero            long    0       ' zero constant
t1              long    0       ' temporary variable
t2              long    0       ' temporary variable
t3              long    0       ' temporary variable
t4              long    0       ' temporary variable

tag_mask        long    (1<<DIRTY_BIT)-1    ' includes EMPTY_BIT
empty_mask      long    1<<EMPTY_BIT

#ifdef RW
dirty_mask      long    1<<DIRTY_BIT
set_dirty_bit   long    0       ' DIRTY_BIT set on writes, clear on reads
#endif

way_width       long    DEFAULT_WAY_WIDTH
index_width     long    DEFAULT_INDEX_WIDTH
offset_width    long    DEFAULT_OFFSET_WIDTH

way_mask        long    0
way_count       long    0
way_size        long    0
index_count     long    0
index_mask      long    0
offset_shift    long    0       ' offset_width - 2
line_size       long    0

' input parameters to BREAD and BWRITE
vmaddr          long    0       ' virtual address
hubaddr         long    0       ' hub memory address to read from or write to
count           long    0       ' number of bytes to read or write

' THE FOLLOWING FUNCTIONS AND VARIABLES ARE REQUIRED TO ACCESS EXTERNAL MEMORY

' the variable "pindir" should be defined and its value set to a bitmask of output pins

'----------------------------------------------------------------------------------------------------
'
' init - initialize the memory functions
'
'----------------------------------------------------------------------------------------------------

'----------------------------------------------------------------------------------------------------
'
' erase_4k_block - erase a 4k block of flash memory
'
' on input:
'   vmaddr is the virtual memory address to erase
'
' Note: only required if FLASH is defined
'
'----------------------------------------------------------------------------------------------------

'----------------------------------------------------------------------------------------------------
'
' write_block - write data to flash memory
'
' on input:
'   vmaddr is the virtual memory address to erase
'   hubaddr is the hub memory address to read
'   count is the number of bytes to write
'
' Note: only required if FLASH is defined
'
'----------------------------------------------------------------------------------------------------

'----------------------------------------------------------------------------------------------------
'
' BREAD - read data from external memory
'
' on input:
'   vmaddr is the virtual memory address to read
'   hubaddr is the hub memory address to write
'   count is the number of bytes to read
'
'----------------------------------------------------------------------------------------------------

'----------------------------------------------------------------------------------------------------
'
' BWRITE - write data to external memory
'
' on input:
'   vmaddr is the virtual memory address to write
'   hubaddr is the hub memory address to read
'   count is the number of bytes to write
'
' Note: only required if RW is defined
'
'----------------------------------------------------------------------------------------------------
