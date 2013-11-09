' crt0_lmm2.S - an LMM kernel for the Propeller 2
'
'  Based on ideas from Bill Henning's original LMM design

        .section .kernel, "ax"
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
        .global pc

        .global __LMM_entry
__LMM_entry
r0	mov	__TMP1, r6	'' get pointer to initialization
r1	call	#__load_extension
r2  	jmp	#__LMM_init
	

r3      nop
r4	nop
r5      nop
r6      long __load_start_start_kerext

r7      nop
r8      nop
r9      nop
r10     nop
r11     nop
r12     nop
r13	nop
r14	long	0	'' flag for first time run
	
r15	'' alias for link register lr
lr	long	__exit
sp	long	0
pc	long	entry		' default pc


        ''
        '' main LMM loop -- read instructions from hub memory
        '' and executes them
        ''
__LMM_loop
        repd #$200,#8
        nop
        nop
        nop
        rdlongc L_ins1,pc
        add pc,#4
        nop
L_ins1  nop
        rdlongc L_ins2,pc
        add pc,#4
        nop
L_ins2  nop
        jmp #__LMM_loop
        
        ''
        '' LMM support functions
        ''

        '' move immediate
        .macro LMM_movi reg
        .global __LMM_MVI_\reg
__LMM_MVI_\reg
        jmpd        #__LMM_loop
          rdlongc   \reg,pc
          add       pc,#4
          nop
        .endm

        LMM_movi r0
        LMM_movi r1
        LMM_movi r2
        LMM_movi r3
        LMM_movi r4
        LMM_movi r5
        LMM_movi r6
        LMM_movi r7
        LMM_movi r8
        LMM_movi r9
        LMM_movi r10
        LMM_movi r11
        LMM_movi r12
        LMM_movi r13
        LMM_movi r14
        LMM_movi lr

        ''
        '' call functions
        ''
        .global __LMM_CALL
        .global __LMM_CALL_INDIRECT
__LMM_CALL
        rdlongc __TMP0,pc
        add     pc,#4
__LMM_CALL_INDIRECT
        jmpd    #__LMM_loop
          mov   lr,pc
          mov   pc,__TMP0
          nop

        ''
        '' direct jmp
        ''
        .global __LMM_JMP
__LMM_JMP
        jmpd    #__LMM_loop
          rdlongc pc,pc
          nop
          nop

        ''
        '' push and pop multiple
        '' these take in __TMP0 a mask
        '' of (first_register|(count<<4))
        ''
        '' note that we push from low register first (so registers
        '' increment as the stack decrements) and pop the other way
        ''
        .global __LMM_PUSHM
        .global __LMM_PUSHM_ret
__LMM_PUSHM
        mov     __TMP1,__TMP0
        and     __TMP1,#0x0f
        movd    L_pushins,__TMP1
        shr     __TMP0,#4
	nop
L_pushloop
        sub     sp,#4
L_pushins
        wrlong  0-0,sp
        add     L_pushins,inc_dest1
        djnz    __TMP0,#L_pushloop
__LMM_PUSHM_ret
        ret

inc_dest1
        long    (1<<9)

        .global __LMM_POPM
        .global __LMM_POPM_ret
        .global __LMM_POPRET
        .global __LMM_POPRET_ret


__LMM_POPRET
        call    #__LMM_POPM
        mov     pc,lr
	    mov	    L_ins2, dec_pc	'compensate 2nd inc_pc if popret is executed in L_ins1
__LMM_POPRET_ret
        ret

dec_pc  sub  pc,#4

        
__LMM_POPM
        mov     __TMP1,__TMP0
        and     __TMP1,#0x0f
        movd    L_poploop,__TMP1
        shr     __TMP0,#4
        nop
L_poploop
        rdlong  0-0,sp
        add     sp,#4
        sub     L_poploop,inc_dest1
        djnz    __TMP0,#L_poploop
__LMM_POPM_ret
        ret

        ''
        '' masks
        ''
        
        .global __MASK_0000FFFF
        .global __MASK_FFFFFFFF

__MASK_0000FFFF long    0x0000FFFF
__MASK_FFFFFFFF long    0xFFFFFFFF

        ''
        '' math support functions
        ''
        .global __TMP0
__TMP0  long    0

        .global __TMP1
__TMP1
        long    0
        
        .global __MULSI
        .global __MULSI_ret
__MULSI
__MULSI
        setmula r0
        setmulb r1
        getmull r0
__MULSI_ret     ret

        ''
        '' code for atomic compare and swap
        ''
        .global __C_LOCK_PTR
__C_LOCK_PTR
        long    __C_LOCK

        ''
        '' compare and swap a variable
        '' r0 == new value to set if (*r2) == r1
        '' r1 == value to compare with
        '' r2 == pointer to memory
        '' output: r0 == original value of (*r2)
        ''         Z flag is set if (*r2) == r1, clear otherwise
        ''
        .global __CMPSWAPSI
        .global __CMPSWAPSI_ret
__CMPSWAPSI
        '' get the C_LOCK
        rdbyte  __TMP1,__C_LOCK_PTR
        mov     __TMP0,r0       '' save value to set
.swaplp
        lockset __TMP1 wc
   IF_C jmp     #.swaplp

        rdlong  r0,r2           '' fetch original value
        cmp     r0,r1 wz        '' compare with desired original value
   IF_Z wrlong  __TMP0,r2       '' if match, save new value
        
        '' now release the C lock
        lockclr __TMP1
__CMPSWAPSI_ret
        ret

	''
	'' code to load a buffer from hub memory into cog memory
	'' parameters: __TMP0 = count of bytes (will be rounded up to multiple
	''                      of 8)
	''             __TMP1 = hub address
	''             __COGA = COG address
	''
	''
__COGA	long 0
	
loadbuf
	movd	.ldlp,__COGA
	shr	__TMP0,#2	'' convert to longs
	nop
.ldlp
	rdlongc	0-0,__TMP1
	add	__TMP1,#4
	add	.ldlp,inc_dest1
	djnz	__TMP0,#.ldlp

loadbuf_ret
	ret

	
	
	''
	'' FCACHE region
	'' The FCACHE is an area where we can
	'' execute small functions or loops entirely
	'' in Cog memory, providing a significant
	'' speedup.
	''

	.global __LMM_FCACHE_ADDR
__LMM_FCACHE_ADDR
	long 0
	
	.global	__LMM_RET
	.global	__LMM_FCACHE_LOAD
__LMM_RET
	long 0
__LMM_FCACHE_LOAD
	rdlong	__TMP0,pc	'' read count of bytes for load
	add	pc,#4
	mov	__TMP1,pc
	cmp	__LMM_FCACHE_ADDR,pc wz	'' is this the same fcache block we loaded last?
	add	pc,__TMP0	'' skip over data
  IF_Z	jmp	#Lmm_fcache_doit

	mov	__LMM_FCACHE_ADDR, __TMP1

	'' copy TMP0 bytes from TMP1 to LMM_FCACHE_START
	mova	__COGA, #__LMM_FCACHE_START	'' mova because src is a cog address immediate
	call	#loadbuf
	
Lmm_fcache_doit
	jmpret	__LMM_RET,#__LMM_FCACHE_START
	jmp	#__LMM_loop


	''
	'' the fcache area should come last in the file
	''
	.global __LMM_FCACHE_START
__LMM_FCACHE_START
	res	128	'' reserve 128 longs = 512 bytes


	''
	'' now include kernel extensions that we always want
	''
#include "kernel.ext"

