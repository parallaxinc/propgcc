	''
	'' some definitions for the standard
	'' COG registers
	''
#ifdef __PROPELLER2__

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
#else

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

#endif
