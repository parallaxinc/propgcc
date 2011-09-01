pub start(pinptr)
    cognew(@pasm, pinptr)

dat             org 0

pasm           'rdlong  pins, par
                mov     dira, pins
:loop
                xor     outa, pins
                jmp     #:loop

pins            long    $3fffffff
