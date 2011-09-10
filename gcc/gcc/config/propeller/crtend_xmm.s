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

	'' make sure the ctors and dtors are null terminated
	.section .ctors, "ax"
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
