{{
=====================================================================
sdram_cache.spin - defines a device driver for ISSI 32Mx8 SDRAM
=====================================================================

    Change History
    --------------
    Oct 25, 2010: Initial release for use with SdramTest.spin
    Nov  5, 2010: Added dirty write-back handling for better read performance.
    Nov  6, 2010: Reverted an aggressive optimization - performance is same.
    Sep 17, 2011: Removed Spin code for use with PropGCC and xBasic.
                  SdramTest not supported.
}}

{{
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
}}

{{
=====================================================================
Any memory interface can be used given enough pins; a parallel
bus interface is challenging since Propeller only has 32 pins.
An 8 bit data bus is a reasonable trade for pin use/performance.
With a cache, the a bus width > 8 bits is best if possible.

This SDRAM solution uses 20 propeller pins.
for A0-12, BA0-1, CS*, CLK, RAS*, CAS*, D0-7 (CKE is pulled high).

The CS* and DQM pins are permanently pulled low for a 32Mx8 memory.

D0-7       on A0-7 via ALE on P15
A8-14      on A8-14
SDRAM ALE  on P15
SDRAM CLK  on P24
SDRAM RAS  on P25
SDRAM CAS  on P26
SDRAM WE*  on P27

With this design, there are 8 Propeller pins (P16-23) free for other IO.
Current pin usage: P0-15 & P24-27 SDRAM, P28-29 I2C, P30-31 Serial Port.
=====================================================================
}}


{{
=====================================================================
SDRAM CACHE device driver code
=====================================================================

This driver must be built with a compiler that supports #ifdef
In BST/BSTC "non-parallax extentions" must be enabled for builds.

The options available with #ifdef are:

LINELEN_64      ' use 64 byte cache line size  - slowest burst rate
LINELEN_128     ' use 128 byte cache line size
LINELEN_256     ' use 256 byte cache line size
LINELEN_512     ' use 512 byte cache line size - fastest possible burst rate

LINELEN is inversely proportional to the number of cache lines in use.
Some applications such as a C program will benefit from smaller LINELEN.
Video or other buffer application need larger a LINELEN.

CACHE_8K
CACHE_4K
CACHE_2K

These defines allow smaller caches for programs needing more HUB
resources. If cache size is not defined, the cache size is 16KB.

The #defines are set in the driver, but can be set in another file.
}}

{{
=====================================================================
These constants define the pin functions:
}}
con
    DBPIN_MASK  = $FF
    ABPIN_MASK  = $7FFF
    ALE_PIN     = 15
    CLK_PIN     = 24
    RASN_PIN    = 25
    CASN_PIN    = 26
    WEN_PIN     = 27

    FREE_MASK   = $00FF0000     ' unused/free pin mask

    ADDR_MASK   = (1 << 25)-1   ' 32MB
    ROW_MASK    = (1 << 13)-1   ' A0-12
    COL_MASK    = (1 << 10)-1   ' A0-9
    CASN_PREPIN = (1 << 10)     ' A10 is precharge pin
    RASN_MASK   = (1 << RASN_PIN)
    CASN_MASK   = (1 << CASN_PIN)
    WEN_MASK    = (1 << WEN_PIN)
    DCMD_MASK   = RASN_MASK | CASN_MASK | WEN_MASK

    MODE_REG    = $37           ' CAS latency 3, full burst

con
{{
=====================================================================
Cache is organized as a table.
The tagline table contains pointers to cache entries.
Each tagline has the following definition:

    TAGLINE:    xxxx_xxxi
    such that:  x=match address i=ignored bits

The actual number of ignored bits depends on the cache line length.
}}

con
{{
It will be difficult to support dynamica cache size with this driver.
Leave it as is for now and only use 8K Cache.
}}
  CACHESIZE   = $2000>>2          ' power of 2  8K cache
' CACHESIZE   = $1000>>2          ' power of 2 4K cache
' CACHESIZE   =  $800>>2          ' power of 2 2K cache

