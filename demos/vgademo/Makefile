CC=propeller-elf-gcc
MODEL=xmmc
CFLAGS=-Os -m$(MODEL)
LDFLAGS=-m$(MODEL)
OBJDIR=./
SRCDIR=./
OBJS = vgademo.o vga.o draw.o text.o text2.o
HDRS =

vgademo.elf:	$(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) -o $@ -s
run:
	propeller-load -b c3f vgademo.elf -r -t

clean:
	rm -f *.o *.elf

$(OBJDIR)/%.o:	$(SRCDIR)/%.c $(HDRS)
	@$(CC) $(CFLAGS) -c $< -o $@
