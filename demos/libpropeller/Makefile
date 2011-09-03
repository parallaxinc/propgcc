# Propeller Demo include
PROPLIB = ../libpropeller
PROPINC = ../include

CFLAGS = -I$(PROPINC)

# basic gnu tools
CC = propeller-elf-gcc $(CFLAGS)
LD = propeller-elf-ld
AR = propeller-elf-ar
OBJCOPY = propeller-elf-objcopy

NAME = libpropeller
OBJS = propeller.o

all: $(NAME).a

$(NAME).a: $(OBJS)
	$(AR) rcs $(NAME).a propeller.o

%.o: %.c
	$(CC) -Os -mlmm -o $@ -c $<

%.o: %.s
	$(CC) -o $@ -c $<


clean:
	rm -f *.o *.elf *.binary *.list *.a
