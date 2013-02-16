/* gdbstub.c - a gdb stub for the Propeller

Copyright (c) 2012 David Michael Betz
Modifications Copyright (c) 2012 Parallax Inc.

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

/* modified by Eric R. Smith to work with the new COG debug protocol */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "cogdebug.h"
#include "config.h"
#include "port.h"
#include "PLoadLib.h"
#include "system.h"
#include "osint.h"

/* booleans */
#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

/* verbose logging */
#define VERBOSE_LOG
/* disable breakpoint support */
#define NO_BKPT
/* enable initial stub download */
#define DEBUG_STUB

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
#define DEF_BOARD   "default"

/* error codes */
#define ERR_NONE    0
#define ERR_TIMEOUT 1
#define ERR_READ    2
#define ERR_WRITE   3
#define ERR_CHKSUM  4
#define ERR_INIT    5

/* timeout waiting for debug kernel to initialize */
#define INI_TIMEOUT 2000

/* timeout waiting for packet data */
#define PKT_TIMEOUT 10000

/* maximum amount of data in a READ or WRITE packet */
#define MAX_DATA    128

/* maximum packet size (cmd + cog + len byte + data) */
#define PKT_MAX     (3 + MAX_DATA)

#define GCC_REG_COUNT       19  // r0-r14, lr, sp, pc, ccr 
#define GCC_REG_PC          17

FILE *logfile = NULL;
char cmd[1028];

#define DEFAULT_COG 0

#ifdef DEBUG_STUB
/* On the first 'c' or 's' request we need to start the real debug kernel that was downloaded as
   part of the user's program.
*/
static int first_run = 0;
#endif

static int no_ack_mode = 0;

static void MyInfo(System *sys, const char *fmt, va_list ap);
static void MyError(System *sys, const char *fmt, va_list ap);

static SystemOps myOps = {
    MyInfo,
    MyError
};

/* prototypes */
static void Usage(void);
static int debug_cmd(int fcn, int len);
#ifdef DEBUG_STUB
static int start_debug_kernel(void);
#endif
static int wait_for_halt(void);
static int read_memory(uint32_t addr, uint8_t *buf, int len);
static int write_memory(uint32_t addr, uint8_t *buf, int len);
static int read_registers(uint32_t addr, uint8_t *buf, int len);
static int write_registers(uint32_t addr, uint8_t *buf, int len);
static uint32_t read_long_register(uint32_t addr);
static void write_long_register(uint32_t addr, uint32_t value);
static int command_loop(void);
static int rx_ack(int ackbyte, int timeout);
static int rx_ack_chksum(int ackbyte, uint8_t *chksum, int timeout);
void reply(char *ptr, int len);

int main(int argc, char *argv[])
{
    extern uint8_t kernel_image_array[];
    extern int kernel_image_size;
    BoardConfig *config, *configSettings;
    char *port, *board, *p, *p2;
    char *logname = NULL;
    int verbose = FALSE, i;
    int baud = 0;
    System sys;
    int ivalue;
    int need_stub = FALSE;

    /* get the environment settings */
    if (!(port = getenv("PROPELLER_LOAD_PORT")))
        port = NULL;
    if (!(board = getenv("PROPELLER_LOAD_BOARD")))
        board = DEF_BOARD;
    
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
                    ShowPorts(PORT_PREFIX);
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
                case 'k':
                    need_stub = TRUE;
                    break;
                case 'v':
                    verbose = TRUE;
                    break;
                case 'l':
                    if(argv[i][2])
                        logname = &argv[i][2];
                    else if(++i < argc)
                        logname = argv[i];
                    else
                        Usage();
                    break;
                default:
                    Usage();
                    break;
            }
        }
        
        /* no arguments other than options are supported */
        else
            Usage();
    }
    
    if (logname) {
        logfile = fopen(logname, "w");
        if (logfile)
            fprintf(logfile, "\n\nStarting log:\n");
    }
    
    sys.ops = &myOps;
    
    /* setup for the selected board */
    if (!(config = ParseConfigurationFile(&sys, board))) {
        fprintf(stderr, "error: can't find board configuration '%s'\n", board);
        return 1;
    }
    
    /* override with any command line settings */
    MergeConfigs(config, configSettings);
    
    /* use the baud rate from the configuration */
    if (GetNumericConfigField(config, "baudrate", &ivalue))
        baud = ivalue;
    
    /* enble verbose progress messages if requested (confuses gdb so should only be used for testing the stub) */
    psetverbose(verbose);
    
    /* find and open the serial port */
    switch (InitPort(PORT_PREFIX, port, baud, verbose, NULL)) {
        case PLOAD_STATUS_OK:
            // port initialized successfully
            break;
        case PLOAD_STATUS_OPEN_FAILED:
            fprintf(stderr, "error: opening serial port '%s'\n", port);
            return 1;
        case PLOAD_STATUS_NO_PROPELLER:
            if (port)
                fprintf(stderr, "error: no propeller chip on port '%s'\n", port);
            else
                fprintf(stderr, "error: can't find a port with a propeller chip\n");
            return 1;
    }
    

