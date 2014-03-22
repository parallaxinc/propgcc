        .section .kernel, "ax"

#if defined(DEBUG_KERNEL)
#include "cogdebug.h"
#endif
        .global r0
        .global r1
        .global r2
        .global r3
        .global r4
        .global r5
        .global r6
        .global r7
        .global r8
        .global r9
        .global r10
        .global r11
        .global r12
        .global r13
        .global r14
        .global lr
        .global sp
        .global pc
        
' at start stack contains cache_mbox, cache_tags, cache_lines, cache_geometry, pc

        .global __LMM_entry
__LMM_entry
r0      jmp     #__LMM_init ' kernel extension .start.kerext is already in place
r1      long    0           ' note: subsequent COGs will start with different
r2      long    0           ' initialization code, set up by the clone routines
r3      long    0
r4      long    0
r5      long    0
r6      long    0
r7      long    0
r8      long    0
r9      long    0
r10     long    0
r11     long    0
r12     long    0
r13     long    0
r14     long    0	'' flag for first time run
r15     '' alias for lr
lr      long    0
sp      long    0
pc      long    0
#if defined(DEBUG_KERNEL)
        .global __ccr__
__ccr__
#endif
ccr     long    0

#if defined(DEBUG_KERNEL)
        ''
        '' there are two "hardware" breakpoints, hwbkpt0 and hwbkpt1
        '' these are useful because XMM code often executes from ROM
        '' or flash where normal software breakpoints are difficult or
        '' impossible to place

hwbkpt0 long    0
        '' register 20 needs to be the breakpoint command
        '' the instruction at "Breakpoint" should be whatever
        '' the debugger should use as a breakpoint instruction
Breakpoint
        call    #__EnterLMMBreakpoint

hwbkpt1 long 0          '' only available in XMM
        
#endif
        ''
        '' main LMM loop -- read instructions from hub memory
        '' and executes them
        ''

#if defined(DEBUG_KERNEL)
__LMM_loop
        muxc    ccr, #1
        muxnz   ccr, #2
#if defined(__PROPELLER2__)
        getp    rxpin wz        ' check for low on RX
#else
        and     rxbit,ina nr,wz ' check for low on RX
#endif
  if_z  call    #__EnterDebugger
        test    ccr, #COGFLAGS_STEP wz
  if_nz call    #__EnterDebugger
        cmp     pc, hwbkpt0 wz
  if_z  call    #__EnterDebugger
        cmp     pc, hwbkpt1 wz
  if_z  call    #__EnterDebugger

        call    #read_code
        add     pc,#4
        shr     ccr, #1 wc,wz,nr        '' restore flags
L_ins0  nop
        jmp     #__LMM_loop
#else
__LMM_loop
        call    #read_code
        add     pc,#4
L_ins0  nop
        jmp     #__LMM_loop
#endif
        .global __C_LOCK_PTR
__C_LOCK_PTR long __C_LOCK

        ''
        '' LMM support functions
        ''

        '' move immediate
        .macro LMM_movi reg
        .global __LMM_MVI_\reg
__LMM_MVI_\reg
        call    #read_code
        mov     \reg,L_ins0
        add     pc,#4
        jmp     #__LMM_loop
        .endm

        LMM_movi r0
        LMM_movi r1
        LMM_movi r2
        LMM_movi r3
        LMM_movi r4
        LMM_movi r5
        LMM_movi r6
        LMM_movi r7
        LMM_movi r8
        LMM_movi r9
        LMM_movi r10
        LMM_movi r11
        LMM_movi r12
        LMM_movi r13
        LMM_movi r14
        LMM_movi lr

        ''
        '' call functions
        ''
        .global __LMM_CALL
__LMM_CALL
        call    #read_code
        add     pc,#4
        mov     lr,pc
        mov     pc,L_ins0
        jmp     #__LMM_loop

        .global __LMM_CALL_INDIRECT
__LMM_CALL_INDIRECT
        mov     lr,pc
        mov     pc,__TMP0
        jmp     #__LMM_loop

        ''
        '' direct jmp
        ''
        .global __LMM_JMP
