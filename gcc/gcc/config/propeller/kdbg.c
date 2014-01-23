#include <stdio.h>

#define KBASE 0x20

#define AS_BYTES	0
#define AS_WORDS	1
#define AS_LONGS	2
//#define AS_CHARS	3
//define AS_INSNS	4

char kernel[512*4], cmd, last_cmd, scrap, buffer[2048];

int address = 0, data = 0, mode = AS_LONGS, v;

void read_hub(char *ptr, int addr, int len);
void write_hub(char *ptr, int addr, int len);

char *help[] = {
	"s\t\tstep one  instruction",
	"x\t\trun program from current pc",
	"r\t\tshow registers",
	"ul\t\tdisplay/enter data as longs",
	"uw\t\tdisplay/enter data as words",
	"ub\t\tdisplay/enter data as bytes",
	"m $addr\tread & display 256 bytes from addr in the hub", 
	"w $addr $val\twrite $val to hub at $addr",
	"?\t\tthis help screen",
	"q\t\tquit debugger test",
	"\n\t\tNOTE: Pressing ENTER repeats the last command",
	NULL,
};

void print_help() {
	char *p;
	int i;
	printf("\nUSAGE: kdbg\n\n");
	for(i=0;help[i];i++)
		printf("\t%s\n",help[i]);
	printf("\n");
}

void pb(char *s,int o) {
	printf("%02x",255&*(char *)((int)s+o));
}

void pw(char *s,int o) {
	pb(s,1+o);
	pb(s,o);
}

void pl(char *s,int o) {
	pw(s,2+o);
	pw(s,o);
}

void pc(char *s,int o) {
	int ch;
	ch = *(char *)((int)s+o) & 255;
	if (ch < 32)
		ch = '.';
	printf("%c",ch);
}

void print_regs(void) {
	int r;
	printf("***PRINT REGS***\n");fflush(stdout);
	read_hub(kernel,0x0020,80);
	printf("*after read_hub in print_regs*\n");fflush(stdout);
	for(r=0;r<15;r++) {
		printf("$%04x: R%02d = ",0x20+(r<<2),r);
		pl(kernel,r<<2);
		printf("\n");
	}
	printf("$005C: LR  = ");
	pl(kernel,15*4);
	printf("\n");
	printf("$0060: SP  = ");
	pl(kernel,16*4);
	printf("\n");
	printf("$0064: PC  = ");
	pl(kernel,17*4);
	printf("\n");
	r = *(int *)((int)kernel+18*4);
	printf("\n$0068: Z = %d C = %d\n\n",(r&2)>>1,r&1);
	fflush(stdout);
}

void print_mem(int mtype, char *buff, int hubaddr, int numbytes) { // 16 bytes at a time
	int	b,p,o;
	printf("\n");
	for(b=0;b<numbytes;b+=16) {
		printf("%04x: ",hubaddr+b);
		switch (mtype) {
			case AS_BYTES:
					for(o=0;o<16;o++) {
						pb(buff,b+o);
						printf(" ");
					}
					printf(" ");
					for(o=0;o<16;o++) {
						pc(buff,b+o);
					}
				break;
			case AS_WORDS:
					for(o=0;o<16;o+=2) {
						pw(buff,b+o);
						printf(" ");
					}
				break;
			case AS_LONGS:
					for(o=0;o<16;o+=4) {
						pl(buff,b+o);
						printf(" ");
					}
				break;
			default: ;
		}
		printf("\n");
	}
	printf("\n");
}

FILE	*in;
int	ch=0,len=0,good=0,bad=0, chk;

void debug_cmd(int n) {
	putc(1,in);

	do {
		ch=getc(in);
		//printf(" [%02x]",ch); fflush(stdout);
	} while (ch != 0x40);

	putc(n,in);
		//printf(" <%02x>",n); fflush(stdout);
}

void read_hub(char *ptr, int addr, int len) {

	debug_cmd(3);

	putc(addr&255,in);
	putc((addr>>8)&255,in);
	putc(len,in);

	if (len) {
		chk = len;
		while (len) {
			ch = getc(in);
			*ptr++ = ch;
			chk += ch;
			len--;			 
		}
		chk &= 255;
		if (chk == getc(in)) {
			good++;
		} else {
			bad++;
		}
	} else
		printf("error - 0 length packet\n");
}


void write_hub(char *ptr, int addr,int len) {

	debug_cmd(4);

	putc(addr&255,in);	// send address and length
	putc((addr>>8)&255,in);
	putc(len,in);

	if (len) {
		chk = len;
		while (len) {
			ch = *ptr++;
			putc(ch,in);	
			chk += ch;
			len--;			 
		}
		chk &= 255;
		if (chk == getc(in)) {
			good++;
		} else {
			bad++;
		}
	} else
		printf("error - 0 length packet\n");
}

int main(int argc, char **argv) {
	int ch, och, lastch;
	char cmdline[256];
	in = fopen("/dev/ttyUSB0","rb+");
	if (in) {

		print_help();

		do {
			printf("debug> ");

			gets(cmdline);

			lastch = ch;
			ch = toupper(cmdline[0]);

			if (ch == 0)
				ch = lastch; // 'enter' repeats last command

			switch (ch) {
				case 'S':
					debug_cmd(1);
//		printf(" after step()",ch); fflush(stdout);
					print_regs();
					break;
				case 'X':
					debug_cmd(2);
					//print_regs();
					break;
				case 'U':
					och = cmdline[1];
					switch (toupper(och)) {
						case 'L':
							mode = AS_LONGS;
							break;
						case 'W':
							mode = AS_WORDS;
							break;
						case 'B':
							mode = AS_BYTES;
							break;
						default:
							printf("Invalid display type '%c'\n",ch);
					}
					break;
				case 'M':
					sscanf(cmdline,"%c $%4x",&scrap,&address);
					read_hub(buffer,address,128);
					read_hub(&buffer[128],address+0x0080,128);				
					print_mem(mode,buffer,address,256);
					address = (address + 0x100) & 0xFFFF;
					break;
				case 'W': 
					sscanf(cmdline,"%c $%4x $%8x",&scrap,&address,&data);
					switch (mode) {
						case AS_LONGS:
							buffer[0] = data & 255;
							buffer[1] = (data>>8) & 255;
							buffer[2] = (data>>16) & 255;
							buffer[3] = (data>>24) & 255;
							write_hub(buffer,address,4);
							break;
						case AS_WORDS:
							buffer[0] = data & 255;
							buffer[1] = (data>>8) & 255;
							write_hub(buffer,address,2);
							break;
						case AS_BYTES:
							buffer[0] = data & 255;
							write_hub(buffer,address,1);
							break;
					}
					break;
				case 'R':
					print_regs();
					break;
				case '?':
				case 'H':
					print_help();
					break;
				case 'Q':
					break;
				default:
					print_help();
					printf("\n'%c' is not a valid command.\n",ch);
			}

		} while (ch!='Q');

		fclose(in);
	} else
		printf("Could not open /dev/ttyUSB0 !\n");
}



