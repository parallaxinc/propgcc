# Introduction #

The Parallax Propeller has a unique architecture which was designed for the Spin and PASM (Propeller Assembler) languages, not for C or C++. Some of the incompatibilities are:
  * Programs run in one of eight cogs, each with their own memory. Cogs can communicate through hub memory, but they don't have access to the memory in any of the other cogs.
  * Hub memory and cog memory maps both start at location 0, but they don't overlap; they are very distinct memory areas that need to be accessed with different machine language instructions.
  * Each cog memory location is a 32-bit longword, but each hub memory location is an 8-bit byte.
  * There are no registers like you might find in other processors: all instructions work with a source and a destination memory location.
  * There is no built-in stack; instead of storing return addresses on a stack, the instruction at the end of a subroutine is modified to jump back to the correct location.

The engineers who ported GCC to the Propeller did an awesome job solving -- or at least working around -- these problems, while still making it possible to use all the same features that Spin and PASM programmers enjoy.

This page provides background information about some of the aspects of how PropGCC compiles your C code to machine language. The information also applies to C++ but the sample code fragments use C. It will also tell you how to combine different memory models to use the advantages of each.

The document was written for programmers who already have experience with C, but are new to the Propeller. Knowledge of Spin or PASM is not required, but you should at least have some idea of the architecture of the Propeller. Before you continue reading the rest of this page, you may want to read the following:
  * The Propeller Datasheet, or Chapter 1 of the Propeller Reference Manual, both available on [this web page](http://www.parallax.com/ProductInfo/Microcontrollers/PropellerGeneralInformation/PropellerMediaPage/tabid/832/Default.aspx), and:
  * The SimpleIDE User Guide, available on [this page](https://sites.google.com/site/propellergcc/documentation/simpleide)

Disclaimer: I (Jac Goudsmit) was not involved in the development of PropGCC or SimpleIDE, and much of this information was reverse-engineered from forum posts by others, from Assembly code generated from sample programs, and from other documentation on the [PropGCC Google Code website](http://sites.google.com/site/propellergcc). Some of it was based on reasoning, based on my experience with many different C compilers and Assembler variations. If you find a mistake, please post a comment here or in the [Parallax PropGCC forums](http://forums.parallax.com/forumdisplay.php?93-Propeller-GCC-Beta).

# Memory Models and Mixed-Mode Programming #

The Propeller microcontroller can run up to (about) 20 million instructions per second per cog, when it's running at 80MHz. When you compile programss in the "cog" memory model, PropGCC generates code that runs at this full speed. This is also called _"native mode"_.

But each cog of the Propeller only has limited memory space for instructions and data: 496 longs of 32 bits each. The PropGCC developers recognized that this is not enough for many programs, so you can choose from a number of different [memory models](PropGccMemory.md) depending on your needs.

For example, when using LMM (Large Memory Model), a small kernel is started which retrieves one machine language instruction from the hub at a time, and then executes it. Obviously this makes the code run slower, but it changes the maximum code size for your program to almost 32KB instead of 2KB. Other memory models allow you to store parts of your program in an EEPROM, or on an SD card, permitting even larger programs.

In many projects, you will find that the speed penalty of a non-native memory model is acceptable for most of the program, but not for some parts. You'll want to compile the time-critical parts, such as the modules that communicate directly with the hardware, in cog mode, and the rest of the program in another memory model. This is called _"mixed-mode programming"_.

> In SimpleIDE, you set the default memory model in the project options, but you can compile some modules in cog mode (regardless of the project's memory model settings) by using file names that end in `.cogc` instead of `.c`.

You can have as many `.cogc` modules in your project as you want. Some early code or documentation may have mentioned that there is only enough space for eight `.cogc` modules but this is no longer true. It made no sense either: there is no reason to limit the number of cog segments (and thus the number of `.cogc` modules) to 8. It's true that only 8 cogs can execute code at a time, but it's very well possible to re-use a hardware cog for many different purposes. For example, your program could have a `.cogc` module that generates a video signal and another `.cogc` module that generates a bitmap file, and decide at run-time which module you want to run on a particular cog.

> In SimpleIDE, all `.cogc` files of your project must be in the same directory as your `.side` file.

The reason that all `.cogc` modules need to be in the project directory is that the build process uses the file location (relative to the `.side` file) to rename some symbols in the object file. If you would put a `.cogc` file in another directory, the build process would attempt to use the slash ('/') or backslash ('\') from the path name as part of the name of those renamed symbols, and the build fails because those characters are illegal as part of those symbols.

# Under the Hood #

Before we get into further details about how to program a `.cogc` module, it may help to take a look at some aspects of how PropGCC translates your C code to Assembler, and how a cog runs your code.

## Startup Code ##

Whenever the Propeller starts a cog, it copies a block of memory from the hub to cog memory, and then starts executing at location 0 in cog memory. The code that gets placed there is the C runtime library startup code, [crt0\_cog.s](http://code.google.com/p/propgcc/source/browse/lib/cog/crt0_cog.s). This is what happens in the initialization code:
  1. The stack pointer (discussed [below](PropGccInDepth#Stack_Pointer:_sp.md)) is copied from `PAR`.
  1. The `r0` pseudo-register is copied from `PAR`. That way, if `main()` is declared with a parameter, this is used as its value.
  1. If this is the first cog that gets started, it allocates a lock for the runtime library. The lock is shared between all cogs that run C code.
  1. If this is the first cog that gets started, it wipes the hub memory area where all static and global variables reside (commonly known as BSS).
  1. The code calls `_main` which is the public symbol of the `main()` function.
  1. After `main()` returns, the code calls `__exit` which stops the cog (see [this source file](http://code.google.com/p/propgcc/source/browse/lib/cog/crtend_cog.s)).

Here's an important thing to notice from the above:

> Static variables in hub memory are only initialized to zero when the first cog is started, right after a reset. Cog memory static variables, however, are initialized every time the code is loaded into a cog and started with `cognew()`.

As you may notice from the `crt0_cog.s` source code, that the pseudo-registers (which are explained [below](PropGccInDepth#General-Purpose_Pseudo-Registers:_r0_-_r15.md)) are in the same locations as the initialization code.

> You **_cannot_** restart the code in your cog by doing something like `__asm__("jmp #0")`. The initialization code gets overwritten with pseudo-register values immediately, so it won't exist anymore by the time your code is executed.

## General-Purpose Pseudo-Registers: `r0`-`r15` ##

The Propeller doesn't have any registers in the usual sense. All machine language instructions have a source and a destination, which are usually locations in cog memory, or 9-bit immediate values, and sometimes hub addresses, depending on the instruction.

The GCC compiler depends on having registers available to hold temporary values, and also to pass parameters to functions. The Propeller port of the GCC compiler generates Assembly code that uses up to 15 pseudo-registers (`r0`-`r14`) for temporary values, all 32 bits. When the assembler translates instructions that use these pseudo-registers, it actually uses the first 15 longwords in cog memory.

## Stack Pointer: `sp` ##

The Propeller doesn't have a built-in stack. The easiest way to call a subroutine in Assembly language is to use the `JMPRET` instruction which stores the location of the next instruction before it jumps to the subroutine. Normally the destination of the `JMPRET` is the location of a `JMP` instruction at the end of the subroutine: the JMPRET stores the return address in the return instruction that way. This is very efficient of course, but doesn't allow for recursivity: if a function is called before it returns (either by itself or by another function), the return address gets overwritten.

In order to implement a stack to make recursion and other features possible, the PropGCC compiler uses the `sp` pseudo-register as a stack pointer. Just like the other pseudo-registers, `sp` is stored near the start of cog memory. The stack itself is stored in hub memory and grows top-down, i.e. to push a value, the code decrements `sp` first, and then stores the value.

> The stack pointer is initialized from the `PAR` register, so if you want to run the same `.cogc` module on multiple cogs simultaneously, you need to provide a different `PAR` value (i.e. second `cognew()` parameter) for each cog that's running. More about this [below](PropGccInDepth#Second_Parameter_to_cognew().md).

## Link Register: `lr` ##

As mentioned above, the stack is stored in the hub. That means pushing data onto the stack and popping data from it are hub operations which are relatively expensive. Also, the `JMPRET` instruction which is the instruction of choice for function calls, uses a cog memory location (not a hub location) to store the return address. For these reasons, the compiler uses another pseudo-register `lr` which is also stored at the top of cog memory. The extra register makes it unnecessary to push and pop the return address to and from the stack if a function doesn't call any other functions.

To illustrate this, here's some sample code. Say, a function `caller` calls a function `calledfunction`. The code that gets generated is as follows:

```
_caller
    sub     sp, #4
    wrlong  lr, sp     ; save "caller"'s return address on stack

    ...

    jmpret  lr, #_calledfunction ; call other function

    ...

    rdlong  lr, sp     ; restore return address
    add     sp, #4

    jmp     lr         ; return from "caller"

_calledfunction
    ... No need to save lr on stack if function doesn't call
    ... any other functions

    jmp     lr         ; return from "calledfunction"
```

As you may have seen in the [C runtime startup code](http://code.google.com/p/propgcc/source/browse/lib/cog/crt0_cog.s), the `lr` register is initialized so that when `main()` returns with the usual `JMP lr`, the code jumps to the `_exit()` function (in assembler: `__exit` because by convention, the compiler adds an underscore to all symbol names) which stops the cog. In other words: all you need to do to stop the cog is return from the `main()` function.

When a function is declared `_NATIVE` or `_NAKED`, it uses a different calling convention that doesn't use the `lr` register. More information about this will follow [below](PropGccInDepth#_NATIVE_and__NAKED.md).

# The `main()` Function #

Each Propeller cog is a processor that executes its own program, independently from the other cogs. And each `.cogc` module should be regarded as a _"program within a program"_, rather than as a module that contains functions you can call.

> Because a `.cogc` module is a "program within a program", each `.cogc` module must have a `main()` function.

Just like the `main()` function at the top level of your main program, the `main()` function of your `.cogc` is the first function that gets executed and it's the last function that returns, unless you have an infinite loop or stop the cog yourself, of course.

It may seem a bit weird to name the top-level function `main`, because `.cogc` modules usually only take care of a small part of the functionality of an application (i.e. they're usually not the "main" function of the program), but the C runtime library depends on this, as you may have seen in the [previous chapter](PropGccInDepth#Startup_Code.md).

> If you forget to put a `main()` in your `.cogc` file, or if you try to rename `main()` to something else, you will get a somewhat weird "relocation truncated" error from the PropGCC compiler.

As part of the processing for `.cogc` modules, SimpleIDE uses the `propeller-elf-objcopy` program to rename the `.text` segment i.e. the part of the object file where all the code is stored. This results in a few things:
  * `.cogc` modules are artificially prevented from calling functions in (or use pointers to functions from) other modules: if you try it anyway, you will get a fatal "relocation truncated" linker error.
  * Similarly, it's artificially made impossible to call functions in, or take function pointers from any other module: a `.cogc` module can call library functions but not any functions in other modules in your project.
  * Even though the `main()` function must be public (so it can be called from `crt0_cog.s`; see [the previous chapter](PropGccInDepth#Startup_Code.md)), there will not be any conflicts with other public symbols `main`, e.g. the `main()` function that's the top level function of your program, or the `main()` functions of any other `.cogc` modules.

Note: renaming the `.text` segment doesn't affect _data_ access: Your `.cogc` module can access any global public data in the hub. You can declare that data either in the `.cogc` file itself, or in an `#include` file.

## Defining `main()` in the `.cogc` Module ##

This is probably the most common form of `main()` in `.cogc` modules:

```
void main(void *mailbox)
{
    ...
}
```

The `mailbox` parameter is passed via the `PAR` special register, but so is the initial value for the stack pointer `sp`. The parameter is usually a pointer to a `struct` in hub memory, so instead of using the `void *` type, you can also use the type of the struct you use (e.g. `void main(struct mailbox_t *struct)`.

> Passing a non-pointer (e.g. an integer that designates a pin number) is not possible unless you verify that your `.cogc` module doesn't use the stack at all.

If you don't need a mailbox, you can declare your `main()` to ignore the optional parameter:

```
void main(void)
{
    ...
}
```

> Even if your `main()` function doesn't need a parameter, the code that starts your cog will still need to provide a stack pointer. More info about this follows [below](PropGccInDepth#How_to_Run_Your_Cog_Module.md).

For the return type of `main()`, you should simply use `void`. Changing the return type to something else is not going to cause any problems, but there's no way to retrieve the return value. If the `main()` function needs to convey some sort of result, you will have to do it through the mailbox or through a global variable.

# `_NATIVE` and `_NAKED` #

Because of the absence of an intrinsic stack, PropGCC uses a calling convention that's unique to the Propeller. By default, functions compiled in the COG memory model use the `lr` pseudo-register to store the return address. If a function needs to call another function, it has to push the `lr` register onto the stack before it can use it for the next calling level. This is a compromise that makes it possible to call functions recursively.

But the use of the stack in hub memory results in some very specific problems: First of all, it's difficult to predict how big the stack needs to be. Secondly, the instructions to push and pop the instructions onto and from the stack will slow down the execution.

> You can force the compiler to use the native `CALL` and `RET` instructions (which are translated to `JMPRET` and `JMP`) by declaring the function with the `_NATIVE` modifier. If a function is declared `_NATIVE`, it cannot be called recursively.

If you call a `_NATIVE` function recursively, the function will never return because the return instruction will always jump back to the instruction after the recursive `JMPRET` instruction.

When using the `_NATIVE` attribute, the compiler generates a label at the end of the function where the `RET` instruction is stored. But the optimizer will eliminate the `RET` instruction and the label if it detects that the function has an infinite loop (e.g. a `for(;;)` with no `break`), and you will get a linker error because the function call needs a reference to the return label for `_NATIVE` functions.

> You can force the compiler to use the native `JMP` instruction to call a function that never returns, by using the `_NAKED` modifier.

Any function declared with `_NAKED` should _never_ return: it should either go into an infinite loop or it should stop the cog. If you use `_NAKED` on a function that is intended to return, the results are unpredictable because the compiler will not generate any code at the end of the function, so the cog will drop into the next function in memory. Neither the compiler nor the linker will catch this mistake.

> In a `.cogc` module, the `main()` module may be declared `_NAKED` if it never returns, but it should _not_ be declared `_NATIVE`. If it's declared `_NATIVE`, the `RET` instruction is translated to a `JMP #0` because it's never initialized by the `JMPRET` in `crt0_cog.s`, so the cog will start executing random instructions.

> If you declare a function `_NATIVE` or `_NAKED` in a module that's _not_ compiled in COG mode, it will change the compilation and linkage for that particular function to native mode, and it will load the function in the same cog as where the kernel for the memory model runs. Note: the kernels of some memory models may be so big that there is little or no spare room to do this.

Compiling a function with `_NATIVE` or `_NAKED` also changes the compiler's behavior for data storage: Local variables (other than the ones that are represented by registers) are stored in cog memory by default, instead of on the stack. More about this in the next section.

# Where the Compiler Stores Data #

## Static Data ##

Whenever you declare static data, it will get stored in the hub by default. This goes for static data inside functions, as well as global data. This is true regardless of which memory model you use.

You can use the `_COGMEM` attribute with declarations of variables to let the compiler store them in cog memory instead of hub memory. This is more efficient because no hub operation is required to access the variable. Note that if the variable is part of a `.cogc` module, declaring it with `_COGMEM` makes it inaccessible to other modules. Similarly, if a `_COGMEM` variable is declared in a non-`.cogc` module, it will be accessible to all other modules but not to any `.cogc` modules because it will end up in the kernel cog of the memory model you use. When this happens, the compilation will succeed but you will get a linker error.

> Global variables in the hub are initialized once, when the Propeller starts the first cog. But variables in the cog are initialized every time a `.cogc` module is started.

In other words, if you stop a cog that's running a particular `.cogc` module, restarting the cog with with same `.cogc` module will reset all variables. This is in line with the "program in a program" paradigm: every time you start the subprogram cog, the variables are initialized to the same values.

Whenever you store data in the hub that's used by multiple cogs, don't forget to declare the data with the `volatile` keyword so the compiler knows the data cannot be cached in a register. For example:

```
  // declarations of global data e.g. in an #include file
  unsigned hubvar1;
  volatile unsigned hubvar2;

  ...

  // In a .cogc file:

  // The following code will get optimized away because the compiler
  // assumes that hubvar1 won't be changed by other threads/cogs
  hubvar1 = 0;
  if (!hubvar1) // compiler thinks this is always false
  {
    ... // Code here doesn't get compiled into the assembly
  }

  // The following behaves as expected because the compiler knows
  // not to make any assumptions about the value of hubvar2, because
  // it was declared volatile
  hubvar2 = 0;
  if (!hubvar2) // compiler behaves as expected
  {
    ... // Code here gets compiled and executed
  }
```

Note: declaring `_COGMEM` variables `volatile` doesn't make sense (except for the I/O registers such as INA) and will only introduce extra assembler operations (copying data into pseudo-registers unnecessarily etc.). Since cog memory can only be accessed by the cog where it's stored, it can never change unexpectedly.

## Automatic Data ##

For automatic data (i.e. local variables in functions) the compiler uses general registers as much as possible. It will use the pseudo-stack in the hub to save registers if necesary. In functions that are declared `_NATIVE` or `_NAKED`, the compiler uses cog memory for automatic variables instead of the stack.

# How to Run Your Cog Module #

Use the `cognew()` function declared in `propeller.h` to start a cog with the code in your `.cogc` module. Somewhat unexpectedly perhaps, what you pass to `cognew()` as first parameter is _**not**_ the address of your `main()` function (and it's impossible to get that address anyway because each `.cogc` module has a different .text segment where code is stored). The second parameter to `cognew()` is stored in the `PAR` pseudo-register of the cog, and can also be used by your `main()` function.

## First Parameter to `cognew()` ##

If you've programmed in PASM before, using the Spin tool, you may wonder about whether your `main()` needs to be implemented at the top of your `.cogc` file. After all, when you use `COGNEW` in Spin or in PASM, the address you pass to it is the start of the PASM code in hub memory, and that's where the new cog will start executing.

In C (or C++ or any other language besides PASM), the location of the `main()` function is of no concern. The compiler and linker make sure that the code from [crt0\_cog.s](PropGccInDepth#Startup_Code.md) is stored at the top of cog memory so it gets executed first. The location in memory of your `main()` function is irrelevant. So what parameter do you need to pass to `cognew()` then?

> To start a `.cogc` module, the first parameter of `cognew( )` must be `_load_start_FILENAME_cog` where FILENAME is the name of your source file without the `.cogc` extension. For example, if your source file is called `my_awesome.cogc`, use `_load_start_my_awesome_cog`.

Note: even though the file name may be case insensitive to the operating system that you use to develop with (particularly Windows), the case of the `_load_start_FILENAME.cog` symbol has to match with the case of the filename. In other words: if you would rename the file from `my_awesome.cogc` to `My_Awesome.cogc`, you also have to change the code so it uses symbol `_load_start_My_Awesome_cog`.

The compiler automatically takes care of generating the global symbol `_load_start_FILENAME_cog` which represents the start of the hub memory block where the code from your `.cogc` module is stored. There also is a symbol for the end of the segment called `_load_stop_FILENAME_cog`.

## Second Parameter to `cognew()` ##

The second parameter to `cognew()` is passed to the cog in the `PAR` pseudo-register. Your `main()` function can use this parameter, but the parameter is also used to initialize the `sp` stack pointer pseudo-register.

That means the `mailbox` parameter to the `main()` function also needs to be the end of the stack. Your code should look similar to this:

```
  #define STACK_SIZE ... // declare stack size

  static struct
  {
    unsigned stack[STACK_SIZE];
    struct mailbox_t mailbox;
  } cogdata;

  ...
  
  if (cognew(_load_start_my_awesome_cog, &cogdata.mailbox) >= 0)
  {
    // success...
  }
```

If you think the above looks kinda messy, or if you only want to pass a single number instead of the address of a mailbox struct, you can also do this:

```
  #define STACK_SIZE ... // declare stack size

  // Allocate an extra long in addition to the required stack size
  static unsigned stack[STACK_SIZE + 1];

  static struct mailbox_t mailbox;

  ...

  // Passing a mailbox pointer via an extra long just beyond the
  // end of the stack:

  stack[STACK_SIZE] = (unsigned)&mailbox;
  if (cognew(_load_start_my_awesome_cog, &stack[STACK_SIZE]) >= 0)
  {
    // success...
  }

  // In your .cogc module, declare your main() as follows:
  void main(struct mailbox_t **ppmailbox)
  {
    // ppmailbox is now a pointer to a pointer to the mailbox
    struct mailbox_t *mailbox = *ppmailbox;

    ...    
  }

  //----------------------

  // In the code above, if you only need to pass a single value
  // (e.g. an I/O pin number) instead of the address of a struct,
  // do this instead:
  stack[STACK_SIZE] = tv_pin;
  if (cognew(_load_start_my_awesome_cog, &stack[STACK_SIZE]) >= 0)
  {
    // success...
  }

  // In your .cogc module:
  void main(unsigned *pmypin)
  {
    unsigned mypin = *pmypin;

    ...
  }
```

If you want to run multiple cogs from the same `.cogc` module, you will have to set up a separate stack for each cog. For example, you can change to code as follows:

```
  #define STACK_SIZE ... // declare stack size

  static struct
  {
    unsigned stack[STACK_SIZE];
    struct mailbox_t mailbox;
  } cogdata[2];

  ...
  
  if ( (cognew(_load_start_my_awesome_cog, &cogdata[0].mailbox) >= 0)
    && (cognew(_load_start_my_awesome_cog, &cogdata[1].mailbox) >= 0))
  {
    // success...
  }
```

With the code shown above, each cog has its own mailbox. If you would like to make all cogs use the same data in the hub, you can simply declare the mailbox struct as a global variable and include the declaration in the `.cogc` module. Example:

```
  #define STACK_SIZE ... // declare stack size

  // global data; you may want to put this in a .h file and #include it
  volatile struct mailbox_t
  {
    ... // add any number of fields
  } globals;
  
  static unsigned stack[STACK_SIZE];
  
  ...
  
  if (cognew(_load_start_my_awesome_cog, &stack[STACK_SIZE]) >= 0)
  {
    // success...
  }
```

In many cases, the mailbox is filled in before the `cognew()`, and then used only by the cog that's being started. But in other cases, the cog that gets started needs to report data back to the "calling" cog.

> If multiple cogs write data into the mailbox, it should be declared `volatile`, otherwise the compiler may optimize your code away because it thinks it can't be changed by other threads/cogs. Don't forget that the main program is also a cog, even if it's not in a `.cogc` module!

# The Stack #

Arguably the most important problem of mixed-mode programming is: how big does the stack need to be?

There is no easy answer to this question, but here are some hints:
  * As long as there are no recursive functions in your `.cogc` module, you can count the number of occurrences of "`sub sp, #4`" in the generated assembly, and use that count as the number of needed unsigned's in the stack.
  * You can reduce the use of the stack by using `_NATIVE` and `_NAKED` in your `.cogc` module to minimize stack usage. _Don't make the main() function in your `.cogc` module `_NATIVE`!_ Making `main()` `_NAKED` is okay.
  * It's pretty easy to detect how many longs of stack your program uses by initializing the stack array with known values that are unlikely to occur on the stack, and later on checking how many stack entries still have that value.

Even so, there may be factors that make it difficult to find out how big the stack needs to be. For example, even if you put some detection code in place, you will still have to make sure that your code execution follows the "worst possible path" that generates the most possible stack operations. If you use recursion you will also have to do some good analysis of your code to determine how many recursions could happen in the worst possible case.

The bottom line is: nothing is better to determine the required stack size than a thorough analysis of the Assembly code that the compiler generates. And the simpler your `.cogc` module is, the easier it is to analyze it, and to make sure that stack overflows are not possible. With a `_NAKED` `main()` function and all other functions `_NATIVE`, you might not need a stack at all, but make sure you check the Assembly of your code to check that the compiler agrees with you!