con
    LINESHFT    = 5                 ' this must match the size of the buffer operations
    LINELEN     = 1<<LINESHFT       ' CACHELINE SIZE = 32 bytes
    TAGCOUNT    = CACHESIZE/LINELEN ' cache with linelen bytes

    TAGMASK     = $003F_FFFF        ' $3FFFFF*512 = 2GB $3FFFFF*32 = 128MB
    DIRTYSHFT   = 31                ' shift by N to get dirty bit   test tag,DIRTYMASK wc
    DIRTYMASK   = 1 << DIRTYSHFT    ' dirty bit mask                muxc tag,DIRTYSHFT

pub image
    return @pasm

{{
=====================================================================
PASM Cache Driver
=====================================================================
}}
dat                 org 0
pasm
'long    0 [8]                               ' debugger stub space - or beginning of tag RAM

'--------------------------------------------------------------------
' startup and some variables
'
                    jmp #sdram_setup        ' 0
long    0 [64-$]                            ' tag ram space

'====================================================================
' mailbox command spinner loop
' user will get a block pointer back and access the block with a mod index such as:
'
'                    mov     index,addr          ' user must do something like this for returned array index
'                    and     index,#(LINELEN-1)  ' index can be only up to LINELEN-1
'
sdramDone
cmdloop             djnz    refresh,#norefresh  ' check refresh every time ... djnz here keeps window
                    call    #sdram_refresh      ' if refresh timer expired, reload and refresh

norefresh           rdlong  addr,cmdptr    wz   ' get command/address ... command is in bit 0..1
    if_z            jmp     #cmdloop            ' if zero, do nothing
                    mov     clineptr,addr       ' get the cache line pointer offset from address

                    shr     addr,#LINESHFT wc   ' test for read/write into carry
                    movs    readtag,addr        ' (addr / LINESIZE) mod TAGCOUNT is tag / cache line offset
                    andn    readtag,#$200-TAGCOUNT ' user sends full address. we only load blocks
'                   add     readtag,#8          ' for debugger only
                    muxnc   dirty,_TAG_DIRTY    ' save new tag dirty state - fill pipe slot
readtag             mov     tag,0-0             ' get tag line contents

                    cmpsub  tag,_TAG_DIRTY wc   ' get the old dirty tag
                    cmp     addr,tag wz
                    muxc    tag,_TAG_DIRTY      ' restore the old dirty tag

    if_e            wrlong  zero,cmdptr         ' if match let user know we're done early ...
                                                ' we rely on the fact that the user is looking for 0 cmdptr
                                                ' and must use another HUB access to get data.
                                                ' prolems may come if the user's cog is lower than this cog number

                    and     clineptr,_CLP_MOD   ' user sends full address. we only load blocks
                    add     clineptr,cacheptr   ' get cache line
                    wrlong  clineptr,datptr     ' send cache buffer to user - data may change before we're done

    if_ne           call    #flush              ' if bad tag, flush - cache code never changes Z flag
    if_ne           wrlong  zero,cmdptr         ' let user know we're ready after flush ...

writeTag            movd    tagit,readtag
                    or      tag,dirty           ' save new tag dirty state
tagit               mov     0-0,tag

                    jmp     #sdramDone          ' 18*4 cycles on cache hit, but 16*4 for delivery.

'--------------------------------------------------------------------
' flush method saves dirty page and loads page from backstore
'
flush               ' if dirty bit set, write old buffer before read
    if_nc           jmp     #flush_rdonly
flush_write         mov     saveaddr,addr       ' save addr for read
                    mov     addr,tag            ' get cacheline tag addr
                    shl     addr,#LINESHFT      ' physical address to write cacheline
                    call    #sdramBuffWrite     ' save cacheline
                    mov     addr,saveaddr       ' restore addr for read
flush_rdonly        mov     tag,addr            ' move address clears dirty/count
                    shl     addr,#LINESHFT      ' physical address to write cacheline
                    call    #sdramBuffRead      ' if zero, load cache line
flush_ret           ret