#ifdef DEBUG_STUB
    /* load the dummy debug kernel */
    if (need_stub) {
      if (ploadbuf("the debug helper", kernel_image_array, kernel_image_size, DOWNLOAD_RUN_BINARY) != 0) {
        fprintf(stderr, "error: debug kernel load failed\n");
        serial_done();
        return 1;
      }
      first_run = 1;
    }
#endif
    if (debug_cmd(DBG_CMD_STATUS, 0) != ERR_NONE) {
      fprintf(stderr, "error: error sending initial status request\n");
    }
    if (rx_ack(RESPOND_STATUS, INI_TIMEOUT) != ERR_NONE) {
        fprintf(stderr, "error waiting for initial response from dummy debug kernel\n");
        serial_done();
        return 1;
    } else {
        uint8_t dummy[3];
	if (rx_timeout(dummy, 3, INI_TIMEOUT) != 3) {
	  fprintf(stderr, "bad initial status packet\n");
	}
    }
printf("connected!\n");
    
    command_loop();
    
    serial_done();
    
    return 0;
}

/* Usage - display a usage message and exit */
static void Usage(void)
{
    fprintf(stderr, "\
            usage: gdbstub\n\
            [ -b <type> ]     select target board (default is 'default')\n\
            [ -p <port> ]     serial port (default is to auto-detect the port)\n\
            [ -P ]            list available serial ports\n\
            [ -I <path> ]     add a directory to the include path\n\
            [ -D var=value ]  define a board configuration variable\n\
            [ -l <logfile> ]  write a log file\n\
            [ -v ]            verbose output\n");
    exit(1);
}

#if defined(LINUX) || defined(MACOSX) || defined(CYGWIN)
#include <sys/select.h>
static int
data_ready_on_stdin()
{
  fd_set readfds;
  struct timeval tv;
  int r;

  FD_ZERO(&readfds);
  FD_SET(0, &readfds);
  memset(&tv, 0, sizeof(tv));

  r = select(1, &readfds, NULL, NULL, &tv);
  if (r > 0)
    return 1;
  else
    return 0;
}
#else
static int
data_ready_on_stdin()
{
  return 0;
}
#endif


static int debug_cmd(int fcn, int len)
{
    uint8_t byte;
    
    /* send the attention byte */
    byte = HOST_PACKET;
    tx(&byte, 1);
    
    /* and now the function code and cog */
    byte = DEFAULT_COG | fcn;
    tx(&byte, 1);

    /* finally the length of remaining data */
    byte = len;
    tx(&byte, 1);

    /* return successfully */
    return ERR_NONE;
}

