# basic gnu tools
CC = propeller-elf-gcc
LD = propeller-elf-ld
OBJCOPY = propeller-elf-objcopy

# simulator command line; replace with appropriate
# binary loader to run on real hardware
#LOAD = spinsim -b115200
LOAD = propeller-load -I../../libpropeller -p1 -b c3

#
# objects for this program
#

NAME = fibo
OBJS = fibo.o


$(NAME).elf: $(OBJS)
	$(CC) -mcog -o $@ $^

%.o: %.c
	$(CC) -mcog -Os -o $@ -c $<

%.o: %.s
	$(CC) -o $@ -c $<

clean:
	rm -f *.o *.elf *.binary


run: $(NAME).elf
	$(LOAD) $(NAME).elf -r -t
