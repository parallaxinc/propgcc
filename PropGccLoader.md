# Propeller GCC Loader #

The loader writes GCC programs to the Propeller via a serial port. The loader and Propeller GCC can run on Linux, Mac OSX, or Windows.

This is a work in progress ....

## Loader Features ##

  * Load GCC COG, LMM, XMM, and XMMC programs
  * Uses a config file for system properties
  * Parameters can overload/replace any config property
  * Loads specified XMM cache driver
  * Load SPIN .binary programs
  * Simple command interface
  * Built-in terminal for command line users
  * Make .pex files for executing from SD Card
  * Auto-identify serial ports

## Installation ##

The Propeller GCC distribution package will have the loader. In the Linux package, it is placed in usr/local/propeller/bin (copied to /opt/parallax/bin on install) and is accessible via the modified PATH variable. Other installations will handle it differently.

## Simple Usage ##

Once a Propeller program is built with GCC, the propeller-load program can send the code to the Propeller.

Below are a simple examples for command line users. The same type of result can be had with and IDE like Eclipse, but it is work in progress.

For makefile users, some demo makefiles let you build and start propeller-load with one command such as $ make PORT=0 run

### LMM Example ###

This example loads the fft\_bench program built with:
  1. $ make clean
  1. $ make MODEL=lmm
```
$ propeller-load -r -t fft_bench.elf
Propeller Version 1 on /dev/ttyUSB0
Writing 25580 bytes to Propeller RAM.
Verifying ... Upload OK!
fft_bench v1.0
Freq.    Magnitude
00000000 1fe
000000c0 1ff
00000140 1ff
00000200 1ff
1024 point bit-reversal and butterfly run time = 47 ms
```

### LMM Example ###

This example loads the demos/fibo/lmm program built with:
  1. $ make clean
  1. $ make
```
$ propeller-load -r -t fibo.elf -b c3
Propeller Version 1 on /dev/ttyUSB0
Writing 14312 bytes to Propeller RAM.
Verifying ... Upload OK!
[ Entering terminal mode. Type ESC or Control-C to exit. ]
hello, world!
fibo(00) = 000000 (00000ms) (1088 ticks)
fibo(01) = 000001 (00000ms) (1088 ticks)
fibo(02) = 000001 (00000ms) (1472 ticks)
fibo(03) = 000002 (00000ms) (1856 ticks)
fibo(04) = 000003 (00000ms) (2608 ticks)
fibo(05) = 000005 (00000ms) (3744 ticks)
fibo(06) = 000008 (00000ms) (5632 ticks)
fibo(07) = 000013 (00000ms) (8656 ticks)
fibo(08) = 000021 (00000ms) (13568 ticks)
fibo(09) = 000034 (00000ms) (21504 ticks)
fibo(10) = 000055 (00000ms) (34352 ticks)
fibo(11) = 000089 (00000ms) (55136 ticks)
fibo(12) = 000144 (00001ms) (88768 ticks)
fibo(13) = 000233 (00001ms) (143184 ticks)
fibo(14) = 000377 (00002ms) (231232 ticks)
fibo(15) = 000610 (00004ms) (373696 ticks)
fibo(16) = 000987 (00007ms) (604208 ticks)
fibo(17) = 001597 (00012ms) (977184 ticks)
fibo(18) = 002584 (00019ms) (1580672 ticks)
fibo(19) = 004181 (00031ms) (2557136 ticks)
fibo(20) = 006765 (00051ms) (4137088 ticks)
fibo(21) = 010946 (00083ms) (6693504 ticks)
fibo(22) = 017711 (00135ms) (10829872 ticks)
fibo(23) = 028657 (00219ms) (17522656 ticks)
fibo(24) = 046368 (00354ms) (28351808 ticks)
fibo(25) = 075025 (00573ms) (45873744 ticks)
fibo(26) = 121393 (00927ms) (74224832 ticks)
```

### XMM Example ###

This example loads the demos/fibo/xmm built with:
  1. $ make clean
  1. $ make