/* wait for a response; return 0 if OK, error if response not seen */
static int
rx_ack(int ackbyte, int timeout)
{
  uint8_t byte;

#ifdef VERBOSE_LOG
  if (logfile) fprintf(logfile, "((rx_ack %02x called))\n", ackbyte);
#endif
  if (rx_timeout(&byte, 1, timeout) != 1) {
    if (logfile) fprintf(logfile, "((rx_ack: timeout on first byte %d))\n", timeout);
    return ERR_TIMEOUT;
  }
  if (byte != ackbyte) {
    if (byte == RESPOND_ERR) {
      rx_timeout(&byte, 1, timeout);  /* eat COG number */
      rx_timeout(&byte, 1, timeout);  /* eat error code */
    }
    if (logfile) fprintf(logfile, "((rx_ack: unexpected byte 0x%02x))\n", byte);
    return byte == 0 ? 0xff : byte;
  }
  if (rx_timeout(&byte, 1, timeout) != 1) {
    if (logfile) fprintf(logfile, "((rx_ack: timeout on second byte))\n");
    return ERR_TIMEOUT;
  }
  /* the byte should be our cog */
  if (byte != DEFAULT_COG) {
    if (logfile) fprintf(logfile, "((rx_ack: bad cog # 0x%02x))\n", byte);
    return ERR_CHKSUM;
  }
#ifdef VERBOSE_LOG
  if (logfile) fprintf(logfile, "((rx_ack: OK))\n");
#endif
  return ERR_NONE;
}

static int
rx_ack_chksum(int ackbyte, uint8_t *chksum_p, int timeout)
{
  int err;
  err = rx_ack(ackbyte, timeout);
  if (err != ERR_NONE)
    return err;
  if (rx(chksum_p, 1) != 1)
    return ERR_TIMEOUT;
  return ERR_NONE;
}

static uint16_t cogpc;
static uint8_t cogflags;


#ifdef DEBUG_STUB
static int start_debug_kernel(void)
{
    int err;
    
    if (logfile) {
      fprintf(logfile, "((start_debug_kernel))\n");
    }
    if ((err = debug_cmd(DBG_CMD_RESUME, 0)) != ERR_NONE)
        return err;
    
    wait_for_halt();
    first_run = 0;
    return ERR_NONE;
}
#endif

static int wait_for_halt(void)
{
    uint8_t byte;
    uint8_t cog;
    int timeout = INI_TIMEOUT;
    char buf[8];

#ifdef VERBOSE_LOG
    if (logfile) {
      fprintf(logfile, "((wait_for_halt))\n");
    }
#endif
    for(;;) {
      if (logfile) fflush(logfile);
      if (data_ready_on_stdin()) {
	// check for command from gdb
	int ch;
	if (logfile) fprintf(logfile, "gdb> ");
	while (data_ready_on_stdin()) {
	  ch = fgetc(stdin);
	  if (ch < 0) return ERR_TIMEOUT;
	  if (logfile) { fputc(ch, logfile); fflush(logfile); }
	  msleep(10);
	  if (ch == 3) {
	    uint8_t byte;
	    // use ^C to interrupt the device
	    byte = 0x03;
	    tx(&byte, 1);
	    // and then ask for debug status
	    debug_cmd(DBG_CMD_STATUS, 0);
	  }
	}
	if (logfile) fputc('\n', logfile);
      }
      if (rx_timeout(&byte, 1, INI_TIMEOUT) == 1) {
	if (byte <= 0xf7) {
	  // print the character for the user to see
	  // we have to ask GDB to do this, using an O format packet
	  sprintf(buf, "O%02x", byte);
	  reply(buf, 3);
	  continue;
	}
	if (rx(&cog, 1) != 1) {
	  if (logfile) fprintf(logfile, "((error: error waiting for cog id))\n");
	  return ERR_TIMEOUT;
	}
	if (byte == RESPOND_EXIT) {
	  if (rx_timeout(&cogflags, 1, timeout) != 1) {
	    if (logfile) fprintf(logfile, "((error: error waiting for exit status))\n");
	    continue;
	  }
	  sprintf(buf, "W%02x", cogflags & 0xff);
	  reply(buf, 3);
	  return ERR_NONE;
	}
	if (byte != RESPOND_STATUS) {
	  if (logfile) fprintf(logfile, "((error: unexpected byte 0x%x))\n", byte);
	  return ERR_INIT;
	}
	if (rx_timeout(&cogflags, 1, timeout) != 1) {
	  if (logfile) {
	    fprintf(logfile, "((wait_for_halt: timeout waiting for flags))\n");
	  }
	  return ERR_TIMEOUT;
	}
	if (rx_timeout(&byte, 1, timeout) != 1) {
	  if (logfile) {
	    fprintf(logfile, "((wait_for_halt: timeout waiting for cogpc))\n");
	  }
	  return ERR_TIMEOUT;
	}
	cogpc = byte;
	if (rx_timeout(&byte, 1, timeout) != 1) {
	  if (logfile) {
	    fprintf(logfile, "((wait_for_halt: timeout waiting for cogpc))\n");
	  }
	  return ERR_TIMEOUT;
	}
	cogpc |= (byte<<8);
	if (logfile) {
	  fprintf(logfile, "((halt at cogpc=%x cogflags=%x))\n", cogpc, cogflags);
	}
	return ERR_NONE;
      }
    }
}

