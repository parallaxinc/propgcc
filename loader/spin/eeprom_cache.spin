{
  EEPROM external code backstore cache driver
  Code by Steve Denson
  Copyright (c) 2011 by Parallax, Inc.

  Based on skeleton_cache.spin:

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
  DEFAULT_TAG_SHIFT   = 7     ' tag size shift defines the number of tag lines used in cache
  DEFAULT_SIZE_SHIFT  = 6     ' line size shift defines the number of bytes per line

  ' using line size 64 bytes. line size for 64KB+ EEPROM must be 128 bytes or less

  ' cache line tag flags
  EMPTY_BIT     = 30
  DIRTY_BIT     = 31

  ' EEPROM SUPPORT
  RDELAY        = 1
  WDELAY        = 4
  EEID          = $A0
  RBIT          = 1   ' devid byte read bit


OBJ
  int : "cache_interface"

PUB image
''
'' image is used by the calling program to get the address of the PASM.
''
  return @init_vm

DAT
        org   $0

' initialization structure offsets
' $0: pointer to a two word mailbox
' $4: pointer to where to store the cache lines in hub ram
' $8: number of bits in the cache line index if non-zero (default is DEFAULT_TAG_SHIFT)
' $a: number of bits in the cache line offset if non-zero (default is DEFAULT_SIZE_SHIFT)
' note that $4 must be at least 2^(TAG_SHIFT+SIZE_SHIFT) bytes in size
' the cache line mask is returned in $0

init_vm
        mov     t1, par             ' get the address of the initialization structure
        rdlong  pvmcmd, t1          ' pvmcmd is a pointer to the virtual address and read/write bit
        mov     pvmaddr, pvmcmd     ' pvmaddr is a pointer into the cache line on return
        add     pvmaddr, #4
        add     t1, #4
        rdlong  cacheptr, t1        ' cacheptr is the base address in hub ram of the cache
        add     t1, #4

        ' always use offset_index and offset_width default values

        mov     index_count, #1
        shl     index_count, index_width
        mov     index_mask, index_count
        sub     index_mask, #1

        rdlong  t2, t1 wz           ' first cache param
  if_nz mov     eebase, t2          ' override the base address used for eeprom code

        mov     line_size, #1
        shl     line_size, offset_width
        mov     t1, line_size
        sub     t1, #1
        wrlong  t1, par

  if_nz andn    eebase, t1          ' make sure eebase is always line_size aligned

        call    #extMemInit
        jmp     #vmflush

fillme  long    0 [128-fillme]           ' first 128 cog locations are used for a direct mapped cache table
        fit   128

        ' initialize the cache lines
vmflush
        movd    :flush, #0
        mov     t1, index_count
:flush  mov     0-0, empty_mask
        add     :flush, dstinc
        djnz    t1, #:flush

        ' start the command loop
waitcmd wrlong  zero, pvmcmd
:wait   rdlong  vmline, pvmcmd wz
  if_z  jmp     #:wait

        test    vmline, #int#EXTEND_MASK wz ' test for an extended command
  if_z  jmp     #extend

        shr     vmline, offset_width wc ' carry is now one for read and zero for write
        mov     set_dirty_bit, #0       ' make mask to set dirty bit on writes
        muxnc   set_dirty_bit,dirty_mask' make mask to set dirty bit on writes
        mov     line, vmline            ' get the cache line index
        and     line, index_mask
        mov     hubaddr, line
        shl     hubaddr, offset_width
        add     hubaddr, cacheptr       ' get the address of the cache line
        wrlong  hubaddr, pvmaddr        ' return the address of the cache line
        movs    :ld, line
        movd    :st, line               ' do not remove even for read-only devices
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
miss
        movd    :st, line               ' do not remove even for read-only devices
:rd     mov     vmaddr, vmline
        shl     vmaddr, offset_width
        call    #rd_cache_line          ' read new cache line
:st     mov     0-0, vmline
miss_ret ret

'
' extend is used for command decoding. several commands can be used. see dispatch below.
'
extend  mov     vmaddr, vmline
        shr     vmaddr, #8
        shr     vmline, #2
        and     vmline, #7
        add     vmline, #dispatch
        jmp     vmline

dispatch        ' ignore all commands except the one we are using: write_data_handler and lock_set_handler
        jmp     #waitcmd    ' jmp     #erase_chip_handler ' no erase command
        jmp     #waitcmd    ' jmp     #erase_4k_block_handler ' no block erase command
        jmp     #write_data_handler
        jmp     #waitcmd    ' jmp     #sd_init_handler ' no sdcard on SSF
        jmp     #waitcmd    ' jmp     #sd_read_handler ' no sdcard on SSF
        jmp     #waitcmd    ' jmp     #sd_write_handler ' no sdcard on SSF
        jmp     #waitcmd    ' jmp     #cache_init_handler ' not used
'       jmp     #lock_set_handler - This is the next instruction - no need to waste a long

'
' Install an optional lock on the I2C bus.  We only provide locks while the
' program "running", so we only need to use these locks on the cache
' operations.  That's why the lock is located solely in the rd_cache_line routine.
'
lock_set_handler
        mov     lock_id, vmaddr
        mov     lck_i2c, lock_set
        mov     nlk_i2c, lock_clr
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
vmline          long    0       ' cache line containing the virtual address
vmcurrent       long    0       ' current selected cache line (same as vmline on a cache hit)
line            long    0       ' current cache line index
set_dirty_bit   long    0       ' DIRTY_BIT set on writes, clear on reads

zero            long    0       ' zero constant
dstinc          long    1<<9    ' increment for the destination field of an instruction
t1              long    0       ' temporary variable
t2              long    0       ' temporary variable

tag_mask        long    !(1<<DIRTY_BIT) ' includes EMPTY_BIT
index_width     long    DEFAULT_TAG_SHIFT
index_mask      long    0
index_count     long    0
offset_width    long    DEFAULT_SIZE_SHIFT
line_size       long    0                       ' line size in bytes
empty_mask      long    (1<<EMPTY_BIT)
dirty_mask      long    (1<<DIRTY_BIT)

' input parameters to rd_cache_line and wr_cache_line
vmaddr          long    0       ' external address
hubaddr         long    0       ' hub memory address


'
' put external memory initialization here
'
extMemInit
        mov     outa,    scl             ' scl 1
        or      outa,    sda             ' scl 1
        mov     dira,    scl             ' scl output
        or      dira,    sda             ' sda output
extMemInit_ret  ret

'------------------------------------------------------------------------------
'
' rd_cache_line - read a cache line from external memory
'
' vmaddr is the external memory address to read
' hubaddr is the hub memory address to write
' line_size is the number of bytes to read
'
'------------------------------------------------------------------------------

rd_cache_line
lck_i2c test    $, #0 wc             ' lock no-op: clear the carry bit
   if_c jmp     #lck_i2c
        mov     _delayCount, #RDELAY
        mov     devs,    #EEID
        mov     psiz,    line_size
        mov     addr,    vmaddr
        add     addr,    eebase      ' add eeprom base to skip the loader and get to the proper EEPROM address
        mov     tp,      hubaddr
        call    #readPage
nlk_i2c nop        
rd_cache_line_ret ret

'------------------------------------------------------------------------------
'
' wr_cache_line - write a cache line to external memory
'
' vmaddr is the external memory address to write
' hubaddr is the hub memory address to read
' line_size is the number of bytes to write
'
'------------------------------------------------------------------------------

wr_cache_line
' eeprom is treated as read only after loader writes program
' write_data_handler is used to program eeprom
wr_cache_line_ret
        ret

{{
'-------------------------------------------------------------------------------
' write buffer to eeprom
' @param       devs- device number
' @param       addr- write address
' @param       bytes-number of bytes to write
' @param       tp  - address of buffer to write
'
}}
write_data_handler
        rdlong  tp,     vmaddr          ' get the buffer pointer
        add     vmaddr, #4
        rdlong  bytes,  vmaddr wz       ' get the byte count
  if_z  jmp     #waitcmd                ' if byte count is zero, return

        add     vmaddr, #4
        rdlong  addr,   vmaddr          ' get the flash address (zero based)

        ' removing MSNIBBLE detect code for now
        add     addr,   eebase          ' add the eeprom image base address

        mov     _delayCount, #WDELAY    ' use write delay
        mov     devs,   #EEID
:loop
        cmp     bytes,  line_size wc
        mov     psiz,   line_size
  if_b  mov     psiz,   bytes           ' if bytes < line_size, write remainder
        call    #writePage
        add     addr,   line_size
        sub     bytes,  line_size wc,wz
  if_a  jmp     #:loop

        jmp     #waitcmd


'-------------------------------------------------------------------------------
' read eeprom page
' @param       *dp - device number
' @param       *ap - read address
' @param       *bp - address of buffer for read
' @param       psiz - number of bytes to read
'
readPage
        call    #sendAddr
        or      devs,    #RBIT   ' send read command
        call    #sendCurrent
        mov     t1,      psiz    ' read psiz bytes
        sub     t1,      #1      ' first psiz-1 are NAK
        tjz     t1,      #:last
        andn    ack,     sda     ' not last byte     
:loop
        call    #recVal          ' read page data
        wrbyte  val,     tp
        add     tp,      #1
        djnz    t1,      #:loop
:last  
        or      ack,     sda     ' last byte has ACK     
        call    #recVal
        wrbyte  val,     tp
        add     tp,      #1      ' increment for next loop too
        call    #sendEnd
readPage_ret   ret

'-------------------------------------------------------------------------------
' read current byte
' @param       *dp - device number
' @param       *ap - read address
' @param       *sp - number of bytes to read
' @param       *bp - address of buffer for read
'
readCurrent
        or      devs,    #RBIT   ' read
        call    #sendCurrent
        or      ack,     sda     ' last byte     
        call    #recVal
        wrbyte  val,     tp
        add     tp,      #1
        call    #sendEnd
readCurrent_ret   ret

'-------------------------------------------------------------------------------
' write eeprom page
' @param       *dp - device number
' @param       *ap - write address
' @param       *bp - address of buffer for write
' @param       psiz - number of bytes to read
'
writePage
        call    #sendAddr        ' send page address
        mov     t1,      psiz
:loop
        rdbyte  val,     tp
        add     tp,      #1
        call    #sendVal         ' send data
        djnz    t1,      #:loop
        call    #writeEndPoll
writePage_ret   ret

'-------------------------------------------------------------------------------
' send end and poll for write complete
'
writeEndPoll
        call    #sendEnd        ' send stop signals eeprom program
        mov     t0, #1
        shl     t0, #20         ' delay for 10ms (Tw = 5ms for most devices)
        add     t0, cnt
        waitcnt t0, t0          ' write does not appear reliable without this wait
{
'
' TODO write complete polling doesn't seem to work ... fix later
' download performance is main impact.
'
        call    #sclDelay
:loop
        call    #sendCurrent     ' now start write polling
        or      outa,    scl
        andn    dira,    sda     ' looking for ACK
        test    sda,     ina wc  ' is ack 0 ?
        call    #sclDelay
  if_c  jmp     #:loop           ' if ack 1, repeat
        call    #sendEnd         ' send stop
}
writeEndPoll_ret  ret  

'-------------------------------------------------------------------------------
' write current eeprom byte
' @param       *dp - device number
' @param       *ap - write address
' @param       *sp - number of bytes to write
' @param       *bp - address of buffer for write
'
writeCurrent
        andn    devs,    #RBIT
        call    #sendStart
        rdbyte  val,     tp
        add     tp,      #1
        call    #sendVal
writeCurrent_ret  ret

'-------------------------------------------------------------------------------
' send eeprom address
' @param       devs - device number
' @param       addr - read address

'
sendAddr
        call    #sendStart
        mov     val,     addr            ' %00000000_00000111_11111111_11111111 ->
        ror     val,     #16             ' %11111111_11111111_00000000_00000111
        shl     val,     #1              ' %11111111_11111110_00000000_00001110 ->
        andn    devs,    #RBIT           ' %10100000 send addr/write needs bit 0 low 
        or      val,     devs            ' %11111111_11111110_00000000_10101110
        call    #sendVal                 ' send device id and upper address bits
        mov     val,     addr            ' %00000000_00000111_11111111_11111111 ->
        ror     val,     #8              ' %11111111_00000000_00000111_11111111
        call    #sendVal                 ' send middle address bits
        mov     val,     addr            ' %00000000_00000111_11111111_11111111
        call    #sendVal                 ' send lower address bits
sendAddr_ret    ret

'-------------------------------------------------------------------------------
' send current is used to initiate a transaction after sendAddr
' @param       devs - device number with bit 0 set for read or clear for write
' @param       addr - read address
'
sendCurrent
        call    #sendStart
        mov     val,     addr            ' %00000000_00000111_11111111_11111111 ->
        ror     val,     #16             ' %11111111_11111111_00000000_00000111
        shl     val,     #1              ' %11111111_11111110_00000000_00001110 ->
        or      val,     devs            ' %11111111_11111110_00000000_1010111r
        call    #sendVal                 ' send device id and upper address bits
sendCurrent_ret ret


'-------------------------------------------------------------------------------
' A start signal is when SDA goes 1 -> 0 while SCL is 1
'
sendStart
        or      outa,    scl             ' scl high
        or      dira,    scl             ' scl output ... but it is always output right?
        or      outa,    sda             ' sda high
        or      dira,    sda             ' sda output
        call    #sclDelayD2              ' device detects high
        andn    outa,    sda             ' sda low for start condition 
        call    #sclDelayD2              ' device detects start
        andn    outa,    scl             ' finish with scl low 
sendStart_ret   ret
  
'-------------------------------------------------------------------------------
' An end signal is when SDA goes 0 -> 1 while SCL is 1
'
sendEnd
        andn    outa,    sda             ' sda high
        or      outa,    scl             ' scl high
        call    #sclDelayD2
        or      outa,    sda             ' sda high
        call    #sclDelayD2              ' device detects stop
        andn    dira,    sda             
        andn    dira,    scl             ' leave sda & scl input so other devices can use the bus
sendEnd_ret     ret

'-------------------------------------------------------------------------------
' scl has limits on how fast it can go. use this routine to throttle scl
'
sclDelay
        call    #sclDelayD2
sclDelay_ret    ret

sclDelayD2      ' write reliability is slower than read
        mov     _delay, _delayCount
        djnz    _delay, #$
sclDelayD2_ret  ret

'-------------------------------------------------------------------------------
' send 8 bits of data
' @param ack
' @param val
'
sendVal
        mov     ack,     #0              ' %00000000_00000000_00000000_11111111 ->
        ror     val,     #8              ' %11111111_00000000_00000000_00000000
        andn    outa,    scl             ' start with scl 0
        mov     t0,      #8              ' Output val on sda
:loop
        shl     val,     #1 wc           ' get next bit state into carry              
        muxc    outa,    sda             ' set bit depending on carry
        call    #sclDelay
        or      outa,    scl             ' toggle clock
        call    #sclDelay
        andn    outa,    scl
        call    #sclDelayD2
        djnz    t0,      #:loop
:findack  
        andn    dira,    sda             ' looking for ACK
        mov     ack,     ina             ' get ack
        and     ack,     sda wz          ' keep only ack bit
        or      outa,    scl             ' scl high
        call    #sclDelay
        andn    outa,    scl             ' scl low
        andn    outa,    sda             ' sda low
        or      dira,    sda             ' sda back to output
sendVal_ret     ret


'-------------------------------------------------------------------------------
' receive 8 bits of data
' @param ack
' @param val
'
recVal
        mov     val,     #0
        andn    dira,    sda             ' sda input
        andn    outa,    scl             ' clock starts at 0
        call    #sclDelay
        mov     t0,      #8              ' input 8 bits of val from sda
:loop
        or      outa,    scl             ' toggle clock
        call    #sclDelayD2
        shl     val,     #1
        test    sda,     ina wz          ' get bit in z
        muxnz   val,     #1
        andn    outa,    scl             ' toggle clock
        call    #sclDelay
        djnz    t0,      #:loop
:sendack
        or      dira,    sda             ' set sda to out so device can interpret ACK
        or      outa,    ack             ' ack is always on sda, but not always set
        call    #sclDelayD2
        or      outa,    scl        
        call    #sclDelay
        andn    outa,    scl             ' final clock pulse
        andn    outa,    sda        
recVal_ret      ret

_delay          long 0
_delayCount     long WDELAY

scl             long 1 << 28   
sda             long 1 << 29

tp              long 0   ' temp pointer
t0              long 0   ' temp vars

cmd             long 0   ' command word
devs            long 0   ' device select
addr            long 0   ' address
bytes           long 0   ' size in bytes
psiz            long 0   ' parameter size in bytes
buff            long 0   ' buffer pointer
ueack           long 0   ' user early ack
ack             long 0   ' ack bit
val             long 0   ' accumulated value
eebase          long $1300 ' eebase sb > eeprom loader size
MSNIBBLE        long $F0000000

eof             long $55555555           
        fit     496

