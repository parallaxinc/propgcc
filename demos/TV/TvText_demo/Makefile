# #########################################################
# this makefile allows building and cleaning the TvDemo
#
# Copyright (c) 2011 Steve Denson
# All rights MIT licensed
# #########################################################

CC = propeller-elf-gcc
OBJCOPY = propeller-elf-objcopy
BSTC = bstc

# flags to use for building
CFLAGS = -Os

#
# objects for this program
#

NAME = TvDemo
OBJS = TvDemo.o TvText.o TV_firmware.o TvDriver.o

SPINDIR = .

all: $(NAME).elf

clean:
	rm -rf *.o *.elf

$(NAME).elf: $(OBJS)
	$(CC) -o $@ $(OBJS)

%.dat: $(SPINDIR)/%.spin
	$(BSTC) -Ox -c -o $(basename $@) $<

%_firmware.o: %.dat
	$(OBJCOPY) -I binary -B propeller -O $(CC) $< $@
	rm -rf $<
