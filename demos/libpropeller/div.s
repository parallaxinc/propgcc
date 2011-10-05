	.text
	.global __DIVSI
	.global __DIVSI_ret
__DIVSGN	long	0
__DIVSI	mov	__DIVSGN, r0
	xor	__DIVSGN, r1
	abs	r0, r0 wc
	muxc	__DIVSGN,#1
	abs	r1, r1
	call	#__UDIVSI
	cmps	__DIVSGN, #0	wz, wc
  IF_B	neg	r0, r0
	test	__DIVSGN, #1 wz
  IF_NZ	neg	r1, r1
__DIVSI_ret	ret
