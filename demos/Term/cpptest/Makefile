NAME = termcpptest

OBJS = \
termcpptest.o

LIBS = -lterm

ifndef MODEL
MODEL = lmm
endif

CFLAGS = -Wall -Os -DPROPELLER_GCC -fno-exceptions -I../include
LDFLAGS = -L../lib

include ../../common/common.mk
