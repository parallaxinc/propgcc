NAME = TermTest

OBJS = \
TermTest.o \
term.o \
term_vga.o \
VGA_firmware.o \
term_tv.o \
TV_firmware.o

ifndef MODEL
MODEL = lmm
endif

CFLAGS = -Wall -Os -DPROPELLER_GCC

include ../common/common.mk
