TARGETS=boetest.elf compass.elf

CC=propeller-elf-gcc
CXX=propeller-elf-gcc
RM=rm -f

CFLAGS=-Os -Wall -mlmm -Iinclude
CXXFLAGS=$(CFLAGS) -fno-exceptions -fno-rtti
LDFLAGS=-mlmm -Llib

LIBS=-lboe

all:	libboe $(TARGETS)

%.elf:	%.cpp
	$(CC) $(CXXFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)
	
libboe:
	$(MAKE) -C libsrc

run:	$(TARGET).elf
	propeller-load -b propboe $< -r -t

clean:
	$(RM) $(TARGETS)

clean-all:	clean
	$(MAKE) -C libsrc clean
	$(RM) -r lib