```
$ propeller-load -r -t fibo.elf -b c3
Propeller Version 1 on /dev/ttyUSB0
Writing 4556 bytes to Propeller RAM.
Verifying ... Upload OK!
Loading cache driver
1968 bytes sent                  
Loading program image
14560 bytes sent                  
Loading .xmmkernel
1632 bytes sent                  
[ Entering terminal mode. Type ESC or Control-C to exit. ]
hello, world!
fibo(00) = 000000 (00000ms) (59968 ticks)
fibo(01) = 000001 (00000ms) (57440 ticks)
fibo(02) = 000001 (00000ms) (57824 ticks)
fibo(03) = 000002 (00000ms) (58208 ticks)
fibo(04) = 000003 (00000ms) (58960 ticks)
fibo(05) = 000005 (00000ms) (60096 ticks)
fibo(06) = 000008 (00000ms) (61984 ticks)
fibo(07) = 000013 (00000ms) (65008 ticks)
fibo(08) = 000021 (00000ms) (69920 ticks)
fibo(09) = 000034 (00000ms) (77856 ticks)
fibo(10) = 000055 (00001ms) (90704 ticks)
fibo(11) = 000089 (00001ms) (111488 ticks)
fibo(12) = 000144 (00001ms) (145120 ticks)
fibo(13) = 000233 (00002ms) (199536 ticks)
fibo(14) = 000377 (00003ms) (287584 ticks)
fibo(15) = 000610 (00005ms) (430048 ticks)
fibo(16) = 000987 (00008ms) (660560 ticks)
fibo(17) = 001597 (00012ms) (1033536 ticks)
fibo(18) = 002584 (00020ms) (1637024 ticks)
fibo(19) = 004181 (00032ms) (2613488 ticks)
fibo(20) = 006765 (00052ms) (4193440 ticks)
fibo(21) = 010946 (00084ms) (6749856 ticks)
fibo(22) = 017711 (00136ms) (10886224 ticks)
fibo(23) = 028657 (00219ms) (17579008 ticks)
fibo(24) = 046368 (00355ms) (28408160 ticks)
fibo(25) = 075025 (00574ms) (45930096 ticks)
fibo(26) = 121393 (00928ms) (74281184 ticks)
```

### XMMC Example ###

This example loads the demos/fibo/xmmc built with:
  1. $ make clean
  1. $ make
```
$ propeller-load -r -t *.elf -b c3
Propeller Version 1 on /dev/ttyUSB0
Writing 4556 bytes to Propeller RAM.
Verifying ... Upload OK!
Loading cache driver
1968 bytes sent                  
Loading program image
12816 bytes sent                  
Loading .xmmkernel
1632 bytes sent                  
[ Entering terminal mode. Type ESC or Control-C to exit. ]
hello, world!
fibo(00) = 000000 (00000ms) (32064 ticks)
fibo(01) = 000001 (00000ms) (29696 ticks)
fibo(02) = 000001 (00000ms) (30080 ticks)
fibo(03) = 000002 (00000ms) (30464 ticks)
fibo(04) = 000003 (00000ms) (31216 ticks)
fibo(05) = 000005 (00000ms) (32352 ticks)
fibo(06) = 000008 (00000ms) (34240 ticks)
fibo(07) = 000013 (00000ms) (37264 ticks)
fibo(08) = 000021 (00000ms) (42176 ticks)
fibo(09) = 000034 (00000ms) (50112 ticks)
fibo(10) = 000055 (00000ms) (62960 ticks)
fibo(11) = 000089 (00001ms) (83744 ticks)
fibo(12) = 000144 (00001ms) (117376 ticks)
fibo(13) = 000233 (00002ms) (171792 ticks)
fibo(14) = 000377 (00003ms) (259840 ticks)
fibo(15) = 000610 (00005ms) (402304 ticks)
fibo(16) = 000987 (00007ms) (632816 ticks)
fibo(17) = 001597 (00012ms) (1005792 ticks)
fibo(18) = 002584 (00020ms) (1609280 ticks)
fibo(19) = 004181 (00032ms) (2585744 ticks)
fibo(20) = 006765 (00052ms) (4165696 ticks)
fibo(21) = 010946 (00084ms) (6722112 ticks)
fibo(22) = 017711 (00135ms) (10858480 ticks)
fibo(23) = 028657 (00219ms) (17551264 ticks)
fibo(24) = 046368 (00354ms) (28380416 ticks)
fibo(25) = 075025 (00573ms) (45902352 ticks)
fibo(26) = 121393 (00928ms) (74253440 ticks)
```

