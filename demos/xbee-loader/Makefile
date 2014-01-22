CFLAGS=-mcmm -Os -Wall -Dprintf=__simple_printf
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
xbeeload_driver.o \
strcasestr.o

COMMON_SERVER_OBJS=\
xbee-server.o \
xbeeframe.o \
xbeeframe_driver.o

LOADER_SERVER_OBJS=\
$(COMMON_SERVER_OBJS) \
loader-requests.o \
xbeeload.o \
xbeeload_driver.o

XABS_SERVER_OBJS=\
$(COMMON_SERVER_OBJS) \
xabs-requests.o

CONFIG=xbee-config.elf
LOADER=xbee-loader.elf
LOADER_SERVER=xbee-server.elf
XABS_SERVER=xabs-server.elf
TEST=xbee-test

all:	$(CONFIG) $(LOADER) $(LOADER_SERVER) $(XABS_SERVER) # $(TEST)

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

$(LOADER_SERVER): $(LOADER_SERVER_OBJS)
	@propeller-elf-gcc $(CFLAGS) $(LDFLAGS) -o $@ $(LOADER_SERVER_OBJS)
	@echo $@

$(XABS_SERVER): $(XABS_SERVER_OBJS)
	@propeller-elf-gcc $(CFLAGS) $(LDFLAGS) -o $@ $(XABS_SERVER_OBJS)
	@echo $@

$(TEST):	xbee-test.c
	cc -Wall -o $@ xbee-test.c

config:	$(CONFIG)
	@propeller-load $(CONFIG) -r -t

run:	$(LOADER)
	@propeller-load $(LOADER) -r -t

serve:	$(LOADER_SERVER)
	@propeller-load $(LOADER_SERVER) -r -t

xabs:	$(XABS_SERVER)
	@propeller-load $(XABS_SERVER) -r -t

test:	$(TEST)
	./$(TEST)
	
clean:
	@rm -rf *.o $(CONFIG) $(LOADER) $(SERVER) $(TEST) *.dat
