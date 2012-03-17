TARGET = spinsim

SOURCES = spinsim.c spininterp.c spindebug.c pasmsim.c pasmdebug.c pasmsim2.c pasmdebug2.c
OBJECTS = $(SOURCES:.c=.o)

ifneq ($(OS),msys)
SOURCES += conion.c
endif

CC = gcc
# I'm not sure why these linker flags were being used but the break the build on Mac OS X so I've
# commented them out for the time being
#LDFLAGS = -Wl,--relax -Wl,--gc-sections
LDFLAGS =
OPT := -O3
CFLAGS  = -c -g -Wall -Wno-format $(OPT) -I/usr/include -D LINUX

all: $(SOURCES) $(OBJECTS) Makefile
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

# Compile .c files into objexts .o
.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean: FORCE
	rm -f *.o $(TARGET)
FORCE:
