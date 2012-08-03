	.section .lmmkernel, "ax"
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
r0	mov	sp, PAR
r1	rdlong  __TMP0, __C_LOCK_PTR  wz ' check for first time run
r2  IF_NE    jmp    #not_first_cog	' if not, skip some stuff
	
	'' initialization for first time run
r3      locknew	__TMP0 wc	' allocate a lock
r4	or	__TMP0, #256	' in case lock is 0, make the 32 bit value nonzero
r5      wrlong __TMP0, __C_LOCK_PTR	' save it to ram
r6      jmp    #__LMM_loop

not_first_cog
	'' initialization for non-primary cogs
r7      rdlong pc,sp		' if user stack, pop the pc
r8      add	sp,#4
r9      rdlong r0,sp		' pop the argument for the function
r10     add	sp,#4
r11     rdlong __TLS,sp	' and the _TLS variable
r12     add	sp,#4
r13	jmp	#__LMM_loop
r14	nop
	
r15	'' alias for link register lr
lr	long	__exit
sp	long	0
pc	long	entry		' default pc

	.global __TMP0
__TMP0	long	0

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
	rdbyte	ifield,pc
	add	pc,#1
	mov	dfield,ifield
	shr	ifield,#4
	and	dfield,#15
	add	ifield,#(jmptab_base-r0)/4
	jmp	ifield

	
jmptab_base
	jmp	#macro	' instruction 0x
	jmp	#regreg	' instruction 1d
	jmp	#macro	' instruction 2d
	jmp	#macro	' instruction 3d
	jmp	#macro	' instruction 4d
	jmp	#macro	' instruction 5d
	jmp	#macro	' instruction 6d
	jmp	#macro	' instruction 7d
	jmp	#macro	' instruction 8d
	jmp	#macro	' instruction 9d
	jmp	#macro	' instruction Ad
	jmp	#macro	' instruction Bd
	jmp	#macro	' instruction Cd
	jmp	#macro	' instruction Dd
	jmp	#macro	' instruction Ed
	jmp	#macro	' instruction Fd

macro
	add	dfield,#(macro_tab_base-r0)/4
	jmp	dfield

macro_tab_base
	jmp	#__LMM_loop	' macro 0 -- NOP
	jmp	#__LMM_loop	' macro 1 -- BREAK
	jmp	#__macro_ret	' macro 2 -- RET
	jmp	#__macro_pushm	' macro 3 -- PUSHM
	jmp	#__macro_popm	' macro 4 -- POPM
	jmp	#__LMM_loop	' macro 5 -- NOP
	jmp	#__LMM_loop	' macro 6 -- NOP
	jmp	#__LMM_loop	' macro 7 -- NOP
	jmp	#__LMM_loop	' macro 8 -- NOP
	jmp	#__LMM_loop	' macro 9 -- NOP
	jmp	#__LMM_loop	' macro A -- NOP
	jmp	#__LMM_loop	' macro B -- NOP
	jmp	#__LMM_loop	' macro C -- NOP
	jmp	#__LMM_loop	' macro D -- NOP
	jmp	#__macro_fcache	' macro E -- FCACHE
	jmp	#__macro_native	' macro F -- NATIVE

	'' utility routine
	'' read a long into sfield
	'' trashes ifield,xfield
get_long
	rdbyte	sfield,pc
	add	pc,#1
	rdbyte	xfield,pc
	add	pc,#1
	shl	xfield,#8
	rdbyte	ifield,pc
	add	pc,#1
	shl	ifield,#16
	rdbyte	itemp,pc
	add	pc,#1
	shl	itemp,#24
	or	sfield,itemp
	or	sfield,xfield
	or	sfield,ifield
get_long_ret
	ret
	
__macro_native
	call	#get_long
	jmp	#sfield

__macro_fcache
	rdbyte	__TMP0,pc
	add	pc,#1
	jmp	#__LMM_FCACHE_DO
	
__macro_ret
	mov	pc,lr
	jmp	#__LMM_loop

__macro_pushm
	rdbyte	__TMP0,pc
	add	pc,#1
	call	#__LMM_PUSHM
	jmp	#__LMM_loop

__macro_popm
	rdbyte	__TMP0,pc
	add	pc,#1
	call	#__LMM_POPM
	jmp	#__LMM_loop

	''
	'' LMM support functions
	''

	'' move immediate
	.macro LMM_movi reg
	.global __LMM_MVI_\reg
