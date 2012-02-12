#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
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

/* disable breakpoint support */
#define NO_BKPT

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

/* timeout waiting for debug kernel to initialize */
#define INI_TIMEOUT 2000

/* timeout waiting for packet data */
#define PKT_TIMEOUT 10000

/* attention byte sent before a command is sent */
#define ATTN        0x01
#define ACK         0x40

/* serial debug kernel function codes */
#define FCN_STEP    0x01
#define FCN_RUN     0x02
#define FCN_READ    0x03
#define FCN_WRITE   0x04

/* protocol ack/nak */
#define NAK         0x15

/* code to send to the serial debug kernel to interrupt program execution */
#define INT         0x01

/* code received from the serial debug kernel when the program halts do to a breakpoint or single step */
#define HALT        '!'

/* maximum amount of data in a READ or WRITE packet */
#define MAX_DATA    128

/* maximum packet size (FCN_WRITE packet with 2 byte addr, 1 byte count, 128 data bytes */
#define PKT_MAX     (3 + MAX_DATA)

#define DEF_GCC_REG_BASE    0
#define GCC_REG_COUNT       18  // r0-r14, lr, sp, pc -- what about flags, breakpt?
#define GCC_REG_PC          17

FILE *logfile = NULL;
char cmd[1028];
uint32_t gcc_reg_base = DEF_GCC_REG_BASE;

uint32_t get_cog_pc(int cog){
  return 0;
}

void set_cog_pc(int cog, uint32_t addr){
}

