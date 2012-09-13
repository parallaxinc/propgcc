
''        F32 - Concise floating point code for the Propeller
''        Copyright (c) 2011 Jonathan "lonesock" Dummer
''
''        Released under the MIT License (see the end of this file for details)
''        Modified by Eric Smith for GAS and LMM
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
'          manA         mantissa (aligned to bit 29)
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
                                 
.subnorm                shl     manA, #7                ' fix justification for subnormals  
.subnorm2               test    manA, Bit29 wz
          if_nz         jmp     #.unpack_exit1
                        shl     manA, #1
                        sub     expA, #1
                        jmp     #.subnorm2

.finite                 shl     manA, #6                ' justify mantissa to bit 29
                        or      manA, Bit29             ' add leading one bit
                        
.unpack_exit1           sub     expA, #127              ' remove bias from exponent
__FUnpack_ret           ret       


'------------------------------------------------------------------------------
' input:   r0           32-bit floating point value
'          r1           32-bit floating point value 
' output:  flagA        fnumA flag bits (Nan, Infinity, Zero, Sign)
'          expA         fnumA exponent (no bias)
'          manA         fnumA mantissa (aligned to bit 29)
'          flagB        fnumB flag bits (Nan, Infinity, Zero, Sign)
'          expB         fnumB exponent (no bias)
'          manB         fnumB mantissa (aligned to bit 29)
' changes: fnumA, flagA, expA, manA, fnumB, flagB, expB, manB, __TMP0
'------------------------------------------------------------------------------
			.global __FUnpack2
			.global __FUnpack2_ret
__FUnpack2
        		mov     manA, r1            ' unpack B to A
                        call    #__FUnpack

                        mov     flagB, flagA
                        mov     expB, expA
                        mov     manB, manA

                        mov     manA, r0               ' unpack A
                        call    #__FUnpack

__FUnpack2_ret          ret


'------------------------------------------------------------------------------
' input:   flagA        fnumA flag bits (Nan, Infinity, Zero, Sign)
'          expA         fnumA exponent (no bias)
'          manA         fnumA mantissa (aligned to bit 29)
' output:  r0           32-bit floating point value
' changes: r0, flagA, expA, manA 
'------------------------------------------------------------------------------
			.global __FPack
			.global __FPack_ret
__FPack
			cmp     manA, #0 wz             ' check for zero                                        
          if_z          mov     expA, #0
          if_z          jmp     #.pack_exit1

                        sub     expA, #380              ' take us out of the danger range for djnz
.normalize              shl     manA, #1 wc             ' normalize the mantissa
          if_nc         djnz    expA, #.normalize       ' adjust exponent and jump

                        add     manA, #$100 wc          ' round up by 1/2 lsb

                        addx    expA, #(380 + 127 + 2)  ' add bias to exponent, account for rounding (in flag C, above)
                        mins    expA, Minus23

                        abs     expA, expA wc,wz        ' check for subnormals, and get the abs in case it is
          if_a          jmp     #.pack_exit1

.packsubnormal          or      manA, #1                ' adjust mantissa
                        ror     manA, #1

                        shr     manA, expA
                        mov     expA, #0                ' biased exponent = 0

.pack_exit1
			max	expA, #255 wc
	  if_nc		mov	manA, #0
        		mov     r0, manA                ' bits 22:0 mantissa
                        shr     r0, #9
                        movi    r0, expA                ' bits 23:30 exponent
                        shl     flagA, #31
                        or      r0, flagA               ' bit 31 sign            
__FPack_ret               ret


'-------------------- constant values -----------------------------------------

Minus23                 long    -23
Mask23                  long    $007F_FFFF
Bit29                   long    $2000_0000
Bit31                   long    $8000_0000


endfloat


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
.FCadd

                        test    flagA, #SignFlag wz     ' negate A mantissa if negative
          if_nz         neg     manA, manA
                        test    flagB, #SignFlag wz     ' negate B mantissa if negative
          if_nz         neg     manB, manB

                        mov     __TMP0, expA                ' align mantissas
                        sub     __TMP0, expB
                        abs     __TMP0, __TMP0          wc
                        max     __TMP0, #31
              if_nc     sar     manB, __TMP0
              if_c      sar     manA, __TMP0
              if_c      mov     expA, expB

                        add     manA, manB              ' add the two mantissas
                        abs     manA, manA      wc      ' store the absolte value,
                        muxc    flagA, #SignFlag        ' and flag if it was negative

                        call    #__FPack                  ' pack result and exit
			jmp	__LMM_RET

.FCmul
                        xor     flagA, flagB            ' get sign of result
                        add     expA, expB              ' add exponents

                        ' standard method: 404 counts for this block
                        mov     __TMP0, #0                  ' __TMP0 is my accumulator
                        mov     __TMP1, #24                 ' loop counter for multiply (only do the bits needed...23 + implied 1)
                        shr     manB, #6                ' start by right aligning the B mantissa

.multiply               shr     __TMP0, #1                  ' shift the previous accumulation down by 1
                        shr     manB, #1 wc             ' get multiplier bit
              if_c      add     __TMP0, manA                ' if the bit was set, add in the multiplicand
	      		djnz	__TMP1, #__LMM_FCACHE_START+(.multiply-.FCfloatstart)

                        mov     manA, __TMP0                ' yes, that's my final answer.

                        call    #__FPack
			jmp	__LMM_RET

.FCdiv
                        xor     flagA, flagB            ' get sign of result
                        sub     expA, expB              ' subtract exponents

                        ' slightly faster division, using 26 passes instead of 30
                        mov     __TMP0, #0                  ' clear quotient
                        mov     __TMP1, #26                 ' loop counter for divide (need 24, plus 2 for rou
.divide
                        cmpsub  manA, manB      wc
                        rcl     __TMP0, #1
                        shl     manA, #1
			sub	__TMP1, #1 wz
            if_nz       jmp     #__LMM_FCACHE_START + (.divide - .FCfloatstart)
                        shl     __TMP0, #4                  ' align the result (we did 26 instead of 30 iterations)
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
		if_nz	brs    #__return_nan
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
			test   flagB, #ZeroFlag wz
		if_nz	brw    #__return_signed_zero
			brw    #__return_signed_infinity

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
			'' inf / 0 == NaN
			'' inf / inf == NaN

			test	flagB, #(ZeroFlag|InfFlag) wz
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
			mov	expA, #29
			call	#__FPack
			mov	pc,lr
#endif