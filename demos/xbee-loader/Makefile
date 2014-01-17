TARGET=xbee-term.elf

CFLAGS=-Os -Wall

HDRS=\
xbeeframe.h \
fds.h

OBJS=\
xbee-loader.o \
xbeeframe.o \
xbeeframe_driver.o \
fds.o \
fds_driver.o

TARGET=xbee-loader.elf

all:	$(TARGET) xbee-test

%.dat:	%.spin
	@spin2cpp --dat -o $@ $<
	@echo $@
	
%.o: %.dat
	@propeller-elf-objcopy -I binary -B propeller -O propeller-elf-gcc --rename-section .data=.text $< $@
	@echo $@
	
%.o: %.c $(HDRS)
	@propeller-elf-gcc $(CFLAGS) -c -o $@ $<
	@echo $@

$(TARGET): $(OBJS)
	@propeller-elf-gcc $(CFLAGS) -o $@ $(OBJS)
	@echo $@

xbee-test:	xbee-test.c
	cc -Wall -o xbee-test xbee-test.c

run:	$(TARGET)
	@propeller-load $(TARGET) -r -t
	
clean:
	@rm -rf $(OBJS) $(TARGET) *.dat xbee-test
