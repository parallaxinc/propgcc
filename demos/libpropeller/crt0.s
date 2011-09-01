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
start
	.long 80000000		' clock frequency
	.byte 0x6f		' clock mode
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
	