' crt0_p2.S - an LMM kernel for the Propeller 2
'
'  Based on ideas from Bill Henning's original LMM design

        .section .lmmkernel, "ax"
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

    CLOCK_FREQ =    60000000
    BAUDRATE =      115200
    RX_PIN =        91
    TX_PIN =        90
    
    REG_DIRC =      0x1fe
    
        .global __LMM_entry
__LMM_entry
r0      getptra sp
r1      rdlong  __TMP0, __C_LOCK_PTR  wz ' check for first time run
r2  IF_NE    jmp    #not_first_cog      ' if not, skip some stuff
        
        '' initialization for first time run
r3      locknew __TMP0 wc       ' allocate a lock
r4      or      __TMP0, #256    ' in case lock is 0, make the 32 bit value nonzero
r5      wrlong __TMP0, __C_LOCK_PTR     ' save it to ram

not_first_cog
r6      setp    #TX_PIN
r7      or      REG_DIRC, tx_mask
r8      jmp     #__LMM_loop
tx_mask
r9      long    1 << (TX_PIN - 64)  ' must be in dirc
r10     long    0
r11     long    0
r12     long    0
r13     long    0
r14     long    0
r15     '' alias for link register lr
lr      long    __exit
sp      long    0x20000     ' 128k of hub memory
pc      long    entry       ' default pc

        ''
        '' main LMM loop -- read instructions from hub memory
        '' and executes them
        ''
/*
__LMM_loop
        rdlongc L_ins1,pc   'read instruction
        jmpd #__LMM_loop
          add pc,#4         'point to next instr
L_ins1    nop               'execute instruction
          nop               '3 delay slots for jumpd
        jmp #__LMM_loop     'loop in case jmpd got cancelled by LMM code
*/

/*
__LMM_loop
        reps #512,#5
        nop
          rdlongc L_ins1,pc
          add pc,#4
          rdlongc L_ins2,pc
L_ins1    nop
          add pc,#4
L_ins2    nop
        jmp #__LMM_loop
*/

/*
__LMM_loop
        rdlongc L_ins1,pc   'read instruction 1
        add pc,#4           'point to ins1+1
        rdlongc L_ins2,pc   'read instruction 2
        jmpd #__LMM_loop
L_ins1    nop               'execute instruction 1
          add pc,#4         'point to ins2+1
L_ins2    nop               'execute instruction 2
        jmp #__LMM_loop
*/

/*
__LMM_loop
        rdlong  L_ins0,pc
        add     pc,#4
        nop
L_ins0  nop
        jmp     #__LMM_loop
*/


/*
__LMM_loop
        reps #$4000,#3
        nop
        rdlongc L_ins1,pc
        add pc,#4
        nop
L_ins1  nop
        jmp #__LMM_loop
*/

/*
__LMM_loop
        reps #$4000,#7
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
*/


__LMM_loop
        repd #$200,#7
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

/*
__LMM_POPRET
        call    #__LMM_POPM
        mov     pc,lr
__LMM_POPRET_ret
        ret
*/

__LMM_POPRET
        call    #__LMM_POPM
        mov     pc,lr
        mov     L_ins2,dec_pc   'compensate 2nd inc_pc if popret is executed in L_ins1 
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
__MASK_00FF00FF long    0x00FF00FF
__MASK_0F0F0F0F long    0x0F0F0F0F
__MASK_33333333 long    0x33333333
__MASK_55555555 long    0x55555555

        .global __CLZSI
        .global __CLZSI_ret
        .global __CTZSI
        .global __CTZSI_ret
__CLZSI rev     r0, #0
__CTZSI neg     __TMP0, r0
        and     __TMP0, r0      wz
        mov     r0, #0
 IF_Z   mov     r0, #1
        test    __TMP0, __MASK_0000FFFF wz
 IF_Z   add     r0, #16
        test    __TMP0, __MASK_00FF00FF wz
 IF_Z   add     r0, #8
        test    __TMP0, __MASK_0F0F0F0F wz
 IF_Z   add     r0, #4
        test    __TMP0, __MASK_33333333 wz
 IF_Z   add     r0, #2
        test    __TMP0, __MASK_55555555 wz
 IF_Z   add     r0, #1
__CLZSI_ret     ret
__CTZSI_ret     ret

__TMP1
        long    0
        ''
        '' calculate r0 = orig_r0/orig_r1, r1 = orig_r0 % orig_r1
        ''
        .global __UDIVSI
        .global __UDIVSI_ret
__UDIVSI
        setdivu r0
        setdivb r1
        getdivq r0
        getdivr r1 
