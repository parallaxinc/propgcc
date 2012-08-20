/*
############################################################################
# This program converts a Spin DAT file to a C file.  The DAT file should
# be created by using the "-c" option with BTSC, as follows.
#
# bstc -c test.spin
#
# Written by Dave Hein
# Copyright (c) 2012 Parallax, Inc.
# MIT Licensed
############################################################################
*/
#include <stdio.h>

int main(int argc, char **argv)
{
    int i;
    FILE *infile = fopen("vga.dat", "r");
    int num;
    unsigned char buffer[2048];

    if (argc != 2)
    {
        fprintf(stderr, "usage: dat2c file.dat\n");
        return 1;
    }

    infile = fopen(argv[1], "r");

    if (!infile)
    {
        fprintf(stderr, "Can't open %s\n", argv[1]);
        return 1;
    }

    num = fread(buffer, 1, 2048, infile);

    printf("#include <stdint.h>\n\n");
    printf("uint8_t vga_array[] = {\n");

    for (i = 0; i < num; i++)
    {
        printf(" 0x%2.2x,", buffer[i]);
        if ((i&7) == 7) printf("\n");
    }

    printf("};\n\n");
    printf("int vga_size = sizeof(vga_array);\n");

    return 0;
}
/*
+--------------------------------------------------------------------
|  TERMS OF USE: MIT License
+--------------------------------------------------------------------
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
+------------------------------------------------------------------
*/

