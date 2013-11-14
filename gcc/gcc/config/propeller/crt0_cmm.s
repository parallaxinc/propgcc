#ifdef DEBUG_KERNEL
#include "cogdebug.h"
#endif

#if defined(__PROPELLER2__)
#define RDBYTEC rdbytec
#else
#define RDBYTEC rdbyte
#endif
	.section .kernel, "ax"
	.compress off
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
r13	nop
r14	long	0	'' flag for first time run
	
r15	'' alias for link register lr
lr	long	__exit
sp	long	0
pc	long	entry		' default pc
#ifdef DEBUG_KERNEL
	global __ccr__
__ccr__
#endif
ccr	long	0		' condition codes
	
	'' the assembler actually relies on _MASK_FFFFFFFF being at register
	'' 19
	.global __MASK_FFFFFFFF
__MASK_FFFFFFFF	long	0xFFFFFFFF

#ifdef DEBUG_KERNEL
	'' gdb relies on register 20 being a breakpoint command
Breakpoint
	call	#__EnterLMMBreakpoint
#endif
	
	.global __TMP0
__TMP0	long	0
	.global __TMP1
__TMP1	long	0


	''
	'' main LMM loop -- read instructions from hub memory
	'' and executes them
	''
	'' the instructions are compressed, so we have to decompress them
itemp    long 0
ifield	long 0
dfield	long 0
xfield  long 0

	'' note, place sfield last so that if NATIVE code jumps here
	'' it falls through into __LMM_loop
sfield  long 0

__LMM_loop
#ifdef DEBUG_KERNEL
	muxc	ccr,#1
	muxnz	ccr,#2
	test	ccr, #COGFLAGS_STEP wz
  if_nz call	#__EnterDebugger
	shr	ccr,#1 wc,wz,nr		'' restore flags
#endif
	RDBYTEC	ifield,pc
	add	pc,#1
	mov	dfield,ifield
	shr	ifield,#4
	and	dfield,#15
#if defined(__PROPELLER2__)
	setspa	ifield
	popar	ifield
	jmp	ifield
	.data
	'' for jmptable
#else
	add	ifield,#(jmptab_base-r0)/4
	jmp	ifield
#endif
	
jmptab_base
	jmp	#macro	' instruction 0x
	jmp	#regreg	' instruction 1d
	jmp	#regimm4	' instruction 2d
	jmp	#regimm12	' instruction 3d
	jmp	#brw	' instruction 4d
	jmp	#mvi32	' instruction 5d
	jmp	#mvi16	' instruction 6d
	jmp	#brs	' instruction 7d
	jmp	#skip2	' instruction 8d
	jmp	#skip3	' instruction 9d
	jmp	#mvi8	' instruction Ad
	jmp	#mvi0	' instruction Bd
	jmp	#leasp	' instruction Cd
	jmp	#xmov_reg	' instruction Dd
	jmp	#xmov_imm	' instruction Ed
	jmp	#pack_native	' instruction Fd

#if defined(__PROPELLER2__)
	.section .kernel
#endif
macro
	add	dfield,#(macro_tab_base-r0)/4
	jmp	dfield

#ifdef DEBUG_KERNEL
__macro_brk
	call	#__EnterLMMBreakpoint
	'' fall through to macro_tab_base
#endif
macro_tab_base
	jmp	#__LMM_loop	' macro 0 -- NOP
#ifdef DEBUG_KERNEL
	jmp	#__macro_brk
#else
	jmp	#__LMM_loop	' macro 1 -- BREAK
#endif
	jmp	#__macro_ret	' macro 2 -- RET
	jmp	#__macro_pushm	' macro 3 -- PUSHM
	jmp	#__macro_popm	' macro 4 -- POPM
	jmp	#__macro_popret ' macro 5 -- POPM and return
	jmp	#__macro_lcall	' macro 6 -- LCALL
	jmp	#__macro_mul	' macro 7 -- multiply
	jmp	#__macro_udiv	' macro 8 -- unsigned divide
	jmp	#__macro_div	' macro 9 -- signed divide
	jmp	#__macro_mvreg	' macro A -- register register move
	jmp	#__macro_xmvreg	' macro B -- two register register moves
	jmp	#__macro_addsp	' macro C -- add a constant to SP
	jmp	#__macro_ljmp	' macro D -- long jump
	jmp	#__macro_fcache	' macro E -- FCACHE
	jmp	#__macro_native	' macro F -- NATIVE

	'' utility routine
	'' read a word into sfield
	'' trashes xfield
