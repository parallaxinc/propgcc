## Introduction ##

In order to keep your program small (for example, so that you can fit it within the LMM requirements of 32K code+data), you need to have detailed information about the size that each routine takes.  In addition, it's often useful to understand exactly how GCC generates Propeller assembly code.

This article tells you how to use PropGCC's tools to allow you to understand: (A) your program's memory usage and (B) how PropGCC generates code.  The primary tools for this are the dumps and maps that the PropGCC tools provide.

### Quick Summary ###

Here is a sample set of compilation commands that allow you to get a lot of detailed information:

```
propeller-elf-gcc -save-temps -mlmm -g -Os -mfcache -m32bit-doubles -fno-exceptions -fno-rtti -Dprintf=__simple_printf -o main.o -c main.cpp
propeller-elf-gcc -save-temps -mlmm -g -Os -mfcache -m32bit-doubles -fno-exceptions -fno-rtti -Dprintf=__simple_printf -o c++-alloc.o -c c++-alloc.cpp
propeller-elf-gcc -Xlinker -Map=main.rawmap -mlmm -o main.elf main.o c++-alloc.o
propeller-elf-c++filt --strip-underscore < main.rawmap | perl propgcc-map-sizes.pl > main.map
propeller-elf-objdump -d -S main.elf | propeller-elf-c++filt --strip-underscore > main.dump
```

Note these lines use the "compile with minimum size" flags mentioned in the PropGccTightMemory wiki page.  In addition to the expected main.o, c++-alloc.o and main.elf outputs, the above commands produce:

| File | Purpose |
|:-----|:--------|
| main.ii and c++-alloc.ii | C preprocessor output |
| main.s and c++-alloc.s | Assembler code for routines in main.cpp and c++-alloc.cpp |
| main.dump | Full dissembly dump of the completely-linked program, including the C kernel and runtime |
| main.rawmap | A memory map of the completely-linked program |
| main.map | A friendlier version of the memory map, with easier to read file names and symbols, and with a summary at the end. |

### Memory Usage ###

For most programs in most modern computer systems, the programmer does not have to worry about memory usage; memory only becomes a concern when the programmer makes a mistake or unless the program has unusually large requirements.  In an embedded system, however, memory is usually very tight and the programmer needs to understand and carefully manage his/her program's memory usage.

PropGCC's heritage is GCC, and it was originally designed as a compiler for UNIX (which is a modern computer system where the programmer typically does not worry about memory usage).  Thus, PropGCC doesn't by default display memory usage information to you.  However, PropGCC includes several commands and options that allow you to get detailed memory usage for your programs.

### Code Generation ###

For most programs in most modern computer systems, the programmer does not have to worry about how the compiler translates the source code language into the machine's executable language.  Code generation typically only becomes a concern when the compiler has bugs in it.  However, in an embedded system with limited speed and memory (like the Propeller), code generation often becomes an issue when you care about optimizing the code for faster speed or less memory usage.

Again, PropGCC's GCC heritage means it typically does not expose the generated code for you.  Again, PropGCC does include several commands and options that allow you to see the generated code.

## Background ##