__LMM_MVI_\reg
	call	#get_long
	mov	\reg,sfield
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


	'''
	''' 16 bit compressed forms of instructions
	''' register-register operations encoded as
	'''    iiii dddd ssss xxxx
	'''
regreg
	rdbyte	xfield,pc
	mov	sfield,xfield
	shr	sfield,#4
doreg
	add	pc,#1
	and	xfield,#15
	add	xfield,#(xtable-r0)/4
	movd	.ins_rr,xfield
	movd	sfield,dfield
.ins_rr	or	sfield,0-0
	jmp	#sfield

	'''
	''' register plus 4 bit immediate
	'''
regimm4
	rdbyte	xfield,pc
	mov	sfield,xfield
	shr	sfield,#4
	or	sfield,__IMM_BIT
	jmp	#doreg

__IMM_BIT	long 0b000000_0001_0000_000000000_000000000

	'''
	''' register plus 12 bit immediate
	'''
	''' encoded as iiii dddd ssss ssss ssss xxxx
regimm12
	rdbyte	itemp,pc
	add	pc,#1
	mov	.ins2,dfield
	rdbyte	xfield,pc
	movs	.ins2,#sfield
	mov	sfield,xfield
	shr	sfield,#4
	shl	sfield,#8
	or	sfield,itemp
	and	xfield,#15
	add	xfield,#(xtable-r0)/4
	movd	.ins_ri,xfield
.ins_ri	or	.ins2,0-0
	add	pc,#1

.ins2
	nop
	jmp	#__LMM_loop
	
	''
	'' table of operations
	''
xtable
	mov	0-0,0-0		'' extended op 0
	add	0-0,0-0		'' extended op 1
	sub	0-0,0-0		'' extended op 2
	cmps	0-0,0-0 wz,wc	'' extended op 3
	and	0-0,0-0		'' extended op 4
	andn	0-0,0-0		'' extended op 5
	neg	0-0,0-0		'' extended op 6
	or	0-0,0-0		'' extended op 7
	xor	0-0,0-0 wz,wc	'' extended op 8
	shl	0-0,0-0		'' extended op 9
	shr	0-0,0-0		'' extended op A
	sar	0-0,0-0		'' extended op B
	rdbyte	0-0,0-0		'' extended op C
	rdlong	0-0,0-0		'' extended op D
	wrbyte	0-0,0-0 wz,wc	'' extended op E
	wrlong	0-0,0-0		'' extended op F
	
	''
	'' call functions
	''
	.global	__LMM_CALL
	.global __LMM_CALL_INDIRECT
__LMM_CALL
	call	get_long
	mov	__TMP0,sfield
__LMM_CALL_INDIRECT
	mov	lr,pc
	mov	pc,__TMP0
	jmp	#__LMM_loop

	''
	'' direct jmp
	''
	.global __LMM_JMP
__LMM_JMP
	call	get_long
	mov	pc,sfield
	jmp	#__LMM_loop

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
	.global __DIVSI
	.global __DIVSI_ret
	.global __UDIVSI
	.global __UDIVSI_ret
	.global __CLZSI
	.global __CLZSI_ret
	.global __CTZSI
__MASK_00FF00FF	long	0x00FF00FF
__MASK_0F0F0F0F	long	0x0F0F0F0F
__MASK_33333333	long	0x33333333
__MASK_55555555	long	0x55555555
__CLZSI	rev	r0, #0
__CTZSI	neg	__TMP0, r0
	and	__TMP0, r0	wz
	mov	r0, #0
 IF_Z	mov	r0, #1
	test	__TMP0, __MASK_0000FFFF	wz
 IF_Z	add	r0, #16
	test	__TMP0, __MASK_00FF00FF	wz
 IF_Z	add	r0, #8
	test	__TMP0, __MASK_0F0F0F0F	wz
 IF_Z	add	r0, #4
	test	__TMP0, __MASK_33333333	wz
 IF_Z	add	r0, #2
	test	__TMP0, __MASK_55555555	wz
 IF_Z	add	r0, #1
__CLZSI_ret	ret
__DIVR	long	0
__TMP1
__DIVCNT
	long	0
	''
	'' calculate r0 = orig_r0/orig_r1, r1 = orig_r0 % orig_r1
	''
