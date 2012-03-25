LIBDIR=../lib
TARGET=$(LIBDIR)/libboe.a

CC=propeller-elf-gcc
CXX=propeller-elf-gcc
OBJCOPY=propeller-elf-objcopy
AR=propeller-elf-ar
BSTC=bstc
MKDIR=mkdir -p
RM=rm -f

OBJS = \
getdir.o \
getpin.o \
high.o \
i2c.o \
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
VGA_firmware.o \
TV_firmware.o

CFLAGS=-Os -I../include
CPPFLAGS=$(CFLAGS)

$(TARGET):	$(OBJS) $(DRIVEROBJS) $(LIBDIR)
	$(AR) rs $(TARGET) $(OBJS) $(DRIVEROBJS)

%.dat: %.spin
	$(BSTC) -Ox -c -o $(basename $@) $<

%_firmware.o: %.dat
	$(OBJCOPY) -I binary -B propeller -O $(CC) $< $@

clean:
	$(RM) *.o $(TARGET)

$(LIBDIR):
	$(MKDIR) $(LIBDIR)
	
