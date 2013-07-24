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
#include <limits.h>
#include "loader.h"
#include "osint.h"

/* defaults */
#if defined(CYGWIN) || defined(WIN32) || defined(MINGW)
#define PORT_PREFIX NULL
#endif
#ifdef LINUX
#define PORT_PREFIX "ttyUSB"
#endif
#ifdef MACOSX
#define PORT_PREFIX "cu.usbserial"
#endif

static void Usage(void);

static void MyInfo(System *sys, const char *fmt, va_list ap);
static void MyError(System *sys, const char *fmt, va_list ap);
static SystemOps myOps = {
    MyInfo,
    MyError
};

int main(int argc, char *argv[])
{
    char actualport[PATH_MAX];
    char *infile = NULL, *p, *p2;
    int terminalMode = FALSE;
    BoardConfig *config, *configSettings;
    char *port, *board, *subtype, *value;
    System sys;
    int baud = 115200;
    int useRtsForReset = FALSE;
    int portFlags = 0;
    int flags = 0;
    int i;
    int terminalBaud = 0;
    int check_for_exit = 0; /* flag to terminal_mode to check for a certain sequence to indicate program exit */
    int showPorts = FALSE;
    int showAll = TRUE;
    
    /* make sure that the serial port gets closed on exit */
    atexit(serial_done);
    
    /* just display a usage message if no arguments are supplied */
    if (argc <= 1)
        Usage();

    /* get the environment settings */
    port = getenv("PROPELLER_LOAD_PORT");
    board = getenv("PROPELLER_LOAD_BOARD");
        
    /* setup a configuration to collect command line -D settings */
    configSettings = NewBoardConfig(NULL, "");

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
                    Usage();
                break;
            case 'p':
                if(argv[i][2])
                    port = &argv[i][2];
                else if(++i < argc)
                    port = argv[i];
                else
                    Usage();
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
            case 'P':
                showPorts = TRUE;
                showAll = FALSE;
                break;
            case 'Q':
                showPorts = TRUE;
                showAll = TRUE;
                break;
            case 'e':
                flags |= LFLAG_WRITE_EEPROM;
                break;
            case 'l':
                flags |= LFLAG_WRITE_SDLOADER;
                break;
            case 'z':
                flags |= LFLAG_WRITE_SDCACHELOADER;
                break;
            case 'r':
                flags |= LFLAG_RUN;
                break;
            case 's':
                flags |= LFLAG_WRITE_BINARY;
                break;
            case 'x':
                flags |= LFLAG_WRITE_PEX;
                break;
            case 't':
                terminalMode = TRUE;
                if (argv[i][2])
                  terminalBaud = atoi(&argv[i][2]);
                break;
            case 'q':
                check_for_exit = 1;
                break;
	        case 'g':
	            flags |= LFLAG_DEBUG;
		        break;
            case 'D':
                if(argv[i][2])
                    p = &argv[i][2];
                else if(++i < argc)
                    p = argv[i];
                else
                    Usage();
                if ((p2 = strchr(p, '=')) == NULL)
                    Usage();
                *p2++ = '\0';
                SetConfigField(configSettings, p, p2);
                break;
            case 'I':
                if(argv[i][2])
                    p = &argv[i][2];
                else if(++i < argc)
                    p = argv[i];
                else
                    Usage();
                xbAddPath(p);
                break;
            case 'v':
                portFlags = IFLAG_VERBOSE;
                break;
            case 'S':
                psetdelay(argv[i][2] ? atoi(&argv[i][2]) : 5);
                break;
            case 'f':
                flags |= LFLAG_WRITE_SDFILE;
                break;
            case '?':
                /* fall through */
            default:
                Usage();
                break;
            }
        }

        /* handle the input filename */
        else {
            if (infile)
                Usage();
            infile = argv[i];
        }
    }
    
    /* make sure an input file was specified if needed */
    if (infile && !flags) {
        printf("error: must specify -e, -r, or -s when an image file is specified\n");
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
    xbAddPath("/opt/parallax/propeller-load");
#endif
    
    sys.ops = &myOps;
    
    /* parse the board option */
    if (board) {
    
        /* split the board type from the subtype */
        if ((p = strchr(board, ':')) != NULL) {
            *p++ = '\0';
            subtype = p;
        }
        
        /* no subtype */
        else
            subtype = DEF_SUBTYPE;
    }
    
    else {
        board = DEF_BOARD;
        subtype = DEF_SUBTYPE;
    }

    /* setup for the selected board */
    if (!(config = ParseConfigurationFile(&sys, board))) {
        printf("error: can't find board configuration '%s'\n", board);
        return 1;
    }
    
    /* select the subtype */
    if (subtype) {
        if (!(config = GetConfigSubtype(config, subtype))) {
            printf("error: can't find board configuration subtype '%s'\n", subtype);
            return 1;
        }
    }
    
    /* override with any command line settings */
    config = MergeConfigs(config, configSettings);
        
    /* use the baud rate from the configuration */
    GetNumericConfigField(config, "baudrate", &baud);
    if ((value = GetConfigField(config, "reset")) != NULL) {
        if (strcasecmp(value, "dtr") == 0)
            useRtsForReset = FALSE;
        else if (strcasecmp(value, "rts") == 0)
            useRtsForReset = TRUE;
        else {
            printf("error: no reset type '%s'\n", value);
            return 1;
        }
    }
    serial_use_rts_for_reset(useRtsForReset);
    
    /* check for being asked to show ports */
    if (showPorts) {
        if (showAll)
            ShowPorts(PORT_PREFIX);
        else
            ShowConnectedPorts(PORT_PREFIX, baud, portFlags);
        return 0;
    }
    
    /* initialize the serial port */
    if ((flags & NEED_PORT) != 0 || terminalMode) {
        int sts = InitPort(PORT_PREFIX, port, baud, portFlags, actualport);
        switch (sts) {
        case PLOAD_STATUS_OK:
            // port initialized successfully
            break;
        case PLOAD_STATUS_OPEN_FAILED:
            printf("error: opening serial port '%s'\n", port);
	    perror("Error is ");
            return 1;
        case PLOAD_STATUS_NO_PROPELLER:
            if (port)
                printf("error: no propeller chip on port '%s'\n", port);
            else
                printf("error: can't find a port with a propeller chip\n");
            return 1;
        }
    }

    /* load the image file */
    if (infile) {
        if (flags & LFLAG_WRITE_SDFILE)
            WriteFileToSDCard(config, infile, NULL);
        else {
            if (!LoadImage(&sys, config, infile, flags)) {
                printf("error: load failed\n");
                return 1;
            }
        }
    }
    
    /* check for loading the sd loader */
    else if (flags & LFLAG_WRITE_SDLOADER) {
        if (!LoadSDLoader(&sys, config, "sd_loader.elf", flags)) {
            printf("error: load failed\n");
            return 1;
        }
    }
    
    /* check for loading the sd cache loader */
    else if (flags & LFLAG_WRITE_SDCACHELOADER) {
        if (!LoadSDCacheLoader(&sys, config, "sd_cache_loader.elf", flags)) {
            printf("error: load failed\n");
            return 1;
        }
    }
    
    /* enter terminal mode if requested */
    if (terminalMode) {
        printf("[ Entering terminal mode. Type ESC or Control-C to exit. ]\n");
        fflush(stdout);
        if (terminalBaud && terminalBaud != baud) {
            serial_done();
            serial_init(actualport, terminalBaud);
        }
        terminal_mode(check_for_exit);
    }
    return 0;
}

