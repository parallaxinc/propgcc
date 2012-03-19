TARGETS=boetest.elf compass.elf

CC=propeller-elf-gcc
RM=rm -f

CFLAGS=-Os -mlmm -Iinclude
CXXFLAGS=$(CFLAGS) -fno-exceptions -fno-rtti
LDFLAGS=-mlmm -Llib

LIBS=-lboe

all:	$(TARGETS)

%.elf:	%.cpp lib/libboe.a
	$(CC) $(CXXFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)
	
boelib:
	$(MAKE) -C libsrc

run:	$(TARGET)
	propeller-load -b propboe $< -r -t

clean:
	$(RM) $(TARGETS)

clean-all:	clean
	$(MAKE) -C libsrc clean
	$(RM) -r lib
