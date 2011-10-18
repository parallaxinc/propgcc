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
	#make -C toggle/cog_c_toggle    # TODO build fails. fix later.
	make -C toggle/c++_toggle
	make -C toggle/lmm_c_toggle
	make -C toggle/lmm_toggle
	make -C toggle/pasm_toggle
	make -C toggle/xmm_toggle
	make -C dhrystone
	make -C fft
	make -C fibo/cog
	make -C fibo/lmm
	make -C fibo/xmm
	make -C TV/TvText_demo
	make -C xbasic

clean:
	make clean -C libpropeller
	make clean -C toggle/cog_c_toggle
	make clean -C toggle/c++_toggle
	make clean -C toggle/lmm_c_toggle
	make clean -C toggle/lmm_toggle
	make clean -C toggle/pasm_toggle
	make clean -C toggle/xmm_toggle
	make clean -C dhrystone
	make clean -C fft
	make clean -C fibo/cog
	make clean -C fibo/lmm
	make clean -C fibo/xmm
	make clean -C TV/TvText_demo
	make clean -C xbasic

