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

	.section .cogregs
	.org 4*0x1F0
PAR	.skip 4
CNT	.skip 4
INA	.skip 4
INB	.skip 4
OUTA	.skip 4
OUTB	.skip 4
DIRA	.skip 4
DIRB	.skip 4
CTRA	.skip 4
CTRB	.skip 4
FRQA	.skip 4
FRQB	.skip 4
PHSA	.skip 4
PHSB	.skip 4
VCFG	.skip 4
VSCL	.skip 4
