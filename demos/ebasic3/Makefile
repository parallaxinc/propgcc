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
db_symbols.o \
db_system.o \
db_vmint.o \
editbuf.o \
osint_propgcc.o \
simple_vsnprintf.o \
ebasic_vm.o

OBJS += db_vmdebug.o

ifndef MODEL
MODEL = xmmc
endif

CFLAGS = -Wall -Os -DPROPELLER_GCC -Dvsnprintf=__simple_vsnprintf
#CFLAGS += -DLOAD_SAVE
CFLAGS += -DUSE_FDS
CFLAGS += -DUSE_ASM

%.dat:	%.spin
	@spin2cpp --dat -o $@ $<
	@echo $@
	
%.o: %.dat
	@propeller-elf-objcopy -I binary -B propeller -O propeller-elf-gcc --rename-section .data=.text $< $@
	@echo $@
	
include ../common/common.mk
