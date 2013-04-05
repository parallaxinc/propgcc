''        F32 - Concise floating point code for the Propeller
''        Copyright (c) 2011 Jonathan "lonesock" Dummer
''
''        Released under the MIT License (see the end of this file for details)
''        Modified by Eric Smith for GAS and LMM, and to fix rounding and
''        handling of infinity.
''
''    
''        +--------------------------------------------------------------------------+
''        | IEEE 754 compliant 32-bit floating point math routines for the Propeller |
''        | Based on Float32 & Float32Full v1.5, by Cam Thompson                     |
''        | Modified by Jonathan (lonesock) to try to fit all functionality into     |
''        |              a single cog, and speed-up the routines where possible.     |
''        +--------------------------------------------------------------------------+

			'' global definitions for everything in this file
#define SignFlag 0x01
#define ZeroFlag 0x02
#define InfFlag  0x04
#define NaNFlag  0x08
#define StickyBit 0x10

	
			.equ	manA, r2
			.equ	flagA, r3
			.equ	expA, r4
			.equ	manB, r5
			.equ	flagB, r6
			.equ	expB, r7

#ifdef L_loadfloat
			.section .float.kerext, "ax"
startfloat
			.long	endfloat - startfloat

			.compress off
'------------------------------------------------------------------------------
' input:   manA         32-bit floating point value 
' output:  flagA        flag bits (Nan, Infinity, Zero, Sign)
'          expA         exponent (no bias)
'          manA         mantissa (aligned to bit 28)
'          C flag       set if value is NaN or infinity
'          Z flag       set if value is zero
' changes: flagA, expA, manA
'------------------------------------------------------------------------------
			.global __FUnpack
			.global __FUnpack_ret
__FUnpack
			mov     flagA, manA            ' get sign
                        shr     flagA, #31
                        mov     expA, manA             ' get exponent
                        and     manA, Mask23
        		shl     manA, #5                ' justify mantissa to bit 28
        		shl     expA, #1
                        shr     expA, #24 wz
          if_z          jmp     #.zeroSubnormal         ' check for zero or subnormal
                        cmp     expA, #255 wz           ' check if finite
          if_nz         jmp     #.finite
	  		'' infinite or NaN
			cmp	manA, #0 wz
          if_nz         or      flagA, #NaNFlag
          if_z          or      flagA, #InfFlag wz
                        jmp     #__FUnpack_ret

.zeroSubnormal          or      manA, expA wz,nr        ' check for zero
          if_nz         jmp     #.subnorm
                        or      flagA, #ZeroFlag        ' yes, then set zero flag
                        jmp     #__FUnpack_ret
                                 
.subnorm
			shl	manA, #1		' adjust for subnormal
.subnorm2               test    manA, Bit28 wz
          if_z          shl     manA, #1
          if_z          djnz    expA, #.subnorm2


.finite
                        or      manA, Bit28             ' add leading one bit
                        
.unpack_exit1           sub     expA, #127              ' remove bias from exponent
__FUnpack_ret           ret       


'------------------------------------------------------------------------------
' input:   flagA        fnumA flag bits (Nan, Infinity, Zero, Sign)
'          expA         fnumA exponent (no bias)
'          manA         fnumA mantissa (aligned to bit 29)
' output:  r0           32-bit floating point value
' changes: r0, flagA, expA, manA 
'------------------------------------------------------------------------------
big_4_28		long 	(2<<28) - 1

			.global __FPack
			.global __FPack_ret
__FPack
			cmp     manA, #0 wz             ' check for zero                     
          if_z          jmp     #.pack_exit1

			'' re-normalize: if bigger than two, shift down
.nmbiglp
			cmp	big_4_28, manA wc
	  if_nc		jmp	#.normlp
	  		add	expA, #1
	  		shr	manA, #1 wc
          if_c          or      flagA, #StickyBit
			jmp	#.nmbiglp
.normlp
			test	manA, Bit28 wz
	  if_z		shl	manA, #1
	  if_z		djnz	expA, #.normlp
	  if_z		jmp	#.normlp
	
			'' check for denormalized numbers
			add	expA, #127
			cmps	expA, #0 wz, wc
	  if_a		jmp	#.normal
			'' denormalize
			abs	expA, expA
			add	expA, #1
			max	expA, #28
