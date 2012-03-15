NAME = ebasic

OBJS = \
ebasic.o \
db_compiler.o \
db_edit.o \
db_expr.o \
db_generate.o \
db_image.o \
db_scan.o \
db_statement.o \
db_strings.o \
db_symbols.o \
db_system.o \
db_vmdebug.o \
db_vmfcn.o \
db_vmint.o \
editbuf.o \
scratchbuf.o \
osint_propgcc.o

ifndef MODEL
MODEL = xmmc
endif

CFLAGS = -Os -DPROPELLER_GCC -DUSE_FDS -DLOAD_SAVE

include ../common/common.mk
