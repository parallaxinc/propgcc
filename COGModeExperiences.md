# Introduction #

I wanted to write a device driver in C so I tried using PropGCC COG mode. This is a mode that generates code that runs entirely in a COG. While the COG is limited to only 496 longs of instructions and data, code in COG mode can run faster than LMM, XMM, or XMMC modes.

# Details #

Some things to remember:

Declare the main function with `_NAKED`:

For example:

```
_NAKED int main(void)
{
    ...
}
```

Declare other functions with `_NATIVE`. These functions should not be recursive and you can probably only get away with a single level of function calls. In other words, the main() function can call other functions but those functions can not call a third level of functions. This will likely generate references to a hub memory stack.

For example:

```
static _NATIVE void do_something(void)
{
    ...
}
```

Declare variables with `_COGMEM`.

For example:

```
static _COGMEM int mask;
```

Beyond these suggestions, I've found it necessary to continually disassemble the generated code to find out whether stack instructions were generated. This can be done with the following command:
```
propeller-elf-objdump -d my_program.elf
```
This will produce a disassembly of your program which you can examine to find references to `sp`. If you find any, you'll have to rearrange the code to eliminate them. Often, the cause of these references to `sp` will be failing to follow one of the preceeding suggestions. Other times the cause may be more subtle.