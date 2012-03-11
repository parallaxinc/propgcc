LIBDIR=../lib
TARGET=$(LIBDIR)/libboe.a

CC=propeller-elf-gcc
CXX=propeller-elf-gcc
AR=propeller-elf-ar
BSTC=bstc
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
term.o \
term_vga.o \
term_serial.o \
term_tv.o \
toggle.o

DRIVEROBJS = \
VGA.o \
TV.o

CFLAGS=-Os -I../include
CPPFLAGS=$(CFLAGS)

$(TARGET):	$(OBJS) $(DRIVEROBJS) $(LIBDIR)
	propeller-elf-ar rs $(TARGET) $(OBJS) $(DRIVEROBJS)

%.binary:	%.spin
	$(BSTC) $(SPINFLAGS) -b -o $(basename $@) $<

%.c:	%.binary bin2c
	./bin2c $< $@

bin2c:	bin2c.c
	cc -o bin2c bin2c.c
    
clean:
	$(RM) *.o $(TARGET) bin2c

$(LIBDIR):
	$(MKDIR) $(LIBDIR)
	
