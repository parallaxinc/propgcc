# Propeller Demo include
PROPINC = ../include

ifndef MODEL
MODEL=lmm
endif

CFLAGS = -Os -m$(MODEL) -I$(PROPINC)

# basic gnu tools
CC = propeller-elf-gcc
LD = propeller-elf-ld
AR = propeller-elf-ar
AS = propeller-elf-as
OBJCOPY = propeller-elf-objcopy

COBJS = stdio.o serial.o clock.o printf.o puts.o f_printf.o \
	strcat.o strncat.o strcmp.o strncmp.o strcpy.o strcpy.o strncpy.o \
	strlen.o memcpy.o malloc.o memset.o sbrk.o \
	exit.o sleep.o \

COGOBJS = udiv.o div.o mult.o mask.o cogregs.o

all: libc.a crt0_cog.o libcog.a

libc.a: $(COBJS)
	$(AR) rcs $@ $^

libcog.a: $(COGOBJS)
	$(AR) rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.s
	$(CC) -o $@ -c $<

f_printf.o: printf.c
	$(CC) $(CFLAGS) -DFLOAT_SUPPORT -o $@ -c $<

clean:
	rm -f *.o *.elf *.binary *.list *.a
