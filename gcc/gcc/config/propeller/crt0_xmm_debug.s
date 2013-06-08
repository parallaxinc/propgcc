#include "cogdebug.h"
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

    .set SIMPLE_XMM_RDWR, 0
    .set INTERMEDIATE_XMM_RDWR, 1
    .set COMPLEX_XMM_RDWR, 0

	.global __LMM_entry
__LMM_entry
r0	mov	__TMP1, r3	'' get pointer to initialization
r1	call	#__load_extension
r2  	jmp	#__LMM_init
r3	long __load_start_start_kerext
	
r4	nop
r5      nop
r6      nop
r7      nop
r8      nop
r9      nop
r10     nop
r11     nop
r12     nop
r13	nop
r14	nop
	
r15	'' alias for link register lr
lr	long	__exit
sp	long	0
pc	long	entry		' default pc

	.global __ccr__
__ccr__
ccr	long	0

	''
	'' there are two "hardware" breakpoints, hwbkpt0 and hwbkpt1
	'' these are useful because XMM code often executes from ROM
	'' or flash where normal software breakpoints are difficult or
	'' impossible to place

hwbkpt0	long	0
	'' register 20 needs to be the breakpoint command
	'' the instruction at "Breakpoint" should be whatever
	'' the debugger should use as a breakpoint instruction
Breakpoint
	call	#__EnterLMMBreakpoint

hwbkpt1	long 0		'' only available in XMM
	
	''
	'' main LMM loop -- read instructions from hub memory
	'' and executes them
	''
__LMM_loop
	muxc	ccr, #1
	muxnz	ccr, #2
#if defined(__PROPELLER2__)
	getp	rxpin wz	' check for low on RX
#else
	and	rxbit,ina nr,wz	' check for low on RX
#endif
  if_z	call	#__EnterDebugger
	test	ccr, #COGFLAGS_STEP wz
  if_nz call	#__EnterDebugger
	cmp	pc, hwbkpt0 wz
  if_z  call	#__EnterDebugger
	cmp	pc, hwbkpt1 wz
  if_z  call	#__EnterDebugger

	call	#read_code
	add	pc,#4
	shr	ccr, #1 wc,wz,nr	'' restore flags
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

    .if SIMPLE_XMM_RDWR
    
    ''
    '' simple memory read instructions
    ''
    '' On call:
    ''   __TMP0 contains the address from which to read
    '' On return:
    ''   __TMP1 contains the value at that address
    ''

	.global __LMM_RDBYTE
	.global __LMM_RDBYTE_ret
__LMM_RDBYTE
    movi    rd_common_read, #RDBYTE_OPCODE
    jmp     #rd_common

	.global __LMM_RDWORD
	.global __LMM_RDWORD_ret
__LMM_RDWORD
    movi    rd_common_read, #RDWORD_OPCODE
    jmp     #rd_common

	.global __LMM_RDLONG
	.global __LMM_RDLONG_ret
__LMM_RDLONG
    movi    rd_common_read, #RDLONG_OPCODE

rd_common
    muxc    save_z_c, #1                    'save the c flag
    cmp     __TMP0, external_start wc       'check for normal memory access
 IF_B   mov     memp, __TMP0
 IF_B   jmp     #rd_common_read
    mov     t1, __TMP0
    muxnz   save_z_c, #2                    'save the z flag
    call    #cache_read                     'also restores the z flag
rd_common_read
    rdlong  __TMP1, memp
    shr     save_z_c, #1 wc                 'restore the c flag
__LMM_RDBYTE_ret
__LMM_RDWORD_ret
__LMM_RDLONG_ret
    ret
    
    ''
    '' simple memory write instructions
    ''
    '' On call:
    ''   __TMP0 contains the address to which to write
    ''   __TMP1 contains the value to write to that address
    ''

	.global __LMM_WRBYTE
	.global __LMM_WRBYTE_ret
__LMM_WRBYTE
    movi    wr_common_write, #WRBYTE_OPCODE
    jmp     #wr_common

	.global __LMM_WRWORD
	.global __LMM_WRWORD_ret
__LMM_WRWORD
    movi    wr_common_write, #WRWORD_OPCODE
    jmp     #wr_common

	.global __LMM_WRLONG
	.global __LMM_WRLONG_ret
__LMM_WRLONG
    movi    wr_common_write, #WRLONG_OPCODE

wr_common
    muxc    save_z_c, #1                    'save the c flag
    cmp     __TMP0, external_start wc       'check for normal memory access
 IF_B   mov     memp, __TMP0
 IF_B   jmp     #wr_common_write
    mov     t1, __TMP0
    muxnz   save_z_c, #2                    'save the z flag
    call    #cache_write                    'also restores the z flag
wr_common_write
    wrlong  __TMP1, memp
    shr     save_z_c, #1 wc                 'restore the c flag
__LMM_WRBYTE_ret
__LMM_WRWORD_ret
__LMM_WRLONG_ret
    ret

    .endif 'SIMPLE_XMM_RDWR
    
    .if INTERMEDIATE_XMM_RDWR

    ''
    '' intermediate memory read instructions
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
    '' intermediate memory write instructions
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
    
    .endif 'INTERMEDIATE_XMM_RDWR

    .if COMPLEX_XMM_RDWR
    
    ''
    '' complex memory read instructions
    ''
    '' JMP iddddssss,#__LMM_RDxxxx
    '' ssss is the register containing the address from which to read
    '' dddd is the register in which to return the value at that address
    '' i=0 for no increment, i=1 to increment by the size of the data

	.global __LMM_RDBYTEX
__LMM_RDBYTEX
    movi    rdx_common_store, #RDBYTE_OPCODE
    movs    rdx_common_inc, #1
    jmp     #rdx_common

	.global __LMM_RDWORDX
