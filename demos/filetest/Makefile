CC=propeller-elf-gcc
MODEL?=cmm
CFLAGS=-m$(MODEL) -Os -mfcache
LDFLAGS=-m$(MODEL) -fno-exceptions -fno-rtti
OBJDIR=./
SRCDIR=./
BOARD?=$(PROPELLER_LOAD_BOARD)

OBJS = filetest.o
HDRS =

ifneq ($(BOARD),)
BOARDFLAG=-b$(BOARD)
endif

ifeq ($(MODEL),cmm)
OBJS += printf.o
endif

ifeq ($(MODEL),lmm)
OBJS += printf.o
endif

ifneq ($(CHIP),)
CHIPFLAG = -m$(CHIP)
endif

filetest.elf:	$(OBJS)
	$(CC) $(LDFLAGS) $(CHIPFLAG) $(OBJS) -o $@

run:
	propeller-load $(BOARDFLAG) filetest.elf -r -t

clean:
	rm -f *.o *.elf

$(OBJDIR)/%.o:	$(SRCDIR)/%.c $(HDRS)
	$(CC) $(CFLAGS) $(CHIPFLAG) -c $< -o $@
