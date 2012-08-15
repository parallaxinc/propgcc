	''
	'' atomic read/write operations for 64 bit variables
	'' these work by always reading or writing the low value
	'' and then the high in 2 consecutive hub slots ; if all
	'' users follow this protocol, then everyone will see
	'' consistent values
	''
	'' the pointers must point to hub memory, and we have to
	'' run the code from inside the LMM or XMM kernel in order
	'' to ensure the timing of the accesses
	'' rather than keeping these (rarely used) functions permanently
	'' in the kernel, we put them in the FCACHE area when we
	'' need them
	''
	
	.section .text
	.global __getAtomic64
	.global	__putAtomic64

	''
	'' __getAtomic64
	'' input: r0 == pointer to 64 bit quantity
	'' output: r0 == low word, r1 == high word, read in
	''   consecutive hub cycles
	''
	
__getAtomic64
	fcache	#.endfunc - .func
	.compress off
.func
	mov	r2,r0
	rdlong	r0,r2
	add	r2,#4
	rdlong	r1,r2
	jmp	__LMM_RET
.endfunc
	.compress default
	mov	pc,lr
	
	''
	'' __putAtomic64
	'' input: r0,r1 == high,low of 64 bit quantity
	''        r2 == pointer to memory
	'' output: 
	''   memory updated in consecutive hub cycles
	''
	
__putAtomic64
	fcache	#.endputfunc - .putfunc
	.compress off
.putfunc
	wrlong	r0,r2
	add	r2,#4
	wrlong	r1,r2
	jmp	__LMM_RET
.endputfunc
	.compress default
	mov	pc,lr
