#
# objects for this program
#

PROPLIB = ../../libpropeller

NAME = fibo
OBJS = fibo.o

# memory model
MODEL = lmm

#
# enable experimental fcache code for this demo
#
CFLAGS = -Os -mfcache

all: $(NAME).elf

include $(PROPLIB)/demo.mk
