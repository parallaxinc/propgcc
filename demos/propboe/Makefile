TARGET=compass.elf

CC=propeller-elf-gcc
CXX=propeller-elf-gcc
RM=rm -f

CFLAGS=-Os -Wall -mlmm -Iinclude
LDFLAGS=-mlmm -Llib

LIBS=-lprop

all:	libboe $(TARGET)

%.elf:	%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)
	
libboe:
	$(MAKE) -C libsrc

run:	$(TARGET)
	propeller-load -b propboe $< -r -t

clean:
	$(RM) $(TARGET)

clean-all:	clean
	$(MAKE) -C libsrc clean
	$(RM) -r lib
