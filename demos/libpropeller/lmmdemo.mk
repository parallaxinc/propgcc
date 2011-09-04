# #########################################################
# This makefile fragment builds LMM demo programs
#   
# To use it, define:
#  PROPLIB to be the path to this directory
#  NAME to be the name of project
#       - this is used to create the final program $(NAME).elf
#  OBJS to be the object files needed for the project
#  CFLAGS to be desired CFLAGS
#
#  Then set up a default "all" target (normally this will be
#    all: $(NAME).elf
#  and finally
#    include $(PROPLIB)/lmmdemo.mk
#
# Copyright (c) 2011 Parallax Inc.
# All rights MIT licensed
# #########################################################

# where we installed the propeller binaries and libraries
PREFIX = /usr/local/propeller
# libgcc directory
LIBGCC = $(PREFIX)/lib/gcc/propeller-elf/4.6.1

STARTOBJS = -L$(LIBGCC) $(LIBGCC)/spinboot.o $(LIBGCC)/crt0_lmm.o $(LIBGCC)/crtbegin_lmm.o
ENDOBJS = -lgcc $(LIBGCC)/crtend_lmm.o

# Current LIBRARY and INCL
PROPINC = $(PROPLIB)/../include

CFLAGS += -I$(PROPINC)
LDSCRIPT = $(PROPLIB)/propeller_lmm.script

# basic gnu tools
CC = propeller-elf-gcc
LD = propeller-elf-ld
OBJCOPY = propeller-elf-objcopy
CHKSUM = propeller-checksum
LOADER = propeller-load -p1 -b c3

# BSTC program
BSTC=bstc -ls
SPINDIR=.
ECHO=echo

# C library to use
LIBC = $(PROPLIB)/libc.a

$(NAME).elf: $(LDSCRIPT) $(OBJS) $(LIBC)
	$(LD) -o $@ -T $(LDSCRIPT) $(STARTOBJS) $^ $(ENDOBJS)

%.o: %.c
	$(CC) $(CFLAGS) -Os -mlmm -o $@ -c $<

%.o: %.s
	$(CC) -o $@ -c $<

%.binary: %.elf
	$(OBJCOPY) -O binary $< $@
	$(CHKSUM) $@

%.dat: $(SPINDIR)/%.spin
	$(BSTC) -Ox -c -o $(basename $@) $<
	$(ECHO) $@

%_firmware.o: %.dat
	$(OBJCOPY) -I binary -B propeller -O $(CC) $< $@
	rm -rf $<
	$(ECHO) $@

clean:
	rm -f *.o *.elf *.binary

#
# we make both libraries together
#
$(LIBC) $(PROPLIB)/libpropeller.a:
	make -C $(PROPLIB)

#
# how to run
#
run: $(NAME).elf
	$(LOADER) -I$(PROPLIB) $(NAME).elf -r -t
