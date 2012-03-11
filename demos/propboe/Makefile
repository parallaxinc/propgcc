TARGET=propboe.elf

CC=propeller-elf-gcc
RM=rm -f

CFLAGS=-Os -mlmm -Iinclude
LDFLAGS=-mlmm -Llib

LIBS=-lboe

$(TARGET):	boetest.cpp boelib
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS) -fno-exceptions -fno-rtti
	
boelib:
	$(MAKE) -C libsrc

run:	$(TARGET)
	propeller-load -b propboe $< -r -t

clean:
	$(RM) $(TARGET)
	$(MAKE) -C libsrc clean
	$(RM) -r lib
