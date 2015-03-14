# Introduction #

This article shows how to use linker scripts that are included in the PropGCC release to link xmm and xmmc programs to run from external RAM or flash/eeprom.


# Details #

Included linker scripts:

|-mxmm|-T xmm.ld|code in external flash/eeprom, data in external ram|
|:----|:--------|:--------------------------------------------------|
|-mxmm|-T xmm\_ram.ld|code and data in external ram|
|-mxmmc|-T xmmc.ld|code in external flash/eeprom, data in hub memory|
|-mxmmc|-T xmmc\_ram.ld|code in external ram, data in hub memory|

Note that the stack and locals are always in hub memory in all of the memory models.

You should also make sure that the -m option matches the linker script. In other words, -mxmmc should always use either xmmc.ld or xmmc\_ram.ld and -xmm should always use xmm.ld or xmm\_ram.ld.

Example of how to link an xmmc program to run from external ram:

```
propeller-elf-gcc -Os -o fibo_xmmc_ram.elf -mxmmc -T xmmc_ram.ld fibo.c
```