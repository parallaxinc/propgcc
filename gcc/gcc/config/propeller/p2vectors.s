        .section .boot, "ax"
__bootrom
        .space 0x0e80
__vectors
        .global __clkfreq
__clkfreq
        .long __clkfreqval      ' clock frequency
        .space 0x1000 - 0x0e84
__cogimage

        ''
        '' and finally some definitions for the standard
        '' COG registers
        ''
        .global PINA
        .global PINB
        .global PINC
        .global PIND
        .global DIRA
        .global DIRB
        .global DIRC
        .global DIRD

        PINA = (4*0x1F8)
        PINB = (4*0x1F9)
        PINC = (4*0x1FA)
        PIND = (4*0x1FB)
        DIRA = (4*0x1FC)
        DIRB = (4*0x1FD)
        DIRC = (4*0x1FE)
        DIRD = (4*0x1FF)
