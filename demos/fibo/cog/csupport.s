	.text
	.global r0
	.global r1
	.global r2
	.global r3
	.global r4
	.global r5
	.global r6
	.global r7
	.global r8
	.global r9
	.global r10
	.global r11
	.global r12
	.global r13
	.global r14
	.global lr
	.global sp

r0	mov	sp, PAR
r1	mov	r0, sp
r2	jmp	#_main
r3	long	0
r4	long	0
r5	long	0
r6	long	0
r7	long	0
r8	long	0
r9	long	0
r10	long	0
r11	long	0
r12	long	0
r13	long	0
r14	long	0
lr	long	0
sp	long	0


	.global __MASK_0000FFFF
	.global __MASK_FFFFFFFF

__MASK_0000FFFF	long	0x0000FFFF
__MASK_FFFFFFFF	long	0xFFFFFFFF

	.global ___main
___main	jmp	lr