__LMM_JMP
        call    #read_code
        mov     pc,L_ins0
        jmp     #__LMM_loop
    
    .set RDBYTE_OPCODE, 0x001
    .set RDWORD_OPCODE, 0x009
    .set RDLONG_OPCODE, 0x011
    .set WRBYTE_OPCODE, 0x000
    .set WRWORD_OPCODE, 0x008
    .set WRLONG_OPCODE, 0x010

    ''
    '' memory read instructions
    ''
    '' JMP #__LMM_RDxxxx
    '' the low order bits of __TMP0 are encoded as iddddssss
    '' ssss is the register containing the address from which to read
    '' dddd is the register in which to return the value at that address
    '' i=0 for no increment, i=1 to increment by the size of the data
    '' (not implemented)
    ''
        '' mask to indicate an external memory address
extern_mem_mask
        long    0xfff00000
        
        .global __LMM_RDBYTEI
        .global __LMM_RDBYTEI_ret
__LMM_RDBYTEI
    movi    rdi_common_store, #RDBYTE_OPCODE
    jmp     #rdi_common

        .global __LMM_RDWORDI
        .global __LMM_RDWORDI_ret
__LMM_RDWORDI
    movi    rdi_common_store, #RDWORD_OPCODE
    jmp     #rdi_common

        .global __LMM_RDLONGI
        .global __LMM_RDLONGI_ret
__LMM_RDLONGI
    movi    rdi_common_store, #RDLONG_OPCODE
        
rdi_common
    muxnz   save_z_c, #2    'save the z flag
    movs    rdi_common_fetch_addr, __TMP0
    andn    rdi_common_fetch_addr, #0x1f0
    shr     __TMP0, #4
    and     __TMP0, #0xf    
    movd    rdi_common_store, __TMP0
rdi_common_fetch_addr
    mov     t1, 0-0
    test    t1, extern_mem_mask wz
  IF_Z mov memp,t1
  IF_NZ call    #cache_read
    test    save_z_c, #2 wz             'restore the z flag
rdi_common_store
    rdlong  0-0, memp
        
__LMM_RDLONGI_ret
__LMM_RDWORDI_ret
__LMM_RDBYTEI_ret
    ret
    
    ''
    '' memory write instructions
    ''
    '' JMP iddddssss,#__LMM_WRxxxx
    '' the low order bits of __TMP0 are encoded as iddddssss
    '' ssss is the register containing the address to which to write
    '' dddd is the register containing the value to write to that address
    '' i=0 for no increment, i=1 to increment by the size of the data

        .global __LMM_WRBYTEI
        .global __LMM_WRBYTEI_ret
__LMM_WRBYTEI
    movi    wri_common_fetch_data, #WRBYTE_OPCODE
    jmp     #wri_common

        .global __LMM_WRWORDI
        .global __LMM_WRWORDI_ret
__LMM_WRWORDI
    movi    wri_common_fetch_data, #WRWORD_OPCODE
    jmp     #wri_common

        .global __LMM_WRLONGI
        .global __LMM_WRLONGI_ret
__LMM_WRLONGI
    movi    wri_common_fetch_data, #WRLONG_OPCODE
        
wri_common
    muxnz   save_z_c, #2    'save the z flag
    movs    wri_common_fetch_addr, __TMP0
    andn    wri_common_fetch_addr, #0x1f0
    shr     __TMP0, #4
    and     __TMP0, #0xf
    movd    wri_common_fetch_data, __TMP0
wri_common_fetch_addr
    mov     t1, 0-0
    test    t1, extern_mem_mask wz
  IF_Z mov memp,t1
  IF_NZ call    #cache_write
    test    save_z_c, #2 wz             'restore the z flag
wri_common_fetch_data
    wrlong  0-0, memp
        
__LMM_WRBYTEI_ret
__LMM_WRWORDI_ret
__LMM_WRLONGI_ret
    ret
    
        ''
        '' push and pop multiple
        '' these take in __TMP0 a mask
        '' of (first_register|(count<<4))
        ''
        '' note that we push from low register first (so registers
        '' increment as the stack decrements) and pop the other way
        ''

        .global __LMM_PUSHM
        .global __LMM_PUSHM_ret
__LMM_PUSHM
        mov     __TMP1,__TMP0
        and     __TMP1,#0x0f
        movd    L_pushins,__TMP1
        shr     __TMP0,#4
L_pushloop
        sub     sp,#4