static int read_registers(uint32_t addr, uint8_t *buf, int len)
{
    uint8_t pkt[PKT_MAX], *p;
    uint8_t pktlen;
    int remaining, cnt, err;
    
    /* read data in MAX_DATA sized chunks */
    while (len > 0) {
        
        /* build the next packet */
        pktlen = (len > MAX_DATA ? MAX_DATA : len);
        
        /* insert the length and address into the packet */
        p = pkt;
        *p++ = pktlen;
        *p++ =  addr        & 0xff;
        *p++ = (addr >>  8) & 0xff;
        
        /* setup for a read command */
        if ((err = debug_cmd(DBG_CMD_READCOG, 3)) != ERR_NONE)
            return err;
        
        /* send the packet to the debug kernel */
        tx(pkt, p - pkt);
        
        /* get the read response */
        if ( 0 != (err = rx_ack(RESPOND_DATA, PKT_TIMEOUT)) )
	  return err;

        /* read the data */
        for (p = buf, remaining = pktlen; remaining > 0; p += cnt, remaining -= cnt)
            if ((cnt = rx_timeout(p, remaining, PKT_TIMEOUT)) <= 0)
                return ERR_TIMEOUT;
                
        /* move ahead to the next packet */
        addr += pktlen;
        len -= pktlen;
        buf += pktlen;
    }
    
    /* return successfully */
    return ERR_NONE;
}

static int write_registers(uint32_t addr, uint8_t *buf, int len)
{
    uint8_t pkt[PKT_MAX];
    uint8_t pktlen, chksum, i, *p;
    uint8_t rchksum;
    int err;
    
    /* write data in MAX_DATA sized chunks */
    while (len > 0) {
        
        /* build the next packet */
        pktlen = (len > MAX_DATA ? MAX_DATA : len);
        
        /* insert the address into the packet */
        p = pkt;
        *p++ = ( addr        & 0xff);
        *p++ = ((addr >>  8) & 0xff);
        
        /* initialize the checksum */
        chksum = pktlen;
        
        /* store the data into the packet */
        for (i = 0; i < pktlen; ++i)
            chksum += (*p++ = *buf++);
        
        /* setup for a write command */
        if ((err = debug_cmd(DBG_CMD_WRITEHUB, pktlen + 2)) != ERR_NONE)
            return err;
        
        /* send the packet to the debug kernel */
        tx(pkt, p - pkt);
        
        /* wait for the debug kernel to acknowledge */
	if ( 0 != (err = rx_ack_chksum(RESPOND_ACK, &rchksum, PKT_TIMEOUT)) )
	  return err;

        /* move ahead to the next packet */
        addr += pktlen;
        len -= pktlen;
    }
    
    /* return successfully */
    return ERR_NONE;
}