.shiftdenorm
			shr	manA, #1 wc
	  if_c		or	flagA, #StickyBit
			djnz	expA, #.shiftdenorm
.normal
			andn	manA, Bit28
			'' perform rounding
			'' we are converting from 4.28 to 9.23,
			'' so one unit in the last place is
			'' 0x20
			test    flagA, #StickyBit wc
			test	manA, #0x20 wz
	   if_nz_or_c	or	manA, #1
			add	manA, #0x0f
			shr	manA, #5
#ifdef __PROPELLER2__
			'' max works differently in P2!
			cmp	expA, #255 wc
			max	expA, #255
#else
			max	expA, #255 wc
#endif
	   if_nc	mov	manA, #0
			shl	expA, #23
			add	manA, expA
.pack_exit1	
			mov	r0, flagA
			shl	r0, #31
			or	r0, manA

__FPack_ret             ret


'-------------------- constant values -----------------------------------------

Mask23                  long    $007FFFFF


endfloat

			.section .kernel
Bit28                   long    $10000000

			.global	__Bit31
__Bit31                 long    $80000000
	
			.compress default
			.text

''
'' make sure everything needed for floats is loaded
'' trashes: r6, TMP1
''
			.global __loadfloat
__loadfloat
			mvi	r6, #__load_start_float_kerext
			fcache	#(.FCfloatend - .FCfloatstart)
.FCfloatstart
			.compress off
			mov	__TMP1, r6
			call	#__load_extension
			mov	pc,lr

			jmp	__LMM_RET

'------------------------------------------------------------------------------
' input:   r0           32-bit floating point value
'          r1           32-bit floating point value 
' output:  flagA        fnumA flag bits (Nan, Infinity, Zero, Sign)
'          expA         fnumA exponent (no bias)
'          manA         fnumA mantissa (aligned to bit 28)
'          flagB        fnumB flag bits (Nan, Infinity, Zero, Sign)
'          expB         fnumB exponent (no bias)
'          manB         fnumB mantissa (aligned to bit 29)
' changes: fnumA, flagA, expA, manA, fnumB, flagB, expB, manB, __TMP0
'------------------------------------------------------------------------------
			.global __FUnpack2
			.global __FUnpack2_ret
			.equ	__FUnpack2, __LMM_FCACHE_START + (.FCunpack2 - .FCfloatstart)
			.equ	__FUnpack2_ret, __LMM_FCACHE_START + (.FCunpack2_ret - .FCfloatstart)
	
.FCunpack2
        		mov     manA, r1            ' unpack B to A
                        call    #__FUnpack

                        mov     flagB, flagA
                        mov     expB, expA
                        mov     manB, manA

                        mov     manA, r0               ' unpack A
                        call    #__FUnpack

.FCunpack2_ret          ret



.FCadd
			'' we can assume magnitude of A is > magnitude of B

                        mov     __TMP0, expA                ' align mantissas
                        sub     __TMP0, expB wz
                        max     __TMP0, #31
			mov	__TMP1, #32
			sub	__TMP1, __TMP0
              if_nz     shl	manB, __TMP1 wz,nr
              if_nz     or      flagA, #StickyBit       ' set sticky bit
                        shr     manB, __TMP0

			xor	flagB,flagA
			test	flagB, #SignFlag wz
	                test	flagA, #StickyBit wc
              if_nz     neg     manB, manB
              if_nz_and_c sub   manB, #1		' adjust for sticky bit

	                add     manA, manB wz

	      if_z_and_nc      andn    flagA, #SignFlag

                        call    #__FPack                  ' pack result and exit
			jmp	__LMM_RET

.FCmul
                        xor     flagA, flagB            ' get sign of result
                        add     expA, expB              ' add exponents

                        ' multiply manA by manB
			' produces the high word in __TMP0, low word in manA
	
                        mov     __TMP0, #0                  ' __TMP0 is my accumulator
                        mov     __TMP1, #32                 ' loop counter for multiply

			shl	manB, #3 wc		' pre-adjust and clear carry
	
