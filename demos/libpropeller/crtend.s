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
	.section .ctors
	long	0

	.section .dtors
	long	0
