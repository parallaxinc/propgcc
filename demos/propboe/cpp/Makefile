LIBDIR=../lib
TARGET=$(LIBDIR)/libprop++.a

CXX=propeller-elf-gcc
OBJCOPY=propeller-elf-objcopy
AR=propeller-elf-ar
MKDIR=mkdir -p
RM=rm -f

OBJS = \
pin_pulsein.o \
pin_pulseout.o \
pingroup_get.o \
pingroup_pingroup.o \
pingroup_set.o

CFLAGS=-Os -Wall -I../include -mno-fcache
CPPFLAGS=$(CFLAGS)

all:	$(TARGET)

$(TARGET):	$(OBJS)
	$(AR) rs $(TARGET) $(OBJS)

$(LIBDIR):
	$(MKDIR) $(LIBDIR)

clean:
	$(RM) *.o $(TARGET)
