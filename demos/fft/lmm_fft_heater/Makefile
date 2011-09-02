# #########################################################
# This makefile builds the fft_bench program
#   
# Copyright (c) 2011 Steve Denson
# All rights MIT licensed
# #########################################################

# where we installed the propeller binaries and libraries
PREFIX = /usr/local/propeller
# libgcc directory
LIBGCC = $(PREFIX)/lib/gcc/propeller-elf/4.6.1

STARTOBJS = -L$(LIBGCC) $(LIBGCC)/spinboot.o $(LIBGCC)/crt0_lmm.o $(LIBGCC)/crtbegin_lmm.o
ENDOBJS = -lgcc $(LIBGCC)/crtend_lmm.o

# Current LIBRARY and INCL
PROPLIB = ../../libpropeller
CFLAGS = -I../../include

# basic gnu tools
CC = propeller-elf-gcc $(CFLAGS)
LD = propeller-elf-ld
OBJCOPY = propeller-elf-objcopy

# BSTC program
BSTC=bstc -ls
SPINDIR=.
ECHO=echo

# simulator command line; replace with appropriate
# binary loader to run on real hardware
SPINSIM = spinsim -b115200

# checksum program; replace with 'echo' if you don't
# have this, and don't care about running on real hardware
CHKSUM = propeller-checksum

#
# objects for this program
#

NAME = fft_bench
OBJS = fft_bench.o $(PROPLIB)/clock.o $(PROPLIB)/printf.o

all: $(NAME).binary

$(NAME).binary: $(NAME).elf
	$(OBJCOPY) -O binary $< $@
	$(CHKSUM) $@

$(NAME).elf: $(PROPLIB)/propeller_lmm.script $(OBJS)
	$(LD) -o $@ -T $(PROPLIB)/propeller_lmm.script $(STARTOBJS) $^ $(ENDOBJS)

%.o: %.c
	$(CC) -Os -mlmm -o $@ -c $<

%.o: %.s
	$(CC) -o $@ -c $<

%.dat: $(SPINDIR)/%.spin
	$(BSTC) -Ox -c -o $(basename $@) $<
	$(ECHO) $@

%_firmware.o: %.dat
	propeller-elf-objcopy -I binary -B propeller -O propeller-elf-gcc $< $@
	rm -rf $<
	$(ECHO) $@

clean:
	rm -f *.o *.elf *.binary


run: $(NAME).binary
	$(SPINSIM) $(NAME).binary
