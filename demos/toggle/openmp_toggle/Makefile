#
# Makefile for OpenMP toggle demo program
#
# builds toggle.elf (single cog version) and toggle_omp.elf (multiple cog version)
#

MODEL=lmm
CC=propeller-elf-gcc
CFLAGS=-Os -m$(MODEL)

all: toggle.elf toggle_omp.elf

toggle.elf: toggle.c
	$(CC) $(CFLAGS) -o $@ $^

toggle_omp.elf: toggle.c
	$(CC) -fopenmp $(CFLAGS) -o $@ $^

clean:
	rm -f *.o *.elf
