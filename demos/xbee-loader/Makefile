CFLAGS=-mcmm -Os -Wall -Dprintf=__simple_printf
LDFLAGS=-Wl,--defsym,__stack_end=0x7000

HOST_CFLAGS=-Wall
HOST_LIBS=

ifeq ($(OSTYPE),msys)
HOST_LIBS+=-lws2_32
EXT=.exe
else
EXT=
endif

HDRS=\
xbeeframe.h \
fds.h

CONFIG_OBJS=\
xbee-config.o \
fds.o \
fds_driver.o

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
LOAD=xbee-load$(EXT)

all:	$(CONFIG) $(LOADER_SERVER) $(XABS_SERVER) # $(LOAD)

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

$(LOADER_SERVER): $(LOADER_SERVER_OBJS)
	@propeller-elf-gcc $(CFLAGS) $(LDFLAGS) -o $@ $(LOADER_SERVER_OBJS)
	@echo $@

$(XABS_SERVER): $(XABS_SERVER_OBJS)
	@propeller-elf-gcc $(CFLAGS) $(LDFLAGS) -o $@ $(XABS_SERVER_OBJS)
	@echo $@

$(LOAD):	xbee-load.c
	cc $(HOST_CFLAGS) -o $@ xbee-load.c $(HOST_LIBS)

config:	$(CONFIG)
	@propeller-load $(CONFIG) -r -t

serve:	$(LOADER_SERVER)
	@propeller-load $(LOADER_SERVER) -r -t

xabs:	$(XABS_SERVER)
	@propeller-load $(XABS_SERVER) -r -t

load:	$(LOAD)
	
clean:
	@rm -rf *.o $(CONFIG) $(LOADER_SERVER) $(XABS_SERVER) $(LOAD) *.dat