#### XMMC SD Example ####

This example loads the demos/fibo/xmmc built with:
  1. $ make clean
  1. $ make

> Additional steps for running the program from SD card.
  1. $ propeller-load -x fibo.elf
  1. Insert SD card (shows as /media/3644-1A33).
  1. $ cp fibo.pex /media/3644-1A33/AUTORUN.PEX
  1. Eject SD card and insert to C3.
```
$ propeller-load -r -t -z -b c3
Propeller Version 1 on /dev/ttyUSB0
Writing 22740 bytes to Propeller RAM.
Verifying ... Upload OK!
[ Entering terminal mode. Type ESC or Control-C to exit. ]
loading cache driver
initializing sd card
mounting sd filesystem
label:                    NO NAME    
type:                     FAT32
bytesPerSector:           512
sectorsPerCluster:        32
reservedSectorCount:      6318
numberOfFATs:             2
rootEntryCount:           0
totalSectorCount:         3842048
FATSize:                  937
firstRootDirectorySector: 8192
rootDirectorySectorCount: 32
firstFATSector:           6318
firstDataSector:          8192
dataSectorCount:          3833856
clusterCount:             119808
opening AUTORUN.PEX
loading kernel
loading cluster map
initializing cache
starting program

fibo(00) = 000000 (00000ms) (4160 ticks)
fibo(01) = 000001 (00000ms) (1792 ticks)
fibo(02) = 000001 (00000ms) (2176 ticks)
fibo(03) = 000002 (00000ms) (2560 ticks)
fibo(04) = 000003 (00000ms) (3312 ticks)
fibo(05) = 000005 (00000ms) (4448 ticks)
fibo(06) = 000008 (00000ms) (6336 ticks)
fibo(07) = 000013 (00000ms) (9360 ticks)
fibo(08) = 000021 (00000ms) (14272 ticks)
fibo(09) = 000034 (00000ms) (22208 ticks)
fibo(10) = 000055 (00000ms) (35056 ticks)
fibo(11) = 000089 (00000ms) (55840 ticks)
fibo(12) = 000144 (00001ms) (89472 ticks)
fibo(13) = 000233 (00001ms) (143888 ticks)
fibo(14) = 000377 (00002ms) (231936 ticks)
fibo(15) = 000610 (00004ms) (374400 ticks)
fibo(16) = 000987 (00007ms) (604912 ticks)
fibo(17) = 001597 (00012ms) (977888 ticks)
fibo(18) = 002584 (00019ms) (1581376 ticks)
fibo(19) = 004181 (00031ms) (2557840 ticks)
fibo(20) = 006765 (00051ms) (4137792 ticks)
fibo(21) = 010946 (00083ms) (6694208 ticks)
fibo(22) = 017711 (00135ms) (10830576 ticks)
fibo(23) = 028657 (00219ms) (17523360 ticks)
fibo(24) = 046368 (00354ms) (28352512 ticks)
fibo(25) = 075025 (00573ms) (45874448 ticks)
fibo(26) = 121393 (00927ms) (74225536 ticks)
```

## Loader Option Details ##

