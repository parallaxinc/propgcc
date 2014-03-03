This program demonstrates the capabilities of the 640x480 4-color VGA driver.
The driver breaks up the display into a 40 by 30 array of tiles, where each
tile is 16 pixels by 16 lines.  The tiles are used to implement character
fonts, small image blocks.

A portion of the display can be treated as a bit-mapped buffer by referencing
a tile in the 40x30 array, and drawing into the reference tile.  This provides
the ability to draw lines and and move sprites anywhere on the screen as long
as less than 21% of the screen is covered.

vgademo is build by running "make".  It is then loaded by typing "make run".

The video generator is written in PASM in vga.spin.  It is compiled by BTSC
using the "-c" option to generate a DAT file.  This is converted to a C array
in vga.c with the dat2c utility.
