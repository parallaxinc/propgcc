pub start(pinptr)
    cognew(@pasm, pinptr)

dat             org 0

pasm           'rdlong  pins, par
                mov     dira, pins
		mov	nextcnt, cnt
		add	nextcnt, waitdelay
:loop
                xor     outa, pins
		waitcnt	nextcnt, waitdelay
                jmp     #:loop

pins            long    $3fffffff
waitdelay	long	40000000
nextcnt		long	0