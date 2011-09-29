# basic gnu tools
CC = propeller-elf-gcc
LD = propeller-elf-ld
OBJCOPY = propeller-elf-objcopy

# simulator command line; replace with appropriate
# binary loader to run on real hardware
#LOAD = spinsim -b115200
LOAD = propeller-load -I../../libpropeller -p1 -b c3

# checksum program; replace with 'echo' if you don't
# have this, and don't care about running on real hardware
CHKSUM = propeller-checksum

#
# objects for this program
#

NAME = fibo
OBJS = fibo.o miniprintf.o clock.o


$(NAME).elf: propeller_cog.script crt0.o csupport.o $(OBJS) div.o mult.o cogregs.o
	$(LD) -o $@ -T propeller_cog.script $^

%.o: %.c
	$(CC) -mcog -Os -o $@ -c $<

%.o: %.s
	$(CC) -o $@ -c $<

clean:
	rm -f *.o *.elf *.binary


run: $(NAME).elf
	$(LOAD) $(NAME).elf -r -t
