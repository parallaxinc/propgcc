#
# Makefile for Propeller C library
#
PREFIX = /usr/local/propeller
CFLAGS = -Os -nostdinc -I./include
AR = propeller-elf-ar
CC = propeller-elf-gcc

VPATH=.:misc:stdio:stdlib:string:sys:sys/propeller:time:drivers

all: libc.a

STDIO = clearerr.o feof.o ferror.o fflush.o \
	fgetc.o fputc.o fputs.o puts.o \
	ungetc.o \
	fopen.o \
	simple_printf.o \
	default_io.o

MISC = malloc.o calloc.o sbrk.o ctype.o atoi.o errno.o atexit.o

STRING = memcpy.o memmove.o memset.o \
	strcat.o strcmp.o strcpy.o strlen.o \
	strncat.o strncmp.o strncpy.o

TIME = clock.o 

DRIVERS = SimpleSerial.o

OBJS = $(STDIO) $(MISC) $(STRING) $(TIME) $(DRIVERS)

libc.a: $(OBJS)
	$(AR) rs libc.a $(OBJS)

hello.elf: hello.c
	$(CC) -o hello.elf hello.c

install: libc.a
	mkdir -p $(PREFIX)/propeller-elf/lib
	cp -r include $(PREFIX)/propeller-elf
	cp libc.a $(PREFIX)/propeller-elf/lib

clean:
	rm -f *.a *.o
