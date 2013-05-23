NAME = termctest

OBJS = \
termctest.o

LIBS = -lterm -lsupc++

ifndef MODEL
MODEL = lmm
endif

CFLAGS = -Wall -Os -DPROPELLER_GCC -I../include
LDFLAGS = -L../lib

include ../../common/common.mk
