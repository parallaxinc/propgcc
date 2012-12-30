'--------------------------------------------------------------------
' LMM debug kernel
'--------------------------------------------------------------------

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

	.set ACK, 6

	.global __LMM_entry
__LMM_entry

r0	mov	sp, PAR
r1	mov	r0, sp
r2	cmp	sp,r14	wz	' see if stack is at top of memory
r3 if_e	jmp	#new_lock
r4      rdlong 	pc,sp		' if user stack, pop the pc	'' initialization for non-primary cogs
r5      add	sp,#4
r6      rdlong 	r0,sp		' pop the argument for the function
r7	add	sp,#4
r8      rdlong 	__TLS,sp	' and the _TLS variable
r9      add	sp,#4
done_lock
r10	mov	dira,txbit	' enable tx and Quickstart led's for debugging
r11	mov	outa,txbit	' make sure tx is high
r12	jmp	#__LMM_debug
r13	long	$1F0-((_AFTER_CACHE-__LMM_entry)/4) ' free longs in cog0
r14	long	0x00008000
r15		' alias for link register lr
lr	long	__exit
sp	long	0
pc	long	entry		' default pc
flags	long	0
breakpt	long	0

'--------------------------------------------------------------------
' LMM debugger entry - as there are 20 registers above, a simple
' "jmp #20" is our breakpoint primitive
'--------------------------------------------------------------------

__LMM_debug
	muxnz	flags,#2	' save zero flag
	muxc	flags,#1	' save Carry flag

	mov ch,#$21 ' "!"	
	call #txbyte		' comment this line out when testing with kdbg.c

wt1	call	#rxbyte
	cmp	ch,#1 wz
 if_nz	jmp	#wt1

	mov	ch,#$40
	call	#txbyte

'--------------------------------------------------------------------
' Debug command dispatcher
'--------------------------------------------------------------------

	call	#dbg_dump

	call	#rxbyte

	cmp	ch,#1 wz	' single step
 if_z	jmp	#dbg_STEP

	cmp	ch,#2 wz	' run
 if_z	jmp	#debug_resume

	cmp	ch,#3 wz	' read bytes
 if_z	call	#tx_packet '#read_bytes

	cmp	ch,#4 wz	' write bytes
 if_z	call	#rx_packet '#write_bytes

	call	#dbg_load

	jmp	#__LMM_debug

'--------------------------------------------------------------------
' packet xmit/rcv control variables, also new_lock at startup
'--------------------------------------------------------------------

new_lock

chksum		locknew	__TMP0 wc	' allocate a lock - initialization for first time run
ix 	IF_NC 	wrlong __TMP0, __C_LOCK_PTR	' save it to ram if successful
len	jmp	#done_lock

'--------------------------------------------------------------------
' dbg_STEP
'--------------------------------------------------------------------

dbg_STEP
	mova	L_target, #dbg_STEP_finish
	jmp	#debug_resume
    
dbg_STEP_finish
	mova	L_target, #__LMM_loop
	jmp	#__LMM_debug
	
'--------------------------------------------------------------------
' LMM virtual machine 
'--------------------------------------------------------------------

__LMM_loop

	muxnz	flags,#2	' save zero flag
	muxc	flags,#1	' save Carry flag

	and	rxbit,ina nr,wz	' check for low on RX

	if_z	jmp	#__LMM_debug

debug_resume
	shr	flags,#1 nr,wc,wz	' restore flags, continue

'--------------------------------------------------------------------
' fetch & execute LMM instruction
'--------------------------------------------------------------------

L_cont
	rdlong	L_ins0,pc	' fetch instruction to execute
	add	pc,#4		' update LMM program counter
L_ins0	nop			' execute instruction

L_jump
	jmp	L_target	' loop
	
L_target
	long	(__LMM_loop - __LMM_entry) / 4

'--------------------------------------------------------------------
' get address and length from serial
'--------------------------------------------------------------------

get_addr_len
	call	#rxbyte
	mov	ix,ch

	call	#rxbyte
	shl	ch,#8
	or	ix,ch

	call	#rxbyte
	mov	len,ch

	mov	chksum,len

get_addr_len_ret
	ret

'--------------------------------------------------------------------
' tx_packet - set up ix&len before calling, sends 0x40,len,dat...,chk
'--------------------------------------------------------------------

tx_packet

	call	#get_addr_len

chrs	rdbyte	ch,ix		' real code
	add	ix,#1
	add	chksum,ch
	call	#txbyte
	djnz	len,#chrs

	mov	ch,chksum
	call	#txbyte