Understanding the generic concept of [LMM (the Large Memory Model)](http://propeller.wikispaces.com/Large+Memory+Model) is a desirable (but not absolute) prerequisite to understanding the PropGCC's memory usage and code generation.  Additional PropGCC-specific LMM information can be found in this Wiki at PropGCCMemory.

The next important concept is "code sections".  When PropGCC compiles your files, it segragates the generated code into different "program" and "data" sections.  According to GCC's documentation:

> An object file is divided into sections containing different types of data. In the most common case, there are three sections: the text section, which holds instructions and read-only data; the data section, which holds initialized writable data; and the bss section, which holds uninitialized data. Some systems have other kinds of sections.

The Propeller is such a system that has "other kinds of sections".  Here is a list of the sections that PropGCC produces:

| **Section Name** | **Source** | **Details** |
|:-----------------|:-----------|:------------|
| boot | System | The Propeller only knows how to boot Spin programs. In LMM programs, this is the Spin program that starts the LMM Kernel. Note that XMM and XMMC programs don't need a boot section because propeller-load uploads a driver program (specified in propeller-load.cfg) that contains the initially-booted Spin program - this driver program then hands over control to the XMM/XMMC Kernel. |
| lmmkernel | System | The library code that runs the LMM "virtual machine" - in other words, this is the main program, and it's the program that manages the way your C/C++ program runs on the Propeller.  This section is produced when you compile/link with the -mlmm flag. |
| xmmckernel | System | The library code that runs the XMM "virtual machine", produced when you compile/link with the -mxmmc flag. |
| xmmkernel | System | The library code that runs the XMM "virtual machine", produced when you compile/link with the -mxmm flag. |
| header | System | In XMM and XMMC, this section contains the program start location and initalization information. |
| init | System | The library code that initializes argc and the environ pointer, calls the C++ static constructors, then calls your main program. |
| fini | System | The library code that provides exit(), which calls the C++ static destructors, the `_`ExitHook, then puts the current cog to sleep indefinately (using a waitpeq assembly instruction that never returns). |
| text | Programmer & Library | Your C/C++ code and the code for any of the functions you call from the library. |
| hub | Library | This section contains any code and/or data that is marked as "hub" so that it is always located in the Propeller's 32K RAM memory. In practice it typically only contains system drivers (e.g. the SimpleSerial driver).  |
| data | Programmer & Library | Your code's "initialized data" storage for static and global variables (i.e. not stack and heap variables), but only those variables that you initialize (e.g. "Hello", static char`*` foo = "Hi", static int bar = 3) |
| ctors | Programmer & Library | This section contains a list of C++ static constructor functions that are called before the program starts. |
| dtors | Programmer & Library | This section contains a list of C++ static destructor functions that are called after the program exits. |
| bss | Programmer & Library | Your code's "uninitialized data" storage for static and global variables that will contain the default value of 0. |
| heap | Programmer & Library | The heap starts here and grows upward toward the end of RAM memory.  The initial cog's stack starts at the top of the RAM memory and grows downward.  Be careful - there is no software or hardware protection to alert you if the two areas collide! |

## Examining Generated Code After Compilation ##

The easiest way to view the generated code is to include the -save-temps flag in your compilation line.  For example:

```
propeller-elf-gcc -save-temps -mlmm -Os -mfcache -m32bit-doubles -fno-exceptions -fno-rtti -Dprintf=__simple_printf -c main.cpp
```

The -save-temps flag causes GCC to:

  * Preserve the preprocessor output in a file named main.i (if your code is C) or main.ii (if your code is C++)
  * Preserve the assembler output in a file named main.s

Looking at the .s assembler file makes it easy to see the code that PropGCC generates for each routine.  A note about the "-g" compilation flag: this flag tells the compiler to include debugging information in the compilation output.  If you include the "-g" flag in your compilation command line then the generated assembly file becomes a bit messy, as the code will have extra debugging labels interspersed and will contain a lot of debugging information at the end.  Therefore, if you want to look at the .s assembler files it may be best to leave out the "-g" flag.

## Examining Generated Code After Linking ##

An alternate way to look at the generated code is asking PropGCC to create an assembly language dump of the entire executable.  The way to do this is:

```
propeller-elf-objdump -d -S main.elf | propeller-elf-c++filt --strip-underscore > main.dump
```

propeller-elf-objdump's -d flag says "disassemble the code"; the propeller-elf-c++filt command translates mangled C++ symbols into human-readable form.  The dissasembly dump of the entire executable can be quite useful, but there are several nuances to consider:

  * This dump includes all the assembly code for the entire executable, including the LMM/XMM/XMMC kernel and all the library routines.
  * The disassembled code sometimes nonsensical, especially if you start reading it from the top.  For example, in an LMM program the first 32 bytes are not assembly code, they are the Spin boot parameters and the bootstrap Spin program that starts the LMM Kernel.  Also, the first 68 bytes of the kernel are the registers, but the registers appear to hold actual assembly code - this is the LMM Kernel's initialization code (because it's run only once, it's located in the register bank where it will soon be overwritten).  Therefore, it may be easiest not to read from the top but instead start by searching for your own code (e.g. "main").
  * The dump contains disassembly code for some data areas.  Consider the following example where the second line represents a 0x41200000 "immediate" value that the LMM kernel moves to [R1](https://code.google.com/p/propgcc/source/detail?r=1); the assembly command to the right of the value is bogus because address 1028 will never be executed.
```
    1024:	5c7c002e 			jmp	b8 <__LMM_MVI_r1> 
    1028:	41200000 	if_c_and_z	mins	0 <__clkfreq>, 0 <__clkfreq> wc, nr
```
  * Similarly, the dump contains disassembly for data tables in the .hub section (remember, the .hub section typically contains both code and data).

The -S flag says "include source code lines intermixed with the disassembly"; there are several things you should consider here:

  * This only works if:
    1. You have compiled using the "-g" flag (note that this is the opposite of looking at the .s file generated by the "-save-temps" flag where "-g" can cause confusing output)
    1. You did not include the "-s" flag (strip all debugging symbols) to the linker.
  * If you are using any optimization flag except -O0, the source lines can be a bit confusing, because the optimizer can heavily rearrange the code.  For example, the source lines can appear out of order, the source lines might not appear next to the corresponding disassembly, and certain source lines might not appear at all while others may appear more than once.  To get a sense of the way PropGCC generates code, first compile without any optimization flags or else use the explicit -O0 flag to disable optimization.  Then, after you get familiar with PropGCC's code generation you'll find the optimized code easier to read.

## Examining the Memory Map After Linking ##

After you compile and link, PropGCC tells you very little about the size of your program.  However, you can get a general idea of the size of your program if you use "-s" linker flag to strip your program and then look at the size of the generated .ELF file.  Or if you can wait until you download the program to your Propeller system, as the "propeller-load" command tells you how big your program is.

However, this is not good enough in a lot of circumstances, particularly if you would like to answer detailed memory usage questions such as: "how big is each object and library module that is linking in" or "how big is each subroutine"?

To get this detailed information, you can use the "-Xlinker -Map=_yourfile_.map flags, which ask the linker to produce a memory map.  For example:

```
propeller-elf-g++ -Xlinker -Map=main.rawmap -mlmm -o main.elf main.o c++-alloc.o 
```

This map is quite useful, but it can be difficult to read for several reasons:

  1. It contains these really long, really noisy file names with longer-than-full paths (because the paths include backtracking in the form of ../../..)
  1. The lengths of objects are in hex
  1. Individual routines have an address but no calculated lengths
  1. Nothing rolls up the individual object files costs in each section to give you a total cost for a each object file.
  1. C++ routine names are mangled.

To fix this, use the propgcc-map-sizes.pl script found in the [Downloads](http://code.google.com/p/propgcc/downloads/list) section, coupled with the C++ demangler:

```
propeller-elf-c++filt --strip-underscore < main.rawmap | perl propgcc-map-sizes.pl > main.map
```

In addition to filtering the "raw" map to a "prettier" form, this command also immediately prints out summary of the memory usage.  You could get a similar map by using the "propeller-elf-size -A" command, but unfortunately that command includes debug information in the totals.  The propgcc-map-sizes.pl script works fine whether or not you've compiled with the -g and/or linked with the -s flag.

This is an example of the summary:

```
section             hex     decimal
boot         0x00000020          32
lmmkernel    0x00000788        1928
init         0x000000b8         184
text         0x00005b6c       23404
fini         0x0000003c          60
hub          0x000005f0        1520
data         0x00000000           0
ctors        0x00000008           8
dtors        0x0000000c          12
bss          0x000002bc         700
heap         0x00000004           4
------       ----------  ----------
Total:       0x00006ccc       27852
```

The net of this is that if you add the above filter to your makefile (or the shell command file you use to compile/link your executable) then you can immediately see the size of your executable after your compile.

Note that to run the script you need to have Perl installed on your machine and in your path.  Perl will almost certainly already installed if you're running Linux.  Otherwise, you can visit [ActiveState.com](http://www.activestate.com/activeperl/downloads) or a similar site and download a Perl installer.