__LMM_RDWORDX
    movi    rdx_common_store, #RDWORD_OPCODE
    movs    rdx_common_inc, #2
    jmp     #rdx_common

	.global __LMM_RDLONGX
__LMM_RDLONGX
    movi    rdx_common_store, #RDLONG_OPCODE
    movs    rdx_common_inc, #4

rdx_common
    muxnz   save_z_c, #2    'save the z flag
    shr	    L_ins0, #9
    movs    rdx_common_fetch_addr, L_ins0
    andn    rdx_common_fetch_addr, #0x1f0
    shr	    L_ins0, #4
    test    L_ins0, #0x10 wz
    and     L_ins0, #0xf    
    movd    rdx_common_inc, L_ins0
    movd    rdx_common_store, L_ins0
rdx_common_inc
 IF_NZ  add 0-0, #4
rdx_common_fetch_addr
    mov     t1, 0-0
    call    #cache_read
rdx_common_store
    rdlong  0-0, memp
    jmp     #__LMM_loop
    
    ''
    '' complex memory write instructions
    ''
    '' JMP iddddssss,#__LMM_WRxxxx
    '' ssss is the register containing the address to which to write
    '' dddd is the register containing the value to write to that address
    '' i=0 for no increment, i=1 to increment by the size of the data

	.global __LMM_WRBYTEX
__LMM_WRBYTEX
    movi    wrx_common_fetch_data, #WRBYTE_OPCODE
    movs    wrx_common_inc, #1
    jmp     #wrx_common

	.global __LMM_WRWORDX
__LMM_WRWORDX
    movi    wrx_common_fetch_data, #WRWORD_OPCODE
    movs    wrx_common_inc, #2
    jmp     #wrx_common

	.global __LMM_WRLONGX
__LMM_WRLONGX
    movi    wrx_common_fetch_data, #WRLONG_OPCODE
    movs    wrx_common_inc, #4

wrx_common
    muxnz   save_z_c, #2    'save the z flag
    shr	    L_ins0, #9
    movs    wrx_common_fetch_addr, L_ins0
    andn    wrx_common_fetch_addr, #0x1f0
    shr	    L_ins0, #4
    test    L_ins0, #0x10  wz
    and     L_ins0, #0xf    
    movd    wrx_common_inc, L_ins0
    movd    wrx_common_fetch_data, L_ins0
wrx_common_inc
 IF_NZ  add 0-0, #4
wrx_common_fetch_addr
    mov     t1, 0-0
    call    #cache_write
wrx_common_fetch_data
    wrlong  0-0, memp
    jmp     #__LMM_loop
    
    .endif 'COMPLEX_XMM_RDWR

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
                        
'get the hub address of a location in external memory to write
' z must be saved using the instruction "muxnz   save_z_c, #2" before calling
' z will be restored on return
cache_write             mov     memp, t1                    'save address for index
                        andn    t1, #CACHE_CMD_MASK         'ensure a write is not a read
                        or      t1, #CACHE_WRITE_CMD
                        jmp     #cache_access

'get the hub address of a location in external memory to read
' z must be saved using the instruction "muxnz   save_z_c, #2" before calling
' z will be restored on return
cache_read              mov     memp, t1                    'save address for index
                        mov     temp, t1                    'ptr + cache_mboxdat = hub address of byte to load
                        andn    temp, cache_linemask
                        cmp     cacheaddr, temp wz          'if cacheaddr == addr, just pull form cache
            if_e        jmp     #cache_hit                  'memp gets overwriteen on a miss
                        
cache_read_miss         or      t1, #CACHE_READ_CMD         'read must be 3 to avoid needing andn addr,#cache#CMD_MASK

cache_access            wrlong  t1, cache_mboxcmd
                        mov     cacheaddr, t1               'save new cache address. it's free time here
                        andn    cacheaddr, cache_linemask   'kill command bits in free time
_waitres                rdlong  temp, cache_mboxcmd wz
            if_nz       jmp     #_waitres
                        rdlong  cacheptr, cache_mboxdat     'Get new buffer
cache_hit               and     memp, cache_linemask
                        add     memp, cacheptr              'add ptr to memp to get data address
                        test    save_z_c, #2 wz             'restore the z flag
cache_read_ret
cache_write_ret         ret

t1                      long    0
save_z_c                long    0
cache_linemask          long    0
cache_mboxcmd           long    0
cache_mboxdat           long    0
temp                    long    0
memp                    long    0
cacheaddr               long    0
cacheptr                long    0

external_start          long    EXTERNAL_MEMORY_START       'start of external memory access window

	''
	'' code to load a buffer from hub or external memory into cog memory
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
	
	.global	__LMM_RET
	.global	__LMM_FCACHE_LOAD
__LMM_RET
	long 0
__LMM_FCACHE_LOAD
	call	#read_code	'' read count of bytes for load
	mov	__TMP0,L_ins0
	add	pc,#4
	mov	__TMP1,pc
	cmp	__LMM_FCACHE_ADDR,pc wz	'' is this the same fcache block we loaded last?
  	add	pc,__TMP0	'' skip over data
  IF_Z	jmp	#Lmm_fcache_doit

	
	mov	__LMM_FCACHE_ADDR, __TMP1
	
	'' copy TMP0 bytes from TMP1 to LMM_FCACHE_START
	'' copy TMP0 bytes from TMP1 to LMM_FCACHE_START
	mova	__COGA, #__LMM_FCACHE_START	'' mova because src is a cog address immediate
	call	#loadbuf

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
#ifdef HEXDEBUG
	res	20	'' 
#else
	res	64	'' reserve 64 longs = 256 bytes
#endif

	#include "kernel.ext"
	#include "cogdebug.ext"
