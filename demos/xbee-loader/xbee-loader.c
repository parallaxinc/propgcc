#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <propeller.h>
#include "xbeeframe.h"

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

/* assumes WPA2 encryption */
#define SSID        "xlisper4"
#define PASSWORD    "shaky2!raven"

/* Xbee pins */
#define XBEE_RX     13
#define XBEE_TX     12
#define XBEE_BAUD   9600

#define MAX_RETRIES 100000

/* list of drivers we can use */
extern _Driver _FullDuplexSerialDriver;
_Driver *_driverlist[] = {
    &_FullDuplexSerialDriver,
    NULL
};

uint32_t ticks_per_ms;

void send(FILE *fp, char *fmt, ...);
void wait(int ms);
int expect(FILE *fp, char *str);

void SendFrame(FILE *fp, uint8_t *frame, int size);
int RecvFrame(FILE *fp, uint8_t *frame, int maxSize);
int nextc(FILE *fp);

int main(void)
{
    FILE *xbeein, *xbeeout;
    uint8_t frame[1024];
    XbeeFrameInit_t init;
    XbeeFrame_t mailbox;
    int cog, ch, i;
    
    ticks_per_ms = CLKFREQ / 1000;
    
    if (!(xbeein = fopen("FDS:9600,13,12", "r")))
        return 1;
    if (!(xbeeout = fopen("FDS:9600,13,12", "w")))
        return 1;
        
    /* set non-blocking mode on stdin and xbee */
    stdin->_flag |= _IONONBLOCK;
    stdin->_flag &= ~_IOCOOKED;
    xbeein->_flag |= _IONONBLOCK;
    xbeein->_flag &= ~_IOCOOKED;
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(xbeeout, NULL, _IONBF, 0);
    
    /* flush an pending input */
    fflush(xbeein);
    
    /* enter AT command mode */
    printf("Entering AT command mode\n");
    wait(1000);
    send(xbeeout, "+++");
    wait(1000);
    if (!expect(xbeein, "OK"))
        printf("failed to enter AT command mode\n");
    
    /* set SSID */
    printf("Set SSID\n");
    send(xbeeout, "ATID%s\r", SSID);
    if (!expect(xbeein, "OK"))
        printf("failed to set SSID\n");
    
    /* enable WPA2 encryption */
    printf("Set WPA2 encryption\n");
    send(xbeeout, "ATEE2\r");
    if (!expect(xbeein, "OK"))
        printf("failed to set WPA2 encryption\n");
    
    /* set security key */
    printf("Set security key\n");
    send(xbeeout, "ATPK%s\r", PASSWORD);
    if (!expect(xbeein, "OK"))
        printf("failed to set security key\n");
        
    /* activate settings */
    printf("Activate settings\n");
    send(xbeeout, "ATAC\r");
    if (!expect(xbeein, "OK"))
        printf("failed to activate settings\n");
        
    /* wait for an IP address to be assigned */
    printf("Waiting for an IP address\n");
    for (i = 0; i < MAX_RETRIES; ++i) {
        send(xbeeout, "ATAI\r");
        if (expect(xbeein, "0"))
            break;
        printf("  waiting...\n");
        wait(500);
    }
    
    /* set API mode */
    printf("Set API mode\n");
    send(xbeeout, "ATAP1\r");
    if (!expect(xbeein, "OK"))
        printf("failed to set API mode\n");

    /* leave command mode */
    printf("Leave command mode\n");
    send(xbeeout, "ATCN\r");
    if (!expect(xbeein, "OK"))
        printf("failed to leave command mode\n");

#if 0
    frame[0] = 0x08;
    frame[1] = 0x01;
    frame[2] = 'M';
    frame[3] = 'Y';
    SendFrame(xbeeout, frame, 4);
#endif

    printf("Closing Xbee input\n");
    fclose(xbeein);
    
    printf("Closing Xbee output\n");
    fclose(xbeeout);
    
    cog = XbeeFrame_start(&init, &mailbox, XBEE_RX, XBEE_TX, 0, XBEE_BAUD);
    printf("cog: %d\n", cog);
    
    printf("Getting our IP address\n");
    frame[0] = 0x08;
    frame[1] = 0x01;
    frame[2] = 'M';
    frame[3] = 'Y';
    XbeeFrame_sendframe(&mailbox, frame, 4);
    
    printf("Listen for packets\n");
    while (1) {
#if 0
        int size = RecvFrame(xbeein, frame, sizeof(frame));
        if (size < 0)
            printf("receive packet error\n");
        else {
            int i;
            for (i = 0; i < size; ++i)
                printf(" %02x", frame[i]);
            putchar('\n');
        }
#endif
        if (mailbox.rxstatus == XBEEFRAME_STATUS_BUSY) {
            int i;
            printf("frame");
            for (i = 0; i < mailbox.rxlength; ++i)
                printf(" %02x", mailbox.rxframe[i]);
            putchar('\n');
            mailbox.rxstatus = XBEEFRAME_STATUS_IDLE;
        }
    }
    
    printf("Enter interactive mode\n");
    
    /* loop copying characters from stdin to xbee and from xbee to stdout */
    while (1) {
        if ((ch = getchar()) != -1) {
            //printf("tch %02x\n", ch);
            putchar(ch);
            putc(ch, xbeeout);
        }
        if ((ch = getc(xbeein)) != -1) {
            //printf("rch %02x\n", ch);
            putchar(ch);
            if (ch == '\r')
                putchar('\n');
        }
    }
  
    return 0;
}