.multiply   if_c	add	__TMP0, manB wc
			rcr	__TMP0, #1 wc
			rcr	manA, #1 wc
	      		djnz	__TMP1, #__LMM_FCACHE_START+(.multiply-.FCfloatstart)

			'' check for rounding
			cmp	manA, #0 wz
	       if_nz    or	flagA, #StickyBit
                        mov     manA, __TMP0                ' yes, that's my final answer.

                        call    #__FPack
			jmp	__LMM_RET

.FCdiv
                        xor     flagA, flagB            ' get sign of result
                        sub     expA, expB              ' subtract exponents

                        ' slightly faster division, using 26 passes instead of 30
                        mov     __TMP0, #0                  ' clear quotient
                        mov     __TMP1, #26                 ' loop counter for divide (need 24, plus 2 for rounding)
.divide
                        cmpsub  manA, manB      wc
                        rcl     __TMP0, #1
                        shl     manA, #1 wz
                        djnz    __TMP1, #__LMM_FCACHE_START + (.divide - .FCfloatstart)
                        shl     __TMP0, #3                  ' align the result (we did 26 instead of 29 iterations)
			'' check for remainders
           if_nz        or	__TMP0, #1
                        mov     manA, __TMP0                ' get result and exit
                        call    #__FPack
			jmp	__LMM_RET


.FCfloatend

	
	
			.compress default

__return_signed_zero
			mov	r0, flagA
			xor	r0, flagB
			shl	r0, #31			'' return signed 0
			mov	pc,lr

__return_nan
			'' return NaN
			mvi	r0, #0x7fc00000
			mov	pc, lr

__return_signed_infinity
			mov    r0, flagA
			xor    r0, flagB		'' get correct sign
			shl    r0, #31
			mvi	r2, #0x7f800000
			or	r0, r2
			mov	pc,lr

'----------------------------
' addition and subtraction
' r0 = r0 +- r1
'----------------------------
			.global ___subsf3
			.global ___addsf3
___subsf3
                        xor     r1, __Bit31            ' negate B

___addsf3
			mov	r7,lr
			lcall	#__loadfloat
			mov	lr,r7
			'' swap so |r0|>|r1|
			mov	manA,r0
			mov	manB,r1
			shl	manA,#1
			shl	manB,#1
			cmp	manA,manB wz,wc
	  if_ae         brs	#.addnoswap
			mov	manA,r0
			mov	r0,r1
			mov	r1,manA
.addnoswap
	               	call    #__FUnpack2            ' unpack two variables

			mov	__TMP0, flagA
			or	__TMP0, flagB
			test	__TMP0, #(ZeroFlag|InfFlag|NaNFlag) wz

          if_nz         brs     #__add_excep         ' check for NaN or 0

	  		'' handle the normal case quickly
			jmpret	  __LMM_RET, #__LMM_FCACHE_START + (.FCadd - .FCfloatstart)
			mov	pc,lr			' return

__add_excep
			test	__TMP0, #NaNFlag wz
		if_nz	brs	#__return_nan
			test	flagA, #InfFlag wz
		if_nz	brs	#.addinfA
			test	flagA, #ZeroFlag wz
		if_nz	brs	#.addzeroA
			'' OK, A (and hence r0) is a regular number here
			test   flagB, #ZeroFlag wz
		if_nz	mov    pc,lr  		'' A + 0 == A
			'' otherwise, B must be an infinity
			mov    r0,r1
			mov    pc,lr
.addinfA
			test	flagB, #InfFlag wz
		if_z	mov	pc, lr 		'' inf + B == inf
			'' inf + inf == NaN if signs differ
			xor    flagA, flagB
			test   flagA, #SignFlag wz
		if_nz	brw    #__return_nan
			mov    pc,lr
.addzeroA
			test	flagB, #ZeroFlag wz
		if_z	mov	r0,r1
		if_z	mov	pc,lr

			'' 0 + 0; handle case where both are 0
			mov     r0, flagA
			and     r0, flagB
			shl	r0, #31
			mov	pc, lr

