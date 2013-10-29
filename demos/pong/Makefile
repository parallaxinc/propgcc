#
# Makefile for simple vga demo
# Written by Eric R. Smith, ersmith@totalspectrum.ca
# and placed in the public domain.
#

CC = propeller-elf-gcc
OBJCOPY = propeller-elf-objcopy
CFLAGS = -Os

all: pong.elf

pong.elf: pong.c
	$(CC) -mcog -Os -o $@ $^

clean:
	rm -rf *.o *.cog *.elf
