TARGET=xbee-term.elf

CFLAGS=-Os -Wall

HDRS=\

OBJS=\
xbee-loader.o \
xbeeframe.o \
xbeeframe_driver.o

TARGET=xbee-loader.elf

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

run:	$(TARGET)
	@propeller-load $(TARGET) -r -t
	
clean:
	@rm -rf $(OBJS) $(TARGET) *.dat