'----------------------------
' multiplication
' fnumA *= fnumB
'----------------------------
			.global	___mulsf3
___mulsf3
			mov	r7,lr
			lcall	#__loadfloat
			mov	lr,r7

	                call    #__FUnpack2               ' unpack two variables
			mov	__TMP0, flagA
			or	__TMP0, flagB
			test	__TMP0, #(ZeroFlag|InfFlag|NaNFlag) wz

              if_nz     brs     #__mul_excep              ' check for NaN

	  		'' handle the normal case quickly
			jmpret	  __LMM_RET, #__LMM_FCACHE_START + (.FCmul - .FCfloatstart)

_FMul_ret               mov	pc,lr

			''
			'' special cases for multiply
			''
__mul_excep
			test	__TMP0, #NaNFlag wz
		if_nz	brw	#__return_nan
			test	flagA, #ZeroFlag wz
		if_nz	brw	#.mulzeroA
			test	flagA, #InfFlag wz
		if_nz	brw	#.mulinfA
			'' ok, A is a normal (nonzero) number
			test	flagB, #ZeroFlag wz
		if_nz	brw	#__return_signed_zero
			brw    	#__return_signed_infinity

.mulzeroA
			test	flagB, #InfFlag wz
		if_nz	brw	#__return_nan		'' 0 * Inf == NaN
			brw	#__return_signed_zero

.mulinfA
			test	flagB, #ZeroFlag wz
		if_nz	brw	#__return_nan
			brw	#__return_signed_infinity

'----------------------------
' division
' fnumA /= fnumB
'----------------------------
			.global	___divsf3
___divsf3
			mov	r7,lr
			lcall	#__loadfloat
			mov	lr,r7

                        call    #__FUnpack2               ' unpack two variables
			mov	__TMP0, flagA
			or	__TMP0, flagB
			test	__TMP0, #(ZeroFlag|InfFlag|NaNFlag) wz
          if_nz         brw     #__div_excep
        
			jmpret	  __LMM_RET, #__LMM_FCACHE_START + (.FCdiv - .FCfloatstart)
_FDiv_ret               mov	pc,lr

			''
			'' special handling for 0 or NaN
			''
__div_excep
			test	__TMP0, #NaNFlag wz
		if_nz	brw	#__return_nan
			test	flagA, #ZeroFlag wz
		if_nz	brw	#.divzeroA
			test	flagA, #InfFlag wz
		if_nz	brw	#.divinfA
			'' ok, A is normal, so either B is inf or 0
			test   flagB, #ZeroFlag wz
			'' A / 0 == infinity
		if_nz	brw    #__return_signed_infinity	'' return signed 0
			brw    #__return_signed_zero

			'' come here if A is 0
.divzeroA
			test	flagB, #ZeroFlag wz
		if_nz	brw	#__return_nan		'' 0/0 == NaN
			brw	#__return_signed_zero

			'' come here if A is infinity
.divinfA
			'' inf / 0 == inf
			'' inf / inf == NaN

			test	flagB, #(InfFlag) wz
		if_nz	brw	#__return_nan
			brw	#__return_signed_infinity
#endif

#ifdef L_floatsisf
			.global	___floatsisf
___floatsisf
			mov	r7,lr
			lcall	#__loadfloat
			mov	lr,r7

			abs	manA, r0	wc,wz
		if_z	mov	pc,lr
			mov	flagA,#0
			muxc	flagA,#SignFlag
			mov	expA, #28
			call	#__FPack
			mov	pc,lr
#endif

#ifdef L_loaddouble
/*
        Double -- double precision floating point routines for
	the Propeller

        Copyright (c) 2012 Total Spectrum Software Inc.

        Released under the MIT License (see the end of this file for details)      
*/

#include "asmdouble.h"
	
		.section .double.kerext, "ax"
		.compress off
startdouble
		long	enddouble - startdouble

bigmask_28	long    $E0000000
one_4_28	long	$10000000

		''
		'' re-normalize A to 4.28 format
		''
Normalize
		'' check for 0
		or	A, Alo wz, nr
	if_nz	jmp	#_down
		or	Aflag, #FLAG_ZERO
		jmp	Normalize_ret

		'' shift down if necessary
