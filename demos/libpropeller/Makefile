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
COBJS = stdio.o serial.o clock.o printf.o puts.o f_printf.o \
	strcat.o strncat.o strcmp.o strncmp.o strcpy.o strcpy.o strncpy.o \
	strlen.o memcpy.o malloc.o sbrk.o \
	exit.o sleep.o \
	s_modf.o

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
