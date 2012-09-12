CC=propeller-elf-gcc
MODEL=xmm
CFLAGS=-m$(MODEL) -Os -mfcache
LDFLAGS=-m$(MODEL) -fno-exceptions -fno-rtti
OBJDIR=./
SRCDIR=../src
BOARD=$(PROPELLER_LOAD_BOARD)

OBJS = filetest.o
HDRS =

ifneq ($(BOARD),)
BOARDFLAG=-b$(BOARD)
endif

filetest.elf:	$(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

run:
	propeller-load $(BOARDFLAG) filetest.elf -r -t

clean:
	rm -f *.o *.elf

$(OBJDIR)/%.o:	$(SRCDIR)/%.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@