_TAG_DIRTY          long DIRTYMASK              ' use to mark pages for write-back
_CLP_MOD            long (TAGCOUNT-1)<<LINESHFT ' use modulo for finding tag address

cmdptr              long 0                      ' cache command pointer
datptr              long 0                      ' cache data pointer
tagptr              long 0                      ' tag pointer
cacheptr            long 0                      ' cache pointer

'====================================================================

refresh_wait        mov     ndx,#1              ' _FRQA_FASTER gives 2 clocks per instruction 
:loop               nop                         ' the call and these instructions gives 10 tRP clocks
                    djnz    ndx,#:loop
refresh_wait_ret    ret

'--------------------------------------------------------------------
' refresh
'
sdram_refresh       ' if we get here refresh is 0
                    mov     refresh, refreshtime' reset refresh counter

                    or      outa,_SET_A10       ' set A10 for precharge all bank select on new board

                    mov     frqa,_FRQA_FASTER
                    mov     phsa,_PHSA2         ' edge offset each enable
                    andn    outa,_CLK_MASK

                    andn    outa,_DCMD_PRECHGNOT' send PRECHARG
                    call    #delay2
                    or      outa,_DCMD_NOP      ' set NOP
                    call    #delay2
                    andn    outa,_DCMD_RFRSH_NOT' set REFRESH
                    call    #delay2
                    or      outa,_DCMD_NOP      ' set NOP
                    call    #refresh_wait
                    andn    outa,_DCMD_RFRSH_NOT' set REFRESH
                    call    #delay2
                    or      outa,_DCMD_NOP      ' set NOP
                    call    #refresh_wait
                    mov     frqa,_FRQA_FAST
                    or      outa,_CLK_MASK      ' disable clock

                    andn    outa,_SET_A10       ' release A10 for precharge all bank select
sdram_refresh_ret   ret

refresh         long  625     ' execute 8192 times each 64ms = 8 times/ms
refreshtime     long  625     ' 625*200ns = 125us = 8 times/ms


'====================================================================

_LINELEN        long LINELEN                    ' greatest address for cache
_MAX_ABIT       long $0100_0000

_TAG_MASK       long TAGMASK                    ' greatest address for cache
_TAG_MOD        long (TAGCOUNT-1)<<2            ' use modulo for finding tag address
_TAG_OFF        long TAGCOUNT<<2                ' offset for tags
_TAG_LINE_MASK  long (TAGCOUNT-1)<<LINESHFT     ' use for modulo
_TAG_MSB        long $8000_0000
_TAG_INC        long $0080_0000
_TAG_CNT        long $FF80_0000
_TAG_CMD        long $C000_0000                 ' used for decoding command
_CLP_OFF        long TAGCOUNT<<LINESHFT         ' offset of second cache space

_CAPIN_MASK     long $0000_03ff
_ABPIN_MASK     long ABPIN_MASK
_ALE_ABPIN_MASK long ABPIN_MASK | (1 << ALE_PIN)
_ALE_MASK       long 1 << ALE_PIN
_CLK_MASK       long 1 << CLK_PIN
_WEN_MASK       long 1 << WEN_PIN
_BANK_MASK      long $6000
_ADDR_MASK      long ADDR_MASK
_RASN_MASK      long RASN_MASK
_CASN_MASK      long CASN_MASK
_CPRE_MASK      long CASN_PREPIN
_DCMD_MASK      long DCMD_MASK                  ' SDRAM COMMAND bits
_DCMD_CLK_MASK  long DCMD_MASK | (1 << CLK_PIN)

_SET_A10        long $0400  ' A10 used for precharge all banks A7..0 A15..8
_MSB32          long $8000_0000

