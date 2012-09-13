	''
	'' crtend for -mcog programs
	''

	.text
	.global __Exit
	.global __exit
__Exit
__exit
	cogid	r1
	cogstop r1

	'' provide (weak) default values for __clkfreqval and __clkmodeval
	'' if the user has provided these the defaults will not be
	'' used (that's what weak means)

	.weak __clkfreqval
	.weak __clkmodeval

	__clkfreqval = 80000000
	__clkmodeval = 0x6f
