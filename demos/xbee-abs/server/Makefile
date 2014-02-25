CFLAGS=-Os -Wall -Dprintf=__simple_printf

HDRS=\
xbeeframe.h \
fds.h

ABSERV_OBJS=\
xbee-abserv.o \
xbeeframe.o \
xbeeframe_driver.o

ABSERV=xbee-abserv.elf

all:	Makefile $(ABSERV)

%.dat:	%.spin
	@spin2cpp --dat -o $@ $<
	@echo $@
	
%.o: %.dat
	@propeller-elf-objcopy -I binary -B propeller -O propeller-elf-gcc --rename-section .data=.text $< $@
	@echo $@
	
%.o: %.c $(HDRS)
	@propeller-elf-gcc $(CFLAGS) -c -o $@ $<
	@echo $@

$(ABSERV): $(ABSERV_OBJS)
	@propeller-elf-gcc $(CFLAGS) -o $@ $(ABSERV_OBJS)
	@echo $@

run:	$(ABSERV)
	@propeller-load $(ABSERV) -r -t

clean:
	@rm -rf $(ABSERV_OBJS) $(ABSERV) *.dat
