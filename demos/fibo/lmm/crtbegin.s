	''
	'' C start up code
	'' this code runs in LMM space
	'' so use LMM conventions
	''
	.text
	.global entry
	.global __exit
	
entry
	mov	r0,#0	' set argc
	jmp	#__LMM_MVI_r0
	.long	argv	' set argv
	jmp	#__LMM_CALL
	.long	_main	' call main

	'' fall througn to _exit
__exit
	cogid	r0
	cogstop	r0

	.global ___main
___main
	mov	pc,lr

	.data
argv
	long	0
