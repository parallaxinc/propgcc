	.section .kernel, "ax"

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

	.global __LMM_entry
__LMM_entry
#if 0 // Eric's changes
r0	mov	__TMP1, r6	'' get pointer to initialization
r1	call	#__load_extension
r2  	jmp	#__LMM_init
	

r3      nop
r4	nop
r5      nop
r6      long __load_start_start_kerext

r7      nop
r8      nop
r9      nop
r10     nop
r11     nop
r12     nop
r13     nop
r14     nop
        
r15     '' alias for link register lr
lr      long    __exit
sp      long    0
pc      long    entry           ' default pc
#else
r0      rdlong  sp, PAR
r1      tjz     sp, #__LMM_entry
r2      rdlong  cache_mboxcmd, sp
r3      add     sp, #4
r4      nop
r5      nop
r6      nop
r7      add     sp, #4
r8      rdlong  pc, sp
r9      add     sp, #4
r10     locknew r2 wc
r11     or      r2,#256
r12 IF_NC wrlong r2,__C_LOCK_PTR
r13     call    #cache_init
r14     jmp     #__LMM_loop
r15     '' alias for lr
lr      long    0
sp      long    0
pc      long    0
#endif
ccr     long    0

	''
	'' main LMM loop -- read instructions from hub memory
	'' and executes them
	''

__LMM_loop
	call	#read_code
	add	pc,#4
L_ins0	nop
	jmp	#__LMM_loop

	.global __C_LOCK_PTR
__C_LOCK_PTR long __C_LOCK

	''
	'' LMM support functions
	''

	'' move immediate
	.macro LMM_movi reg
	.global __LMM_MVI_\reg
__LMM_MVI_\reg
	call	#read_code
	mov	\reg,L_ins0
	add	pc,#4
	jmp	#__LMM_loop
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
	.global	__LMM_CALL
__LMM_CALL
	call	#read_code
	add	pc,#4
	mov	lr,pc
	mov	pc,L_ins0
	jmp	#__LMM_loop

	.global __LMM_CALL_INDIRECT
__LMM_CALL_INDIRECT
	mov	lr,pc
	mov	pc,__TMP0
	jmp	#__LMM_loop

	''
	'' direct jmp
	''
	.global __LMM_JMP
__LMM_JMP
	call	#read_code
	mov	pc,L_ins0
	jmp	#__LMM_loop
    
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
	long	0xfff00000
	
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
    shr	    __TMP0, #4
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
    shr	    __TMP0, #4
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
	mov	__TMP1,__TMP0
	and	__TMP1,#0x0f
	movd	L_pushins,__TMP1
	shr	__TMP0,#4
L_pushloop
	sub	sp,#4
L_pushins
	wrlong	0-0,sp
	add	L_pushins,inc_dest1
	djnz	__TMP0,#L_pushloop
__LMM_PUSHM_ret
	ret
	
inc_dest1
	long	(1<<9)

	.global __LMM_POPM
	.global __LMM_POPM_ret
	.global __LMM_POPRET
	.global __LMM_POPRET_ret
	
__LMM_POPRET
	call	#__LMM_POPM
	mov	pc,lr
__LMM_POPRET_ret
	ret

__LMM_POPM
	mov	__TMP1,__TMP0
	and	__TMP1,#0x0f
	movd	L_poploop,__TMP1
	shr	__TMP0,#4
L_poploop
	rdlong	0-0,sp
	add	sp,#4
	sub	L_poploop,inc_dest1
	djnz	__TMP0,#L_poploop
__LMM_POPM_ret
	ret

	''
	'' masks
	''
	
	.global __MASK_0000FFFF
	.global __MASK_FFFFFFFF

__MASK_0000FFFF	long	0x0000FFFF
__MASK_FFFFFFFF	long	0xFFFFFFFF

	''
	'' math support functions
	''
	.global __TMP0
	.global __TMP1
__TMP0	long	0
__TMP1	long	0

	.global __MULSI
	.global __MULSI_ret
__MULSI
	mov	__TMP0, r0
	min	__TMP0, r1
	max	r1, r0
	mov	r0, #0
__MULSI_loop
	shr	r1, #1	wz, wc
 IF_C	add	r0, __TMP0
	add	__TMP0, __TMP0
 IF_NZ	jmp	#__MULSI_loop
__MULSI_ret	ret

' the code below comes from the xbasic virtual machine which borrowed code from zog

    .set EXTERNAL_MEMORY_START, 0x20000000
    .set CACHE_CMD_MASK,        0x00000003
    .set CACHE_WRITE_CMD,       0x00000002
    .set CACHE_READ_CMD,        0x00000003

