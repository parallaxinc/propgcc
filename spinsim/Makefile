# CC, EXT, and BUILD may be overridden by a top level makefile (e.g. for
# cross compiling)
CC = gcc
EXT = 
BUILD = ./obj

TARGET = spinsim$(EXT)

SOURCES = spinsim.c spininterp.c spindebug.c pasmsim.c pasmdebug.c pasmsim2.c pasmdebug2.c eeprom.c debug.c gdb.c

ifneq ($(OS),msys)
SOURCES += conion.c
endif

OBJECTS = $(patsubst %,$(BUILD)/%, $(SOURCES:.c=.o))

# I'm not sure why these linker flags were being used but the break the build on Mac OS X so I've
# commented them out for the time being
#LDFLAGS = -Wl,--relax -Wl,--gc-sections
LDFLAGS =
OPT := -O3
CFLAGS  = -c -g -Wall -Wno-format $(OPT) -D LINUX

all: directory $(SOURCES) $(OBJECTS) Makefile
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

directory:
	mkdir -p $(BUILD)

# Compile .c files into objexts .o
$(BUILD)/%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean: FORCE
	rm -f *.o $(TARGET)
FORCE:
