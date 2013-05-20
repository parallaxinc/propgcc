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
	lret

	.section .fini
	.global __Exit
	.global	__exit

	'' finish up the exit function
	mov	r0,r12		' restore exit status
	'' fall through
	.balign 4
__Exit
__exit
	'' provide for a special hook to do something like send
	'' a message to the host to indicate we have stopped
	lcall	#__ExitHook
	
	'' terminate by putting cog to sleep so QuickStart serial port stays quiet
	'' (pins & 0) == nonzero will always be false, so cog
	'' will sleep forever
	waitpeq __MASK_FFFFFFFF, #0

	.balign 4
	'' this is a weak reference so the user can override it
	.weak __ExitHook
__ExitHook
	lret

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

#ifdef __PROPELLER2__
	__clkfreqval = 60000000
	__clkmodeval = 0x6f
#else
	__clkfreqval = 80000000
	__clkmodeval = 0x6f
#endif