' read a long from the current pc
read_code               muxc    save_z_c, #1                'save the c flag
                        cmp     pc, external_start wc       'check for normal memory access
            if_b        jmp     #read_hub_code
                        mov     t1, pc
                        muxnz   save_z_c, #2                'save the z flag
                        call    #cache_read
                        rdlong  L_ins0, memp
                        jmp     #read_restore_c
read_hub_code           rdlong	L_ins0, pc
read_restore_c          shr     save_z_c, #1 wc             'restore the c flag
read_code_ret           ret

  ' cache driver commands
  .set READ_DATA_CMD, 0xd       ' SD_INIT_CMD repurposed for this test
  
  ' default cache geometry
  .set DEFAULT_INDEX_WIDTH, 7   ' number of bits in the index offset (index size is 2^n)
  .set DEFAULT_OFFSET_WIDTH, 6  ' number of bits in the line offset (line size is 2^n)

  ' cache line tag flags
  .set EMPTY_BIT, 30
  .set DIRTY_BIT, 31

        ' initialize the cache variables
cache_init
        mov     index_count, #1
        shl     index_count, index_width
        mov     index_mask, index_count
        sub     index_mask, #1
        mov     line_size, #1
        shl     line_size, offset_width
        mov     offset_mask, line_size
        sub     offset_mask, #1
        mov     offset_shift, offset_width
        sub     offset_shift, #2

        ' initialize the cache lines
cache_flush
        mov     t1, index_count
        mov     t2, tagsptr
flush   wrlong  empty_mask, t2
        add     t2, #4
        djnz    t1, #flush
cache_init_ret
cache_flush_ret
        ret
        
' on call:
'   t1 contains the address of the data in the cache line
'   z saved in save_z_c using the instruction "muxnz save_z_c, #2"
' on return:
'   memp contains a pointer to the data
'   t1 is trashed
'   z will be restored from save_z_c
cache_read
        mov     t2, t1                  ' ptr + cache_mboxdat = hub address of byte to load
        andn    t2, offset_mask
        cmp     cacheaddr, t2 wz        ' if cacheaddr == addr, just pull form cache
 if_e   jmp     #skip                   ' memp gets overwriteen on a miss
        mov     vmpage, t1
        shr     vmpage, offset_width wc ' carry is now one for read and zero for write
        mov     tagptr, vmpage          ' get the tag address
        and     tagptr, index_mask
        shl     tagptr, #2
        add     tagptr, tagsptr
        
        ' get hub address of cache line
        mov     cacheline, tagptr
        sub     cacheline, tagsptr
        shl     cacheline, offset_shift
        add     cacheline, cacheptr
                
        rdlong  t2, tagptr              ' get the cache line tag
        and     t2, tag_mask
        cmp     t2, vmpage wz           ' z set means there was a cache hit
  if_z  jmp     #hit                    ' handle a cache hit
  
        mov     t2, cmdptr
        wrlong  cacheline, t2
        add     t2, #4
        wrlong  line_size, t2
        add     t2, #4
        mov     t3, t1
        andn    t3, offset_mask
        wrlong  t3, t2
        mov     t2, cmdptr
        shl     t2, #8
        or      t2, #READ_DATA_CMD
        wrlong  t2, cache_mboxcmd
wait    rdlong  t2, cache_mboxcmd wz
 if_nz  jmp     #wait
        wrlong  vmpage, tagptr

hit     mov     cacheaddr, t1               'save new cache address
        andn    cacheaddr, offset_mask
skip    mov     memp, t1
        and     memp, offset_mask
        add     memp, cacheline
cache_write
        test    save_z_c, #2 wz         ' restore the z flag
cache_read_ret
cache_write_ret
        ret
        
t1              long    0
t2              long    0
t3              long    0
vmpage          long    0
tagptr          long    0
memp            long    0
save_z_c        long    0
cacheaddr       long    0
cacheline       long    0

cache_mboxcmd   long    0
cmdptr          long    0x8000 - 8192 - 512 - 12
tagsptr         long    0x8000 - 8192 - 512
cacheptr        long    0x8000 - 8192

index_width     long    DEFAULT_INDEX_WIDTH
offset_width    long    DEFAULT_OFFSET_WIDTH
index_mask      long    0
index_count     long    0
offset_mask     long    0
offset_shift    long    0
line_size       long    0