_DCMD_NOP       long RASN_MASK | CASN_MASK | WEN_MASK   ' 111 RAS,CAS,WEN
'_DCMD_BTERM     long RASN_MASK | CASN_MASK             ' 110
_DCMD_BTERM_NOT long WEN_MASK                           ' 001
'_DCMD_READ      long RASN_MASK | WEN_MASK              ' 101
_DCMD_READ_NOT  long CASN_MASK                          ' 010
'_DCMD_WRITE     long RASN_MASK                         ' 100
_DCMD_WRITE_NOT long CASN_MASK | WEN_MASK               ' 011
'_DCMD_ACTIVE    long CASN_MASK | WEN_MASK              ' 011
_DCMD_ACTIVENOT long RASN_MASK                          ' 100
'_DCMD_PRECHG    long CASN_MASK                         ' 010
_DCMD_PRECHGNOT long RASN_MASK | WEN_MASK               ' 101
'_DCMD_REFRESH   long WEN_MASK                          ' 001 RAS,CAS,WEN
_DCMD_RFRSH_NOT long RASN_MASK | CASN_MASK              ' 110 RAS,CAS,WEN
_DCMD_MODE      long 0                                  ' 000

_MODE_REG       long MODE_REG                           ' dont shift on new board

OUTPUT_MASK     long ABPIN_MASK| RASN_MASK| CASN_MASK| (1<<ALE_PIN)| (1<<WEN_PIN)

_FRQA_FASTER    long $8000_0000
_FRQA_FAST      long $2000_0000
_PHSA           long $8000_0000
_PHSA8          long $8000_0000
_PHSA4          long $4000_0000
_PHSA2          long $2000_0000
_PHSA1          long $1000_0000

_CTRA           long (%100_000 << 23) | CLK_PIN

data            long 0
dat1            long 0
dat2            long 0
dat3            long 0
dat4            long 0
dat5            long 0
dat6            long 0
dat7            long 0

zero            long 0
dirty           long 0
set2            long 0

'--------------------------------------------------------------------
' setup hardware and mailbox interface
'
sdram_setup
' initialization structure offsets
' $0: pointer to a two word mailbox
' $4: pointer to where to store the cache lines in hub ram
' $8: number of bits in the cache line index if non-zero (default is DEFAULT_INDEX_WIDTH)
' $a: number of bits in the cache line offset if non-zero (default is DEFAULT_OFFSET_WIDTH)
' note that $4 must be at least 2^($8+$a)*2 bytes in size
' the cache line mask is returned in $0

init_vm mov     tmp,par             ' get the address of the initialization structure
        rdlong  cmdptr, tmp         ' pvmcmd is a pointer to the virtual address and read/write bit
        mov     datptr,  cmdptr     ' pvmaddr is a pointer into the cache line on return
        add     datptr,  #4
        add     tmp,#4
        rdlong  cacheptr, tmp       ' cacheptr is the base address in hub ram of the cache
'       add     t1, #4
'       rdlong  t2, t1 wz
' if_nz mov     index_width, t2     ' override the index_width default value
'       add     t1, #4
'       rdlong  t2, t1 wz
' if_nz mov     offset_width, t2    ' override the offset_width default value

'       mov     index_count, #1
'       shl     index_count, index_width
'       mov     index_mask, index_count
'       sub     index_mask, #1

        mov     tmp,#(LINELEN-1)
        wrlong  tmp,par

sdram_enable

' enable outputs
                    or      outa,_DCMD_NOP      ' set NOP for startup
bank                andn    outa,_ABPIN_MASK    ' clear address bits
ndx                 mov     dira, OUTPUT_MASK   ' start with read
tmp                 or      dira,#DBPIN_MASK    ' dbus output for setup

' setup sdram clock
bufp                or      outa,_CLK_MASK      ' clock starts "disabled"
saveaddr            or      dira,_CLK_MASK      ' turn on clock output forever
index               mov     frqa,_FRQA_FAST     ' set access frequency
clineptr            mov     ctra,_CTRA

' write SDRAM mode bits.
' DQM is pulled high on the board; it should be low - rework.
'

' send mode command sequence

send_initmode
                    ' necessary to stabalize SDRAM before first precharge
