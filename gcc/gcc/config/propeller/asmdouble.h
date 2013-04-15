/*
        Double -- double precision floating point routines for
	the Propeller

        Copyright (c) 2012 Total Spectrum Software Inc.

        Released under the MIT License (see the end of lib1funcs.asm for details)      
*/
#define  FLAG_SIGN       0x01
#define  FLAG_ZERO       0x02
#define  FLAG_INF	 0x04
#define  FLAG_NAN        0x08
#define  FLAG_STICKY	 0x10

#define  FBIAS_EXP	 127
#define  FMAX_EXP	 255

	 // r0-r3 are parameters
	 // r2 and r3 may be re-used after the unpacking is done

		.equ	 tmp0, __TMP0
		.equ	 tmp1, __TMP1

		// make sure these registers agree with
		// the ones in the float code in lib1funcs.asm
		.equ	 B, r2
		.equ	 Bflag, r3
		.equ	 expB, r4

		.equ	 A, r5
		.equ	 Aflag, r6
		.equ	 expA, r7

		.equ	 Blo, r12
		.equ	 Alo, r13

		.equ	 count, r14
		.equ	 Btmp, count

	 	'' save the caller-saved registers (r12-r15)
	 	.macro SAVEREGS
		lpushm  #12+(4<<4)
		.endm

		.macro RESTOREREGS
		lpopm   #15+(4<<4)
		.endm

		.macro ENTER
		lpushm #15+(1<<4)
		.endm

		.macro LEAVE
		lpopm #15+(1<<4)
		.endm

		.macro LDJNZ reg, label
		sub    \reg,#1 wz
	if_nz	brw    \label
		.endm

		.macro LMMRET
		lret
		.endm



