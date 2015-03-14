# Alpha Test Release Notes #

propgcc-v0\_2\_4 - snap time: 2012-03-07-14:00:23

Various propeller load changes:

Directory /opt/parallax/propeller-load is reorganized so that board types are in separate .cfg config files. Added rcfast.cfg and rcslow.cfg.

The propeller-load program now exits more quickly if a propeller is not found on a port.

There were cases where propeller-load waited too long after reset before starting the load process. An extra 50 milli-second delay was removed and the loader is solid now. The old loader was prone to having trouble with QuickStart boards.


Fixed a bug in the SimpleSerial driver that could cause trouble at lower baud-rates.

The ebasic demo program now has CAT, LOAD, and SAVE commands (enabled with -DLOAD\_SAVE flags). Some propeller specific commands have also been added such as HIGH, LOW, TOGGLE, DIR, GETDIR, CNT, PAUSE, PULSEIN, and PULSEOUT - see ebasic README.txt.


Most other changes are related to gdb.


propgcc-v0\_2\_2 - snap time: 2011-12-21

Fixed the C3 XMM driver so that it works correctly if started when an SD card is inserted.

Changed the way the linker allocates cog overlays. Previously there were fixed cog overlay sections named ".cogsys0" - ".cogsys7" and ".coguser0" -".coguser7". Now any section starting or ending with ".cog" is treated as an overlay for cog memory.

Released library source code as a separate archive (propgcc-libsrc) and moved demos into their own file (propgcc-demos).

Some minor bug fixes:
```
Updated demos for new linker naming scheme.
Improved output of code in -mspin mode (and include a small spin wrapper for the pasm code).
Fixed an issue where _COGMEM variables were sometimes placed in the wrong sections.
Added a C++ wrapper for assert.h.
Added the isnan function to libm.
```

propgcc-v0\_2\_1 - snap time: 2011-12-10

Fixed several packaging problems with the release.

Other changes:
```
Fixed the missing gmtime function and incorrect prototype for it.
Fixed the copy_from_xmm function in propeller.h which was causing some demos to fail to build in XMM mode.
Added a single-cog pong demo.
```
NOTE: there is an unresolved problem with downloading XMM and XMMC code to the C3 board when an SD card is inserted. Please remove any SD card before launching XMM and XMMC programs on your C3 board. We are looking into this.


propgcc-v0\_2\_0 - snap time: 2011-12-09

We've bumped the version number for this release, because there's been a major change in the way the compiler accesses data in external memory. The new way is smaller (2 longs per access instead of 3) and faster, but is not compatible -- you'll have to make sure you delete any old XMM object files before using the new compiler.

Other changes:
```
New batch files PropGCC.bat and addpath.bat to simplify command line development on Windows (see README_WINDOWS.txt).
Made memcpy work when copying data from code space to hub RAM in XMMC mode.
Fixed the __builtin_propeller_waitvid function to generate the correct code.
Fixed initialization of variables declared in cog memory.
The disassembler shows the "brs" pseudo-instruction in LMM code.
Added a simple basic interpreter demo (ebasic) from David Betz.
Added a pthreads_toggle demo.
Various other bug fixes and demo improvements.
Added C3F board type for faster C3 XMMC programs. Thanks Dave Hein.
Added a -v switch to propeller-load to show progress in port scanning.
```

NOTE: there is an unresolved problem with downloading XMM and XMMC code to the C3 board when an SD card is inserted. Please remove any SD card before launching XMM and XMMC programs on your C3 board. We are looking into this.

propgcc-v0\_1\_9 - snap time: 2011-11-25-16:55:5

Here's a list of things we have in the test package:
```
SD Card XMMC allows running programs from SD Card.
Auto-detect and use first port with propeller-load.
Full C++ support. Use what you need.
More hardware boot configurations.
Demo of pthreads running N threads on all COGs (LMM only; 1 COG for XMM/XMMC) with non-preemtive multiprocessing.
Term demo allows using TV,VGA,or Serial port with obj.str, obj.hex, etc....
Smaller 32bit double library support (64bit still default).
Build and load small main programs for COG mode, other improvements, and bug fixes.
Licensing files and information - more complete. Waiting for legal review.
Windows support without Cygwin.
Various Alpha tester demos (Ariba, blittled, Heater, Kevin Wood, Mindrobots, Rienhart, others).
Console input/output LF/CR fixes for PST
More library support for ANSI C99
Add ASC to propeller-load.cfg
User contributions in demos/forumists/<username>
CLKMODE/clkset bug fix
Added PropGCC.bat to start a Propeller-GCC windows command console.

Many other enhancements.
```