L_pushins
        wrlong  0-0,sp
        add     L_pushins,inc_dest1
        djnz    __TMP0,#L_pushloop
__LMM_PUSHM_ret
        ret
        
inc_dest1
        long    (1<<9)

        .global __LMM_POPM
        .global __LMM_POPM_ret
        .global __LMM_POPRET
        .global __LMM_POPRET_ret
        
__LMM_POPRET
        call    #__LMM_POPM
        mov     pc,lr
__LMM_POPRET_ret
        ret

__LMM_POPM
        mov     __TMP1,__TMP0
        and     __TMP1,#0x0f
        movd    L_poploop,__TMP1
        shr     __TMP0,#4
L_poploop
        rdlong  0-0,sp
        add     sp,#4
        sub     L_poploop,inc_dest1
        djnz    __TMP0,#L_poploop
__LMM_POPM_ret
        ret

        ''
        '' masks
        ''
        
        .global __MASK_0000FFFF
        .global __MASK_FFFFFFFF

__MASK_0000FFFF long    0x0000FFFF
__MASK_FFFFFFFF long    0xFFFFFFFF

        ''
        '' math support functions
        ''
        .global __TMP0
        .global __TMP1
__TMP0  long    0
__TMP1  long    0

        .global __MULSI
        .global __MULSI_ret
__MULSI
        mov     __TMP0, r0
        min     __TMP0, r1
        max     r1, r0
        mov     r0, #0
__MULSI_loop
        shr     r1, #1  wz, wc
 IF_C   add     r0, __TMP0
        add     __TMP0, __TMP0
 IF_NZ  jmp     #__MULSI_loop
__MULSI_ret     ret

' the code below comes from the xbasic virtual machine which borrowed code from zog

' read a long from the current pc
read_code
        muxc    save_z_c, #1                'save the c flag
        cmp     pc, external_start wc       'check for normal memory access
  if_b  jmp     #read_hub_code
        mov     t1, pc
        muxnz   save_z_c, #2                'save the z flag
        call    #cache_read_common
        rdlong  L_ins0, memp
        jmp     read_code_ret
read_hub_code
        rdlong  L_ins0, pc
        shr     save_z_c, #1 wc             'restore the c flag
read_code_ret
        ret

  ' start of external memory
  .set EXTERNAL_MEMORY_START, 0x20000000

  ' default cache geometry
  .set DEFAULT_INDEX_WIDTH, 7   ' number of bits in the index offset (index size is 2^n)
  .set DEFAULT_OFFSET_WIDTH, 6  ' number of bits in the line offset (line size is 2^n)

  ' cache line tag flags
  .set EMPTY_BIT, 30
  .set DIRTY_BIT, 31

' on call:
'   t1 contains the address to write
'   z saved in save_z_c using the instruction "muxnz save_z_c, #2"
' on return:
'   memp contains a pointer to the data
'   z will be restored from save_z_c
cache_write
        muxc    save_z_c, #1            ' save the c flag
        mov     t2, t1                  ' get the cache line address
        andn    t2, offset_mask
        mov     set_dirty_bit, dirty_mask
        jmp     #rd_wr

' on call:
'   t1 contains the address to read
'   z saved in save_z_c using the instruction "muxnz save_z_c, #2"
' on return:
'   memp contains a pointer to the data
'   z will be restored from save_z_c
cache_read
        muxc    save_z_c, #1            ' save the c flag
cache_read_common
        mov     t2, t1                  ' get the cache line address
        andn    t2, offset_mask
        cmp     cacheaddr, t2 wz        ' check to see if it's the same cache line as last time
 if_z   jmp     #skip                   ' if z, use the already translated cache line
        mov     set_dirty_bit, #0       ' don't set the dirty bit in the tag

' at this point:
'   t1 contains the external address
'   t2 contains the external address of the page containing t1
rd_wr   mov     newtag, t1              ' get the new cache tag
        shr     newtag, offset_width
        mov     tagptr, newtag          ' get the tag line offset
        and     tagptr, index_mask
        mov     cacheline, tagptr
        shl     tagptr, #2              ' get the tag address
        add     tagptr, tagsptr
        shl     cacheline, offset_width ' get the cache line address
        add     cacheline, cacheptr
        
        rdlong  tag, tagptr             ' get the cache line tag
        mov     t3, tag
        and     t3, tag_mask
        cmp     t3, newtag wz           ' z set means there was a cache hit
  if_z  jmp     #hit                    ' handle a cache hit
  
' at this point:
'   t1 contains the external address
'   t2 contains the external address of the page containing t1
miss

lck_spi test    $, #0 wc                ' lock no-op: clear the carry bit
   if_c jmp     #lck_spi

        test    tag, dirty_mask wz      ' check to see if old cache line is dirty
  if_z  jmp     #do_read                ' skip the write if the cache line isn't dirty
  
do_write
        mov     t3, tag                 ' get current tag's external address
        shl     t3, offset_width
        wrlong  t3, xmem_extaddrp       ' setup the external address of the write
        mov     t3, size_select         ' setup the hub address of the write
        or      t3, cacheline
        or      t3, #8                  ' set the write flag
        wrlong  t3, xmem_hubaddrp       ' initiate the write
wwait   rdlong  t3, xmem_hubaddrp wz    ' wait for the write to complete
  if_nz jmp     #wwait

' at this point:
'   t1 contains the external address
'   t2 contains the external address of the page containing t1
do_read wrlong  t2, xmem_extaddrp
        mov     t3, size_select         ' setup the hub address of the read
        or      t3, cacheline
        wrlong  t3, xmem_hubaddrp       ' initiate the read
rwait   rdlong  t3, xmem_hubaddrp wz    ' wait for the read to complete
  if_nz jmp     #rwait
  
nlk_spi nop        

        mov     tag, newtag             ' setup the new tag
 
' at this point:
'   t1 contains the external address
'   t2 contains the external address of the page containing t1
hit     or      tag, set_dirty_bit      ' set the dirty bit on writes
        wrlong  tag, tagptr             ' store the tag for the newly loaded cache line

        mov     cacheaddr, t2           ' save new cache line address
        
skip    mov     memp, t1                ' get cache line offset
        and     memp, offset_mask
        add     memp, cacheline         ' get address of data in cache line
        test    save_z_c, #2 wz         ' restore the z flag
        shr     save_z_c, #1 wc, nr     ' restore the c flag
cache_write_ret
cache_read_ret
cache_read_common_ret
        ret
        
		.global __enable_spi_locking
		.global __enable_spi_locking_ret
__enable_spi_locking
        mov     lock_id, r0
        mov     lck_spi, lock_set
        mov     nlk_spi, lock_clr
__enable_spi_locking_ret
        ret

lock_set
        lockset lock_id wc
lock_clr
        lockclr lock_id

lock_id long    0                       ' lock id for optional spi bus interlock

' external memory driver command syntax
' long 0 (xmem_hubaddrp) - hub address, size selector, and write flag
' long 1 (xmem_extaddrp) - external address
' size selector in bits 2:0 are 1=16, 2=32, 3=64 ... 7=1024
' write mask is $8
        
t1              long    0               ' temporary variables
t2              long    0
t3              long    0
tagptr          long    0
newtag          long    0
tag             long    0
memp            long    0
save_z_c        long    0

		.global cacheaddr
		.global cacheline
		.global set_dirty_bit
cacheaddr       long    0               ' external address of last matching cache line
cacheline       long    0               ' hub address of last matching cache line
set_dirty_bit   long    0               ' DIRTY_BIT set on writes, clear on reads

		.global xmem_hubaddrp	' thread code needs access to the mailbox array address
xmem_hubaddrp   long    0
xmem_extaddrp   long    0

tagsptr         long    0
cacheptr        long    0
index_width     long    DEFAULT_INDEX_WIDTH
offset_width    long    DEFAULT_OFFSET_WIDTH

index_mask      long    0
index_count     long    0
offset_mask     long    0
size_select     long    0

tag_mask        long    (1 << DIRTY_BIT) - 1    ' includes EMPTY_BIT
empty_mask      long    1 << EMPTY_BIT
dirty_mask      long    1 << DIRTY_BIT

external_start  long    EXTERNAL_MEMORY_START   ' start of external memory

        ''
        '' code to load a buffer from hub memory into cog memory
        ''
        '' parameters: __TMP0 = count of bytes
        ''             __TMP1 = hub address
        ''             __COGA = COG address
        ''
        ''
__COGA  long 0
dst1    long 1 << 9
dummy   long 0
        
