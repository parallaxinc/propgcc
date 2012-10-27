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
                                 
.subnorm                shl     manA, #6                ' fix justification for subnormals  
.subnorm2               test    manA, Bit28 wz
          if_nz         jmp     #.unpack_exit1
                        shl     manA, #1
                        sub     expA, #1
                        jmp     #.subnorm2

.finite                 shl     manA, #5                ' justify mantissa to bit 28
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
			cmp	big_4_28, manA wc
	  if_c		add	expA, #1
	  if_c		shr	manA, #1 wc
          if_c          or      flagA, #StickyBit
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
			max	expA, #255 wc
	   if_nc	mov	manA, #0
			shl	expA, #23
			add	manA, expA
.pack_exit1	
			mov	r0, flagA
			shl	r0, #31
			or	r0, manA

__FPack_ret             ret


'-------------------- constant values -----------------------------------------

Mask23                  long    $007F_FFFF
Bit28                   long    $1000_0000


endfloat

			.section .kernel
Bit31                   long    $8000_0000
	
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
                        xor     r1, Bit31            ' negate B

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
