Directory structure:

	Term/include contains the library include files
	Term/lib contains the library code
	Term/ctest contains the C test program
	Term/cpptest contains the C++ test program

To build in lmm mode:

	make
or
	make MODEL=lmm

To build in xmmc mode:

	make MODEL=xmmc

To build in xmm mode:

	make MODEL=xmm

You have to be careful not to build the library in one mode and the test programs in another.