tags                andn    outa,_CLK_MASK      ' turn on clock
tag                 mov     ndx,#$100
addr                shl     ndx,#7              ' wait 100us
                    add     ndx,cnt
                    waitcnt ndx,ndx
                    or      outa,_CLK_MASK      ' turn off clock

                    call    #sdcmd_PRECHALL
                    call    #sdcmd_NOP
                    call    #sdcmd_NOP
                    call    #sdcmd_NOP
                    call    #sdcmd_NOP
                    mov     ndx,#12             ' at least 8 REFRESH to startup for ISSI
:initloop           call    #sdcmd_REFRESH
                    call    #sdcmd_NOP
                    djnz    ndx,#:initloop

                    call    #sdcmd_MODE
                    call    #sdcmd_NOP
                    call    #sdcmd_NOP
                    call    #sdcmd_NOP

                    ' preload cache line 0 here
                    mov     addr,#0
                    mov     clineptr,cacheptr   ' cachestart
                    call    #sdramBuffRead      ' if zero, load cache line ... changes carry

' all done
                    wrlong  zero,cmdptr         ' let user know we're ready ...
                    jmp     #sdramDone

'--------------------------------------------------------------------
' SDRAM commands
' use movd later to set commands ... can save andn _DCMD_MASK and toggleClock
'
toggleClock         mov     phsa,_PHSA          ' edge offset each enable
                    andn    outa,_CLK_MASK      ' turn clock on
                    or      outa,_CLK_MASK      ' turn clock off
toggleClock_ret     ret

sdcmd_ACTIVE        andn    outa,_DCMD_ACTIVENOT' send ACTIVE
                    call    #toggleClock
sdcmd_ACTIVE_ret    ret

sdcmd_MODE          mov     addr,_MODE_REG      ' new board needs address latched on D0-7 too
                    call    #setAddress         ' set address bits for mode
                    andn    outa,_DCMD_MASK     ' mode CMD is 0
                    call    #toggleClock
                    or      outa,_DCMD_NOP      ' send NOP
sdcmd_MODE_ret      ret

sdcmd_NOP           or      outa,_DCMD_NOP      ' send NOP
                    call    #toggleClock
sdcmd_NOP_ret       ret

sdcmd_PRECHALL
'                   or      addr,_SET_A10       ' set A10 for precharge all bank select
                    or      outa,_SET_A10       ' set A10 for precharge all bank select
'                   call    #setAddress
                    call    #sdcmd_PRECHARG     ' precharge
                    andn    outa,_SET_A10       ' clear A10 when done
sdcmd_PRECHALL_ret  ret

sdcmd_PRECHARG
                    or      outa,_DCMD_NOP      ' send NOP
                    call    #toggleClock
                    andn    outa,_DCMD_PRECHGNOT' send PRECHARG
                    call    #toggleClock
                    or      outa,_DCMD_NOP      ' send NOP
                    call    #toggleClock
                    call    #toggleClock
sdcmd_PRECHARG_ret  ret

sdcmd_REFRESH       andn    outa,_DCMD_RFRSH_NOT' send REFRESH
                    call    #toggleClock
sdcmd_REFRESH_ret   ret

delay2              nop
delay2_ret          ret

'--------------------------------------------------------------------
' this set address is used for refresh and startup precharge only.
'
setAddress          or      dira,#DBPIN_MASK    ' dbus output for data or address write
                    mov     tmp, addr
                    and     tmp,_ABPIN_MASK
                    andn    outa,_ABPIN_MASK
                    or      outa,tmp
                    or      outa,_ALE_MASK
                    andn    outa,_ALE_MASK      ' hold upper address bits
setAddress_ret      ret

'--------------------------------------------------------------------
' Send address sets address into sdram with upper address ACTIVE
' and leaves lower address on the bus for a READ or WRITE to take over.
' This is a burst setup
'
' SDRAM addressing architecture is a little strange. 25 bits for 32MB.
' BA0:1+A0:12 make up the 15 RAS bits. A0:9 make up 10 CAS bits.
' A10 is precharge control, and A11:12 are ignored during CAS cycle.
'
' a full address looks like this B_BHHH_HHhh_hhhh_hhLL_llll_llll
' CBBH_HHHH_hhhh_hhhh shr 10
'