_down
		test	A, bigmask_28 wz
	if_z	jmp	#_up
		add	expA, #1
		shr	A, #1 wc
		rcr	Alo, #1 wc
	if_c	or	Aflag, #FLAG_STICKY	' remember we lost bits
		jmp	#_down

_up
		test   A, one_4_28 wz
	if_nz	jmp    #_renorm_done
		shl    Alo, #1 wc
		rcl    A, #1
		sub    expA, #1
		jmp    #_up

_renorm_done
Normalize_ret
		ret


_mul_lp
		shr	tmp0,#1 wc
		rcr	tmp1,#1 wc
	if_nc	jmp	#_mul_skip_add
		add	Alo,Blo wc
		addx	A,B
_mul_skip_add
		shr	A,#1 wc
		rcr	Alo,#1 wc
	if_c	or	Aflag,#FLAG_STICKY
		djnz	count,#_mul_lp
_mul_lp_ret
		ret


_divloop
		cmp	tmp0, Blo wc,wz
		cmpx	tmp1, B wc,wz
	if_b	jmp	#_div_skip_sub
		sub	tmp0, Blo wc,wz
		subx	tmp1, B
		shl	Alo, #1 wc
		or	Alo, #1
		jmp	#_div_next
_div_skip_sub
		shl	Alo, #1 wc
_div_next
		rcl	A, #1
		shl	tmp0, #1 wc
		rcl	tmp1, #1
		djnz	count, #_divloop

		'' set sticky bit if necessary
		or     	 tmp0,tmp1 nr,wz
	if_nz	or	 Aflag, #FLAG_STICKY

_divloop_ret
		ret

		// helper code for add
_addshift
		max	 tmp0,#31
		mov	 tmp1,#32
		sub	 tmp1,tmp0
		mov	 Btmp,Blo
		shl	 Btmp,tmp1 wz
	if_nz	or	 Aflag, #FLAG_STICKY
		shr	 Blo, tmp0
		mov	 Btmp, B
		shl	 Btmp, tmp1
		shr	 B, tmp0
		or	 Blo, Btmp
_addshift_ret
		ret

enddouble

#define LDLABEL(x) (__LMM_FCACHE_START + (x-.LDstart))
#define LFLABEL(x) (__LMM_FCACHE_START + (x-.LFstart))

		.compress default
		.text

		.global __load_double_code
__load_double_code
		mvi	r4,#__load_start_double_kerext
		fcache	#(.LDend - .LDstart)
		.compress off
.LDstart
		mov	__TMP1,r4
		call	#__load_extension
		jmp	__LMM_RET


		''
		'' code to unpack a double in A, Alo
		'' the IEEE format is 1 bit sign, 11 bit exponent,
		'' then 52 bit mantissa
		''
.LDDunpack
		mov	Aflag, #0
		mov	expA, A
		shl	expA, #1 wc
	if_c	or	Aflag, #FLAG_SIGN
		and	A, DMANTMASK	' mask off exponent and sign bit
		shr	expA, #21 wz	' extract exponent
	if_z	jmp	#LDLABEL(_Ddenorm)	' zero or denormal
		cmp	expA, DMAX_EXP wz
		sub	expA, DBIAS_EXP  ' remove bias
	if_z	jmp	#LDLABEL(_Dnan)	      	' NaN or Infinity

		'' now shift up to 4.28 to give head room
		'' we start with 1.20
		mov	tmp0, Alo
		shl	A, #8
		shl	Alo, #8
		shr	tmp0, #24
		or	A, tmp0
		or	A, one_4_28	'' or in implied one
.LDDunpack_ret
		ret

		'' normalize a denormalized number
_Ddenorm
		sub	expA, DBIAS_EXP
		'' adjust for converting from 1.52 to 1.60
		add	expA, #(1+8)
		'' check for all 0
		or 	 A, Alo nr,wz
	if_z	sub	 expA, #64
	if_z	or	 Aflag, #FLAG_ZERO
	if_z	jmp	 DUnpack_ret
		'' not all 0, renormalize
		call	 #Normalize
		jmp      DUnpack_ret

		'' handle NaN or Infinity
