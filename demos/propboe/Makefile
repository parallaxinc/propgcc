TARGET=propboe.elf

CC=propeller-elf-gcc
RM=rm -f

CFLAGS=-Os -mlmm -Iinclude
LDFLAGS=-mlmm -Llib

LIBS=-lboe

$(TARGET):	boetest.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)

run:	$(TARGET)
	propeller-load -b propboe $< -r -t

clean:
	$(RM) $(TARGET)
