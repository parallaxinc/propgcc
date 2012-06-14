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
r2	sub	r12,r13 wz
r3	IF_Z	jmp _start
__bss_clear
r4	wrbyte	r14,r13
r5	add	r13,#1
r6	djnz	r12,__bss_clear
r7	jmp	_start
r8	long	0
r9	long	0
r10	long	0
r11	long	0
r12	long	__bss_end
r13	long	__bss_start
r14	long	0		'' this must remain zero
lr	long	0
sp	long	0

_start
	jmpret	lr,#_main
	jmp	#__exit
