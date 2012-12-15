	.section .boot, "ax"
__bootrom
	.space 0x0e80
__vectors
	'.space 0x1000 - 0x0e80