_Dnan
		mov	expA, DMAX_EXP
		or	A, Alo nr, wz	'' check for infinity
	if_z	or	Aflag, #FLAG_INF
	if_z	mov	A, one_4_28
	if_nz	or	Aflag, #FLAG_NAN
	if_nz	add	expA, expA
		jmp	DUnpack_ret

.LD_dpack_round
		andn	A, one_4_28
		test	Aflag, #FLAG_STICKY wz

		'' we have 4.60, we want to round to 4.52
		'' half of the lsb is therefore 0x80
		'' we also want to round to nearest even, so
		'' add a sticky bit if the lsb is set
		test    Alo, #$100 wc
    if_nz_or_c	or	Alo, #1
		add	Alo, #$7f wc
		addx  	A, #0
.LD_dpack_round_ret
		ret

.LD_DBIAS_EXP	long	1023
.LD_DMAX_EXP	long	$7ff
'' mask for double mantissa (high word)
.LD_DMANTMASK	long	$000FFFFF

		.equ dpack_round,LDLABEL(.LD_dpack_round)
		.equ dpack_round_ret,LDLABEL(.LD_dpack_round_ret)

		.equ DUnpack,LDLABEL(.LDDunpack)
		.equ DUnpack_ret,LDLABEL(.LDDunpack_ret)

		.equ DBIAS_EXP, LDLABEL(.LD_DBIAS_EXP)
		.equ DMAX_EXP, LDLABEL(.LD_DMAX_EXP)
		.equ DMANTMASK, LDLABEL(.LD_DMANTMASK)
.LDend
		.compress default
		mov	pc,lr



		''
		'' pack a 4.60 number in A,Alo back to an IEEE double
		'' in r1,r0
		''
		'' need to handle rounding and such!
		''
		'' input is assumed to be normalized
		''
		.global __DPack
__DPack
		call	#Normalize

		test	Aflag, #(FLAG_INF|FLAG_NAN|FLAG_ZERO) wz
	if_nz	brw	#dpack_excep

		'' fix up exponent
		add	expA, DBIAS_EXP
		cmps	expA, #0 wz, wc
	if_be	brw	#dpack_denorm
ddenorm_done
#ifdef __PROPELLER2__
		cmp	expA, DMAX_EXP wc
		max	expA, DMAX_EXP
#else
		max	expA, DMAX_EXP wc
#endif
	if_nc	brw	#dpack_excep

		'' round here
		'' we clear the implied one first, and allow the
		'' rounding to propagate up to it
		call	#dpack_round

dpack_exp
		'' now shift down to 12.52
		shr     Alo,#8
		mov     tmp0,A
		shr     A,#8
		shl     tmp0,#24
		or      Alo,tmp0

		shl	expA, #20

		mov	r0, Alo
		mov	r1, A
		add	r1, expA

		shl	Aflag, #31
		or	r1, Aflag
DPack_ret
		LMMRET

		''
		'' exponent is <=0, so we have to create an IEEE denormalized
		'' number
dpack_denorm

		abs	expA, expA
		add	expA, #1
_ddlp
		shr	A, #1 wc
		rcr	Alo, #1 wc
	if_c	or	Aflag, #FLAG_STICKY
		LDJNZ	expA, #_ddlp

dpack_denorm_ret
		brw	#ddenorm_done

dpack_excep
		mov	A, #0
		mov	Alo, #0
		mov	expA, DMAX_EXP
		test	Aflag, #FLAG_NAN wz
	if_nz	mov	A, one_4_28
	if_nz	shr	A, #1
	if_nz	brw	#dpack_exp
		test	Aflag, #FLAG_ZERO wz
	if_nz	mov	expA, #0
		brw	#dpack_exp

		''
		'' unpack (r1,r0) into A and (r3,r2) into B
		'' assumes registers have already been saved
		''
		.global __DUnpack2
__DUnpack2
		mov	A,r3
		mov	Alo,r2
		call	#DUnpack
		mov	B,A
		mov	Blo,Alo
		mov	Bflag,Aflag
		mov	expB,expA
		mov	A,r1
		mov	Alo,r0
		call	#DUnpack