send_ADDRESS
                    or      dira,#DBPIN_MASK    ' set dbus to output 

                    ' setup ROW address
                    mov     tmp,addr            ' %000B_Bhhh_hhhh_hhhh_hhll_llll_llll
                    shr     tmp,#10             ' put hi addr bits on P0-14 %0BBh_hhhh_hhhh_hhhh
                    mov     bank,tmp            ' save bank address bits A13-14 for CAS phase
                    and     bank,_BANK_MASK     ' use only bank bits
                    andn    outa,_ABPIN_MASK    ' clear address bits
                    or      outa,tmp            ' send address
                    or      outa,_ALE_MASK      ' send latch address
                    andn    outa,_ALE_MASK      ' address latched

                    ' send RAS ACTIVE - could call #sdcmd_ACTIVE
                    andn    outa,_DCMD_ACTIVENOT' send ACTIVE
                    mov     phsa,_PHSA          ' clock edge offset
                    andn    outa,_CLK_MASK      ' turn on clock
                    nop
                    or      outa,_DCMD_NOP      ' send NOP
                    call    #delay2
                    or      outa,_CLK_MASK      ' turn off clock

                    ' get lower 10 address bits A0..9 ' could be a subroutine
                    mov     tmp,addr            ' %0000_00ll_llll_llll
                    and     tmp,_CAPIN_MASK     ' put low addr bits on P0-14
                    or      tmp,bank            ' include bank bits for read/write
                    andn    outa,_BANK_MASK     ' clear for next write
                    andn    outa,_CAPIN_MASK    ' clear for next write
                    or      outa,tmp            ' set low bits
                    or      outa,_ALE_MASK      ' send latch address
                    andn    outa,_ALE_MASK      ' address latched

                    or      outa,_SET_A10       ' A10 high for full-burst precharge terminate

send_ADDRESS_ret    ret


