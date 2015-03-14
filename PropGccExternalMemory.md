This is a work in progress.

# Propeller GCC External Memory #

Propeller is a 32 bit microcontroller. It is designed specifically for microcontroller applications where it shines.

Somewhere along the way as far back as 2006, people decided that having external memory solutions for Propeller would allow even more applications. One 2006 SRAM design is [described here](http://forums.parallax.com/showthread.php?90158-Adding-more-RAM-to-the-Propellor&p=618599&viewfull=1#post618599) but it uses almost all free Propeller pins. More recently, different SPI devices have become common and with the advent of caching on Propeller external memory designs have become much more useful.

Also in 2006 Bill Henning posted [his LMM concept](http://forums.parallax.com/showthread.php?89640-ANNOUNCING-Large-memory-model-for-Propeller-assembly-language-programs!&p=615022&viewfull=1#post615022) for allowing other languages to execute programs from HUB memory. Over the years, this concept has been also used for executing programs from external memory.

Propeller is a flexible and fun chip. We have all had fun stretching it beyond it's intended design. Code executed from external memory solutions will most likely never be as fast as code executed on chip - so if you need that in-chip speed, an external memory solution will not get it. If you need size and can sacrifice the speed of "business logic" then this section will be interesting to you.

The current design philosophy for interfacing new hardware is described here. A section that shows step by step integration instructions for new designs is also provided.

External memory solutions available for use with Propeller GCC are listed at the end with brief instructions on how to use them. It also reserves space for designs that are not supported just yet.

Parallax has been patient with our desire and pursuit for external memory solutions. In 2010, the C3 platform appeared in the Parallax store, and today Propeller GCC supports the C3 in several different memory models.


# Current External Memory Solutions #

| **Board Type** | **Memory Types** | **Description** |
|:---------------|:-----------------|:----------------|
| [HUB](http://www.parallax.com/Portals/0/Downloads/docs/prod/prop/PropellerHardwareConnections.pdf) | LMM	| Generic LMM board type for any Parallax board. |
| [BACKPACK](http://www.parallax.com/Portals/0/Downloads/docs/prod/prop/PropellerBackpackv1.0.pdf) | LMM XMMC	| Parallax Propeller Backpack with 64KB EEPROM |
| [C3](http://www.parallax.com/Store/Microcontrollers/PropellerDevelopmentBoards/tabid/514/CategoryID/73/List/0/SortField/0/Level/a/ProductID/721/Default.aspx) | LMM XMM XMMC | Parallax Open Source Hardware with Flash |
| [DEMOBOARD](http://www.parallax.com/Portals/0/Downloads/docs/prod/prop/PropellerDemoBd-RevG-Schem.pdf) | LMM | Parallax Propeller Demoboard |
| [DRACBLADE](http://www.smarthome.jigsy.com/purchase_kits) | LMM XMM XMMC | 3rd Party SBC with SRAM |
| [EEPROM](http://www.parallax.com/Portals/0/Downloads/docs/prod/prop/PropellerHardwareConnections.pdf) | LMM XMMC | Any Propeller with a single 64KB/128KB/256KB EEPROM |
| [HYDRA](http://www.parallax.com/Store/Microcontrollers/PropellerKits/tabid/144/CategoryID/20/List/0/SortField/0/Level/a/ProductID/467/Default.aspx) | LMM | Parallax Hydra also has 128KB EEPROM (for EEPROM Board type) |
| [PPDB](http://www.parallax.com/Portals/0/Downloads/docs/prod/schem/PPDB_A_Schematic.pdf) | LMM	| Parallax Professional Development Board |
| [PROTOBOARD](http://www.parallax.com/Portals/0/Downloads/docs/prod/prop/32212-32812-PropellerProtoBoard-v1.3.pdf) | LMM XMMC	| Parallax Propeller Protoboard with 64KB EEPROM |
| [QUICKSTART](http://www.parallaxsemiconductor.com/sites/default/files/parallax/P8X32AQuickStartSchematicRevA_0.pdf) | LMM XMMC	| Parallax Propeller QuickStart with 64KB  EEPROM |
| [SDRAM](http://gadgetgangster.com/find-a-project/56?projectnum=359) | LMM XMM XMMC | 3rd Party GadgetGangster SDRAM Module |
| [SSF](http://www.microcsource.com/SpinSocket/SpinSocket32-Flash-A2.pdf) | LMM XMMC | 3rd Party Spin Socket Flash or compatible design |


Volatile memory types SRAM, SDRAM, and SPI-RAM are supported only via serial download now. An SD card based loader is in development.


# External Memory Interface Design #
Today Propeller GCC uses a cache for encapsulating external memory hardware. The cache is managed in part by the execution [COG](P8x32a.md) and in part by a Cache COG. There is always a debate about what might be better or worse. Propeller GCC uses a Cache COG today because the cache design interface is easier and faster than the non-cached alternative.

The Propeller GCC Cache COG uses the following features which are encapsulated in the cache\_interface.spin API:
  1. Simple Cache access API
  1. Extra memory access API
  1. Cache Line Pointer

The cache\_interface.spin API is written in the SPIN language mainly because of legacy and small code size. The SPIN code can be found in the propgcc tree at propgcc/loader/spin/cache\_interface.spin. The cache interface is presented here in pseudo-code terms. The interface complies with the so-called JCACHE interface designed by Steve Denson and David Betz.

### Cache Interface API pseudo-code ###

This is a work in progress. The entire presentation idea may change from pseudo-code to something else.

> #### Conceptual Method Summary ####

```
  start(codePointer, mailboxPointer, cachePointer, config1, config2)
  readLong(address)
  writeLong(address, data)
  readWord(address)
  writeWord(address, data)
  readByte(address)
  writeByte(address, data)
  eraseFlashChip(address)
  eraseFlashBlock(address)
  writeFlash(address, buffer, count)
  sdCardInit()
  sdCardRead(address, buffer, count)
  sdCardWrite(address, buffer, count)
```

> The way Propeller GCC reads or writes memory today is to fetch a cache line pointer from the cache cog and use a `"`modulo index`"` for getting the data. This reduces the cache code complexity in the COG, but  increases complexity some in the XMM VM kernel. The advantage in having the XMM VM kernel do some work means that data within a certain address range can be quickly retrieved without asking the Cache COG for data again in many cases.


> #### Conceptual Method Details ####
```
  start(codePointer, mailboxPointer, cachePointer, config1, config2)
```
> The start method initializes the cache with pointers to the Propeller ASM _codePointer_ block, a _mailboxPointer_, and the _cachePointer_. The  _config1_ and _config2_ values can be passed to the cache interface to define the number of cache lines in use and the expected cache line length; the values are optional and if set to zero will be ignored by the cache driver start up code. The start method returns line mask so the XMM VM kernel "in-line detect" cache code will work.

```
  readLong(address)
```
> This method will ask the cache to deliver a long from the physical backstore _address_.

```
  writeLong(address, data)
```
> This method will ask the cache to write a long _data_ value to physical the back-store _address_.

> Propeller GCC writes to cache using the write-back method. The only time a value actually gets written to the backstore is when a read is requested on a cache line that has the dirty bit set.

```
  readWord(address)
```
> This is the same as readLong except the data returned is word wide.

```
  writeWord(address, data)
```
> This is the same as writeLong except the _data_ is word wide.

```
  readByte(address)
```
> This is the same as readLong except the data returned is byte wide.

```
  writeByte(address, data)
```
> This is the same as writeLong except the _data_ is byte wide.

```
  eraseFlashChip(address)
```
> This method would be used to erase the entire flash chip at the _address_.

```
  eraseFlashBlock(address)
```
> This method would be used to erase a block at the _address_. The cache driver is responsible for erasing the block(s) up to 4KB.

```
  writeFlash(address, buffer, count)
```
> This method would be used to program a _buffer_ block of _count_ sized data to the _address_.

```
  sdCardInit()
```
> This method would be used for a board such as C3 where the SD Card shares some chip select logic with other devices such as SPI Flash or SPI SRAM. It initializes the SD card for use with read and write methods.

```
  sdCardRead(address, buffer, count)
```
> This method is available for same reasons as sdCardInit. This method would be used to read data into a _buffer_ block of _count_ size from the _address_.

```
  sdCardWrite(address, buffer, count)
```
> This method is available for same reasons as sdCardInit. This method would be used to write data from a _buffer_ block of _count_ size to the _address_.

> Propeller-GCC uses a mailbox for performing the access functions. The following pseudo-code shows the mailbox commands for the methods described above. The % sign indicates the numbers are a binary.
```
// cache access commands
WRITE_CMD             = %10
READ_CMD              = %11

// extended commands
ERASE_CHIP_CMD        = %000_01  // only for flash
ERASE_BLOCK_CMD       = %001_01  // only for flash
WRITE_DATA_CMD        = %010_01  // only for flash
SD_INIT_CMD           = %011_01  // only for C3 because of shared SPI pins
SD_READ_CMD           = %100_01  // only for C3 because of shared SPI pins
SD_WRITE_CMD          = %101_01  // only for C3 because of shared SPI pins

CMD_MASK              = %11
EXTEND_MASK           = %10      // used to detect an extended command
```

# Integrating a new Design #
> The GCC linker provides great flexibility in defining memory layouts. A new hardware design may have different restrictions on external memory. Generally in XMMC mode, only code will go into external memory - you can define the size in a linker script that will generate an error if that code size is exceeded. In XMM mode, you can specify code and data in various sections defined by the linker.
> With the new linker scripts it's easy to make a custom script for a board.
> As an example, here is a C3 script used with XMM that defines the memory model layout and sizes.
```
MEMORY {
  hub     : ORIGIN = 0x00000000, LENGTH = 24K /* c3 cache driver uses upper 8K */
  ram     : ORIGIN = 0x20000000, LENGTH = 64K
  rom     : ORIGIN = 0x30000000, LENGTH = 1M
}

REGION_ALIAS("REGION_TEXT", rom);
REGION_ALIAS("REGION_DATA", ram);

INCLUDE propeller_xmm.ld
```

more to do .....