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

	'' These variables used to be .global, but are now defined in
	'' the linker script. This is because the .boot section is
	'' discarded when doing a relocatable link (which is how we
	'' often build COGC programs), which will cause a program using
	'' these symbols from the .boot sections to get a linker error.
	'' Defining them in the linker script avoids the issue.
	
	''.global __clkfreq
	''.global __clkmode
	.extern __hub_end
	.extern __stack_end
start
__clkfreq
	.long __clkfreqval	' clock frequency
__clkmode
	.byte __clkmodeval	' clock mode
chksum	.byte 0x00		' checksum: see above

	.word 0x0010		' PBASE
	.global __sys_mbox	' added 20/09/11 for debugger/system support (WH)
__sys_mbox
'	.word 0x7fe8		' VBASE - start of variables
	.word __hub_end		' VBASE - start of variables
'	.word 0x7ff0		' DBASE - start of stack 
	.word __hub_end+8	' DBASE - start of stack 
	.word 0x0018		' PCURR - initial program counter
'	.word 0x7ff8		' DCURR - initial stack pointer
	.word __hub_end+12	' DCURR - initial stack pointer
pbase
	'' 8 byte header
	.word 0x0008		' length of object ?
	.byte 0x02		' number of methods ?
	.byte 0x00		' number of objects ?
	.word 0x0008		' pcurr - 0x10?
	.word __stack_end

	'' here is the spin code to switch to pasm mode
	'' removed a load of the constant 0 and replaced it with cogid
	.byte 0x3F		' Register op $1E9 Read - cogid
	.byte 0x89
	.byte 0xc7		' memory op: push PBASE + next byte
	.byte 0x10
	.byte 0xA4		' load PAR with Stack address
	.byte 6
	.byte 0x2C		' CogInit(Id, Addr, Ptr)
	.byte 0x32		' Return (unused)

'' here is where the pasm code actually goes

	.section .bootpasm, "ax", @progbits
	.compress off
	mov	param, PAR
	shl	param,#16
	shl	addr,#2
	or	param,addr
	cogid	addr
	or	param,addr
	coginit	param
	cogstop	addr
	
param
	long 0
addr
	long __load_start_kernel
