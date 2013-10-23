/*
 * sample program to illustrate simulating
 * command line arguments (_argv) and
 * environment variables (_environ)
 * Copyright (c) 2011 Parallax Inc.
 * MIT licensed, see end of file for details
 */

#include <stdio.h>

char *_argv[] = {
  "program name",
  "arg 1",
  NULL
};

char *_environ[] = {
  "HELLO=world",
  NULL
};

int
main(int argc, char **argv, char **environp)
{
  int i;

  printf(" ** argv (one per line):\n");
  for (i = 0; i < argc; i++) {
    printf("%s\n", argv[i]);
  }
  printf(" ** environ (one per line):\n");
  for (i = 0; environp[i]; i++) {
    printf("%s\n", environp[i]);
  }
  printf(" ** done\n");
  return 0;
}

/* +--------------------------------------------------------------------
 * ¦  TERMS OF USE: MIT License
 * +--------------------------------------------------------------------
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * +--------------------------------------------------------------------
 */
