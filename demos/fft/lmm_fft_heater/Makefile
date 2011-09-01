# #########################################################
# This makefile builds the fft_bench program
#   
# Copyright (c) 2011 Steve Denson
# All rights MIT licensed
# #########################################################

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

$(NAME).elf: $(PROPLIB)/propeller_lmm.script $(PROPLIB)/crt0.o $(PROPLIB)/crtbegin.o $(PROPLIB)/lmm.o $(OBJS) $(PROPLIB)/cogregs.o $(PROPLIB)/crtend.o
	$(LD) -o $@ -T $(PROPLIB)/propeller_lmm.script $^

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
