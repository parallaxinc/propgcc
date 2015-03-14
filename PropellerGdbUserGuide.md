# This is a work in progress. Pardon the mess. #

# Introduction #

GDB is the GNU debugger available with Propeller GCC for debugging programs.

# Contents #

  * Requirements
  * Starting GDB
  * Setting Breakpoints
  * Code Stepping
  * Displaying Data

> ## Requirements ##
> To use Propeller GDB, the Propeller GCC version must be at least Alpha\_1\_9. This is built from the propgcc.googlecode.com default branch.
> Currently, the command is the only debugger available. Some GUI tools may be integrated at some point.

> ## Starting GDB ##
> Since GDB is only available on the command line at the moment, open a window that has propeller-gcc (C:\propgcc\bin or /opt/parallax/bin) in it's PATH. On windows, this can be done by clicking the C:\propgcc\PropGCC.bat file in explorer.

> We will use a hello project just to get started.
> Go to some development folder and do the following steps.
    1. Create a hello.c file as shown below.
    1. Compile using this command: **propeller-elf-gcc -g -o hello.elf -c hello.c**
    1. Load (assuming 5MHz crystal) like this: **propeller-load -g -r hello.elf**
    1. Start gdb like this: **propeller-elf-gdb hello.elf**
    1. Enter "c" (without quotes) to run the program
    1. Press CTRL-C a few times, then type quit

> The output should look like below. Most of the output of GDB is actually meaningless, but that's GDB output that is controlled by FSF and not the Propeller-GCC developers. While it may seem a little overwhelming now, the output will make much more sense after reading the sections in this wiki page.

> The hello.c source.
```
#include <stdio.h>
void main(void)
{
  puts("Hello, world.");
}
```

> The gdb output.
```
Microsoft Windows [Version 6.1.7601]
Copyright (c) 2009 Microsoft Corporation.  All rights reserved.

C:\propgcc>cd hello

C:\propgcc\hello>ls
hello.c  hello.elf

C:\propgcc\hello>rm hello.elf

C:\propgcc\hello>propeller-elf-gcc -g -o hello.elf hello.c

C:\propgcc\hello>propeller-load -g -r hello.elf
Propeller Version 1 on COM52
patching for debug at offset 0x68
Loading hello.elf to hub memory
6964 bytes sent
Verifying RAM ... OK

C:\propgcc\hello>propeller-elf-gdb hello.elf
GNU gdb (GDB) 7.3.1
Copyright (C) 2011 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
and "show warranty" for details.
This GDB was configured as "--host=i586-mingw32msvc --target=propeller-elf".
For bug reporting instructions, please see:
<http://www.gnu.org/software/gdb/bugs/>...
0x000005b0 in ?? ()
Reading symbols from C:\propgcc\hello/hello.elf...done.
(gdb) c
Continuing.
Hello, world.
Remote communication error.  Target disconnected.: Interrupted function call.
(gdb) quit

C:\propgcc\hello>
```

> Ignore these lines if you see them:
```
warning: Can not parse XML memory map; XML support was disabled at compile time
Remote communication error.  Target   disconnected.: Interrupted function call.
```

> The last line is expected in this case because the program finished and exited from main.

> ## Code Stepping ##
> Stepping through code allows us to enter (and test assumptions) about a thread of execution in a program. Mechanically inclined people learn more about coding from this than by reading half-dozen programming books.

> ## Setting Breakpoints ##
> Setting a breakpoint is a very common thing to do in a debugger.


> ## Displaying Data ##
> While stepping through code it may be necessary to examine and/or change the value of some variables.