get_word
	RDBYTEC	sfield,pc
	add	pc,#1
	RDBYTEC	xfield,pc
	add	pc,#1
	shl	xfield,#8
	or	sfield,xfield
get_word_ret
	ret

	'' utility routine
	'' read a long into sfield
	'' trashes ifield,xfield
get_long
	call	#get_word
	mov	ifield, sfield
	call	#get_word
	shl	sfield, #16
	or	sfield,ifield
get_long_ret
	ret

	''
	'' read a signed byte into sfield
	''
get_sbyte
	call	#get_byte
	shl	sfield,#24
	sar	sfield,#24
get_sbyte_ret
	ret

	''
	'' read an unsigned byte
	''
get_byte
	RDBYTEC	sfield, pc
	add	pc, #1
get_byte_ret
	ret
	
__macro_native
	call	#get_long
	jmp	#sfield

	'' see later for __macro_fcache
	
__macro_ret
	mov	pc,lr
	jmp	#__LMM_loop

__macro_mul
	call	#__MULSI
	jmp	#__LMM_loop
__macro_udiv
	call	#__UDIVSI
	jmp	#__LMM_loop
__macro_div
	call	#__DIVSI
	jmp	#__LMM_loop

__fetch_TMP0
	RDBYTEC	__TMP0,pc
	add	pc,#1
__fetch_TMP0_ret
	ret
	
__macro_pushm
	call	#__fetch_TMP0
	call	#__LMM_PUSHM
	jmp	#__LMM_loop

__macro_popret
	call	#__fetch_TMP0
	call	#__LMM_POPRET
	jmp	#__LMM_loop

__macro_popm
	call	#__fetch_TMP0
	call	#__LMM_POPM
	jmp	#__LMM_loop

	''
	'' dual register-register move
	''
__macro_xmvreg
	call	#xmov

	'' fall through to mvreg
	
	''
	'' register register move
	'' second byte is (dest<<4) | src
	''
__macro_mvreg
	call	#xmov
	jmp	#__LMM_loop


	''
	'' add a signed 8 bit constant to sp
	''
__macro_addsp
	call	#get_sbyte
	add	sp,sfield
	jmp	#__LMM_loop

	''
	'' LMM support functions
	''


	'''
	''' move immediate of a 32 bit value
	'''
mvi32
	movd	mvi_set,dfield
	call	#get_long
mvi_set
	mov	0-0,sfield
	jmp	#__LMM_loop

	'''
	''' move immediate of a 16 bit value
	'''
mvi16
	movd	mvi_set,dfield
	call	#get_word
	jmp	#mvi_set

	'''
	''' move immediate of an 8 bit value
	'''
mvi8
	movd	mvi_set,dfield
	call	#get_byte
	jmp	#mvi_set

	'''
	''' zero a register
	'''
mvi0
	movd	mvi_set,dfield
	mov	sfield,#0
	jmp	#mvi_set


	'''
	''' leasp dst,#x
	''' sets dst = sp + x
	''' 
leasp
	movd	.doleasp1,dfield
	movd	.doleasp2,dfield
	call	#get_byte
.doleasp1
	mov	0-0,sp
.doleasp2
	add	0-0,sfield
	jmp	#__LMM_loop


	'''
	''' helper function:
	''' loads xfield and sfield with next byte
	''' with sfield having upper 4 bits
	''' and xfield having lower 4 bits
	'''
reg_helper
	call	#get_byte
	mov	xfield,sfield
	shr	sfield,#4
	and	xfield, #15
reg_helper_ret
	ret
	
	'''
	''' 16 bit compressed forms of instructions
	''' register-register operations encoded as
	'''    iiii dddd ssss xxxx
	'''
regreg
	call	#reg_helper
doreg
	add	xfield,#(xtable-r0)/4
	movs	.ins_rr,xfield
	movd	sfield,dfield
#if defined(__PROPELLER2__)
	nop
#endif
.ins_rr	or	sfield,0-0
	jmp	#sfield

	'''
	''' decode an embedded move instruction
	''' dddd ssss
xmov
	call	#reg_helper
	'' note reg-helper assumes sfield is upper 4 bits, xfield is lower
	'' 4, which is kind of the reverse of what xmov wants
	'' (the dest is in the upper, src in the lower)
	movs	.xmov,xfield
	movd	.xmov,sfield
	nop
