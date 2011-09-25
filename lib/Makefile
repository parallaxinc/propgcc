#
# Makefile for Propeller C library
#
PREFIX = /usr/local/propeller
CFLAGS = -Os -nostdinc -I./include
AR = propeller-elf-ar
CC = propeller-elf-gcc
MKDIR = mkdir -p

OBJDIR=obj

MODELS=lmm xmm

VPATH=.:misc:stdio:stdlib:string:sys:sys/propeller:time:drivers

all: $(foreach x, $(MODELS), $(OBJDIR)/$(x) $(OBJDIR)/$(x)/libc.a)

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

LMM_OBJS = $(addprefix $(OBJDIR)/lmm/, $(OBJS))
XMM_OBJS = $(addprefix $(OBJDIR)/xmm/, $(OBJS))

$(OBJDIR)/lmm/libc.a: $(LMM_OBJS)
	$(AR) rs $@ $^

$(OBJDIR)/xmm/libc.a: $(XMM_OBJS)
	$(AR) rs $@ $^

$(OBJDIR)/lmm/%.o: %.c
	$(CC) $(CFLAGS) -mlmm -o $@ -c $<

$(OBJDIR)/xmm/%.o: %.c
	$(CC) $(CFLAGS) -mxmm -o $@ -c $<

hello.elf: hello.c
	$(CC) -o hello.elf hello.c

install: $(OBJDIR)/lmm/libc.a $(OBJDIR)/xmm/libc.a
	mkdir -p $(PREFIX)/propeller-elf/lib/xmm/
	cp -r include $(PREFIX)/propeller-elf
	cp $(OBJDIR)/lmm/libc.a $(PREFIX)/propeller-elf/lib
	cp $(OBJDIR)/xmm/libc.a $(PREFIX)/propeller-elf/lib/xmm/

clean:
	rm -rf $(OBJDIR) *.a

$(foreach x, $(MODELS), $(OBJDIR)/$(x)):
	$(MKDIR) $@