__DUnpack2_ret
		LMMRET

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'' Actual commands start here
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
		.global ___subdf3
		.global	___adddf3

	'' addition and subtraction
___subdf3
		xor	r3, __Bit31
		'' fall through
___adddf3
		SAVEREGS
		lcall	#__load_double_code
do_adddf3
		'' swap (r0,r1) and (r2,r3) if necessary
		'' so that |A| > |B|
		mov	r4,r1
		mov	r5,r3
		shl	r4,#1
		shl	r5,#1
		cmp	r0,r2 wz,wc
		cmpx	r4,r5 wz,wc
    if_ae       brw	#dskipswap
    		mov	r4,r0
		mov	r5,r1
		mov	r0,r2
		mov	r1,r3
		mov	r2,r4
		mov	r3,r5
dskipswap
		lcall	#__DUnpack2
		lcall	#_Add
		lcall	#__DPack
		RESTOREREGS
		LMMRET

		.global	___muldf3
		.global	___divdf3
___muldf3
		SAVEREGS
		lcall	#__load_double_code
		lcall	#__DUnpack2
		lcall	#_Mul
		lcall	#__DPack
		RESTOREREGS
		LMMRET

___divdf3
		SAVEREGS
		lcall	#__load_double_code
		lcall	#__DUnpack2
		lcall	#_Div
		lcall	#__DPack
		RESTOREREGS
		LMMRET

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'' Utility functions go here
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

		''
		'' the actual add routine
		''
		'' NOTE: this should be called with magnitude
		'' of A being greater than the magnitude of B
		''
_Add
		'' shift B down as necessary
		mov	 tmp0,expA
		sub	 tmp0,expB wz
	if_z	brw	 #_doadd
		cmp	 tmp0,#32 wz,wc
	if_b	brw	 #_do_shift
		cmp	 Blo,#0   wz,wc
	if_nz	or	 Aflag, #FLAG_STICKY
		mov	 Blo, B
		mov	 B, #0
		sub	 tmp0,#32 wz
	if_z	brw	 #_doadd
_do_shift
		call	#_addshift
_doadd
		'' now perform the addition
		mov	tmp0, Aflag
		xor     tmp0, Bflag
		test    tmp0, #FLAG_SIGN wz
	if_nz	brw     #_dosub
		add     Alo, Blo wc
		addx    A, B
		brw     #_Add_ret
_dosub
		'' check for INF - INF
		'' note that if B is INF, then A is NAN or INF, so
		'' in either case NAN is appropriate to return
		test	Bflag, #FLAG_INF wz
	if_nz	or	Aflag, #FLAG_NAN
		'' watch out for the sticky bit; it can affect the low word
		test	Aflag, #FLAG_STICKY wc
		subx	Alo, Blo wc, wz
		subx	A, B wc, wz
	if_z	andn	Aflag, #FLAG_SIGN
	if_z	or	Aflag, #FLAG_ZERO
_Add_ret
		LMMRET

		'' the actual multiply routine
_Mul
		mov	tmp0,Aflag
		or	tmp0,Bflag
		test	tmp0,#(FLAG_INF|FLAG_NAN) wz
	if_nz	brw	#_mul_excep
		'' regular multiply
		add	expA,expB

		'' shift (B,Blo) up by 1 to prepare loop
		add	Blo,Blo wc
		addx	B,B

		'' (A, Alo) will be the accumulator
		mov    tmp0,A
		mov    tmp1,Alo wz
		mov    A,#0
		mov    Alo,#0

		mov	count,#61

		'' shorten the loop in the common case that
		'' tmp1 is zero
	if_z	mov	tmp1,tmp0
	if_z	mov	tmp0,#0
	if_z	sub	count,#32

		call	#_mul_lp

_mul_sign
		mov	tmp0,Aflag
		xor	tmp0,Bflag
		test	tmp0,#FLAG_SIGN wz
		muxnz	Aflag,#FLAG_SIGN
_Mul_ret
		LMMRET

		'' special cases for inf, NaN
