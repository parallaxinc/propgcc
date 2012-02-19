TARGET = spinsim

SOURCES = spinsim.c spininterp.c spindebug.c pasmsim.c pasmdebug.c pasmsim2.c pasmdebug2.c conion.c
OBJECTS = $(SOURCES:.c=.o)

CC = gcc
LDFLAGS = -Wl,--relax -Wl,--gc-sections
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