tag_mask        long    (1 << DIRTY_BIT) - 1    ' includes EMPTY_BIT
empty_mask      long    1 << EMPTY_BIT

external_start          long    EXTERNAL_MEMORY_START       'start of external memory access window

	''
	'' code to load a buffer from hub memory into cog memory
	''
	'' parameters: __TMP0 = count of bytes
	''             __TMP1 = hub address
	''             __COGA = COG address
	''
	''
__COGA	long 0
dst1	long 1 << 9
dummy	long 0
	
loadbuf
	shr	__TMP0,#2	'' convert to longs
	movd	.ldhlp,__COGA
	mov	dummy, extern_mem_mask
	and	dummy, __TMP1
	tjz	dummy, #loadbuf_hub
loadbuf_xmm
	mov	dummy,pc
	mov	pc,__TMP1
	movd	.ldxfetch,__COGA
.ldxlp
	call	#read_code
.ldxfetch
	mov	0-0, L_ins0
	add	pc,#4
	add	.ldxfetch,dst1
	djnz	__TMP0,#.ldxlp
	mov	pc,dummy
	jmp	loadbuf_ret

loadbuf_hub
.ldhlp
	rdlong	0-0, __TMP1
	add	__TMP1,#4
	add	.ldhlp,dst1
	djnz	__TMP0,#.ldhlp
	jmp	loadbuf_ret
	
loadbuf_ret
	ret

	''
	'' FCACHE region
	'' The FCACHE is an area where we can
	'' execute small functions or loops entirely
	'' in Cog memory, providing a significant
	'' speedup.
	''

__LMM_FCACHE_ADDR
	long 0
inc_dest
	long (1<<9)
	
	.global	__LMM_RET
	.global	__LMM_FCACHE_LOAD
__LMM_RET
	long 0
__LMM_FCACHE_LOAD
	call	#read_code	'' read count of bytes for load
	mov	__TMP0,L_ins0
	add	pc,#4
	cmp	__LMM_FCACHE_ADDR,pc wz	'' is this the same fcache block we loaded last?
  IF_Z	add	pc,__TMP0	'' skip over data
  IF_Z	jmp	#Lmm_fcache_doit

	mov	__LMM_FCACHE_ADDR, pc
	
	'' assembler awkwardness here
	'' we would like to just write
	'' movd	Lmm_fcache_loop,#__LMM_FCACHE_START
	'' but binutils doesn't work right with this now
	movd Lmm_fcache_fetch,#(__LMM_FCACHE_START-__LMM_entry)/4
	shr  __TMP0,#2		'' we process 4 bytes per loop iteration
Lmm_fcache_loop
	call	#read_code
Lmm_fcache_fetch
	mov	0-0,L_ins0
	add	pc,#4
	add	Lmm_fcache_fetch,inc_dest
	djnz	__TMP0,#Lmm_fcache_loop

Lmm_fcache_doit
	jmpret	__LMM_RET,#__LMM_FCACHE_START
	jmp	#__LMM_loop

	''
	'' compare and swap a variable
	'' r0 == new value to set if (*r2) == r1
	'' r1 == value to compare with
	'' r2 == pointer to memory
	'' output: r0 == original value of (*r2)
	''         Z flag is set if (*r2) == r1, clear otherwise
	''
	.global __CMPSWAPSI
	.global __CMPSWAPSI_ret
__CMPSWAPSI
	rdlong	__TMP1,__C_LOCK_PTR
	mov	__TMP0,r0	'' save value to set
.swaplp
	lockset	__TMP1 wc
   IF_C jmp	#.swaplp

	cmp	r2,external_start wc	'' check for hub or external memory
IF_B	jmp	#.cmpswaphub
	mov	t1,r2		'' save address
	call	cache_read
	rdlong	r0, memp	'' fetch original value
	cmp	r0,r1 wz	'' compare with desired original value
   IF_NZ jmp	#.cmpswapdone
	mov	t1,r2
	call	cache_write
	wrlong	__TMP0,memp	'' if match, save new value
	jmp	#.cmpswapdone
	
.cmpswaphub
	rdlong	r0,r2		'' fetch original value
	cmp	r0,r1 wz	'' compare with desired original value
   IF_Z wrlong  __TMP0,r2	'' if match, save new value

.cmpswapdone
	'' now release the C lock
	lockclr __TMP1
__CMPSWAPSI_ret
	ret

	''
	'' the fcache area should come last in the file
	''
	.global __LMM_FCACHE_START
__LMM_FCACHE_START
	res	64	'' reserve 64 longs = 256 bytes

	#include "kernel.ext"

