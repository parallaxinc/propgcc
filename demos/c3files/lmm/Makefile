CC=propeller-elf-gcc
MODEL=lmm
CFLAGS=-Os -m$(MODEL)
LDFLAGS=-m$(MODEL)
OBJDIR=./
SRCDIR=../src
ECHO=echo
OBJS = filetest.o dfs.o dosfs.o file_io.o c3_cache.o printf.o
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
