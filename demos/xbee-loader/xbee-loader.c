#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <propeller.h>
#include "xbeeframe.h"
#include "fds.h"

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
#define BUFSIZE     1024

/* list of drivers we can use */
extern _Driver _FullDuplexSerialDriver;
_Driver *_driverlist[] = {
    &_FullDuplexSerialDriver,
    NULL
};

uint32_t ticks_per_ms;

void send(FdSerial_t *fds, char *fmt, ...);
int expect(FdSerial_t *fds, char *str);
void wait(int ms);

int main(void)
{
    uint8_t frame[1024];
    FdSerial_t xbee;
    XbeeFrameInit_t init;
    XbeeFrame_t mailbox;
    int cog, i;
    
    ticks_per_ms = CLKFREQ / 1000;
    
    /* set non-blocking mode on stdin and xbee */
    stdin->_flag |= _IONONBLOCK;
    stdin->_flag &= ~_IOCOOKED;
    setvbuf(stdout, NULL, _IONBF, 0);
    
    /* open the xbee */
    if (FdSerial_start(&xbee, 13, 12, 0, 9600) < 0)
        printf("failed to open xbee\n");
    
    /* enter AT command mode */
    printf("Entering AT command mode\n");
    wait(1000);
    send(&xbee, "+++");
    wait(1000);
    if (!expect(&xbee, "OK"))
        printf("failed to enter AT command mode\n");
    
    /* set SSID */
    printf("Set SSID\n");
    send(&xbee, "ATID%s\r", SSID);
    if (!expect(&xbee, "OK"))
        printf("failed to set SSID\n");
    
    /* enable WPA2 encryption */
    printf("Set WPA2 encryption\n");
    send(&xbee, "ATEE2\r");
    if (!expect(&xbee, "OK"))
        printf("failed to set WPA2 encryption\n");
    
    /* set security key */
    printf("Set security key\n");
    send(&xbee, "ATPK%s\r", PASSWORD);
    if (!expect(&xbee, "OK"))
        printf("failed to set security key\n");
        
    /* activate settings */
    printf("Activate settings\n");
    send(&xbee, "ATAC\r");
    if (!expect(&xbee, "OK"))
        printf("failed to activate settings\n");
        
    /* wait for an IP address to be assigned */
    printf("Waiting for an IP address\n");
    for (i = 0; i < MAX_RETRIES; ++i) {
        send(&xbee, "ATAI\r");
        if (expect(&xbee, "0"))
            break;
        printf("  waiting...\n");
        wait(500);
    }
    
    /* set API mode */
    printf("Set API mode\n");
    send(&xbee, "ATAP1\r");
    if (!expect(&xbee, "OK"))
        printf("failed to set API mode\n");

    /* leave command mode */
    printf("Leave command mode\n");
    send(&xbee, "ATCN\r");
    if (!expect(&xbee, "OK"))
        printf("failed to leave command mode\n");

    printf("Closing Xbee\n");
    FdSerial_stop(&xbee);
    
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
        uint8_t *frame;
        int length, i;
        if ((frame = XbeeFrame_recvframe(&mailbox, &length)) != NULL) {
            printf("frame");
            for (i = 0; i < length; ++i)
                printf(" %02x", frame[i]);
            putchar('\n');
            XbeeFrame_release(&mailbox);
        }
    }
    
    return 0;
}

void send(FdSerial_t *fds, char *fmt, ...)
{
    char buf[1024], *p;
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    for (p = buf; *p != '\0'; ++p)
        FdSerial_tx(fds, *p);
}

int expect(FdSerial_t *fds, char *str)
{
    char buf[BUFSIZE + 1];
    int ch, i = 0;
    while (1) {
        while ((ch = FdSerial_rx(fds)) == -1)
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

void wait(int ms)
{
    waitcnt(ms * ticks_per_ms + CNT);
}
