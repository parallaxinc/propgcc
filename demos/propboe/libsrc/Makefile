LIBDIR=../lib
TARGET=$(LIBDIR)/libprop.a

CC=propeller-elf-gcc
OBJCOPY=propeller-elf-objcopy
AR=propeller-elf-ar
LOAD=propeller-load
BSTC=bstc.osx
MKDIR=mkdir -p
RM=rm -f

OBJS = \
getdir.o \
getfield.o \
getpin.o \
high.o \
input.o \
low.o \
output.o \
pause.o \
pulsein.o \
pulseout.o \
setdir.o \
setfield.o \
setpin.o \
term.o \
term_vga.o \
term_serial.o \
term_tv.o \
toggle.o

DRIVEROBJS = \
VGA_firmware.o \
TV_firmware.o \
Keyboard_firmware.o

CFLAGS=-Os -Wall -I../include -mno-fcache
CPPFLAGS=$(CFLAGS)
DRVFLAGS=$(CFLAGS) -mcog -fomit-frame-pointer

all:	$(TARGET)

$(TARGET):	$(OBJS) $(DRIVEROBJS) $(LIBDIR)
	$(AR) rs $(TARGET) $(OBJS) $(DRIVEROBJS)

%.dat: %.spin
	$(BSTC) -Ox -c -o $(basename $@) $<

%_firmware.o: %.dat
	$(OBJCOPY) -I binary -B propeller -O $(CC) $< $@

# compile a COG driver
%_image.elf:	%.c
	$(CC) $(DRVFLAGS) -o $@ $<

# convert an ELF file to a .dat file
%.dat:	%.elf
	$(OBJCOPY) -O binary -j .text $< $@
	
# convert a .dat to an object file
%.o:	%.dat
	$(OBJCOPY) -O elf32-propeller -I binary -B propeller $< $@

$(LIBDIR):
	$(MKDIR) $(LIBDIR)

clean:
	$(RM) *.dat *.o *.elf $(TARGET)
