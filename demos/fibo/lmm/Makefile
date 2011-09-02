# where we have located things
PREFIX = /usr/local/propeller
# libgcc directory
LIBGCC = $(PREFIX)/lib/gcc/propeller-elf/4.6.1

STARTOBJS =  $(LIBGCC)/spinboot.o $(LIBGCC)/crt0_lmm.o $(LIBGCC)/crtbegin_lmm.o
ENDOBJS = -lgcc $(LIBGCC)/crtend_lmm.o

# basic gnu tools
CC = propeller-elf-gcc
LD = propeller-elf-ld
OBJCOPY = propeller-elf-objcopy

# simulator command line; replace with appropriate
# binary loader to run on real hardware
SPINSIM = spinsim -b115200

# checksum program; replace with 'echo' if you don't
# have this, and don't care about running on real hardware
CHKSUM = propeller-checksum

#
# objects for this program
#

NAME = fibo
OBJS = fibo.o printf.o clock.o

$(NAME).binary: $(NAME).elf
	$(OBJCOPY) -O binary $< $@
	$(CHKSUM) $@

$(NAME).elf: propeller_lmm.script $(OBJS)
	$(LD) -o $@ -T propeller_lmm.script $(STARTOBJS) $^ $(ENDOBJS)

%.o: %.c
	$(CC) -Os -mlmm -o $@ -c $<

%.o: %.s
	$(CC) -o $@ -c $<

clean:
	rm -f *.o *.elf *.binary


run: $(NAME).binary
	$(SPINSIM) $(NAME).binary