static int read_memory(uint32_t addr, uint8_t *buf, int len)
{
    uint8_t pkt[PKT_MAX], *p;
    uint8_t pktlen;
    int remaining, cnt, err;
    
    /* read data in MAX_DATA sized chunks */
    while (len > 0) {
        
        /* build the next packet */
        pktlen = (len > MAX_DATA ? MAX_DATA : len);
        
        /* insert the length and address into the packet */
        p = pkt;
        *p++ = pktlen;
        *p++ =  addr        & 0xff;
        *p++ = (addr >>  8) & 0xff;
	*p++ = (addr >> 16) & 0xff;
	*p++ = (addr >> 24) & 0xff;
        
        /* setup for a read command */
        if ((err = debug_cmd(DBG_CMD_READHUB, 5)) != ERR_NONE) {	  
	    if (logfile) fprintf(logfile, "((error %d sending readhub cmd))\n", err);
            return err;
	}
        /* send the packet to the debug kernel */
        tx(pkt, p - pkt);
        
        /* get the read response */
        if ( 0 != (err = rx_ack(RESPOND_DATA, PKT_TIMEOUT)) ) {
	    if (logfile) fprintf(logfile, "((error %d in rx_ack))\n", err);
	    return err;
	}
        /* read the data */
        for (p = buf, remaining = pktlen; remaining > 0; p += cnt, remaining -= cnt)
	    if ((cnt = rx_timeout(p, remaining, PKT_TIMEOUT)) <= 0) {
	        if (logfile) fprintf(logfile, "((error %d in rx_timeout))\n", err);
                return ERR_TIMEOUT;
	    }
        /* move ahead to the next packet */
        addr += pktlen;
        len -= pktlen;
        buf += pktlen;
    }
    
    /* return successfully */
    return ERR_NONE;
}

static int write_memory(uint32_t addr, uint8_t *buf, int len)
{
    uint8_t pkt[PKT_MAX];
    uint8_t pktlen, chksum, i, *p;
    uint8_t rchksum;
    int err;
    
    /* write data in MAX_DATA sized chunks */
    while (len > 0) {
        
        /* build the next packet */
        pktlen = (len > MAX_DATA ? MAX_DATA : len);
        
        /* insert the address into the packet */
        p = pkt;
        *p++ = ( addr        & 0xff);
        *p++ = ((addr >>  8) & 0xff);
        *p++ = ((addr >>  16) & 0xff);
        *p++ = ((addr >>  24) & 0xff);
        
        /* initialize the checksum */
        chksum = pktlen;
        
        /* store the data into the packet */
        for (i = 0; i < pktlen; ++i)
            chksum += (*p++ = *buf++);
        
        /* setup for a write command */
        if ((err = debug_cmd(DBG_CMD_WRITEHUB, pktlen + 4)) != ERR_NONE)
            return err;
        
        /* send the packet to the debug kernel */
        tx(pkt, p - pkt);
        
        /* wait for the debug kernel to acknowledge */
	if ( 0 != (err = rx_ack_chksum(RESPOND_ACK, &rchksum, PKT_TIMEOUT)) )
	  return err;

        /* move ahead to the next packet */
        addr += pktlen;
        len -= pktlen;
    }
    
    /* return successfully */
    return ERR_NONE;
}

static uint32_t read_long_register(uint32_t addr)
{
    uint32_t value;
    if (read_registers(addr, (uint8_t *)&value, sizeof(uint32_t)) != ERR_NONE)
        fprintf(stderr, "error: reading %08x\n", addr);
    return value;
}

static void write_long_register(uint32_t addr, uint32_t value)
{
    if (write_registers(addr, (uint8_t *)&value, sizeof(uint32_t)) != ERR_NONE)
        fprintf(stderr, "error: writing %08x\n", addr);
}

static void MyInfo(System *sys, const char *fmt, va_list ap)
{
    vfprintf(stdout, fmt, ap);
}

static void MyError(System *sys, const char *fmt, va_list ap)
{
    vfprintf(stderr, fmt, ap);
}

uint32_t get_cog_pc(int cog)
{
  return 0;
}

void set_cog_pc(int cog, uint32_t addr)
{
}

void reply(char *ptr, int len)
{
    unsigned char cksum = 0;
    int i;
    
    putc('$', stdout);
    if(logfile) fprintf(logfile, "stub>$");
    for(i = 0; i < len; i++){
        putc(ptr[i], stdout);
        if(logfile) putc(ptr[i], logfile);
        cksum += ptr[i];
    }
    fprintf(stdout, "#%02x", cksum);
    if(logfile) fprintf(logfile, "#%02x", cksum);
    if(logfile) putc('\n', logfile);
    fflush(stdout);
    if(logfile) fflush(logfile);
}

