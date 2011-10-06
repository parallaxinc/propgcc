#
# Makefile for Propeller C library
#
PREFIX = /usr/local/propeller
WARN = -Wall -Werror
#WARN =
CFLAGS = $(WARN) -Os -nostdinc -I./include
#CFLAGS = $(WARN) -Os -mfcache -nostdinc -I./include
AR = propeller-elf-ar
CC = propeller-elf-gcc
MKDIR = mkdir -p

OBJDIR=obj

MODELS=lmm xmm xmmc

VPATH=.:misc:stdio:stdlib:string:sys:sys/propeller:time:drivers:fdlibm

all: $(foreach x, $(MODELS), $(OBJDIR)/$(x) $(OBJDIR)/$(x)/libc.a $(OBJDIR)/$(x)/libm.a)

# $(PRINTF) are the functions which depend on vfprintf;
# we include floating point versions of these in the math library
PRINTF = printf.o fprintf.o vprintf.o sprintf.o snprintf.o vsprintf.o vsnprintf.o
SCANF =  scanf.o vscanf.o sscanf.o
 
STDIO = clearerr.o feof.o ferror.o fflush.o \
	fgetc.o fputc.o fputs.o puts.o putchar.o \
	ungetc.o gets.o fgets.o \
	setvbuf.o setbuf.o setlinebuf.o setbuffer.o \
	fopen.o fopen_intern.o freopen.o \
	fwrite.o fread.o fseek.o ftell.o fgetpos.o fsetpos.o \
	stringio.o vfprintf.o vfscanf.o \
	$(PRINTF) $(SCANF) \
	simple_printf.o \
	default_io.o init_io.o

MISC = malloc.o calloc.o sbrk.o ctype.o atoi.o thread.o atexit.o abort.o \
	toupper.o tolower.o isprint.o isspace.o isalnum.o isdigit.o \
	qsort.o bsearch.o signal.o \
	assert.o write.o

STRING = memcpy.o memmove.o memset.o memchr.o memcmp.o \
	strcat.o strcmp.o strcpy.o strlen.o \
	strncat.o strncmp.o strncpy.o \
	strchr.o strrchr.o strspn.o strcspn.o strtok.o \
	strcoll.o strxfrm.o \
	strerror.o

TIME = clock.o sleep.o

DRIVERS = SimpleSerial.o serialparam.o serial_break.o serial_exit.o memory.o \
	FdSerial.o FullDuplexSerial.o null.o drivers.o

MATH = 	k_standard.o k_rem_pio2.o \
	k_cos.o k_sin.o k_tan.o \
	e_acos.o e_acosh.o e_asin.o e_atan2.o \
	e_atanh.o e_cosh.o e_exp.o e_fmod.o \
	e_gamma.o e_gamma_r.o e_hypot.o e_j0.o \
	e_j1.o e_jn.o e_lgamma.o e_lgamma_r.o \
	e_log.o e_log10.o e_pow.o e_rem_pio2.o e_remainder.o  \
	e_scalb.o e_sinh.o e_sqrt.o \
	w_acos.o w_acosh.o w_asin.o w_atan2.o \
	w_atanh.o w_cosh.o w_exp.o w_fmod.o \
	w_gamma.o w_gamma_r.o w_hypot.o w_j0.o \
	w_j1.o w_jn.o w_lgamma.o w_lgamma_r.o \
	w_log.o w_log10.o w_pow.o w_remainder.o \
	w_scalb.o w_sinh.o w_sqrt.o \
	s_asinh.o s_atan.o s_cbrt.o s_ceil.o s_copysign.o \
	s_cos.o s_erf.o s_expm1.o s_fabs.o s_finite.o s_floor.o \
	s_frexp.o s_ilogb.o s_isnan.o s_ldexp.o s_lib_version.o \
	s_log1p.o s_logb.o s_matherr.o s_modf.o s_nextafter.o \
	s_rint.o s_scalbn.o s_signgam.o s_significand.o s_sin.o \
	s_tan.o s_tanh.o \
	float_printf.o $(PRINTF) \
	float_scanf.o $(SCANF)

PROPELLER = propeller.o

OBJS = $(STDIO) $(MISC) $(STRING) $(TIME) $(DRIVERS) $(PROPELLER)

LMM_OBJS = $(addprefix $(OBJDIR)/lmm/, $(OBJS))
XMM_OBJS = $(addprefix $(OBJDIR)/xmm/, $(OBJS))
XMMC_OBJS = $(addprefix $(OBJDIR)/xmmc/, $(OBJS))
LMM_MATH_OBJS = $(addprefix $(OBJDIR)/lmm/, $(MATH))
XMM_MATH_OBJS = $(addprefix $(OBJDIR)/xmm/, $(MATH))
XMMC_MATH_OBJS = $(addprefix $(OBJDIR)/xmmc/, $(MATH))

$(OBJDIR)/lmm/libc.a: $(LMM_OBJS)
	$(AR) rs $@ $^

$(OBJDIR)/xmm/libc.a: $(XMM_OBJS)
	$(AR) rs $@ $^

$(OBJDIR)/xmmc/libc.a: $(XMMC_OBJS)
	$(AR) rs $@ $^

$(OBJDIR)/lmm/libm.a: $(LMM_MATH_OBJS)
	$(AR) rs $@ $^

$(OBJDIR)/xmm/libm.a: $(XMM_MATH_OBJS)
	$(AR) rs $@ $^

$(OBJDIR)/xmmc/libm.a: $(XMMC_MATH_OBJS)
	$(AR) rs $@ $^

$(OBJDIR)/lmm/%.o: %.c
	$(CC) $(CFLAGS) -mlmm -o $@ -c $<

$(OBJDIR)/xmm/%.o: %.c
	$(CC) $(CFLAGS) -mxmm -o $@ -c $<

$(OBJDIR)/xmmc/%.o: %.c
	$(CC) $(CFLAGS) -mxmmc -o $@ -c $<

$(OBJDIR)/lmm/%.o: %.s
	$(CC) $(CFLAGS) -mlmm -o $@ -c $<

$(OBJDIR)/xmm/%.o: %.s
	$(CC) $(CFLAGS) -mxmm -o $@ -c $<

$(OBJDIR)/xmmc/%.o: %.s
	$(CC) $(CFLAGS) -mxmmc -o $@ -c $<

hello.elf: hello.c
	$(CC) -o hello.elf hello.c

install: $(OBJDIR)/lmm/libc.a $(OBJDIR)/lmm/libm.a $(OBJDIR)/xmm/libc.a $(OBJDIR)/xmm/libm.a $(OBJDIR)/xmmc/libc.a $(OBJDIR)/xmmc/libm.a
	mkdir -p $(PREFIX)/propeller-elf/lib/xmm/
	mkdir -p $(PREFIX)/propeller-elf/lib/xmmc/
	cp -r include $(PREFIX)/propeller-elf
	cp $(OBJDIR)/lmm/libc.a $(OBJDIR)/lmm/libm.a $(PREFIX)/propeller-elf/lib
	cp $(OBJDIR)/xmm/libc.a $(OBJDIR)/xmm/libm.a $(PREFIX)/propeller-elf/lib/xmm/
	cp $(OBJDIR)/xmmc/libc.a $(OBJDIR)/xmmc/libm.a $(PREFIX)/propeller-elf/lib/xmmc/

clean:
	rm -rf $(OBJDIR) *.a

$(foreach x, $(MODELS), $(OBJDIR)/$(x)):
	$(MKDIR) $@
