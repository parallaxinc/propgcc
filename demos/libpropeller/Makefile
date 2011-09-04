# Propeller Demo include
PROPLIB = ../libpropeller
PROPINC = ../include

CFLAGS = -Os -mlmm -I$(PROPINC)

# basic gnu tools
CC = propeller-elf-gcc $(CFLAGS)
LD = propeller-elf-ld
AR = propeller-elf-ar
AS = propeller-elf-as
OBJCOPY = propeller-elf-objcopy

POBJS = propeller.o
COBJS = stdio.o serial.o clock.o printf.o

all: libpropeller.a libc.a

libpropeller.a: $(POBJS)
	$(AR) rcs $@ $^

libc.a: $(COBJS)
	$(AR) rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.s
	$(CC) -o $@ -c $<


clean:
	rm -f *.o *.elf *.binary *.list *.a