void error_reply(int sts)
{
    char buf[10];
    sprintf(buf, "E%02x", sts);
    reply(buf, strlen(buf));
}

char parse_byte(char *ch)
{
    char s[3];
    char val;
    s[0] = ch[0];
    s[1] = ch[1];
    s[2] = 0;
    val = 0xff & strtol(s, NULL, 16);
    return val;
}

int get_cmd()
{
    int i = 0;
    int ch;
    
    if(logfile) fprintf(logfile, "gdb>");
    do{
        if ((ch = getc(stdin)) < 0)
            return -1;
	if(logfile) putc(ch, logfile);
    } while(ch != '$');
    
    for(i = 0; i < sizeof(cmd); i++){
        if ((ch = getc(stdin)) < 0)
            return -1;
        if(logfile) putc(ch, logfile);
        if(ch == '#') break;
        cmd[i] = ch;
    }
    cmd[i] = 0;
    // eat the checksum
    if ((ch = getc(stdin)) < 0)
        return -1;
    if(logfile) putc(ch, logfile);
    if ((ch = getc(stdin)) < 0)
        return -1;
    if(logfile) putc(ch, logfile);
    if(logfile) putc('\n', logfile);
    // send an ACK
    putchar('+');
    fflush(stdout);
    if(logfile) fflush(logfile);
    
    return 0;
}

int tohex(char x)
{
    if(x >= '0' && x <= '9') return x - '0';
    if(x >= 'a' && x <= 'f') return 10 + x - 'a';
    if(x >= 'A' && x <= 'F') return 10 + x - 'A';
    return 0;
}

int32_t get_addr(int *i)
{
    int32_t reg;
    reg  = tohex(cmd[(*i)++]) << 4;
    reg |= tohex(cmd[(*i)++]) << 0;
    reg |= tohex(cmd[(*i)++]) << 12;
    reg |= tohex(cmd[(*i)++]) << 8;
    reg |= tohex(cmd[(*i)++]) << 20;
    reg |= tohex(cmd[(*i)++]) << 16;
    reg |= tohex(cmd[(*i)++]) << 28;
    reg |= tohex(cmd[(*i)++]) << 24;
    return reg;
}

struct bkpt {
    struct bkpt *next;
    uint32_t addr;
    uint32_t len;
};

struct bkpt *bkpt = 0;

/* 'g' - get registers */
void cmd_g_get_registers(int cog)
{
    uint8_t buf[GCC_REG_COUNT * sizeof(uint32_t)];
    char response[1024];
    int j;
    int err;
    
    if((err = read_registers(0, buf, sizeof(buf))) != ERR_NONE){
        error_reply(err);
        return;
    }
    for(j = 0; j < sizeof(buf); j++){
        sprintf(response+2*j, "%02x", buf[j]);
    }
    reply(response, sizeof(buf)*2);
}

/* 'G' - set registers */
void cmd_G_set_registers(int cog, int i)
{
    uint8_t buf[GCC_REG_COUNT * sizeof(uint32_t)];
    int j;
    int err;
    
    for(j = 0; j < GCC_REG_COUNT * sizeof(uint32_t); j++){
        buf[j] = parse_byte(&cmd[i]);
        i += 2;
    }
    if((err = write_registers(0, buf, sizeof(buf))) != ERR_NONE){
        error_reply(err);
        return;
    }
#if 0
    set_cog_pc(cog, (get_addr(&i) & ~0xfffff800) >> 2);
#endif
    reply("OK",2);
}

/* 'm' - read memory */
void cmd_m_read_memory(int i)
{
    int j;
    uint8_t buf[512];
    char response[1024];
    unsigned int addr;
    int len;
    char *end;
    char val;
    int err;
    
    addr = strtol(&cmd[i], &end, 16);
    i = (size_t)end - (size_t)cmd;
    i++;
    len = strtol(&cmd[i], NULL, 16);
    if(len > 512) len = 512;
    if((err = read_memory(addr, buf, len)) != ERR_NONE){
        error_reply(err);
        return;
    }        
    i = j = 0;
    while(len--){
        val = buf[i++];
        sprintf(&response[j], "%02x", (unsigned char)val);
        j += 2;
    }
    reply(response, j);
}

