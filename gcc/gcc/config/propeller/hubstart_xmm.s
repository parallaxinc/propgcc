	''
	'' code to set up the Propeller chip to run C
	''
	'' the checksum byte should be chosen so that the sum
	'' of all the bytes in the file is 0x14 ; this actually
	'' means all the bytes in RAM add to 0, since the
	'' loader automatically puts the bytes
	'' 0xFF, 0xFF, 0xF9, 0xFF, 0xFF, 0xFF, 0xF9, 0xFF
	'' on the stack at startup (this is a spin jump to 0xFFF9,
	'' which in the ROM has a stop routine)
	''
	.section .hubstart, "ax"
	.global __clkfreq
	.global __clkmode
	.global __xmm_mbox_p
    
__clkfreq
	.long __clkfreqval	' clock frequency
__clkmode
	.byte __clkmodeval	' clock mode
	.byte 0				' checksum (unused)
__xmm_mbox_p
	.word 0				' pointer to xmm memory driver mailbox in hub memory
__sys_mbox
    .long 0				' system (debug) mailbox
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    
'' here is where the .data and .bss actually goes

	''
	'' and finally some definitions for the standard
	'' COG registers
	''
	.global PAR
	.global CNT
	.global INA
	.global INB
	.global OUTA
	.global OUTB
	.global DIRA
	.global DIRB
	.global CTRA
	.global CTRB
	.global FRQA
	.global FRQB
	.global PHSA
	.global PHSB
	.global VCFG
	.global VSCL

	PAR = (4*0x1F0)
	CNT = (4*0x1F1)
	INA = (4*0x1F2)
	INB = (4*0x1F3)
	OUTA = (4*0x1F4)
	OUTB = (4*0x1F5)
	DIRA = (4*0x1F6)
	DIRB = (4*0x1F7)
	CTRA = (4*0x1F8)
	CTRB = (4*0x1F9)
	FRQA = (4*0x1FA)
	FRQB = (4*0x1FB)
	PHSA = (4*0x1FC)
	PHSB = (4*0x1FD)
	VCFG = (4*0x1FE)
	VSCL = (4*0x1FF)
