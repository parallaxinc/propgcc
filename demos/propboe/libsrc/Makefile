LIBDIR=../lib
TARGET=$(LIBDIR)/libboe.a

CC=propeller-elf-gcc
CXX=propeller-elf-gcc
AR=propeller-elf-ar
MKDIR=mkdir -p
RM=rm -f

OBJS = \
getdir.o \
getpin.o \
high.o \
input.o \
low.o \
output.o \
pause.o \
pin_pulsein.o \
pin_pulseout.o \
pingroup_get.o \
pingroup_pingroup.o \
pingroup_set.o \
pulsein.o \
pulseout.o \
setdir.o \
setpin.o \
toggle.o

CFLAGS=-Os -I../include
CPPFLAGS=$(CFLAGS)

$(TARGET):	$(OBJS) $(LIBDIR)
	propeller-elf-ar rs $(TARGET) $(OBJS)

clean:
	$(RM) *.o $(TARGET)

$(LIBDIR):
	$(MKDIR) $(LIBDIR)
	
