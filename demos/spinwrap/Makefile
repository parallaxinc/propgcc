spinwrap:	spinwrap.c
	cc -Wall -o spinwrap spinwrap.c

test.cpp test.h:	test.spin spinwrap
	./spinwrap $<

test.elf:	test_main.cpp test.cpp test.h
	propeller-elf-gcc -Os -Wall -o $@ test_main.cpp test.cpp -fno-exceptions -fno-rtti

run:	test.elf
	propeller-load test.elf -r -t
	
clean:
	rm -f spinwrap test.cpp test.h test_proxy.spin *.elf *.o *.binary
