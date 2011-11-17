	''
	'' crtend for -mcog programs
	''

	.text
	.global __Exit
	.global __exit
__exit
	cogid	r1
	cogstop r1