/* 'M' - write memory */
void cmd_M_write_memory(int i)
{
    int j;
    uint8_t buf[512];
    unsigned int addr;
    int len;
    char *end;
    char val;
    int err;
    
    addr = strtol(&cmd[i], &end, 16);
    i = (size_t)end - (size_t)cmd;
    i++;
    len = strtol(&cmd[i], &end, 16);
    i = (size_t)end - (size_t)cmd;
    i++;
    for(j = 0; j < len; ++j){
        val = parse_byte(&cmd[i]);
        i += 2;
        buf[j] = val & 0xff;
    }
    if((err = write_memory(addr, buf, len)) != ERR_NONE){
        error_reply(err);
        return;
    }
    reply("OK",2);
}

/* 's' - single step */
char *cmd_s_step(int cog, int i)
{
    char *halt_code;
    int err;

#ifdef DEBUG_STUB
    if (first_run) {
        if ((err = start_debug_kernel()) != ERR_NONE){
            error_reply(err);
            return "E99";   // BUG: what should this be?
        }
    }
#endif
    if(cmd[i]){
        uint8_t buf[sizeof(uint32_t)];
        int j;
        for (j = 0; j < sizeof(uint32_t); j++){
            buf[j] = parse_byte(&cmd[i]);
            i += 2;
        }
        if((err = write_registers(GCC_REG_PC, buf, sizeof(buf))) != ERR_NONE){
            error_reply(err);
            return "E99"; // BUG: what should this be?
        }
    }
    if ((err = debug_cmd(DBG_CMD_LMMSTEP, 0)) != ERR_NONE){
        error_reply(err);
        return "E99";   // BUG: what should this be?
    }
    if ((err = wait_for_halt()) != ERR_NONE){
        error_reply(err);
        return "E99";   // BUG: what should this be?
    }
#if 0
    halt_code = "S05";
#else
    {
      static char code[80];
      uint32_t lmmpc;
      lmmpc = read_long_register(GCC_REG_PC);
      sprintf(code, "T05%x:%02x%02x%02x%02x;",
	      GCC_REG_PC,
	      lmmpc & 0xff,
	      (lmmpc>>8) & 0xff,
	      (lmmpc>>16) & 0xff,
	      (lmmpc>>24) & 0xff);
      halt_code = code;
    }
#endif
    reply(halt_code, strlen(halt_code));
    return halt_code;
}

/* 'c' - continue */
char *cmd_c_continue(int cog, int i)
{
    char *halt_code;
    int err;
    
#ifdef DEBUG_STUB
    if (first_run) {
        if ((err = start_debug_kernel()) != ERR_NONE){
            error_reply(err);
            return "E99";   // BUG: what should this be?
        }
    }
#endif
    if(cmd[i]){
        uint8_t buf[sizeof(uint32_t)];
        int j;
        for (j = 0; j < sizeof(uint32_t); j++){
            buf[j] = parse_byte(&cmd[i]);
            i += 2;
        }
        if((err = write_registers(GCC_REG_PC, buf, sizeof(buf))) != ERR_NONE){
            error_reply(err);
            return "E99"; // BUG: what should this be?
        }
    }
    if ((err = debug_cmd(DBG_CMD_RESUME, 0)) != ERR_NONE){
        error_reply(err);
        return "E99";   // BUG: what should this be?
    }
    if ((err = wait_for_halt()) != ERR_NONE){
        error_reply(err);
        return "E99";   // BUG: what should this be?
    }
#if 0
    halt_code = "S05";
#else
    {
      static char code[80];
      uint32_t lmmpc;
      lmmpc = read_long_register(GCC_REG_PC);
      sprintf(code, "T05%x:%02x%02x%02x%02x;",
	      GCC_REG_PC,
	      lmmpc & 0xff,
	      (lmmpc>>8) & 0xff,
	      (lmmpc>>16) & 0xff,
	      (lmmpc>>24) & 0xff);
      halt_code = code;
    }
#endif
    reply(halt_code, strlen(halt_code));
    
    return halt_code;
}

