# #########################################################
# this makefile allows building and cleaning demo projects
# the demos have unique names mostly to make it easier to
# use tools like eclipse.
#
# Copyright (c) 2011 Steve Denson
# MIT Licensed
# #########################################################

all:
	make -C libpropeller
	make -C toggle/pasm_toggle
	make -C fft
	make -C fibo/cog
	make -C fibo/lmm
	make -C TV/TvText_demo

clean:
	make clean -C libpropeller
	make clean -C toggle/pasm_toggle
	make clean -C fft
	make clean -C fibo/cog
	make clean -C fibo/lmm
	make clean -C TV/TvText_demo

