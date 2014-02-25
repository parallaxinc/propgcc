spinwrap:	spinwrap.c
	cc -Wall -o spinwrap spinwrap.c

test.cpp:	test.spin spinwrap
	./spinwrap $<

test.c:	test.spin spinwrap
	./spinwrap -c $<

test.elf:	test_main.cpp test.cpp test.h
	propeller-elf-gcc -Os -Wall -o $@ test_main.cpp test.cpp -fno-exceptions -fno-rtti

testc.elf:	test_main.c test.c test.h
	propeller-elf-gcc -Os -Wall -o $@ test_main.c test.c

run:	test.elf
	propeller-load test.elf -r -t
	
runc:	testc.elf
	propeller-load testc.elf -r -t
	
clean:
	rm -f spinwrap test.cpp test.c test.h test_proxy.spin *.elf *.o *.binary
