MODEL = cmm

CFLAGS = -Wall -Os -m$(MODEL) -m32bit-doubles

expr.elf:	Makefile expr.c
	propeller-elf-gcc $(CFLAGS) -DMAIN -o expr.elf expr.c -lm

run:
	propeller-load expr.elf -r -t

clean:
	rm -rf expr.elf