__UDIVSI_ret    ret

        .global __DIVSI
        .global __DIVSI_ret
__DIVSI
        setdiva r0
        setdivb r1
        getdivq r0
        getdivr r1 
__DIVSI_ret     ret

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
        
'*******************************************************************************
' Get one character from the input port.
' Input none
' Changes r0, temp, temp1, temp2, c, z
' Output r0
' Code from Dave Hein's dfth as modified by David Betz for P2
'*******************************************************************************
                        .global __RX
                        .global __RX_ret
__RX                    getp    #RX_PIN wz
        if_nz           jmp     #__RX
                        getcnt  temp2
                        mov     temp, bitcycles
                        shr     temp, #1
                        add     temp2, temp
                        mov     temp1, #10
                        mov     r0, #0
__RX_loop               waitcnt temp2, bitcycles
                        ror     r0, #1
                        getp    #RX_PIN wc
        if_c            or      r0, #1
                        djnz    temp1, #__RX_loop
                        rol     r0, #8
                        and     r0, #255
__RX_ret                ret

'*******************************************************************************
' Send one character to the output port.
' Input r0
' Changes r0, temp1, temp2, c
' Output none             
' Code from Dave Hein's dfth as modified by David Betz for P2
'*******************************************************************************
                        .global __TX
                        .global __TX_ret
__TX                    and     r0, #$0ff
                        or      r0, #$100
                        shl     r0, #1
                        mov     temp1, #10
                        getcnt  temp2
                        add     temp2, bitcycles
__TX_loop               ror     r0, #1                  wc
                        setpc   #TX_PIN
                        waitcnt temp2, bitcycles
                        djnz    temp1, #__TX_loop
__TX_ret                ret

bitcycles               long    CLOCK_FREQ / BAUDRATE
temp                    long    0
temp1                   long    0
temp2                   long    0

                        .global __GETCNT
                        .global __GETCNT_ret
__GETCNT                getcnt  r0
__GETCNT_ret            ret

        ''
        '' FCACHE region
        '' The FCACHE is an area where we can
        '' execute small functions or loops entirely
        '' in Cog memory, providing a significant
        '' speedup.
        ''

__LMM_FCACHE_ADDR
        long 0
inc_dest4
        long (4<<9)
        
        .global __LMM_RET
        .global __LMM_FCACHE_LOAD
__LMM_RET
        long 0
__LMM_FCACHE_LOAD
        rdlong  __TMP0,pc       '' read count of bytes for load
        add     pc,#4
        mov     __TMP1,pc
        cmp     __LMM_FCACHE_ADDR,pc wz '' is this the same fcache block we loaded last?
        add     pc,__TMP0       '' skip over data
  IF_Z  jmp     #Lmm_fcache_doit

        mov     __LMM_FCACHE_ADDR, __TMP1
        
        '' assembler awkwardness here
        '' we would like to just write
        '' movd Lmm_fcache_loop,#__LMM_FCACHE_START
        '' but binutils doesn't work right with this now
        movd Lmm_fcache_loop,#(__LMM_FCACHE_START-__LMM_entry)/4
        movd Lmm_fcache_loop2,#1+(__LMM_FCACHE_START-__LMM_entry)/4
        movd Lmm_fcache_loop3,#2+(__LMM_FCACHE_START-__LMM_entry)/4
        movd Lmm_fcache_loop4,#3+(__LMM_FCACHE_START-__LMM_entry)/4
        add  __TMP0,#15         '' round up to next multiple of 16
        shr  __TMP0,#4          '' we process 16 bytes per loop iteration
Lmm_fcache_loop
        rdlong  0-0,__TMP1
        add     __TMP1,#4
        add     Lmm_fcache_loop,inc_dest4
Lmm_fcache_loop2
        rdlong  0-0,__TMP1
        add     __TMP1,#4
        add     Lmm_fcache_loop2,inc_dest4
Lmm_fcache_loop3
        rdlong  0-0,__TMP1
        add     __TMP1,#4
        add     Lmm_fcache_loop3,inc_dest4
Lmm_fcache_loop4
        rdlong  0-0,__TMP1
        add     __TMP1,#4
        add     Lmm_fcache_loop4,inc_dest4

        djnz    __TMP0,#Lmm_fcache_loop

Lmm_fcache_doit
        jmpret  __LMM_RET,#__LMM_FCACHE_START
        jmp     #__LMM_loop


        ''
        '' the fcache area should come last in the file
        ''
        .global __LMM_FCACHE_START
__LMM_FCACHE_START
        res     128     '' reserve 256 longs = 1K


