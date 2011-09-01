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
	.section .boot, "ax", @progbits
	.global __clkfreq
	.global __clkmode
start
__clkfreq
	.long __clkfreqval	' clock frequency
__clkmode
	.byte __clkmodeval	' clock mode
chksum	.byte 0x00		' checksum: see above

	.word 0x0010		' PBASE
	.word 0x7fe8		' VBASE - start of variables
	.word 0x7ff0		' DBASE - start of stack 
	.word 0x0018		' PCURR - initial program counter
	.word 0x7ff8		' DCURR - initial stack pointer
pbase
	'' 8 byte header
	.word 0x0008		' length of object ?
	.byte 0x02		' number of methods ?
	.byte 0x00		' number of objects ?
	.word 0x0008		' pcurr - 0x10?
	.word 0			' ??

	'' here is the spin code to switch to pasm mode
	.byte 0x35		' constant 1 $00000000 (id)
	.byte 0xc7		' memory op: push PBASE + next byte
	.byte 0x10
	.byte 0x37		' constant mask Y= 14 0x00008000
	.byte 14
	.byte 0x2C		' CogInit(Id, Addr, Ptr)
	.byte 0x32		' Return (unused)
	.byte 0x00		' padding

'' here is where the pasm code actually goes

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