The command line loader has a built-in syntax help that explains -r and -t and the other parameters. Use propeller-load -h for help.
```
  Begin Example

C:\>propeller-load -h
usage: propeller-load
         [ -b <type> ]     select target board (default is 'default')
         [ -p <port> ]     serial port (default is to auto-detect the port)
         [ -I <path> ]     add a directory to the include path
         [ -D var=value ]  define a board configuration variable
         [ -e ]            write the program into EEPROM
         [ -r ]            run the program after loading
         [ -s ]            write a spin .binary file for use with the Propeller Tool
         [ -x ]            write a .pex binary file for use with the SD loader
         [ -l ]            load the sd loader into either hub memory or EEPROM
         [ -z ]            load the sd cache loader into either hub memory or EEPROM
         [ -t ]            enter terminal mode after running the program
         [ -t<baud> ]      enter terminal mode with a different baud rate
         [ -q ]            quit on the exit sequence (0xff, 0x00, status)
         <name>            elf or spin binary file to load

Variables that can be set with -D are:
  clkfreq clkmode baudrate rxpin txpin tvpin cache-driver cache-size cache-param1 cache-param2
  sd-cache sdspi-do sdspi-clk sdspi-di sdspi-cs eeprom-first

Value expressions for -D can include:
  rcfast rcslow xinput xtal1 xtal2 xtal3 pll1x pll2x pll4x pll8x pll16x k m mhz true false
  an integer or two operands with a binary operator + - * / % & | or unary + or -
  all operators have the same precedence

The -b option defaults to the value of the environment variable PROPELLER_LOAD_BOARD.
The -p option defaults to the value of the environment variable PROPELLER_LOAD_PORT
if it is set. If not the port will be auto-detected.

The "sd loader" loads AUTORUN.PEX from an SD card into external memory.
It requires a board with either external RAM or ROM.

The "sd cache loader" arranges to run AUTORUN.PEX directly from the SD card.
It can be used on any board with an SD card slot.

  End Example
```

> All propeller-load options are optional except for `<`name`>`.

  * -b `<`type`>` select target board (default is default)
> This option allows users to select a board type from the propeller-load.cfg configuration file. The configuration file contains several board types and variables for each type. The configuration file variables are described further down in the page.

> The target board type "default" which is used if -b is not specified will use an 80MHz XTAL1+PLL16x clock unless overridden byte a -D variable.

> The board type is not needed for LMM and COG only programs which match the default board type. A board type can be defined in the propeller-load.cfg file that will match properties of boards that do not use defaults.

> XMM and XMMC programs always require specific hardware, and the -b option allows the user to select the board type from the configuration file for those modes.

> It is useful to note that the build MODEL (LMM, XMM, XMMC) determines how the code is executed rather than the board type. Even if you specify -b eeprom with a program compiled for LMM mode, the program will execute from HUB memory rather than from eeprom.

> It is also useful to note that if you build an XMM or XMMC model program and the loader board type is not specified, you will end up with an error because the loader doesn't know how to load the program.


  * -p `<`port`>` serial port (default is /dev/ttyUSB0)
> Normally -p sets the port. If -p is not specified the port will be /dev/ttyUSB0 for Linux, COM1 for Windows, or the value of environment variable PROPELLER\_LOAD\_PORT if set. MAC OSX users must specify the serial port name used by propeller-load either on the command line or in PROPELLER\_LOAD\_PORT.

> If -p 1 is set for propeller-load on the linux command line, /dev/ttyUSB1 will be used. If you are not using USB, you should specify the -p; I.E. -p /dev/ttyS0

  * -I `<`path`>` add a directory to the include path
> The loader can use a specific path to search for an alternative propeller-load.cfg config file and external memory cache driver.dat files. Cache driver .dat files are output of compiled PASM, GAS, or possibly COG C files designed to inter-operate with the XMM VM kernel code to give it access to external memory. This parameter allows you to specify the path for the alternative files.

  * -D var`=`value define a board configuration variable
> The loader reads -D to change values programmed on the Propeller. Normally the values will be specified by board type on the propeller-load.cfg file, but there are situations where this is less desirable.
    1. For example, the default board type used when -b is not specified uses 80MHz clock frequency and XTAL1+PLL16x clock mode; this is not appropriate for all boards. Some boards such as the Parallax SpinStamp need clock mode XTAL1+PLL8x, so using -Dclkmode=XTAL1+PLL8x will allow running a program on the SpinStamp (these values can also be set by the compiled program).
    1. For another example, during Engineering Environmental Corner testing one of the variables used is clock rate. The -D option will allow the user to specify the clock rate without needing to change a file. This type of feature also allows the user to have an interactive record for data collection.

