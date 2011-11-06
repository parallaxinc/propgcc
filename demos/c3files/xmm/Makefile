CC=propeller-elf-gcc
MODEL=xmm
CFLAGS=-Os -m$(MODEL)
LDFLAGS=-m$(MODEL)
OBJDIR=./
SRCDIR=../src
ECHO=echo
OBJS = filetest.o dfs.o dosfs.o
HDRS = $(SRCDIR)/dfs.h $(SRCDIR)/dosfs.h

c3files.elf:	$(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) -o $@
	@$(ECHO) $@
run:
	propeller-load -b c3 c3files.elf -r -t

clean:
	rm -f *.o *.elf

$(OBJDIR)/%.o:	$(SRCDIR)/%.c $(HDRS)
	@$(CC) $(CFLAGS) -c $< -o $@
	@$(ECHO) $@
