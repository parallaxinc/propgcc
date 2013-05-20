	''
	'' C start up code for LMM
	'' this code runs in LMM space
	'' so use LMM conventions
	''
#if defined(__PROPELLER_XMM__)

# if 0
	'' original XMM CODE
	.macro STORE reg,mem
	mov	__TMP0,\mem
	mov	__TMP1,\reg
	call	#__LMM_WRLONG
	.endm
	.macro LOAD reg,mem
	mov	__TMP0,\mem
	call	#__LMM_RDLONG
	mov	\reg,__TMP1
	.endm
# else
	'' new XMM CODE using INTERMEDIATE mode loads
	'' __TMP0 == ddddssss where dddd is the data reg,
	''                          ssss holds the address
	.macro STORE reg,mem
	xmmio	wrlong,\reg,\mem
	.endm
	'' __TMP0 == ddddssss where dddd is the output reg,
	''                          ssss holds the address
	.macro LOAD reg,mem
	xmmio	rdlong,\reg,\mem
	.endm
# endif
#else
	.macro STORE reg,mem
	wrlong	\reg,\mem
	.endm
	.macro LOAD reg,mem
	rdlong	\reg,\mem
	.endm
#endif

	.section .init
	.global entry
	.global start
	.global _exit
	
entry
start
	lcall	#___init

	mvi	r1,#__argv  ' set argv
	'' calculate argc by counting non-zero elements of argv
	mov	r3,r1
	mov	r0,#0
argc_cnt
	LOAD	r2,r3
	add	r3,#4
	cmp	r2,#0 wz
  IF_NZ add	r0,#1
  IF_NZ brs	#argc_cnt
	
	mvi	r2,#__environ   ' set environment pointer

	mov	r14,#0		' set frame pointer to 0
	
	lcall	#_main	' call main

	'' and call _exit with whatever main returns
	lcall	#_exit
	
	''
	'' initialization function, responsible for calling all ctors
	'' we can be careless here about saving registers, because
	'' our only caller is the entry point
	'' we do need to save the link register, though, since
	'' we are making subroutine calls
	''
	.global ___init
	.balign 4
___init
	sub	sp,#4
	wrlong	lr,sp
	sub	sp,#4
	wrlong	r14,sp
	mov	r14,sp

	'' zero out bss
	mvi	r0,#__bss_start
	mvi	r2,#__bss_end
	sub	r2,r0
	mov	r1,#0
	lcall	#_memset

	'' call the constructors
	mvi	r8,#___CTOR_LIST__
L_loop
	LOAD	r0, r8
	cmps	r0, #0 wz,wc
	IF_Z 	brs #L_loopend
	mov	__TMP0, r0
	jmp	#__LMM_CALL_INDIRECT
	add	r8,#4
	brs	#L_loop
L_loopend
	'' note -- the return is in crtend.s ; that is so
	'' per-function initializers can be added here

	.section .fini
	.balign 4
_exit
	mov	r12,r0		' save exit status
	mvi	r8,#___DTOR_LIST__
L_loopn
	LOAD	r0,r8
	cmps	r0, #0 wz,wc
	IF_Z	brs #L_loopnend
	mov	__TMP0,r0
	jmp	#__LMM_CALL_INDIRECT
	add	r8,#4
	brs	#L_loopn
L_loopnend
	
