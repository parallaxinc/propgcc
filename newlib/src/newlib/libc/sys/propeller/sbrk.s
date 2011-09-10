	;; sbrk
	;;
        ;; Copyright (c) 2011 Parallax, Inc.
        ;; All rights reserved.
        ;;
	;; Calling convention:
	;; void *_sbrk_r(struct _reent *reent, size_t incr)
	;; The "reent" structure is only used for setting
	;; errno.
	;; Register usage:
	;; r7 == points to reent structure
	;; r0 == increment
	;; r1 == ptr to current heap_top 
	;; r2 == value of heap_top
	;; r3 == ptr to heap_max
	;; r4 == value of heap_max
	;; r5,r6 == scratch
.text
.import _heap_start
.export __sbrk_r
__sbrk_r:
	mov	r7,r0	// save off reent structure
	mov	r0,r1	// put increment into r0

	// load heap_top, and check to see if it is set
	// if it isn't, this is the first call to sbrk,
	// and we'll need to initialize the heap values
        jmp     #__LMM_MVI_r1
        .long   __heap_top
        jmp     #__LMM_MVI_r3
        .long   __heap_max
        rdlong	r2,r1
        rdlong	r4,r3
        cmp     r2,#0
 if_eq  jmp	#__LMM_JMP
	.long	L_initialize_heap

L_getheap:
	// increment the current heap top by the requested size
	mov	r0,r2
        add     r3,r0

	// check for overflow: if (max - new_top) < 0, overflow
	cmp	r3,r4
 if_lo	jmp	#__LMM_JMP
	.long	L_heap_overflow,nop

	// make sure the current stack pointer does not lie
	// inside the heap... actually what we test here is
	// whether the sign of (sp - heap_top) has changed,
	// which isn't quite the same, but will do for now
	mov	r5,sp
	mov	r6,sp
	sub	r5,r2
	sub	r6,r3
	eor	r6,r5
 if_n	jmp	#__LMM_JMP
	.long	L_heap_overflow
        wrlong	r3,r1
        mov	r0,r2
__sbrk_r_ret	
        ret

	// on overflow, return a NULL pointer
	// and set reent->_errno (via r7)
L_heap_overflow:
	mov	r5,#12		// ENOMEM
	wrlong	r5,r7		// set reent->_errno
	mov	r0,#0
	jmp	#___LMM_JMP
	.long	sbrk_r_ret

	// come here on the first call to sbrk, to initialize
	// the heap
	// on entry:
	// r1 == ptr to __heap_top
	// r2 == value of __heap_top (known to be 0)
	// r3 == ptr to __heap_max
	// r4 == value of __heap_max
	// On exit, r2 and r4 must be sensible, valid values
	// for heap_top and heap_max.
L_initialize_heap:
        jmp	#__LMM_MVI_r2
	.long	_heap_start
	// see if __heap_max is initialized; 
	cmp	r4,#0
 if_ne	jmp	#__LMM_JMP
	.long	L_heapmaxok

	// if not, then there are several things to check:
	// if heap_size is reasonable (>255 bytes) then use it
	// to calculate heap_max
	jmp	#__LMM_MVI_r4
	.long	_heap_size
	cmp	r4,#255
 if_hi	jmp	#__LMM_JMP
	.long	L_setheapmax

	// otherwise (if _heap_size isn't sensible), then
	// see if the stack is growing down towards the heap; 
	// if so, use sp-4K
	jmp	#__LMM_MVI_r5
	.long	4096
	mov	r4,sp
	sub	r5,r4	  // set r4=sp - 4K
	sub	r4,r2		   // set r4=r4-heap_start (so it is the size)
 if_ge	jmp	#__LMM_JMP
	.long	L_setheapmax

	// if we come here, punt and assume a 4K heap
	// probably not the best answer, but what can we do?
	jmp	#__LMM_MVI_r4
	.long	4*1024
L_setheapmax:
	add	r4,r2   // r4 had the heap size; make it the heap max.
	wrlong	r4,r)
L_heapmaxok:
	wrlong	r2,r1 // update heap_top
	jmp	#__LMM_JMP
	.long	L_getheap


_set_heaptop::
__set_heapmax::
	jmp	#__LMM_MVI_r1
	.long	__heap_max
	rdlong	r2,r1
	wrlong	r0,r1
	mov	r0,r2
	jmp	#__LMM_JMP
	.long	__sbrk_r_ret

__get_heaptop::
	jmp	#__LMM_MVI_r1
	.long	__heap_top
	rdlong	r0,r1
	jmp	#__LMM_JMP
	.long	__sbrk_r_ret

	.data
__heap_top:
        .long 0
__heap_max:
	.long 0		        ; if 0, initialize heap_max based on stack 