'--------------------------------------------------------------------
' read Burst ... 32 bytes ...
'
sdramBuffRead       'init and write set dbus to input
                    call    #send_ADDRESS

                    mov     bufp, clineptr      ' load cacheline

                    mov     phsa,_PHSA1         ' edge offset each enable
                    andn    outa,_DCMD_READ_NOT ' set READ from NOP
                    andn    outa,_CLK_MASK      ' turn on clock now for CAS delay time
                    nop
                    or      outa,_CLK_MASK      ' turn on clock now for CAS delay time
                    or      outa,_DCMD_NOP      ' set nops
                    mov     phsa,_PHSA4         ' edge offset each enable
                    andn    outa,_CLK_MASK      ' turn on clock now for CAS delay time
                    andn    dira,#DBPIN_MASK    ' set dbus as input for read

                    mov     phsa,_PHSA4         ' edge offset each enable
                    andn    outa,_CLK_MASK
                    movs    data,ina            ' $0000_0aAA
                    ror     data,#8     wc      ' $AA00_0001 C = LSB
                    movs    data,ina            ' $AA00_0bBB
                    ror     data,#8             ' $BBAA_000b
                    movs    data,ina            ' $BBAA_0cCC
                    ror     data,#17            ' $00cC_BbAa shifted right x 1
                    movi    data,ina            ' $DDcC_BBAa
                    rcl     data,#1             ' $DDCC_BBAA restore LSB
                    movs    dat1,ina            ' $0000_0aAA
                    ror     dat1,#8     wc      ' $AA00_0001 C = LSB
                    movs    dat1,ina            ' $AA00_0bBB
                    ror     dat1,#8             ' $BBAA_000b
                    movs    dat1,ina            ' $BBAA_0cCC
                    ror     dat1,#17            ' $00cC_BbAa shifted right x 1
                    movi    dat1,ina            ' $DDcC_BBAa
                    rcl     dat1,#1             ' $DDCC_BBAA restore LSB
                    movs    dat2,ina            ' $0000_0aAA
                    ror     dat2,#8     wc      ' $AA00_0001 C = LSB
                    movs    dat2,ina            ' $AA00_0bBB
                    ror     dat2,#8             ' $BBAA_000b
                    movs    dat2,ina            ' $BBAA_0cCC
                    ror     dat2,#17            ' $00cC_BbAa shifted right x 1
                    movi    dat2,ina            ' $DDcC_BBAa
                    rcl     dat2,#1             ' $DDCC_BBAA restore LSB
                    movs    dat3,ina            ' $0000_0aAA
                    ror     dat3,#8     wc      ' $AA00_0001 C = LSB
                    movs    dat3,ina            ' $AA00_0bBB
                    ror     dat3,#8             ' $BBAA_000b
                    movs    dat3,ina            ' $BBAA_0cCC
                    ror     dat3,#17            ' $00cC_BbAa shifted right x 1
                    movi    dat3,ina            ' $DDcC_BBAa
                    rcl     dat3,#1             ' $DDCC_BBAA restore LSB
                    movs    dat4,ina            ' $0000_0aAA
                    ror     dat4,#8     wc      ' $AA00_0001 C = LSB
                    movs    dat4,ina            ' $AA00_0bBB
                    ror     dat4,#8             ' $BBAA_000b
                    movs    dat4,ina            ' $BBAA_0cCC
                    ror     dat4,#17            ' $00cC_BbAa shifted right x 1
                    movi    dat4,ina            ' $DDcC_BBAa
                    rcl     dat4,#1             ' $DDCC_BBAA restore LSB
                    movs    dat5,ina            ' $0000_0aAA
                    ror     dat5,#8     wc      ' $AA00_0001 C = LSB
                    movs    dat5,ina            ' $AA00_0bBB
                    ror     dat5,#8             ' $BBAA_000b
                    movs    dat5,ina            ' $BBAA_0cCC
                    ror     dat5,#17            ' $00cC_BbAa shifted right x 1
                    movi    dat5,ina            ' $DDcC_BBAa
                    rcl     dat5,#1             ' $DDCC_BBAA restore LSB
                    movs    dat6,ina            ' $0000_0aAA
                    ror     dat6,#8     wc      ' $AA00_0001 C = LSB
                    movs    dat6,ina            ' $AA00_0bBB
                    ror     dat6,#8             ' $BBAA_000b
                    movs    dat6,ina            ' $BBAA_0cCC
                    ror     dat6,#17            ' $00cC_BbAa shifted right x 1
                    movi    dat6,ina            ' $DDcC_BBAa
                    rcl     dat6,#1             ' $DDCC_BBAA restore LSB
                    movs    dat7,ina            ' $0000_0aAA
                    ror     dat7,#8     wc      ' $AA00_0001 C = LSB
                    movs    dat7,ina            ' $AA00_0bBB
                    ror     dat7,#8             ' $BBAA_000b
                    movs    dat7,ina            ' $BBAA_0cCC
                    ror     dat7,#17            ' $00cC_BbAa shifted right x 1
                    movi    dat7,ina            ' $DDcC_BBAa

                    wrlong  data,bufp
                    add     bufp,#4
                    or      outa,_CLK_MASK
                    wrlong  dat1,bufp
                    add     bufp,#4
                    rcl     dat7,#1             ' $DDCC_BBAA restore LSB
                    wrlong  dat2,bufp
                    add     bufp,#4
                    andn    outa,_DCMD_PRECHGNOT' set PRECHARGE to terminate burst A10 autoprechare ok
                    wrlong  dat3,bufp
                    add     bufp,#4
                    andn    outa,_CLK_MASK      ' turn on clock
                    wrlong  dat4,bufp
                    add     bufp,#4
                    or      outa,_DCMD_NOP      ' send NOP
                    wrlong  dat5,bufp
                    add     bufp,#4
                    or      outa,_CLK_MASK      ' turn off clock
                    wrlong  dat6,bufp
                    add     bufp,#4
                    wrlong  dat7,bufp

