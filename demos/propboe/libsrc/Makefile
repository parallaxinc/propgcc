LIBDIR=../lib
TARGET=$(LIBDIR)/libboe.a

CC=propeller-elf-gcc
CXX=propeller-elf-gcc
OBJCOPY=propeller-elf-objcopy
AR=propeller-elf-ar
LOAD=propeller-load
BSTC=bstc
MKDIR=mkdir -p
RM=rm -f

OBJS = \
getdir.o \
getpin.o \
high.o \
i2c_interface.o \
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
TV_firmware.o \
i2c_driver_image.o

CFLAGS=-Os -Wall -I../include -mno-fcache
CPPFLAGS=$(CFLAGS)
DRVFLAGS=$(CFLAGS) -mcog -fomit-frame-pointer

all:	$(TARGET) # i2c_driver_image.elf

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