/* Usage - display a usage message and exit */
static void Usage(void)
{
printf("\
usage: propeller-load\n\
         [ -b <type> ]     select target board (default is 'default:default')\n\
         [ -p <port> ]     serial port (default is to auto-detect the port)\n\
         [ -P ]            list serial ports with Propeller chips\n\
         [ -Q ]            list available serial ports\n\
         [ -I <path> ]     add a directory to the include path\n\
         [ -D var=value ]  define a board configuration variable\n\
         [ -e ]            write the program into EEPROM\n\
         [ -r ]            run the program after loading\n\
         [ -g ]            set up the program for debugging after loading\n\
         [ -s ]            write a spin .binary file for use with the Propeller Tool\n\
         [ -x ]            write a .pex binary file for use with the SD loader or SD cache\n\
         [ -l ]            write a program to the sd card and use the SD loader\n\
         [ -z ]            write a program to the sd card and use the SD cache\n\
         [ -f ]            write a file to the SD card\n\
         [ -t ]            enter terminal mode after running the program\n\
         [ -t<baud> ]      enter terminal mode with a different baud rate\n\
         [ -q ]            quit on the exit sequence (0xff, 0x00, status)\n\
         [ -v ]            verbose output\n\
         [ -S ]            slow down the loader by adding 5 microseconds delay\n\
         [ -S<n> ]         slow down the loader by adding <n> microseconds delay\n\
         [ -? ]            display a usage message and exit\n\
         <name>            elf or spin binary file to load\n\
\n\
Target board type can be either a single identifier like 'propboe' in which case the subtype\n\
defaults to 'default' or it can be of the form <type>:<subtype> like 'c3:ram'.\n\
\n\
Variables that can be set with -D are:\n\
  clkfreq clkmode baudrate reset rxpin txpin tvpin\n\
  cache-driver cache-size cache-param1 cache-param2\n\
  sd-driver sdspi-do sdspi-clk sdspi-di sdspi-cs\n\
  sdspi-clr sdspi-inc sdspi-start sdspi-width spdspi-addr\n\
  sdspi-config1 sdspi-config2 eeprom-first\n\
\n\
Value expressions for -D can include:\n\
  rcfast rcslow xinput xtal1 xtal2 xtal3 pll1x pll2x pll4x pll8x pll16x k m mhz true false\n\
  an integer or two operands with a binary operator + - * / %% & | or unary + or -\n\
  all operators have the same precedence\n\
\n\
The -b option defaults to the value of the environment variable PROPELLER_LOAD_BOARD.\n\
The -p option defaults to the value of the environment variable PROPELLER_LOAD_PORT\n\
if it is set. If not the port will be auto-detected.\n\
\n\
The 'sd loader' loads AUTORUN.PEX from an SD card into external memory.\n\
It requires a board with either external RAM or ROM.\n\
\n\
The 'sd cache' arranges to run AUTORUN.PEX directly from the SD card.\n\
It can be used on any board with an SD card slot.\n\
");
    exit(1);
}

static void MyInfo(System *sys, const char *fmt, va_list ap)
{
    vfprintf(stdout, fmt, ap);
}

static void MyError(System *sys, const char *fmt, va_list ap)
{
    vprintf(fmt, ap);
}