> The -D variables are listed in the help output. Below is the same list with some explanation and valid usage examples.
    1. clkfreq  : An integer number such as 80000000, 96000000
    1. clkmode  : XTAL1+PLL16x, RCFAST, or other`*`
    1. baudrate : 115200, 57600, 38400, 19200, 9600
    1. rxpin    : An integer from 0 to 31 inclusive - typically 31
    1. txpin    : An integer from 0 to 31 inclusive - typically 30
    1. tvpin    : Only used for loader debug if enabled in code
    1. cache-driver : `<`filename`>`.dat
    1. cache-size   : depends on cache-driver 1K, 2K, 4K, 8K (some power of 2).
    1. cache-param1 : depends on cache-driver
    1. cache-param2 : depends on cache-driver
> > Note`*` clkmode can be RCSLOW, RCFAST, XINPUT or a combination of these symbols as xtal+pll: XTAL1, XTAL2, XTAL3, PLL1X, PLL2X, PLL4X, PLL8X, PLL16X.

  * -e write the program into EEPROM

> This option is used to save the Propeller GCC boot-up program into EEPROM. The boot-up program will be different for COG, LMM, and XMM/XMMC programs. COG and LMM programs will be saved into EEPROM as Propeller bootable .binary image. If the external memory code (.text) storage is flash, the XMM/XMMC program will be saved and the flash boot-up program will be saved in EEPROM. If the code storage is SRAM, the boot-up program will be saved in EEPROM, but the XMM/XMMC program will not be saved for next boot-up. The Propeller may reset after writing EEPROM, but the loader does not explicitly tell the Propeller to reset. To guarantee bootup, use -e -r together.

  * -r run the program after loading
> This option is used to save Propeller GCC boot-up code in HUB memory and cause execution to start without resetting the Propeller. It will also save any XMM/XMMC code into the hardware external memory device specified by the board in the propeller-load.cfg file. This mode can be used for testing XMM/XMMC programs loaded into external SRAM, SDRAM, or other non-volatile memory.

  * -s write a spin binary file for use with the Propeller Tool
> This option allows the user to convert a COG or LMM GCC elf program into a binary that can boot on any Propeller having the correct clock configuration.

  * -x write a .pex binary file for use with the SD loader
> This option is used to convert a linked Propeller GCC program to an image that can be booted on Propeller from an SD card. An image file to boot on the Propeller from SD card should be named AUTORUN.PEX (yes, all upper case. It may work if saved as lower case. Your mileage may vary).

  * -l load the sd loader into either hub memory or EEPROM
> This option must be used with the -r or -e option. The SD card loader may live in HUB or EEPROM. The purpose of the SD card loader is to copy the image from the SD card file AUTORUN.PEX to external memory and start the program. To put the loader into EEPROM, one must also specify -e on the command line. This will allow you to boot the SD card program without being connected to the PC. To put the loader into HUB ram, one must use -r.

> An AUTORUN.PEX file intended to run in SRAM or other non-volatile memory type must be compiled/linked using an appropriate linker script. For example, to run the fibo program as an XMMC program, it must be generated with something like this:
```
 $ propeller-elf-gcc -Os -mxmmc -Txmmc_ram.ld -o fibo.elf fibo.c
```

> More details on default scripts for linking different XMM programs can be found in the [Propeller-GCC linker scripts](LinkerScripts.md) page.

  * -t enter terminal mode after running the program
> This option will start a 115200 serial terminal immediately after loading. To exit the terminal today, press ESC. This escape mode may not be suitable for programs that use arrows or other extended keyboard command sequences.

  * -t`<`baud`>`  enter terminal mode with a different baud rate
