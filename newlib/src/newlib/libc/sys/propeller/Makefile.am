## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = \
	access.c	\
	close.c		\
	creat.c		\
	crt0.s		\
	crtbegin.s	\
	crtend.s	\
	crt0_lmm.s	\
	crtbegin_lmm.s	\
	crtend_lmm.s	\
	crt0_xmm.s	\
	crtbegin_xmm.s	\
	crtend_xmm.s	\
	dup2.c		\
	exec.c		\
	execvp.c	\
	fork.c		\
	fstat.c		\
	gettimeofday.c	\
	hubstart_xmm.s	\
	initlib.c	\
	ioctl.c		\
	isatty.c	\
	kill.c		\
	link.c		\
	lseek.c		\
	lstat.c		\
	open.c		\
	params.c	\
	pipe.c		\
	read.c		\
	sbrk.s		\
	stat.c		\
	times.c		\
	unlink.c	\
	usleep.c	\
	version.c	\
	write.c


all: crt0.o crtend.o

ACLOCAL_AMFLAGS = -I ../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host