#if defined(__PROPELLER2__)
	nop
#endif
.xmov	mov	0-0,0-0
xmov_ret
	ret
	
	'''
	''' like regreg, but has an additional move instruction embedded
	''' as the first byte after the opcode
	'''

xmov_reg
	call	#xmov
	jmp	#regreg

	''' similarly for an immediate 4
xmov_imm
	call	#xmov

	''' fall through to regimm4
	
	'''
	''' register plus 4 bit immediate
	'''
regimm4
	call	#reg_helper
	or	sfield,__IMM_BIT
	jmp	#doreg

__IMM_BIT	long (1<<22)

	'''
	''' register plus 9 bit immediate
	'''
	''' encoded as iiii_dddd ssss_ssss xxxx_ssss
	''' note that we actually make this a *signed* 12 bit
	''' immediate. Normal instructions only need an
	''' unsigned 9 bit, so making the whole field signed
	''' lets us do tricks like generating an xor r0,#-1
	''' instead of xor r0,__MASK_FFFFFFFF
	'''
regimm12
	RDBYTEC	itemp,pc		'' read low 8 bits
	add	pc,#1
	mov	.ins2,#(sfield-r0)/4	'' set the source to "sfield" register
	RDBYTEC	xfield,pc
	movd	.ins2,dfield
	mov	sfield,xfield
	and	sfield,#15		'' get the high 4 bits of sfield
	shl	sfield,#28
	sar	sfield,#20		'' sign extend
	shr	xfield,#4
	add	xfield,#(xtable-r0)/4
	movs	.ins_ri,xfield
	or	sfield,itemp
#if defined(__PROPELLER2__)
	nop
#endif
.ins_ri	or	.ins2,0-0
	add	pc,#1
#if defined(__PROPELLER2__)
	nop
#endif
.ins2
	nop
	jmp	#__LMM_loop
	
	''
	'' table of operations
	''
xtable
	add	0-0,0-0		'' extended op 0
	sub	0-0,0-0		'' extended op 1
	cmps	0-0,0-0 wz,wc	'' extended op 2
	cmp	0-0,0-0 wz,wc	'' extended op 3
	and	0-0,0-0		'' extended op 4
	andn	0-0,0-0		'' extended op 5
	neg	0-0,0-0		'' extended op 6
	or	0-0,0-0		'' extended op 7
	xor	0-0,0-0		'' extended op 8
	shl	0-0,0-0		'' extended op 9
	shr	0-0,0-0		'' extended op A
	sar	0-0,0-0		'' extended op B
	rdbyte	0-0,0-0		'' extended op C
	rdlong	0-0,0-0		'' extended op D
	wrbyte	0-0,0-0		'' extended op E
	wrlong	0-0,0-0		'' extended op F

	''
	'' conditional branches
	'' the dfield gives the condition
	'' for brw, the next 2 bytes give the address for the pc
	'' for brs, the next byte is a signed offset to add to the pc
	''
	''
cond_mask long (0xf<<18)
	
brw
	call	#get_word
	'' sign extend
	shl	sfield, #16
	sar	sfield, #16
do_relbranch
	add	sfield,pc
	andn	.brwins,cond_mask
	shl	dfield,#18	'' get dfield into the cond field
	or	.brwins,dfield
	nop
#if defined(__PROPELLER2__)
	nop
#endif
.brwins	mov	pc,sfield
	jmp	#__LMM_loop

brs
	call	#get_sbyte
	jmp	#do_relbranch

skip2
	mov	sfield,#2
	jmp	#do_relbranch
	

skip3
	mov	sfield,#3
	jmp	#do_relbranch

	''
	'' packed native instructions
	''
	'' native instructions are 32 bits like:
	''
	'' oooo_ooee eICC_CCdd dddd_ddds ssss_ssss
	''
	'' (o=opcode, e=effect, I=immediate, C=condition, d=dest, s=src)
	''
	'' if CCCC == 1111 (always execute), then we can efficiently store
	'' this as
	''
	'' CCCC_eeeI + 24 bits:(little endian) oooo_oodd dddd_ddds ssss_ssss
	'' and relocations will even work as long as they're offset by 1
	''
	'' on entry to this function eeeI is in dfield
	''
pack_native
	call	#get_word	'' set sfield to first 16 bits
	RDBYTEC	xfield,pc
	add	pc,#1
	mov	itemp,xfield   '' get the opcodes
	and	xfield,#3
	or	xfield,#0x3C   '' set condition bits
	shl	xfield,#16
	or	sfield,xfield
	andn	itemp,#3       '' just the 6 opcode bits, shifted left by 2
	shl	itemp,#24
	or	sfield,itemp
	shl	dfield,#22
	or	sfield,dfield
	jmp	#sfield

	''
	'' call functions
	''
	.global	__LMM_CALL
	.global __LMM_CALL_INDIRECT
__LMM_CALL
	call	#get_long
	jmp	#do_call
__macro_lcall
	call	#get_word
#if defined(__PROPELLER2__)
	'' in order to extend the address space, we require that
	'' subroutines be on longword boundaries
	shl	sfield,#2
#endif
do_call
	mov	lr,pc
do_jmp
	mov	pc,sfield
	jmp	#__LMM_loop

	''
	'' long unconditional jump to anywhere in the address space
	''
	.global __LMM_JMP
__LMM_JMP
__macro_ljmp
	call	#get_long
	jmp	#do_jmp

__LMM_CALL_INDIRECT
	mov	sfield,__TMP0
	jmp	#do_call


	''
	'' push and pop multiple macros
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
#if defined(__PROPELLER2__)
	nop
#endif
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

__MASK_0000FFFF	long	0x0000FFFF

	.global __MULSI
	.global __MULSI_ret
__MULSI
#if defined(__PROPELLER2__)
        setmula r0
        setmulb r1
        getmull r0
#else
	mov	__TMP0, r0
	min	__TMP0, r1
	max	r1, r0
	mov	r0, #0
__MULSI_loop
	shr	r1, #1	wz, wc
 IF_C	add	r0, __TMP0
	add	__TMP0, __TMP0
 IF_NZ	jmp	#__MULSI_loop
#endif
__MULSI_ret
	ret

	''
	'' code for atomic compare and swap
	''
	.global __C_LOCK_PTR
__C_LOCK_PTR
	long	__C_LOCK

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
	'' get the C_LOCK
	rdbyte	__TMP1,__C_LOCK_PTR
	mov	__TMP0,r0	'' save value to set
.swaplp
	lockset	__TMP1 wc
   IF_C jmp	#.swaplp

	rdlong	r0,r2		'' fetch original value
	cmp	r0,r1 wz	'' compare with desired original value
   IF_Z wrlong  __TMP0,r2	'' if match, save new value
	
	'' now release the C lock
	lockclr __TMP1
__CMPSWAPSI_ret
	ret
	
	''
	'' code to load a buffer from hub memory into cog memory
	''
	'' parameters: __TMP0 = count of bytes
	''             __TMP1 = hub address
	''             __COGA = COG address
	''
	''
__COGA	long 0
	
loadbuf
	movd	.ldlp,__COGA
	shr	__TMP0,#2	'' convert to longs
#if defined(__PROPELLER2__)
	nop
#endif
.ldlp
	rdlong	0-0,__TMP1
	add	__TMP1,#4
	add	.ldlp,inc_dest1
	djnz	__TMP0,#.ldlp

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
	
	.global	__LMM_RET
__LMM_RET
	long 0

__macro_fcache
	call	#get_word
	mov	__TMP0, sfield

__LMM_FCACHE_DO
	add	pc,#3		'' round up to next longword boundary
	andn	pc,#3
	mov	__TMP1,pc
	cmp	__LMM_FCACHE_ADDR,pc wz	'' is this the same fcache block we loaded last?
	add	pc,__TMP0	'' skip over data
  IF_Z	jmp	#Lmm_fcache_doit

	mov	__LMM_FCACHE_ADDR, __TMP1
	mova	__COGA, #__LMM_FCACHE_START
	call	#loadbuf
Lmm_fcache_doit
	jmpret	__LMM_RET,#__LMM_FCACHE_START
	jmp	#__LMM_loop


	''
	'' the fcache area 
	''
	.global __LMM_FCACHE_START
__LMM_FCACHE_START
	res	64	'' reserve 64 longs = 256 bytes

	''
	'' include various kernel extensions
	''
#include "kernel.ext"

#ifdef DEBUG_KERNEL
#include "cogdebug.ext"
#endif
