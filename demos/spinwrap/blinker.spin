PUB start
    delta := clkfreq / 2
    cognew(@cog_code, 0)
        
DAT

cog_code
        or      dira, mask
        mov     target, delta
        add     target, cnt
loop    xor     outa, mask
        waitcnt target, delta
        jmp     #loop

mask    long    1 << 23
target  long    0
delta   long    0