propgcc-v0\_1\_8 - was preliminary only.

propgcc-v0\_1\_7 - snap time: 2011-11-03-18:02:28

Notes:

Build with latest fixes. Windows v0\_1\_7 has more features than Linux v0\_1\_7 because of version tracker issue.

  1. Includes grep and .dll missing from previous build.
  1. Includes updates to loader and board types.
  1. Includes propeller.h which has common propeller functions.
  1. Includes c3files filetest for SD card.

```
C:\propgcc\demos\c3files>propeller-load -r -t *.elf -p 21
Propeller Version 1 on COM21
Writing 27816 bytes to Propeller RAM.
Verifying ... Upload OK!
[ Entering terminal mode. Type ESC or Control-C to exit. ]

Available commands are help, cat, rm, ls and echo

> ls
HELLO

> cat HELLO
echo "Hello World"

> echo "Hello World"
"Hello World"

>
```

Changes:
```
Rev	Commit log message	Date	Author
5f065330e946	Build number bump for distribution.	Today (10 minutes ago)	Steve Denson <jsdenson@gmail.com>
80d01ad22dc4	Added exit(1) for cygwin serial port pull problem. Added propeller-load.cfg entries.	Today (11 minutes ago)	Steve Denson <jsdenson@gmail.com>
76f050de4a51	Removing unsupported taskswitch.	Today (12 minutes ago)	Steve Denson <jsdenson@gmail.com>
61e13ae541b4	Enhanced lib/propeller.h and removed old demos/include/propeller.h	Today (2 hours ago)	Steve Denson <jsdenson@gmail.com>
0313971fe15a	Fixed INSTALL.txt type-o. Added comments to README_WINDOWS.txt. Added grep, etc... to release.sh	Today (2 hours ago)	Steve Denson <jsdenson@gmail.com>
da89214f7e6e	Added build.sh. It works like rebuild.sh. Someday we should use make to do rebuild.sh stuff.	Today (2 hours ago)	Steve Denson <jsdenson@gmail.com>
10d6b0390ff0	Add a banner to terminal mode to say how to exit.	Today (4 hours ago)	David Betz <dbetz@xlisper.com>
1956b6b88521	added documentation for printf	Yesterday (37 hours ago)	Eric Smith <ersmith@totalspectrum.ca>
f788fbc5ab15	merged with trunk	Yesterday (37 hours ago)	Eric Smith <ersmith@totalspectrum.ca>
113b0d851a65	added some defines to sys/types.h	Yesterday (37 hours ago)	Eric Smith <ersmith@totalspectrum.ca>
d6d58e242701	Fixed a bug with the SD driver mailbox in the c3files demo	Nov 1 (46 hours ago)	Dave Hein <davehein3@gmail.com>
9ac147b04bf7	committing daily build.	Nov 1 (2 days ago)	Daniel Harris <Teknician89@gmail.com>
1b741c547f39	fixed cog linker specs	Nov 1 (2 days ago)	Eric Smith <ersmith@totalspectrum.ca>
7780030079c1	added libcog to build	Nov 1 (2 days ago)	Eric Smith <ersmith@totalspectrum.ca>
06438bad1444	Adding c3files to demos	Nov 1 (2 days ago)	Dave Hein <davehein3@gmail.com>
1423f757fcfb	Some updates to the pex file generation.	Nov 1 (2 days ago)	David Betz <dbetz@xlisper.com>
d5eb125daec2	Added cut to release.sh. Changed line endings on INSTALL.txt to windows.	Oct 31 (3 days ago)	Steve Denson <jsdenson@gmail.com>
7957a2cfca6f	clean up library makefile	Oct 31 (3 days ago)	Eric Smith <ersmith@totalspectrum.ca>
7b36ce8485e8	merge with main repo	Oct 30 (3 days ago)	Eric Smith <ersmith@totalspectrum.ca>
8d8ff283cec9	provide a way for another cog to update the default real time clock driver	Oct 30 (3 days ago)	Eric Smith <ersmith@totalspectrum.ca>
b4dda06b72a1	revise how we link .cog files so that they can access symbols in the main program, and the linker can layout the hub data sections for all the cogs	Oct 30 (3 days ago)	Eric Smith <ersmith@totalspectrum.ca>
6044b1dbf562	add some comments to the toggle code Makefile	Oct 30 (4 days ago)	Eric Smith <ersmith@totalspectrum.ca>
495d1122e7c7	Added V0_1_3 timestamp.	Oct 29 (5 days ago)	Steve Denson <jsdenson@gmail.com>
bea6b9ed4676	Remove the prototypes for the console_xxx() functions since they are now local to the osint_xxx.c files.	Oct 28 (5 days ago)	David Betz <dbetz@xlisper.com>
77d0ba7d2e4f	Added the ability to write Propeller executable files (.pex) that will be used by the SD loader. Renamed the -x option to -q. Added a -x option to write a .pex file.	Oct 28 (6 days ago)	David Betz <dbetz@xlisper.com>
6c1068294601	documented cooked mode	Oct 28 (6 days ago)	Eric Smith <ersmith@totalspectrum.ca>
731db2d6c3c0	changed rtc to use struct timeval, and added gettimeofday and settimeofday functions	Oct 28 (6 days ago)	Eric Smith <ersmith@totalspectrum.ca>
```