/* 'H' - set thread */
void cmd_H_set_thread(int i)
{
    if((cmd[i] == 'g') && (cmd[i+1] == '0')){
        reply("OK", 2);
    } else {
        reply("", 0);
    }
}

/* 'q' - query features */
void cmd_q_query(int i)
{
    if (!strncmp(&cmd[i], "Supported", 8)){
      reply("QStartNoAckMode+", 16);
    } else {
        reply("", 0);
    }
}

/* 'Q' - set features */
void cmd_Q_set(int i)
{
    if (!strncmp(&cmd[i], "StartNoAckMode-", 15)) {
      reply("OK", 2);
      no_ack_mode = 0;
    } else if (!strncmp(&cmd[i], "StartNoAckMode", 14)) {
      reply("OK", 2);
      no_ack_mode = 1;
    } else {
      reply("", 0);
    }
}

/* 'z' - remove breakpoint */
void cmd_z_remove_breakpoint(int i)
{
#ifdef NO_BKPT
    reply("", 0);
#else
    /* Remove breakpoint */
    if(cmd[i++] == '0'){
        long addr;
        long len;
        struct bkpt *b;
        char *p = &cmd[i];
        
        p++;   /* Skip the comma */
        addr = strtol(p, &p, 16);
        p++;   /* Skip the other comma */
        len = strtol(p, NULL, 16);
        for(b = (struct bkpt *)&bkpt; b && b->next; b = b->next){
            if((b->next->addr == addr) && (b->next->len == len)){
                struct bkpt *t = b->next;
                b->next = t->next;
                free(t);
            }
        }
        reply("OK", 2);
    } else {
        reply("", 0);
    }
#endif
}

/* 'Z' - set breakpoint */
void cmd_Z_set_breakpoint(int i)
{
#ifdef NO_BKPT
    reply("", 0);
#else
    /* Set breakpoint */
    if(cmd[i++] == '0'){
        long addr;
        long len;
        struct bkpt *b;
        char *p = &cmd[i];
        p++;   /* Skip the comma */
        addr = strtol(p, &p, 16);
        p++;   /* Skip the other comma */
        len = strtol(p, NULL, 16);
        for(b = (struct bkpt *)&bkpt; b->next; b = b->next){
            if((b->addr == addr) && (b->len == len)){
                /* Duplicate; bail out */
                break;
            }
        }
        if(b->next){
            /* Was a duplicate, do nothing */
        } else {
            struct bkpt *t = (struct bkpt *)malloc(sizeof(struct bkpt));
            if(!t){
                fprintf(stderr, "Failed to allocate a breakpoint structure\n");
                exit(1);
            }
            t->addr = addr;
            t->len = len;
            t->next = bkpt;
            bkpt = t;
        }
        reply("OK", 2);
    } else {
        reply("", 0);
    }
#endif
}

static int command_loop(void)
{
    int i;
    int cog = 0;
    char *halt_code = "S05";
    
    for (;;){
        if (get_cmd() != 0) {
            if (logfile) fprintf(logfile, "[ gdb closed the pipe ]\n");
            goto out;
        }
        i = 0;
        switch(cmd[i++]){
            case 'g':   cmd_g_get_registers(cog);           break;
            case 'G':   cmd_G_set_registers(cog, i);        break;
            case 'm':   cmd_m_read_memory(i);               break;
            case 'M':   cmd_M_write_memory(i);              break;
            case 's':   halt_code = cmd_s_step(cog, i);     break;
            case 'c':   halt_code = cmd_c_continue(cog, i); break;
            case 'H':   cmd_H_set_thread(i);                break;
            case 'k':   reply("OK", 2); goto out;           break;
	    case 'q':   cmd_q_query(i);                     break;
	    case 'Q':   cmd_Q_set(i);                       break;
            case 'z':   cmd_z_remove_breakpoint(i);         break;
            case 'Z':   cmd_Z_set_breakpoint(i);            break;
            case '?':   reply(halt_code, 3);                break;
            default:    reply("", 0);                       break;
        }
    }
out:
    ;
    
    return 0;
}
