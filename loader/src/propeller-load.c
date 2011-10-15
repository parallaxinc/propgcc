/* propeller-load.c - an elf and spin binary loader for the Parallax Propeller microcontroller

Copyright (c) 2011 David Michael Betz

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "loader.h"
#include "osint.h"

/* defaults */
#if defined(CYGWIN) || defined(WIN32)
#define DEF_PORT    "COM1"
#endif
#ifdef LINUX
#define DEF_PORT    "/dev/ttyUSB0"
#endif
#ifdef MACOSX
#define DEF_PORT    "/dev/cu.usbserial-A8004ILf"
#endif
#define DEF_BOARD   "default"

static void Usage(char *board, char *port);

static void MyInfo(System *sys, const char *fmt, va_list ap);
static void MyError(System *sys, const char *fmt, va_list ap);
static SystemOps myOps = {
    MyInfo,
    MyError
};

/* flag to terminal_mode to check for a certain sequence to indicate
   program exit */
int check_for_exit = 0;

int main(int argc, char *argv[])
{
    char *infile = NULL, *p, *p2;
    int terminalMode = FALSE;
    BoardConfig *config, *configSettings;
    char *port, *board;
    System sys;
    int baud = 0;
    int flags = 0;
    int i;
    int terminalBaud = 0;

    /* get the environment settings */
    if (!(port = getenv("PROPELLER_LOAD_PORT")))
        port = DEF_PORT;
    if (!(board = getenv("PROPELLER_LOAD_BOARD")))
        board = DEF_BOARD;
        
    /* setup a configuration to collect command line -D settings */
    configSettings = NewBoardConfig("");

    /* get the arguments */
    for(i = 1; i < argc; ++i) {

        /* handle switches */
        if(argv[i][0] == '-') {
            switch(argv[i][1]) {
            case 'b':   // select a target board
                if (argv[i][2])
                    board = &argv[i][2];
                else if (++i < argc)
                    board = argv[i];
                else
                    Usage(board, port);
                break;
            case 'p':
                if(argv[i][2])
                    port = &argv[i][2];
                else if(++i < argc)
                    port = argv[i];
                else
                    Usage(board, port);
#if defined(CYGWIN) || defined(WIN32) || defined(LINUX)
                if (isdigit((int)port[0])) {
#if defined(CYGWIN) || defined(WIN32)
                    static char buf[10];
                    sprintf(buf, "COM%d", atoi(port));
                    port = buf;
#endif
#if defined(LINUX)
                    static char buf[64];
                    sprintf(buf, "/dev/ttyUSB%d", atoi(port));
                    port = buf;
#endif
                }
#endif
#if defined(MACOSX)
                if (port[0] != '/') {
                    static char buf[64];
                    sprintf(buf, "/dev/cu.usbserial-%s", port);
                    port = buf;
                }
#endif
                break;
            case 'e':
                flags |= LFLAG_WRITE_EEPROM;
                break;
            case 'r':
                flags |= LFLAG_RUN;
                break;
            case 's':
                flags |= LFLAG_WRITE_BINARY;
                break;
            case 't':
                terminalMode = TRUE;
                if (argv[i][2])
                  terminalBaud = atoi(&argv[i][2]);
                break;
            case 'x':
                check_for_exit = 1;
                break;
            case 'D':
                if(argv[i][2])
                    p = &argv[i][2];
                else if(++i < argc)
                    p = argv[i];
                else
                    Usage(board, port);
                if ((p2 = strchr(p, '=')) == NULL)
                    Usage(board, port);
                *p2++ = '\0';
                SetConfigField(configSettings, p, p2);
                break;
            case 'I':
                if(argv[i][2])
                    p = &argv[i][2];
                else if(++i < argc)
                    p = argv[i];
                else
                    Usage(board, port);
                xbAddPath(p);
                break;
            default:
                Usage(board, port);
                break;
            }
        }

        /* handle the input filename */
        else {
            if (infile)
                Usage(board, port);
            infile = argv[i];
        }
    }
    
    /* make sure an input file was specified if needed */
    if (infile && !flags) {
        fprintf(stderr, "error: must specify -e, -r, or -s when an image file is specified\n");
        return 1;
    }

/*
1) look in the directory specified by the -I command line option (added above)
2) look in the directory where the elf file resides
3) look in the directory pointed to by the environment variable PROPELLER_ELF_LOAD
4) look in the directory where the loader executable resides if possible
5) look in /usr/local/propeller/propeller-load
*/

    /* finish the include path */
    if (infile)
        xbAddFilePath(infile);
    xbAddEnvironmentPath("PROPELLER_LOAD_PATH");
    xbAddProgramPath(argv);
#if defined(LINUX) || defined(MACOSX) || defined(CYGWIN)
    xbAddPath("/usr/local/propeller/propeller-load");
#endif
    
    sys.ops = &myOps;
    ParseConfigurationFile(&sys, "propeller-load.cfg");

    /* setup for the selected board */
    if (!(config = GetBoardConfig(board))) {
        fprintf(stderr, "error: can't find board configuration '%s'\n", board);
        return 1;
    }
    
    /* override with any command line settings */
    MergeConfigs(config, configSettings);
        
    /* use the baud rate from the configuration */
    baud = config->baudrate;

    /* initialize the serial port */
    if ((flags & (LFLAG_RUN | LFLAG_WRITE_EEPROM)) != 0 || terminalMode) {
        if (!InitPort(port, baud)) {
            fprintf(stderr, "error: opening serial port\n");
            return 1;
        }
    }

    /* load the image file */
    if (infile) {
        if (!LoadImage(&sys, config, port, infile, flags)) {
            fprintf(stderr, "error: load failed\n");
            return 1;
        }
    }
    
    /* enter terminal mode if requested */
    if (terminalMode)
      {
        if (terminalBaud != 0)
          {
            serial_done();
            serial_init(port, terminalBaud);
          }
        terminal_mode();
      }
    return 0;
}

/* Usage - display a usage message and exit */
static void Usage(char *board, char *port)
{
    fprintf(stderr, "\
usage: propeller-elf-load\n\
         [ -b <type> ]     select target board (default is %s)\n\
         [ -p <port> ]     serial port (default is %s)\n\
         [ -I <path> ]     add a directory to the include path\n\
         [ -D var=value ]  define a board configuration variable\n\
         [ -e ]            write the program into EEPROM\n\
         [ -r ]            run the program after loading\n\
         [ -s ]            write a spin binary file for use with the Propeller Tool\n\
         [ -t ]            enter terminal mode after running the program\n\
         [ -t<baud> ]      enter terminal mode with a different baud rate\n\
         <name>            file to compile\n\
\n\
Variables that can be set with -D are:\n\
  clkfreq clkmode baudrate rxpin txpin tvpin cache-driver cache-size cache-param1 cache-param2\n\
", board, port);
    exit(1);
}

static void MyInfo(System *sys, const char *fmt, va_list ap)
{
    vfprintf(stdout, fmt, ap);
}

static void MyError(System *sys, const char *fmt, va_list ap)
{
    vfprintf(stderr, fmt, ap);
}
