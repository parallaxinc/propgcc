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
	.global _exit
	
entry
	jmp	#__LMM_CALL
	.long	___init
	jmp	#__LMM_MVI_r1
	.long	__argv	' set argv
	'' calculate argc by counting non-zero elements of argv
	mov	r3,r1
	mov	r0,#0
argc_cnt
	LOAD	r2,r3
	add	r3,#4
	cmp	r2,#0 wz
  IF_NZ add	r0,#1
  IF_NZ sub	pc,#(.+4)-argc_cnt
	
	jmp	#__LMM_MVI_r2
	.long	__environ	' set environment pointer

	jmp	#__LMM_CALL
	.long	_main	' call main

	'' and call _exit with whatever main returns
	jmp	#__LMM_CALL
	.long	_exit
	
	''
	'' initialization function, responsible for calling all ctors
	'' we can be careless here about saving registers, because
	'' our only caller is the entry point
	'' we do need to save the link register, though, since
	'' we are making subroutine calls
	''
	.global ___init
___init
	sub	sp,#4
	wrlong	lr,sp
	sub	sp,#4
	wrlong	r14,sp
	mov	r14,sp

	'' zero out bss
	jmp	#__LMM_MVI_r0
	.long	__bss_start
	jmp	#__LMM_MVI_r2
	.long	__bss_end
	sub	r2,r0
	mov	r1,#0
	jmp	#__LMM_CALL
	.long	_memset

	'' call the constructors
	jmp	#__LMM_MVI_r8
	.long	___CTOR_LIST__
L_loop
	rdlong	__TMP0,r8 wz
	IF_Z add pc,#(L_loopend - (.+4))
	jmp	#__LMM_CALL_INDIRECT
	add	r8,#4
	sub	pc,#((.+4) - L_loop)
L_loopend
	'' note -- the return is in crtend.s ; that is so
	'' per-function initializers can be added here

	.section .fini
_exit
	mov	r12,r0		' save exit status
	jmp	#__LMM_MVI_r8
	.long	___DTOR_LIST__
L_loopn
	rdlong	__TMP0,r8 wz
	IF_Z add pc,#(L_loopnend - (.+4))
	jmp	#__LMM_CALL_INDIRECT
	add	r8,#4
	sub	pc,#((.+4) - L_loopn)
L_loopnend
	