void send(FILE *fp, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(fp, fmt, ap);
    va_end(ap);
    fflush(fp);
}

void wait(int ms)
{
    waitcnt(ms * ticks_per_ms + CNT);
}

#define BUFSIZE 1024

int expect(FILE *fp, char *str)
{
    char buf[BUFSIZE + 1];
    int ch, i = 0;
    while (1) {
        while ((ch = getc(fp)) == EOF)
            ;
        if (i >= BUFSIZE) {
            printf("buffer overflow\n");
            return FALSE;
        }
        printf(" %02x", ch); fflush(stdout);
        if (ch == '\r')
            break;
        buf[i++] = ch;
    }
    putc('\n', stdout);
    buf[i] = '\0';
    printf("got: '%s'\n", buf);
    return strcmp(str, buf) == 0;
}

void SendFrame(FILE *fp, uint8_t *frame, int size)
{
    unsigned int chksum = 0;
    int i;
            
    /* send the start byte */
    putc(0x7e, fp);
    
    /* send the data length */
    putc((size >> 8) & 0xff, fp);
    putc(size & 0xff, fp);
    
    /* send the data */
    for (i = 0; i < size; ++i) {
        int ch = frame[i];
        putc(ch, fp);
        chksum += ch;
    }
    
    /* send the checksum */
    putc(0xff - (chksum & 0xff), fp);
}

int RecvFrame(FILE *fp, uint8_t *frame, int maxSize)
{
    unsigned int chksum = 0;
    unsigned int len, ch, i;
    
    /* look for the start byte */
    while ((ch = nextc(fp)) != 0x7e)
        ;
        
    /* get the data length */
    len = nextc(fp) << 8;
    len |= nextc(fp);
    
    /* make sure the data will fit in the buffer */
    if (len >= maxSize)
        return -1;
        
    /* get the data */
    for (i = 0; i < len; ++i) {
        int ch = nextc(fp);
        frame[i] = ch;
        chksum += ch;
    }
    
    /* check the checksum */
    if (((chksum + nextc(fp)) & 0xff) != 0xff)
        return -1;
        
    /* return the frame length */
    return len;
}

int nextc(FILE *fp)
{
    int ch;
    while ((ch = getc(fp)) == EOF)
        ;
    return ch;
}
