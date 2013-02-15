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
	
	'' check for first time run
r2	rdlong	r1, __C_LOCK_PTR wz
r3  IF_NE jmp	#_start
	'' allocate a lock, and clear the bss
r4	locknew	r1
r5	or	r1,#256
r6	wrlong	r1, __C_LOCK_PTR
r7	sub	lr,r13 wz
r8  IF_Z  jmp #_start

__bss_clear
r9	wrbyte	r14,r13
r10	add	r13,#1
r11	djnz	lr,#__bss_clear
r12	jmp	#_start

r13	long	__bss_start
r14	long	0		'' this must remain zero
lr	long	__bss_end
sp	long	0

__C_LOCK_PTR
	long	__C_LOCK
_start
	jmpret	lr,#_main
	jmp	#__exit