void reply(char *ptr, int len){
  unsigned char cksum = 0;
  int i;

  putc('$', stdout);
  if(logfile) fprintf(logfile, "sim>$");
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

void error_reply(int sts){
  char buf[10];
  sprintf(buf, "E%02x", sts);
  reply(buf, strlen(buf));
}

char parse_byte(char *ch){
  char s[3];
  char val;
  s[0] = ch[0];
  s[1] = ch[1];
  s[2] = 0;
  val = 0xff & strtol(s, NULL, 16);
  return val;
}

int debug_cmd(int fcn) {
	uint8_t byte;
	
	/* send the attention byte */
	byte = ATTN;
	tx(&byte, 1);

	/* wait for an ACK or a timeout */
	do {
	    if (rx_timeout(&byte, 1, PKT_TIMEOUT) != 1)
	        return ERR_TIMEOUT;
	} while (byte != ACK);

    /* send the function code */
	byte = fcn;
	tx(&byte, 1);
	
	/* return successfully */
	return ERR_NONE;
}

int read_memory(uint32_t addr, uint8_t *buf, int len){
    uint8_t pkt[PKT_MAX];
    uint8_t pktlen, chksum, i, *p;
    int err;
    
    /* read data in MAX_DATA sized chunks */
    while (len > 0) {
        
        /* build the next packet */
        pktlen = (len > MAX_DATA ? MAX_DATA : len);
        
        /* insert the address into the packet */
        p = pkt;
        *p++ =  addr        & 0xff;
        *p++ = (addr >>  8) & 0xff;
        *p++ = pktlen;

        /* setup for a read command */
        if ((err = debug_cmd(FCN_READ)) != ERR_NONE)
            return err;
        
        /* send the packet to the debug kernel */
        tx(pkt, p - pkt);
        
        /* read the data */
        if (rx_timeout(buf, pktlen + 1, PKT_TIMEOUT) != pktlen + 1)
            return ERR_TIMEOUT;
            
        /* initialize the checksum */
        chksum = pktlen;
        
        /* verify the checksum */
        for (i = 0; i < pktlen; ++i)
            chksum += buf[i];
        if (chksum != buf[pktlen])
            return ERR_CHKSUM;
            
        /* move ahead to the next packet */
        addr += pktlen;
        len -= pktlen;
        buf += pktlen;
    }
    
    /* return successfully */
    return ERR_NONE;
}

int write_memory(uint32_t addr, uint8_t *buf, int len){
    uint8_t pkt[PKT_MAX], byte;
    uint8_t pktlen, chksum, i, *p;
    int err;
    
    /* write data in MAX_DATA sized chunks */
    while (len > 0) {
        
        /* build the next packet */
        pktlen = (len > MAX_DATA ? MAX_DATA : len);
        
        /* insert the address into the packet */
        p = pkt;
        *p++ = ( addr        & 0xff);
        *p++ = ((addr >>  8) & 0xff);
        *p++ = pktlen;

        /* initialize the checksum */
        chksum = pktlen;
        
        /* store the data into the packet */
        for (i = 0; i < pktlen; ++i)
            chksum += (*p++ = *buf++);
        
        /* setup for a write command */
        if ((err = debug_cmd(FCN_WRITE)) != ERR_NONE)
            return err;
        
        /* send the packet to the debug kernel */
        tx(pkt, p - pkt);
        
        /* wait for the debug kernel to acknowledge */
        if (rx_timeout(&byte, 1, PKT_TIMEOUT) != 1)
            return ERR_TIMEOUT;
        else if (byte != chksum)
            return ERR_CHKSUM;
            
        /* move ahead to the next packet */
        addr += pktlen;
        len -= pktlen;
    }
    
    /* return successfully */
    return ERR_NONE;
}

void get_cmd(){
  int i = 0;
  int ch;

  do{
    ch = getc(stdin);
  } while(ch != '$');

  if(logfile) fprintf(logfile, "gdb>");
  if(logfile) putc(ch, logfile);

  for(i = 0; i < sizeof(cmd); i++){
    ch = getc(stdin);
    if(logfile) putc(ch, logfile);
    if(ch == '#') break;
    cmd[i] = ch;
  }
  cmd[i] = 0;
  // eat the checksum
  ch = getc(stdin);
  if(logfile) putc(ch, logfile);
  ch = getc(stdin);
  if(logfile) putc(ch, logfile);
  if(logfile) putc('\n', logfile);
  // send an ACK
  putchar('+');
  fflush(stdout);
  if(logfile) fflush(logfile);
}

int tohex(char x){
  if(x >= '0' && x <= '9') return x - '0';
  if(x >= 'a' && x <= 'f') return 10 + x - 'a';
  if(x >= 'A' && x <= 'F') return 10 + x - 'A';
  return 0;
}

int32_t get_addr(int *i){
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
void cmd_g_get_registers(int cog){
      uint8_t buf[GCC_REG_COUNT * sizeof(uint32_t)];
      char response[1024];
      int32_t reg;
      int j;
      int err;
      
          if((err = read_memory(gcc_reg_base, buf, sizeof(buf))) != ERR_NONE){
            error_reply(err);
            return;
          }
          for(j = 0; j < sizeof(buf); j++){
            sprintf(response+2*j, "%02x", buf[j]);
          }
          reg = get_cog_pc(cog) * 4 + 0x80000000 + cog * 0x10000000;
          sprintf(response+2*j,
                  "%02x%02x%02x%02x",
                  (unsigned char)(reg & 0xff),
                  (unsigned char)((reg>>8) & 0xff),
                  (unsigned char)((reg>>16) & 0xff),
                  (unsigned char)((reg>>24) & 0xff));
          reply(response, sizeof(buf)*2+8);
}

/* 'G' - set registers */
void cmd_G_set_registers(int cog, int i){
      uint8_t buf[GCC_REG_COUNT * sizeof(uint32_t)];
      int j;
      int err;
      
          for(j = 0; j < GCC_REG_COUNT * sizeof(uint32_t); j++){
            buf[j] = parse_byte(&cmd[i]);
            i += 2;
          }
          if((err = write_memory(gcc_reg_base, buf, sizeof(buf))) != ERR_NONE){
            error_reply(err);
            return;
          }
          set_cog_pc(cog, (get_addr(&i) & ~0xfffff800) >> 2);
          reply("OK",2);
}

/* 'm' - read memory */
void cmd_m_read_memory(int i){
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
void cmd_M_write_memory(int i){
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
char *cmd_s_step(int cog, int i){
      char *halt_code;
      int err;

          if(cmd[i]){
            uint8_t buf[sizeof(uint32_t)];
            int j;
            for (j = 0; j < sizeof(uint32_t); j++){
              buf[j] = parse_byte(&cmd[i]);
              i += 2;
            }
            if((err = write_memory(gcc_reg_base + GCC_REG_PC * sizeof(uint32_t), buf, sizeof(buf))) != ERR_NONE){
              error_reply(err);
              return "E99"; // BUG: what should this be?
            }
          }
          if ((err = debug_cmd(FCN_STEP)) != ERR_NONE){
            error_reply(err);
            return "E99";   // BUG: what should this be?
          }
          halt_code = "S05";
          reply(halt_code, 3);
          
      return halt_code;
}

/* 'c' - continue */
char *cmd_c_continue(int cog, int i){
      char *halt_code;
      int err;
      
          if(cmd[i]){
            uint8_t buf[sizeof(uint32_t)];
            int j;
            for (j = 0; j < sizeof(uint32_t); j++){
              buf[j] = parse_byte(&cmd[i]);
              i += 2;
            }
            if((err = write_memory(gcc_reg_base + GCC_REG_PC * sizeof(uint32_t), buf, sizeof(buf))) != ERR_NONE){
              error_reply(err);
              return "E99"; // BUG: what should this be?
            }
          }
          halt_code = "S02";
          if ((err = debug_cmd(FCN_RUN)) != ERR_NONE){
            error_reply(err);
            return "E99";   // BUG: what should this be?
          }
          reply(halt_code, 3);
          
      return halt_code;
}

/* 'H' - set thread */
void cmd_H_set_thread(int i){
          if((cmd[i] == 'g') && (cmd[i+1] == '0')){
            reply("OK", 2);
          } else {
            reply("", 0);
          }
}

/* 'z' - remove breakpoint */
void cmd_z_remove_breakpoint(int i){
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
void cmd_Z_set_breakpoint(int i){
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

int command_loop(void){
      int i;
      int cog = 0;
      char *halt_code = "S05";

      logfile = fopen("gdbstub.log", "w");
      if(logfile) fprintf(logfile, "\n\nStarting log:\n");

#if 0
      reboot();
#endif

      for(;;){
        get_cmd();
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

static void MyInfo(System *sys, const char *fmt, va_list ap)
{
    vfprintf(stdout, fmt, ap);
}

static void MyError(System *sys, const char *fmt, va_list ap)
{
    vfprintf(stderr, fmt, ap);
}

static SystemOps myOps = {
    MyInfo,
    MyError
};

/* Usage - display a usage message and exit */
static void Usage(void)
{
    fprintf(stderr, "\
usage: propeller-load\n\
         [ -b <type> ]     select target board (default is 'default')\n\
         [ -p <port> ]     serial port (default is to auto-detect the port)\n\
         [ -P ]            list available serial ports\n\
         [ -I <path> ]     add a directory to the include path\n\
         [ -D var=value ]  define a board configuration variable\n\
         [ -v ]            verbose output\n");
    exit(1);
}

int main(int argc, char *argv[])
{
    extern uint8_t dummy_binary_array[];
    extern int dummy_binary_size;
    BoardConfig *config, *configSettings;
    char *port, *board, *p, *p2;
    int verbose = FALSE, i;
    int baud = 0;
    uint8_t byte;
    System sys;

    /* get the environment settings */
    if (!(port = getenv("PROPELLER_LOAD_PORT")))
        port = NULL;
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
                if (!SetConfigField(configSettings, p, p2))
                    return 1;
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
                verbose = TRUE;
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
    
    sys.ops = &myOps;

    /* setup for the selected board */
    if (!(config = ParseConfigurationFile(&sys, board))) {
        fprintf(stderr, "error: can't find board configuration '%s'\n", board);
        return 1;
    }
    
    /* override with any command line settings */
    MergeConfigs(config, configSettings);
        
    /* use the baud rate from the configuration */
    baud = config->baudrate;
    
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
        
    /* load the serial helper program */
    if (ploadbuf(dummy_binary_array, dummy_binary_size, DOWNLOAD_RUN_BINARY) != 0) {
        fprintf(stderr, "error: debug kernel load failed\n");
        return 1;
    }

    if (rx_timeout(&byte, 1, INI_TIMEOUT) != 1) {
        fprintf(stderr, "error: timeout waiting for initial response from debug kernel\n");
        return 1;
    }
    else if (byte != HALT) {
        fprintf(stderr, "error: bad initial response from debug kernel: %02x\n", byte);
        return 1;
    }

    command_loop();
    
    serial_done();
    
    return 0;
}