loadbuf
        shr     __TMP0,#2       '' convert to longs
        movd    .ldhlp,__COGA
        mov     dummy, extern_mem_mask
        and     dummy, __TMP1
        tjz     dummy, #loadbuf_hub
loadbuf_xmm
        mov     dummy,pc
        mov     pc,__TMP1
        movd    .ldxfetch,__COGA
.ldxlp
        call    #read_code
.ldxfetch
        mov     0-0, L_ins0
        add     pc,#4
        add     .ldxfetch,dst1
        djnz    __TMP0,#.ldxlp
        mov     pc,dummy
        jmp     loadbuf_ret

loadbuf_hub
.ldhlp
        rdlong  0-0, __TMP1
        add     __TMP1,#4
        add     .ldhlp,dst1
        djnz    __TMP0,#.ldhlp
        jmp     loadbuf_ret
        
loadbuf_ret
        ret

        ''
        '' FCACHE region
        '' The FCACHE is an area where we can
        '' execute small functions or loops entirely
        '' in Cog memory, providing a significant
        '' speedup.
        ''
	.global __LMM_FCACHE_ADDR
__LMM_FCACHE_ADDR
        long 0
inc_dest
        long (1<<9)
        
        .global __LMM_RET
        .global __LMM_FCACHE_LOAD
__LMM_RET
        long 0
__LMM_FCACHE_LOAD
        call    #read_code      '' read count of bytes for load
        mov     __TMP0,L_ins0
        add     pc,#4
        cmp     __LMM_FCACHE_ADDR,pc wz '' is this the same fcache block we loaded last?
  IF_Z  add     pc,__TMP0       '' skip over data
  IF_Z  jmp     #Lmm_fcache_doit

        mov     __LMM_FCACHE_ADDR, pc
        
        '' assembler awkwardness here
        '' we would like to just write
        '' movd Lmm_fcache_loop,#__LMM_FCACHE_START
        '' but binutils doesn't work right with this now
        movd Lmm_fcache_fetch,#(__LMM_FCACHE_START-__LMM_entry)/4
        shr  __TMP0,#2          '' we process 4 bytes per loop iteration
Lmm_fcache_loop
        call    #read_code
Lmm_fcache_fetch
        mov     0-0,L_ins0
        add     pc,#4
        add     Lmm_fcache_fetch,inc_dest
        djnz    __TMP0,#Lmm_fcache_loop

Lmm_fcache_doit
        jmpret  __LMM_RET,#__LMM_FCACHE_START
        jmp     #__LMM_loop

        ''
        '' compare and swap a variable
        '' r0 == new value to set if (*r2) == r1
        '' r1 == value to compare with
        '' r2 == pointer to memory
	'' r3,r4 == scratch registers
        '' output: r0 == original value of (*r2)
        ''         Z flag is set if (*r2) == r1, clear otherwise
        ''
        .global __CMPSWAPSI
        .global __CMPSWAPSI_ret
__CMPSWAPSI
        rdlong  r3,__C_LOCK_PTR
        mov     r4,r0       '' save value to set
.swaplp
        lockset r3 wc
   IF_C jmp     #.swaplp

        cmp     r2,external_start wc    '' check for hub or external memory
   IF_B  jmp     #.cmpswaphub
        xmmio   rdlong,r0,r2           '' fetch original value
        cmp     r0,r1 wz        '' compare with desired original value
   IF_NZ jmp    #.cmpswapdone
        xmmio	wrlong,r4,r2     '' if match, save new value
        jmp     #.cmpswapdone
        
.cmpswaphub
        rdlong  r0,r2           '' fetch original value
        cmp     r0,r1 wz        '' compare with desired original value
   IF_Z wrlong  r4,r2       '' if match, save new value

.cmpswapdone
        '' now release the C lock
        lockclr r3
__CMPSWAPSI_ret
        ret

        ''
        '' the fcache area should come last in the file
        ''
        .global __LMM_FCACHE_START
__LMM_FCACHE_START
#if defined(DEBUG_KERNEL)
        res     16      '' token amount, not really useful in debug
#else
        res     64      '' reserve 64 longs = 256 bytes
#endif

        #include "kernel.ext"
#ifdef DEBUG_KERNEL
        #include "cogdebug.ext"
#endif
