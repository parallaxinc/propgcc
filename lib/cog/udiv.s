	.text
	.global __TMP0
	.global __UDIVSI
	.global __UDIVSI_ret
	.global __CLZSI
	.global __CLZSI_ret
	.global __CTZSI
__TMP0	long	0
__MASK_00FF00FF	long	0x00FF00FF
__MASK_0F0F0F0F	long	0x0F0F0F0F
__MASK_33333333	long	0x33333333
__MASK_55555555	long	0x55555555
__CLZSI	rev	r0, #0
__CTZSI	neg	__TMP0, r0
	and	__TMP0, r0	wz
	mov	r0, #0
 IF_Z	mov	r0, #1
	test	__TMP0, __MASK_0000FFFF	wz
 IF_Z	add	r0, #16
	test	__TMP0, __MASK_00FF00FF	wz
 IF_Z	add	r0, #8
	test	__TMP0, __MASK_0F0F0F0F	wz
 IF_Z	add	r0, #4
	test	__TMP0, __MASK_33333333	wz
 IF_Z	add	r0, #2
	test	__TMP0, __MASK_55555555	wz
 IF_Z	add	r0, #1
__CLZSI_ret	ret
__DIVR	long	0
__DIVCNT	long	0
__UDIVSI
	mov	__DIVR, r0
	call	#__CLZSI
	neg	__DIVCNT, r0
	mov	r0, r1
	call	#__CLZSI
	add	__DIVCNT, r0
	mov	r0, #0
	cmps	__DIVCNT, #0	wz, wc
 IF_C	jmp	#__UDIVSI_done
	shl	r1, __DIVCNT
	add	__DIVCNT, #1
__UDIVSI_loop
	cmpsub	__DIVR, r1	wz, wc
	addx	r0, r0
	shr	r1, #1
	djnz	__DIVCNT, #__UDIVSI_loop
__UDIVSI_done
	mov	r1, __DIVR
__UDIVSI_ret	ret