__UDIVSI
	mov	__DIVR, r0
	call	#__CLZSI
	neg	__DIVCNT, r0
	mov	r0, r1 wz
 IF_Z   jmp	#__UDIV_BY_ZERO
	call	#__CLZSI
	add	__DIVCNT, r0
	mov	r0, #0
	cmps	__DIVCNT, #0	wz, wc
 IF_C	jmp	#__UDIVSI_done
	shl	r1, __DIVCNT
	add	__DIVCNT, #1
__UDIVSI_loop
	cmpsub	__DIVR, r1	wz, wc
	addx	r0, r0
	shr	r1, #1
	djnz	__DIVCNT, #__UDIVSI_loop
__UDIVSI_done
	mov	r1, __DIVR
__UDIVSI_ret	ret
__DIVSGN	long	0
__DIVSI	mov	__DIVSGN, r0
	xor	__DIVSGN, r1
	abs	r0, r0 wc
	muxc	__DIVSGN, #1	' save original sign of r0
	abs	r1, r1
	call	#__UDIVSI
	cmps	__DIVSGN, #0	wz, wc
 IF_B	neg	r0, r0
	test	__DIVSGN, #1 wz	' check original sign of r0
 IF_NZ	neg	r1, r1		' make the modulus result match
__DIVSI_ret	ret

	'' come here on divide by zero
	'' we probably should raise a signal
__UDIV_BY_ZERO
	neg	r0,#1
	mov	r1,#0
	jmp	#__UDIVSI_ret
	
	.global __MULSI
	.global __MULSI_ret
__MULSI
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
	'' FCACHE region
	'' The FCACHE is an area where we can
	'' execute small functions or loops entirely
	'' in Cog memory, providing a significant
	'' speedup.
	''

__LMM_FCACHE_ADDR
	long 0
inc_dest4
	long (4<<9)
	
	.global	__LMM_RET
	.global	__LMM_FCACHE_LOAD
__LMM_RET
	long 0
__LMM_FCACHE_LOAD
	call	get_long
	mov	__TMP0,sfield	'' read count of bytes for load
__LMM_FCACHE_DO
	add	pc,#3		'' round up to next longword boundary
	andn	pc,#3
	mov	__TMP1,pc
	cmp	__LMM_FCACHE_ADDR,pc wz	'' is this the same fcache block we loaded last?
	add	pc,__TMP0	'' skip over data
  IF_Z	jmp	#Lmm_fcache_doit

	mov	__LMM_FCACHE_ADDR, __TMP1
	
	'' assembler awkwardness here
	'' we would like to just write
	'' movd	Lmm_fcache_loop,#__LMM_FCACHE_START
	'' but binutils doesn't work right with this now
	movd Lmm_fcache_loop,#(__LMM_FCACHE_START-__LMM_entry)/4
	movd Lmm_fcache_loop2,#1+(__LMM_FCACHE_START-__LMM_entry)/4
	movd Lmm_fcache_loop3,#2+(__LMM_FCACHE_START-__LMM_entry)/4
	movd Lmm_fcache_loop4,#3+(__LMM_FCACHE_START-__LMM_entry)/4
	add  __TMP0,#15		'' round up to next multiple of 16
	shr  __TMP0,#4		'' we process 16 bytes per loop iteration
Lmm_fcache_loop
	rdlong	0-0,__TMP1
	add	__TMP1,#4
	add	Lmm_fcache_loop,inc_dest4
Lmm_fcache_loop2
	rdlong	0-0,__TMP1
	add	__TMP1,#4
	add	Lmm_fcache_loop2,inc_dest4
Lmm_fcache_loop3
	rdlong	0-0,__TMP1
	add	__TMP1,#4
	add	Lmm_fcache_loop3,inc_dest4
Lmm_fcache_loop4
	rdlong	0-0,__TMP1
	add	__TMP1,#4
	add	Lmm_fcache_loop4,inc_dest4

	djnz	__TMP0,#Lmm_fcache_loop

Lmm_fcache_doit
	jmpret	__LMM_RET,#__LMM_FCACHE_START
	jmp	#__LMM_loop


	''
	'' the fcache area should come last in the file
	''
	.global __LMM_FCACHE_START
__LMM_FCACHE_START
	res	128	'' reserve 128 longs = 512 bytes


