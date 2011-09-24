	''
	'' C start up code for LMM
	'' this code runs in LMM space
	'' so use LMM conventions
	''
	.section .init
	.global entry
	.global _exit
	
entry
	jmp	#__LMM_CALL
	.long	___init
	mov	r0,#0	' set argc
	jmp	#__LMM_MVI_r1
	.long	argv	' set argv
	jmp	#__LMM_CALL
	.long	_main	' call main

	'' and call _exit with whatever main returns
	jmp	#__LMM_CALL
	.long	__exit
	
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
	
	.data
argv
	long	0


	.section .fini
_exit
	jmp	#__LMM_MVI_r8
	.long	___DTOR_LIST__
L_loopn
	rdlong	__TMP0,r8 wz
	IF_Z add pc,#(L_loopnend - (.+4))
	jmp	#__LMM_CALL_INDIRECT
	add	r8,#4
	sub	pc,#((.+4) - L_loopn)
L_loopnend
	
