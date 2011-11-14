	''
	'' end of code stuff
	''
	.section .init
	'' finish up the ___init function
	mov	sp,r14
	rdlong	r14,sp
	add	sp,#4
	rdlong	lr,sp
	add	sp,#4
	mov	pc,lr

	.section .fini
	.global __Exit
	.global	__exit
__Exit
__exit
	'' finish up the exit function
	mov	r0,r12		' restore exit status

	'' provide for a special hook to do something like send
	'' a message to the host to indicate we have stopped
	jmp	#__LMM_CALL
	long	__ExitHook
	
	'' terminate by putting cog to sleep so QuickStart serial port stays quiet
	'' (pins & 0) == nonzero will always be false, so cog
	'' will sleep forever
	'' the waitpeq instruction itself is nonzero, so it will
	'' work nicely
	'' waitpeq . , #0 ' this might work in pure ASM, but i get a linker error here.
	waitpeq __MASK_FFFFFFFF, #0

	'' this is a weak reference so the user can override it
	.weak __ExitHook
__ExitHook
	mov	pc,lr

	'' make sure the ctors and dtors are null terminated
	''
	'' Take the opportunity to provide weak aliases for argv
	'' and environ. The user can override these, but if she
	'' doesn't then they will just be set to arrays with a
	'' single NULL element
	''
	.weak	__argv
	.weak	__environ

	.section .ctors, "ax"
__argv
__environ
	long	0

	.section .dtors, "ax"
	long	0

	'' provide (weak) default values for __clkfreqval and __clkmodeval
	'' if the user has provided these the defaults will not be
	'' used (that's what weak means)

	.weak __clkfreqval
	.weak __clkmodeval

	__clkfreqval = 80000000
	__clkmodeval = 0x6f

