NAME = ebasic

EDITOR_OBJS = \
db_edit.o \
editbuf.o

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
osint_propgcc.o

OBJS = pico-basic.o $(EDITOR_OBJS) $(COMPILER_OBJS) $(RUNTIME_OBJS)

ifndef MODEL
MODEL = xmmc
endif

CFLAGS = -Wall -Os -DPROPELLER_GCC -DUSE_FDS -DLOAD_SAVE

include common.mk