tx_packet_ret
	ret

'--------------------------------------------------------------------
' rx_packet - returns ix&len after receiving 0x40,len,dat...,chk 
'--------------------------------------------------------------------

rx_packet

	call	#get_addr_len

rxchr	call	#rxbyte
	wrbyte	ch,ix		' real code
	add	ix,#1
	add	chksum,ch
	djnz	len,#rxchr

	mov	ch,chksum
	call	#txbyte

	call	#dbg_load

rx_packet_ret
	ret

'--------------------------------------------------------------------
' txbyte	Send one byte using kernel serial code
'--------------------------------------------------------------------

txbyte	or	ch,twostop
	shl	ch,#1
	mov	bits,#11 wz ' 10 for one stop bit, 11 for two stop bits, 22 for one char delay between chars

txloop	shr	ch,#1 wc
	muxc	outa,txbit  ' 22
'-------
	mov	now,CNT
	add	now,onebit
	sub	now,#18	' tuned for 78us for 9 bits (should be 78.125us) so 0.26% too slow
	waitcnt	now,#0
'-------
	djnz	bits,#txloop ' P2 doesn't allow this effect:  wz

txbyte_ret
	ret

twostop	long	0xFFFFFF00 ' allows up to 23 stop bits

' 115200
halfbit	long	347  '80000000 / (2*115200) = 347.2222
onebit		long	694

' 57600
'halfbit	long	694  
'onebit		long	1389

' 38400
'halfbit		long	1041 
'onebit		long	2083

' 19200
'halfbit		long	2083 
'onebit		long	4167

' 9600
'halfbit		long	2083-11 
'onebit		long	4166-16 

'--------------------------------------------------------------------
' debugger control variables
'--------------------------------------------------------------------

start		long	0
now		long	0
bits		long	0
ch		long	0


dbg_image	long	0x20
rxbit		long	0x80000000
txbit		long	0x40000000

'--------------------------------------------------------------------
' rxbyte	Receive byte into ch
'--------------------------------------------------------------------

rxbyte	call	#rx8bits
	shr	ch,#24
rxbyte_ret
	ret

'--------------------------------------------------------------------
' rx8bits	Receive 8 bits
'--------------------------------------------------------------------

rx8bits	mov	bits,#8
	waitpeq	zero,rxbit	' wait for startbit
	mov	now,CNT
	add	now,halfbit
	sub	now,#8
	waitcnt	now,#0

rxloop	mov	now,CNT
	add	now,onebit
	waitcnt	now,#0
	test	rxbit,ina wc
	rcr	ch,#1
	djnz	bits,#rxloop

	mov	now,CNT
	add	now,onebit
	waitcnt	now,#0
rx8bits_ret
	ret

zero	long 	0

'--------------------------------------------------------------------
' dbg_dump - save kernel image over the initial starting kernel
'--------------------------------------------------------------------

dbg_dump	
	mov	start,#0x01EF
	mov	dbg_image, cog_end
	
dbg_lp1
	movd	inx,start
	sub	dbg_image,#4
inx	wrlong	0-0,dbg_image
	djnz	start,#dbg_lp1
	sub	dbg_image,#4
	wrlong	0,dbg_image
dbg_dump_ret
	ret

cog_end long	(0x01F0 << 2)+0x20

'--------------------------------------------------------------------
' dbg_load - load kernel registers from hub kernel image
'--------------------------------------------------------------------

dbg_load
	mov	dbg_image,#$68
	mov	start,#$12
dbg_lp2	
	movd	ixr,start
	nop
ixr	rdlong	0-0,dbg_image
	sub	dbg_image,#4
	djnz	start,#dbg_lp2
dbg_load_ret
	ret

'-----------------------------------------------------------------------
' GCC support code
'-----------------------------------------------------------------------

	''
	'' LMM support functions
	''

	'' move immediate
	.macro LMM_movi reg
	.global __LMM_MVI_\reg
__LMM_MVI_\reg
	rdlong	\reg,pc
	add	pc,#4
	jmp	L_target
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
	.global __LMM_CALL_INDIRECT
__LMM_CALL
	rdlong	__TMP0,pc
	add	pc,#4
__LMM_CALL_INDIRECT
	mov	lr,pc
	mov	pc,__TMP0
	jmp	L_target

	''
	'' direct jmp
	''
	.global __LMM_JMP
