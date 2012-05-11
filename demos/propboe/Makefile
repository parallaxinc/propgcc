TARGETS=boetest.elf

CC=propeller-elf-gcc
CXX=propeller-elf-gcc
RM=rm -f

CFLAGS=-Os -Wall -mlmm -Iinclude
LDFLAGS=-mlmm -Llib

LIBS=-lprop

all:	libboe $(TARGETS)

%.elf:	%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)
	
libboe:
	$(MAKE) -C libsrc

run:	$(TARGET).elf
	propeller-load -b propboe $< -r -t

clean:
	$(RM) $(TARGETS)

clean-all:	clean
	$(MAKE) -C libsrc clean
	$(RM) -r lib
