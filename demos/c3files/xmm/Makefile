CC=propeller-elf-gcc
MODEL=xmm
CFLAGS=-Os -m$(MODEL)
LDFLAGS=-m$(MODEL)
OBJDIR=./
SRCDIR=../src
OBJS = filetest.o
HDRS =

c3files.elf:	$(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) -o $@ -s
run:
	propeller-load -b c3 c3files.elf -r -t

clean:
	rm -f *.o *.elf

$(OBJDIR)/%.o:	$(SRCDIR)/%.c $(HDRS)
	@$(CC) $(CFLAGS) -c $< -o $@
