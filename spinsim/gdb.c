/*******************************************************************************
' Version 0.54
' Copyright (c) 2010, 2011, 2012
' See end of file for terms of use.
'******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#ifdef LINUX
#include <dirent.h>
#include <sys/stat.h>
#include "conion.h"
#else
#include <conio.h>
#include <direct.h>
#endif
#include "interp.h"
#include "spinsim.h"

#define GCC_REG_BASE 0

extern FILE *logfile;
extern FILE *tracefile;
extern FILE *cmdfile;
extern PasmVarsT PasmVars[8];
extern char *hubram;
char cmd[1028];
extern int32_t proptwo;
extern int32_t profile;
extern int32_t loopcount;

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

char parse_byte(char *ch){
  char s[3];
  char val;
  s[0] = ch[0];
  s[1] = ch[1];
  s[2] = 0;
  val = 0xff & strtol(s, NULL, 16);
  return val;
}

char get_byte(uint32_t addr){
  if(addr & 0x80000000){
    int cog = (addr >> 28) - 8;
    uint32_t tmp;
    tmp = PasmVars[cog].mem[(addr & 0x000007ff) >> 2];
    return (tmp >> 8*(addr & 0x3)) & 0xff;
  } else {
    return BYTE(addr);
  }
}

void put_byte(uint32_t addr, unsigned char val){
  if(addr & 0x80000000){
    int cog = (addr >> 28) - 8;
    uint32_t tmp;
    tmp = PasmVars[cog].mem[(addr & 0x000007ff) >> 2];
    switch(addr & 3){
    case 0:
      tmp &= 0xffffff00;
      tmp |= val;
      break;
    case 1:
      tmp &= 0xffff00ff;
      tmp |= val << 8;
      break;
    case 2:
      tmp &= 0xff00ffff;
      tmp |= val << 16;
      break;
    case 3:
      tmp &= 0x00ffffff;
      tmp |= val << 24;
      break;
    }
    PasmVars[cog].mem[(addr & 0x000007ff) >> 2] = tmp;
    return;
  } else {
    BYTE(addr) = val;
    return;
  }
}

void get_cmd(){
  int i = 0;
  int ch;

  do{
    ch = getc(cmdfile);
  } while(ch != '$');

  if(logfile) fprintf(logfile, "gdb>");
  if(logfile) putc(ch, logfile);

  for(i = 0; i < sizeof(cmd); i++){
    ch = getc(cmdfile);
    if(logfile) putc(ch, logfile);
    if(ch == '#') break;
    cmd[i] = ch;
  }
  cmd[i] = 0;
  // eat the checksum
  ch = getc(cmdfile);
  if(logfile) putc(ch, logfile);
  ch = getc(cmdfile);
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

// Check the cog's PC to see if the LMM microcode is at an appropriate place
// for a breakpoint to occur.  We don't want to break on LMM administrative
// instructions.
// FIXME we need a more general way to do this.  This is too dependent on special knowledge.
int breakable_point(int i){
  int32_t pc = PasmVars[i].pc;
  if ((pc == 0x0000004c/4)
   || (pc == 0x00000058/4)
   || (pc == 0x00000064/4)
   || (pc == 0x00000070/4)
   || (pc == 0x0000007c/4)
   || (pc == 0x00000088/4)
   || (pc == 0x00000094/4)
   || (pc == 0x000000a0/4))
     return 1;
   else
     return 0;
}

void gdb(void)
{
      int i;
      int j;
      char response[1024];
      unsigned int addr;
      int len;
      char *end;
      char val;
      int32_t reg;
      int cog = 0;
      char *halt_code = "S05";

      if(logfile) fprintf(logfile, "\n\nStarting log:\n");

      RebootProp();

      for(;;){
	get_cmd();
	i = 0;
	switch(cmd[i++]){
	case 'g':
	  for(i = 0; i < 18; i++){
	    reg = PasmVars[cog].mem[GCC_REG_BASE + i];
	    sprintf(response+8*i,
		    "%02x%02x%02x%02x",
		    (unsigned char)(reg & 0xff),
		    (unsigned char)((reg>>8) & 0xff),
		    (unsigned char)((reg>>16) & 0xff),
		    (unsigned char)((reg>>24) & 0xff));
	  }
	  reg = PasmVars[cog].pc * 4 + 0x80000000 + cog * 0x10000000;
	  sprintf(response+8*i,
		  "%02x%02x%02x%02x",
		  (unsigned char)(reg & 0xff),
		  (unsigned char)((reg>>8) & 0xff),
		  (unsigned char)((reg>>16) & 0xff),
		  (unsigned char)((reg>>24) & 0xff));
	  reply(response, 8*18+8);
	  break;

	case 'G':
	  for(j = 0; j < 18; j++){
	    PasmVars[cog].mem[GCC_REG_BASE + j] = get_addr(&i);
	  }
	  // Ignore writes to cog pc.  Instead, reset it to be
	  // ready for a fresh instruction.
	  // This is too magical.  How do we fix it?
	  PasmVars[cog].pc = 0x12;
	  reply("OK",2);
	  break;

	case 'm':
	  addr = strtol(&cmd[i], &end, 16);
	  i = (size_t)end - (size_t)cmd;
	  i++;
	  len = strtol(&cmd[i], NULL, 16);
	  if(len > 512) len = 512;
	  j = 0;
	  while(len--){
	    val = get_byte(addr);
	    addr++;
	    sprintf(&response[j], "%02x", (unsigned char)val);
	    j += 2;
	  }
	  reply(response, j);
	  break;

	case 'M':
	  addr = strtol(&cmd[i], &end, 16);
	  i = (size_t)end - (size_t)cmd;
	  i++;
	  len = strtol(&cmd[i], &end, 16);
	  i = (size_t)end - (size_t)cmd;
	  i++;
	  while(len--){
	    val = parse_byte(&cmd[i]);
	    i += 2;
	    put_byte(addr, val & 0xff);
	    addr++;
	  }
	  reply("OK",2);
	  break;

	case 's':
	  if(cmd[i]){
	    // Get the new LMM PC, reset the microcode
	    PasmVars[cog].mem[GCC_REG_BASE + 17] = get_addr(&i);
	    PasmVars[cog].pc = 0x12;
	  }
	  {
	    int brk = 0;
	    do {
	      int i;
	      // Step through a full LMM instruction
	      step_chip();
              for(i = 0; i < 8; i++){
	        if(breakable_point(i)) brk = 1;
	      }
	    } while (!brk);
	  }
	  halt_code = "S05";
	  reply(halt_code, 3);
	  break;

	case 'c':
	  if(cmd[i]){
	    PasmVars[cog].mem[GCC_REG_BASE + 17] = get_addr(&i);
	    PasmVars[cog].pc = 0x12;
	  }
	  halt_code = "S02";
	  do {
	    int brk = 0;
            struct bkpt *b;

            for (i = 0; i < 8; i++) {
              if(breakable_point(i)){
                // Look to see if the cog's LMM PC is at a breakpoint
                for(b = (struct bkpt *)&bkpt; b->next; b = b->next){
                  if((PasmVars[i].mem[GCC_REG_BASE + 17] >= b->next->addr)
                     && (PasmVars[i].mem[GCC_REG_BASE + 17] <= b->next->addr + b->next->len)){
                    brk = 1;
                  }
                }
              }
            }
	    if(brk){
	      halt_code = "S05";
	      break;
	    } else {
	      step_chip();
	    }
	  } while(getch() != 0x03); // look for a CTRL-C
	  reply(halt_code, 3);
	  break;

	case 'H':
	  if((cmd[i] == 'g') && (cmd[i+1] == '0')){
	    reply("OK", 2);
	  } else {
	    reply("", 0);
	  }
	  break;

	case 'k':
	  reply("OK", 2);
	  goto out;

	case 'z':
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
	  break;

	case 'Z':
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
	  break;

	case '?':
	  reply(halt_code, 3);
	  break;

	default:
	  reply("", 0);
	  break;
	}
      }
    out:
      ;
}
/*
+------------------------------------------------------------------------------------------------------------------------------+
|                                                   TERMS OF USE: MIT License                                                  |
+------------------------------------------------------------------------------------------------------------------------------+
|Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation    |
|files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,    |
|modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software|
|is furnished to do so, subject to the following conditions:                                                                   |
|                                                                                                                              |
|The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.|
|                                                                                                                              |
|THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE          |
|WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR         |
|COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,   |
|ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                         |
+------------------------------------------------------------------------------------------------------------------------------+
*/
