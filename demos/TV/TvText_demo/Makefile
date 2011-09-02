# #########################################################
# this makefile allows building and cleaning the TvDemo
#
# Copyright (c) 2011 Steve Denson
# All rights MIT licensed
# #########################################################

# where we have located things
PREFIX = /usr/local/propeller
# libgcc directory
LIBGCC = $(PREFIX)/lib/gcc/propeller-elf/4.6.1

STARTOBJS =  $(LIBGCC)/spinboot.o $(LIBGCC)/crt0_lmm.o $(LIBGCC)/crtbegin_lmm.o
ENDOBJS = -lgcc $(LIBGCC)/crtend_lmm.o

# Propeller Demo include
LIBNAME = libpropeller
PROPLIB = ../../$(LIBNAME)
PROPINC = ../../include

CFLAGS = -I$(PROPINC)

# basic gnu tools
CC = propeller-elf-gcc
LD = propeller-elf-ld
OBJCOPY = propeller-elf-objcopy
LDSCRIPT = $(PROPLIB)/propeller_lmm.script

# BSTC program
BSTC=bstc
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

NAME = TvDemo
OBJS = TvDemo.o TvText.o TV_firmware.o \
	$(PROPLIB)/clock.o $(PROPLIB)/printf.o $(PROPLIB)/propeller.o

all: $(NAME).binary

$(NAME).binary: $(NAME).elf
	$(OBJCOPY) -O binary $< $@
	$(CHKSUM) $@

$(NAME).elf: $(LDSCRIPT) $(OBJS)
	$(LD) -o $@ -T $(LDSCRIPT) $(STARTOBJS) $^ $(ENDOBJS)

%.o: %.c
	$(CC) $(CFLAGS) -Os -mlmm -o $@ -c $<

%.o: %.s
	$(CC) $(CFLAGS) -o $@ -c $<

%.dat: $(SPINDIR)/%.spin
	$(BSTC) -Ox -c -o $(basename $@) $<
	$(ECHO) $@

%_firmware.o: %.dat
	$(OBJCOPY) -I binary -B propeller -O $(CC) $< $@
	rm -rf $<
	$(ECHO) $@

clean:
	rm -f *.o *.elf *.binary *.list

run: $(NAME).binary
	$(SPINSIM) $(NAME).binary
