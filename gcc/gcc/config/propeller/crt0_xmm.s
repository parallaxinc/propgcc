	.section .xmmkernel, "ax"

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
r0	mov	sp, PAR
r1	rdlong cache_mboxcmd, sp
r2  add sp, #4
r3  mov cache_mboxdat, cache_mboxcmd
r4  add cache_mboxdat, #4
r5  rdlong cache_linemask, sp
r6	add sp, #4
r7	rdlong pc, sp
r8	add sp, #4
r9	jmp #__LMM_loop
r10	long	0
r11	long	0
r12	long	0
r13	long	0
r14	long	0
lr	long	0
sp	long	0
pc	long	0


	''
	'' main LMM loop -- read instructions from hub memory
	'' and executes them
	''
__LMM_loop
    call	#read_code
	add	pc,#4
L_ins0	nop
	jmp	#__LMM_loop

	''
	'' LMM support functions
	''

	'' move immediate
	.macro LMM_movi reg
	.global __LMM_MVI_\reg
__LMM_MVI_\reg
    call	#read_code
	mov	\reg,L_ins0
	add	pc,#4
	jmp	#__LMM_loop
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
	.global	__LMM_CALL
__LMM_CALL
    call	#read_code
	add	pc,#4
	mov	lr,pc
	mov	pc,L_ins0
	jmp	#__LMM_loop

	.global __LMM_CALL_INDIRECT
__LMM_CALL_INDIRECT
	mov	lr,pc
	mov	pc,__TMP0
	jmp	#__LMM_loop

	''
	'' direct jmp
	''
	.global __LMM_JMP
__LMM_JMP
    call	#read_code
    mov pc,L_ins0
	jmp	#__LMM_loop

	''
	'' masks
	''
	
	.global __MASK_0000FFFF
	.global __MASK_FFFFFFFF

__MASK_0000FFFF	long	0x0000FFFF
__MASK_FFFFFFFF	long	0xFFFFFFFF

	''
	'' math support functions
	''
	.global __TMP0
	.global __DIVSI
	.global __DIVSI_ret
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
__DIVSGN	long	0
__DIVSI	mov	__DIVSGN, r0
	xor	__DIVSGN, r1
	abs	r0, r0
	abs	r1, r1
	call	#__UDIVSI
	cmps	__DIVSGN, #0	wz, wc
 IF_B	neg	r0, r0
 IF_B	neg	r1, r1
__DIVSI_ret	ret

	.global __MULSI
	.global __MULSI_ret
__MULSI
__MULSI
	mov	__TMP0, r0
	min	__TMP0, r1
	max	r1, r0
	mov	r0, #0
__MULSI_loop
	shr	r1, #1	wz, wc
 IF_C	add	r0, __TMP0
	add	__TMP0, __TMP0
 IF_NZ	jmp	#__MULSI_loop
__MULSI_ret	ret

' the code below comes from the xbasic virtual machine which borrowed code from zog

    .set EXTERNAL_MEMORY_START, 0x20000000
    .set CACHE_CMD_MASK,        0x00000003
    .set CACHE_WRITE_CMD,       0x00000002
    .set CACHE_READ_CMD,        0x00000003

' read a long from the current pc
read_code               muxc    save_z_c, #1
                        cmp     pc,external_start wc    'check for normal memory access
                IF_NC   jmp     #read_hub_code
                        mov     t1, pc
                        call    #cache_read
                        rdlong  L_ins0, memp
                        jmp     #read_restore_c
read_hub_code           rdlong	L_ins0, pc
read_restore_c          shr     save_z_c, #1 wc
read_code_ret           ret
                        
' read a long from external memory
' address is in t1
' result returned in t1
read_external_long      call    #cache_read
                        rdlong  t1, memp
read_external_long_ret  ret

' write a long to external memory
' address is in t1
' value to write is in t2
write_external_long     call    #cache_write
                        wrlong  t2, memp
write_external_long_ret ret

t1                      long    0
t2                      long    0
save_z_c                long    0
cache_linemask          long    0
cache_mboxcmd           long    0
cache_mboxdat           long    0
temp                    long    0
memp                    long    0
cacheaddr               long    0
cacheptr                long    0

external_start          long    EXTERNAL_MEMORY_START       'start of external memory access window

cache_write             muxz    save_z_c, #2                'save the z flag
                        mov     memp, t1                    'save address for index
                        andn    t1, #CACHE_CMD_MASK         'ensure a write is not a read
                        or      t1, #CACHE_WRITE_CMD
                        jmp     #cache_access

cache_read              muxz    save_z_c, #2                'save the z flag
                        mov     temp, t1                    'ptr + cache_mboxdat = hub address of byte to load
                        andn    temp, cache_linemask
                        cmp     cacheaddr, temp wz          'if cacheaddr == addr, just pull form cache
            if_e        jmp     #cache_hit                  'memp gets overwriteen on a miss
                        
cache_read_miss         mov     memp, t1                    'save address for index
                        or      t1, #CACHE_READ_CMD         'read must be 3 to avoid needing andn addr,#cache#CMD_MASK

cache_access            wrlong  t1, cache_mboxcmd
                        mov     cacheaddr, t1               'save new cache address. it's free time here
                        andn    cacheaddr, cache_linemask   'kill command bits in free time
_waitres                rdlong  temp, cache_mboxcmd wz
            if_nz       jmp     #_waitres
                        and     memp, cache_linemask        'memp is index into buffer
                        rdlong  cacheptr, cache_mboxdat     'Get new buffer
                        add     memp, cacheptr              'memp is now HUB buf address of data to read
                        jmp     #cache_restore_z

cache_hit               mov     memp, t1                    'ptr + cache_mboxdat = hub address of byte to load
                        and     memp, cache_linemask
                        add     memp, cacheptr              'add ptr to memp to get data address
                        
cache_restore_z         test    save_z_c, #2 wz             'restore the z flag
cache_read_ret
cache_write_ret         ret