propgcc-v0\_1\_3 - snap time: 2011-10-27-10:38:11

Notes:

Users can build XMMC programs (code only in external memory) and run them in EEPROM. One example is xbasic for 128KB Hydra EEPROM (or C3).

```
$ cd demos/xbasic
$ make

#for 128K byte EEPROM
$ propeller-load -r -t xbasic.elf -b eeprom -p <port name>
#or for C3
$ propeller-load -r -t xbasic.elf -b c3 -p <port name>
```

```
In xbasic enter this program:
10 printf("Count to 100\n")
20 for n = 0 to 100
30  if n mod 10 = 0 then
40    printf("\n")
50  end if
60  printf("%d ",n)
70 next n

In xbasic enter: run

Output:

run
H:496 O:4 D:114 V:1 T:124
Count to 100

0 1 2 3 4 5 6 7 8 9
10 11 12 13 14 15 16 17 18 19
20 21 22 23 24 25 26 27 28 29
30 31 32 33 34 35 36 37 38 39
40 41 42 43 44 45 46 47 48 49
50 51 52 53 54 55 56 57 58 59
60 61 62 63 64 65 66 67 68 69
70 71 72 73 74 75 76 77 78 79
80 81 82 83 84 85 86 87 88 89
90 91 92 93 94 95 96 97 98 99
100 OK
```

Changes:
```
3798487afca8	adjust eeprom_cache.spin to allow running bigger programs. set xbasic serial I/O driver to FullDuplexSerial.	Yesterday (38 hours ago)
f3f7bf258d9f	added generic terminal layer for FdSerial and SimpleSerial	Yesterday (40 hours ago)
d43d503e87c3	fixed XMM code generation	Yesterday (40 hours ago)
f37f21baae05	added function versions of some macros in stdio.h	Oct 27 (42 hours ago)
9b025ffb32fd	minor cleanup	Oct 26 (2 days ago)
731eeec0aae6	remove .hub_ram directive if USE_HUBCOG_DIRECTIVES is 0	Oct 26 (2 days ago)
b4b9ea86cb31	fixed a few issues with native functions in LMM mode	Oct 26 (2 days ago)
fceb8f959792	re-organized code so that native functions will always go in COG memory, even in LMM mode	Oct 26 (2 days ago)
b3bc4317581f	updated driver documentation	Oct 26 (2 days ago)
5957e1e3da6c	added information on time zones	Oct 26 (2 days ago)
9f6f171b6d24	updated documentation	Oct 25 (3 days ago)
c3bff3a3091a	Make all propeller-load.cfg entries 80MHz default. Added demoboard, hydra. Removed special versions like hub96. Updated README.txt to more closely reflect the current build. Fixed copy/paste error in release.sh.	Oct 23 (5 days ago)
```

propgcc-v0\_1\_2 - snap time: 2011-10-22-19:08:52