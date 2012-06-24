NAME = tvedit

OBJS = \
tvedit.o \
db_edit.o \
db_system.o \
osint_posix.o \
editbuf.o \
initio.o \
TvDriver.o \
TvText.o \
keyboard.o \
TV_firmware.o \
Keyboard_firmware.o

ifndef MODEL
MODEL = xmmc
endif

CFLAGS = -Os -DPROPELLER_GCC -DLOAD_SAVE

include ../common/common.mk
