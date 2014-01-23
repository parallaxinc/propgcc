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
db_vmfcn.o \
db_vmint.o \
editbuf.o \
scratchbuf.o \
osint_propgcc.o \
simple_vsnprintf.o

ifndef MODEL
MODEL = xmmc
endif

CFLAGS = -Os -DPROPELLER_GCC -Dvsnprintf=__simple_vsnprintf
#CFLAGS = -Os -DPROPELLER_GCC -DLOAD_SAVE -DUSE_FDS -Dvsnprintf=__simple_vsnprintf

include ../common/common.mk
