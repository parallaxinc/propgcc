MODEL = cmm

expr.elf:	Makefile expr.c
	propeller-elf-gcc -m$(MODEL) -Wall -Os -m32bit-doubles -DMAIN -o expr.elf expr.c -lm

run:
	propeller-load expr.elf -r -t

clean:
	rm -rf expr.elf
