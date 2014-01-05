NAME = pico-basic

HDRS = \
db_compiler.h \
db_edit.h \
db_image.h \
db_inttypes.h \
db_system.h \
db_types.h \
db_vm.h \
db_vmdebug.h \
db_vmheap.h

EDITOR_OBJS = \
db_edit.c \
editbuf.c

COMPILER_OBJS = \
db_compiler.o \
db_statement.o \
db_expr.o \
db_scan.o \
db_generate.o

RUNTIME_OBJS = \
db_vmint.o \
db_vmfcn.o \
db_vmheap.o \
db_vmdebug.o \
db_system.o \
osint_posix.o

OBJS = pico-basic.o $(EDITOR_OBJS) $(COMPILER_OBJS) $(RUNTIME_OBJS)

CFLAGS = -Wall -Os -DMAC -m32 -DLOAD_SAVE
LDFLAGS = $(CFLAGS)

$(NAME): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f *.o $(NAME).elf