_mul_excep
		'' if we get here, we know that either the
		'' NAN or INF bit is set
		'' if 0 is set as well, we have an illegal condition
		'' NAN*anything = NAN
		'' 0*inf == NAN
		test	tmp0, #(FLAG_NAN|FLAG_ZERO) wz
	if_nz	or	Aflag,#FLAG_NAN
	if_z	or	Aflag,#FLAG_INF
		brw	#_mul_sign

		''
		'' the actual division routine
		''
		'' DivSmall just does bottom 28 bits, Div does all
		'' 61
_DivSmall
		'' we're given just 1.28 bits
		'' make sure we produce a few extra bits for rounding
		mov	count, #31
		sub	expA, #2	'' compensate for rounding bits
		brw	#_doDiv
_Div
		mov	count,#61
_doDiv
		'' set the sign of the result
		mov	tmp0, Aflag
		xor	tmp0, Bflag
		test	tmp0, #FLAG_SIGN wz
		muxnz	Aflag,#FLAG_SIGN

		mov	tmp0, Aflag
		or	tmp0, Bflag
		'' check for divide by infinity or NAN
		test	tmp0, #(FLAG_INF|FLAG_NAN) wz
	if_nz	brw	#_div_excep
		'' check for divide by 0
		test	Bflag, #FLAG_ZERO wz
	if_nz	brw	#_div_by_zero

		'' regular divide loop here
		sub	expA, expB
		mov	tmp0, Alo
		mov	tmp1, A
		'' initialize quotient
		mov	A, #0
		mov	Alo, #0

		call	#_divloop

_Div_ret
_DivSmall_ret
		LMMRET

_div_by_zero
		test	Aflag, #(FLAG_NAN|FLAG_INF|FLAG_ZERO) wz
	if_nz	or	Aflag, #FLAG_NAN
	if_z	or	Aflag, #FLAG_INF
		brw	#_Div_ret

		''
		'' if some number is infinity or NaN, come here
		''
_div_excep
		test	tmp0, #FLAG_NAN wz
_div_nan
	if_nz	or	Aflag, #FLAG_NAN
	if_nz	brw	#_Div_ret

		test	Aflag, #FLAG_INF wz
	if_z	brw	#_a_finite
		'' infinity/x
		test	Bflag, #(FLAG_INF) wz
	if_nz	brw	#_div_nan
		brw	#_Div_ret

		'' x/infinity
_a_finite
		or	Aflag, #FLAG_ZERO
		mov	A, #0
		mov	Alo, #0
		brw	#_Div_ret
#endif
#ifdef L_floatsfdf
#include "asmdouble.h"
	'' conversion operations
		'' single to double
		.global ___floatsfdf
___floatsfdf
		SAVEREGS
		mov	A, r0
		lcall	#__loadfloat
		call	#__FUnpack
		lcall	#__load_double_code
		lcall	#__DPack
		RESTOREREGS
		LMMRET
#endif
#ifdef L_floatsidf
#include "asmdouble.h"
		.global ___floatsidf
		.global ___floatunssidf
___floatunssidf
		SAVEREGS
		mov	A, r0 wz
		mov	Aflag, #0
		brs	#.doconv
___floatsidf
		SAVEREGS
		abs	A, r0 wc, wz
		mov	Aflag, #0
	if_c	or	Aflag, #FLAG_SIGN
.doconv
	if_z	mov	r1, #0
	if_z	brw	#doint_ret		'' 0 -> 0
		mov	Alo, #0
		mov	expA,#28	'' set the exponent
		lcall	#__load_double_code
		lcall	#__DPack
doint_ret
		RESTOREREGS
		LMMRET
#endif

/*


+------------------------------------------------------------------------------------------------------------------------------+
|                                                   TERMS OF USE: MIT License                                                  |                                                            
+------------------------------------------------------------------------------------------------------------------------------+
|Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation    | 
|files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,    |
|modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software|
|is furnished to do so, subject to the following conditions:                                                                   |
|                                                                                                                              |
|The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.|
|                                                                                                                              |
|THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE          |
|WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR         |
|COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,   |
|ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                         |
+------------------------------------------------------------------------------------------------------------------------------+
*/
