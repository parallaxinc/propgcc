how to start gdb

(1) Load the program with propeller-load and -g -r options:
    propeller-load foo.elf -g -r 
(2) Run propeller-elf-gdb:
    propeller-elf-gdb foo.elf
(3) type commands to propeller-elf-gdb:
      target remote |gdbstub (done automatically)
      (any breakpoints you want)
      c