__LMM_JMP
	rdlong	pc,pc
	jmp	L_target

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
	mov	__TMP1,__TMP0
	and	__TMP1,#0x0f
	movd	L_pushins,__TMP1
	shr	__TMP0,#4
L_pushloop
	sub	sp,#4
L_pushins
	wrlong	0-0,sp
	add	L_pushins,inc_dest1
	djnz	__TMP0,#L_pushloop
__LMM_PUSHM_ret
	ret

inc_dest1
	long	(1<<9)

	.global __LMM_POPM
	.global __LMM_POPM_ret
	.global __LMM_POPRET
	.global __LMM_POPRET_ret

__LMM_POPRET
	call	#__LMM_POPM
	mov	pc,lr
__LMM_POPRET_ret
	ret
	
__LMM_POPM
	mov	__TMP1,__TMP0
	and	__TMP1,#0x0f
	movd	L_poploop,__TMP1
	shr	__TMP0,#4
L_poploop
	rdlong	0-0,sp
	add	sp,#4
	sub	L_poploop,inc_dest1
	djnz	__TMP0,#L_poploop
__LMM_POPM_ret
	ret

	
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
__TMP1
__DIVCNT
	long	0
	''
	'' calculate r0 = orig_r0/orig_r1, r1 = orig_r0 % orig_r1
	''
__UDIVSI
	mov	__DIVR, r0
	call	#__CLZSI
	neg	__DIVCNT, r0
	mov	r0, r1 wz
 IF_Z   jmp	#__UDIV_BY_ZERO
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
	abs	r0, r0 wc
	muxc	__DIVSGN, #1	' save original sign of r0
	abs	r1, r1
	call	#__UDIVSI
	cmps	__DIVSGN, #0	wz, wc
 IF_B	neg	r0, r0
	test	__DIVSGN, #1 wz	' check original sign of r0
 IF_NZ	neg	r1, r1		' make the modulus result match
__DIVSI_ret	ret

	'' come here on divide by zero
	'' we probably should raise a signal
__UDIV_BY_ZERO
	neg	r0,#1
	mov	r1,#0
	jmp	#__UDIVSI_ret
	
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


	''
	'' code for atomic compare and swap
	''
	.global __C_LOCK_PTR
__C_LOCK_PTR
	long	__C_LOCK

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
	rdlong	__TMP1,__C_LOCK_PTR
	mov	__TMP0,r0	'' save value to set
.swaplp
	lockset	__TMP1 wc
   IF_C jmp	#.swaplp

	rdlong	r0,r2		'' fetch original value
	cmp	r0,r1 wz	'' compare with desired original value
   IF_Z wrlong  __TMP0,r2	'' if match, save new value
	
	'' now release the C lock
	lockclr __TMP1
__CMPSWAPSI_ret
	ret
	
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
	
	'' assembler awkwardness here
	'' we would like to just write
	'' movd	Lmm_fcache_loop,#__LMM_FCACHE_START
	'' but binutils doesn't work right with this now
	movd Lmm_fcache_loop,#(__LMM_FCACHE_START-__LMM_entry)/4
	movd Lmm_fcache_loop2,#1+(__LMM_FCACHE_START-__LMM_entry)/4
	movd Lmm_fcache_loop3,#2+(__LMM_FCACHE_START-__LMM_entry)/4
	movd Lmm_fcache_loop4,#3+(__LMM_FCACHE_START-__LMM_entry)/4
	add  __TMP0,#15		'' round up to next multiple of 16
	shr  __TMP0,#4		'' we process 16 bytes per loop iteration
Lmm_fcache_loop
	rdlong	0-0,__TMP1
	add	__TMP1,#4
	add	Lmm_fcache_loop,inc_dest4
Lmm_fcache_loop2
	rdlong	0-0,__TMP1
	add	__TMP1,#4
	add	Lmm_fcache_loop2,inc_dest4
Lmm_fcache_loop3
	rdlong	0-0,__TMP1
	add	__TMP1,#4
	add	Lmm_fcache_loop3,inc_dest4
Lmm_fcache_loop4
	rdlong	0-0,__TMP1
	add	__TMP1,#4
	add	Lmm_fcache_loop4,inc_dest4

	djnz	__TMP0,#Lmm_fcache_loop

Lmm_fcache_doit
	jmpret	__LMM_RET,#__LMM_FCACHE_START
	jmp	L_target



	''
	'' the fcache area should come last in the file
	''
	.global __LMM_FCACHE_START
__LMM_FCACHE_START
	res	$80	' normally reserve 256 longs
_AFTER_CACHE
	long	_AFTER_CACHE

