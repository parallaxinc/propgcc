#
# objects for this program
#

PROPLIB = ../common

NAME = expr
OBJS = expr.o

# memory model
MODEL = cmm

#
# enable experimental fcache code for this demo
#
CFLAGS = -Os -m32bit-doubles -DMAIN
LIBS = -lm

all: $(NAME).elf

include $(PROPLIB)/common.mk
