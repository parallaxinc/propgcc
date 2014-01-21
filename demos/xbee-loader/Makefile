CFLAGS=-Os -Wall
LDFLAGS=-Wl,--defsym,__stack_end=0x7000

HDRS=\
xbeeframe.h \
fds.h

CONFIG_OBJS=\
xbee-config.o \
fds.o \
fds_driver.o

LOADER_OBJS=\
xbee-loader.o \
xbeeframe.o \
xbeeframe_driver.o \
xbeeload.o \
xbeeload_driver.o

CONFIG=xbee-config.elf
LOADER=xbee-loader.elf
TEST=xbee-test

all:	$(CONFIG) $(LOADER) # $(TEST)

%.dat:	%.spin
	@spin2cpp --dat -o $@ $<
	@echo $@
	
%.o: %.dat
	@propeller-elf-objcopy -I binary -B propeller -O propeller-elf-gcc --rename-section .data=.text $< $@
	@echo $@
	
%.o: %.c $(HDRS)
	@propeller-elf-gcc $(CFLAGS) -c -o $@ $<
	@echo $@

$(CONFIG): $(CONFIG_OBJS)
	@propeller-elf-gcc $(CFLAGS) -o $@ $(CONFIG_OBJS)
	@echo $@

$(LOADER): $(LOADER_OBJS)
	@propeller-elf-gcc $(CFLAGS) $(LDFLAGS) -o $@ $(LOADER_OBJS)
	@echo $@

$(TEST):	xbee-test.c
	cc -Wall -o $@ xbee-test.c

config:	$(CONFIG)
	@propeller-load $(CONFIG) -r -t

run:	$(LOADER)
	@propeller-load $(LOADER) -r -t

test:	$(TEST)
	./$(TEST)
	
clean:
	@rm -rf $(CONFIG_OBJS) $(CONFIG) $(LOADER_OBJS) $(LOADER) $(TEST) *.dat
