ifndef MODEL
MODE = lmm
endif

all:
	make -C lib
	make -C ctest
	make -C cpptest

runc:
	make -C ctest run

runcpp:
	make -C cpptest run

clean:
	make -C lib clean
	make -C ctest clean
	make -C cpptest clean