> This option is the same as -t except it lets the user specify the baud-rate for communications.

  * -q quit on the exit sequence (0xff, 0x00, status)
> This option is used to make propeller-load exit when a program finishes by sending the 0xff, 0x00, status sequence. It is used primarily in the GCC test suite.

  * `<`name`>`    GCC elf or SPIN binary file to load
> This required parameter is the input program for the loader to use. In the case of the -s option, it is the program to convert to a SPIN binary.

## Loader Configuration File ##

The Propeller GCC loader uses a [configuration file](PropGccLoaderConfig.md) for specifying board information used to boot an image. The configuration file is not a run-time parameter file for devices, that information is defined by the user program.

The [propeller-load.cfg](PropGccLoaderConfig.md) file contains properties for board types such as hub, ssf, dracblade, sdram, and eeprom. Each board type contains a set of properties that are used by the loader for boot up.

> ### COG or LMM Board Type ###
> Here is a simple LMM example:
```
Begin Example

[hub]
    clkfreq: 80000000
    clkmode: XTAL1+PLL16X
    baudrate: 115200
    rxpin: 31
    txpin: 30
    tvpin: 12   # only used if TV_DEBUG is defined

End Example
```
> Each of these fields are the same as the -D variable names mentioned above. As mentioned, these values can be overloaded by the -D flag if necessary.
> To use the HUB board type run as:
> $ propeller-load -r -t fft\_bench.elf -b hub

> To modify the HUB board type to use 96MHz, the loader can be run as:
> $ propeller-load -r -t fft\_bench.elf -b hub -Dclkfreq=96000000


XMM/XMMC programs require a suitable board type to define loading the cache driver.

> ### XMMC Board Type ###
> An XMM program will typically keep code and global data in external memory. The C3 board type will run an XMM program from Flash and have data in the SRAM. The C3 configuration properties look like this:
```
Begin Example

[c3]
    clkfreq: 80000000
    clkmode: XTAL1+PLL16X
    baudrate: 115200
    rxpin: 31
    txpin: 30
    tvpin: 12   # only used if TV_DEBUG is defined
    cache-driver: c3_cache.dat
    cache-size: 8K
    cache-param1: 0
    cache-param2: 0

End Example
```
> In this example you will notice that c3\_cache.dat is defined as the cache driver.

> ### XMMC Board Type ###
> An XMMC program will typically keep only code external memory. The EEPROM board type will run an XMMC program from Flash and have data and stack in the Propeller HUB memory. The EEPROM configuration properties look like this:
```
Begin Example

[eeprom]
    clkfreq: 80000000
    clkmode: XTAL1+PLL16X
    baudrate: 115200
    rxpin: 31
    txpin: 30
    tvpin: 20   # only used if TV_DEBUG is defined
    cache-driver: eeprom_cache.dat
    cache-size: 8K
    cache-param1: 0
    cache-param2: 0
    eeprom-first: TRUE

End Example
```
> In this particular example you will notice that eeprom\_cache.dat is defined as the cache driver. The cache size and the two cache parameters are also defined. Normally cache-param1 and cache-param2 are 0 to signal using the default values in the cache driver. The eeprom driver can use different values, but it's best to leave them as they are.

> The Propeller program can literally be stored in the EEPROM upper address space and be executed by fetching instructions from the EEPROM via the cache. Some may scoff at this option as being too slow, but it is the minimalist hardware configuration for XMMC programs and it performs reasonably well considering the EEPROM access speed. Programs using this cache model can be up to 96KB for an AT24C1024B 128KB EEPROM.

> The eeprom-first parameter tells the loader to program the EEPROM boot image before programming the Propeller-GCC program. This allows a single programming step with propeller-load. If the EEPROM was not programmed first, part of your program would be wiped clean by the Propeller ROM, and would fail to boot. The EEPROM XMMC board type is the only one where this is a concern now.


> ## propeller-load.cfg ##
> The [propeller-load.cfg](PropGccLoaderConfig.md) file usually found in /opt/parallax/propeller-load contains basic board type descriptions. The file does not specify user device information.