sdramBuffRead_ret   ret

'--------------------------------------------------------------------
' write Burst ... 32 bytes
'
sdramBuffWrite
                    mov     bufp, clineptr      ' get all 32 bytes now

                    call    #send_ADDRESS       ' activate address/bank

                    rdbyte  data,bufp
                    andn    outa,#$ff
                    or      outa,data           ' First byte for write
                    rdlong  data,bufp
                    add     bufp,#4
                    shr     data,#8             ' get next byte ready
                    rdlong  dat1,bufp
                    add     bufp,#4
                    rdlong  dat2,bufp
                    add     bufp,#4
                    rdlong  dat3,bufp
                    add     bufp,#4
                    rdlong  dat4,bufp
                    add     bufp,#4
                    rdlong  dat5,bufp
                    add     bufp,#4
                    rdlong  dat6,bufp
                    add     bufp,#4
                    andn    outa,_DCMD_WRITE_NOT' set WRITE command from NOP
                    rdlong  dat7,bufp

                    mov     phsa,_PHSA8         ' edge offset each enable
                    andn    outa,_CLK_MASK      ' turn on clock now for CAS delay time
                    mov     phsa,_PHSA4         ' clock edge offset
                    or      outa,_DCMD_NOP      ' set NOP after write

                    movs    outa,data           ' A0 has no impact after WRITE cmd
                    shr     data,#8
                    movs    outa,data           
                    shr     data,#8
                    movs    outa,data           
                    nop
                    movs    outa,dat1           ' next byte for write
                    shr     dat1,#8
                    movs    outa,dat1           
                    shr     dat1,#8
                    movs    outa,dat1           
                    shr     dat1,#8
                    movs    outa,dat1           
                    nop
                    movs    outa,dat2           ' next byte for write
                    shr     dat2,#8
                    movs    outa,dat2           
                    shr     dat2,#8
                    movs    outa,dat2           
                    shr     dat2,#8
                    movs    outa,dat2           
                    nop
                    movs    outa,dat3           ' next byte for write
                    shr     dat3,#8
                    movs    outa,dat3           
                    shr     dat3,#8
                    movs    outa,dat3           
                    shr     dat3,#8
                    movs    outa,dat3           
                    nop
                    movs    outa,dat4           ' next byte for write
                    shr     dat4,#8
                    movs    outa,dat4           
                    shr     dat4,#8
                    movs    outa,dat4           
                    shr     dat4,#8
                    movs    outa,dat4           
                    nop
                    movs    outa,dat5           ' next byte for write
                    shr     dat5,#8
                    movs    outa,dat5           
                    shr     dat5,#8
                    movs    outa,dat5           
                    shr     dat5,#8
                    movs    outa,dat5           
                    nop
                    movs    outa,dat6           ' next byte for write
                    shr     dat6,#8
                    movs    outa,dat6           
                    shr     dat6,#8
                    movs    outa,dat6           
                    shr     dat6,#8
                    movs    outa,dat6           
                    nop
                    movs    outa,dat7           ' next byte for write
                    shr     dat7,#8
                    movs    outa,dat7           
                    shr     dat7,#8
                    movs    outa,dat7           
                    shr     dat7,#8
                    movs    outa,dat7           
                    or      outa,_CLK_MASK

                    call    #burstTerminate
sdramBuffWrite_ret  ret

burstTerminate
                    andn    outa,_DCMD_PRECHGNOT' set PRECHARGE to terminate burst A10 autoprechare ok
                    andn    outa,_CLK_MASK      ' turn on clock
                    mov     phsa,_PHSA4         ' clock offset guarantees clock for BURST TERM
                    or      outa,_DCMD_NOP      ' send NOP
                    call    #delay2
                    or      outa,_CLK_MASK      ' turn off clock

burstTerminate_ret  ret

'====================================================================

fillpasm            long 0 [$1ef-$]
endpasm             fit $1ef
