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

//#define DEBUG_AT

/* assumes WPA2 encryption */
#define SSID        "xlisper4"
#define PASSWD      "shaky2!raven"

/* Xbee pins */
#define XBEE_RX     13
#define XBEE_TX     12
#define XBEE_BAUD   9600

#define MAX_RETRIES 100000
#define BUFSIZE     1024
#define FRAMEHDRLEN 3

#define ID_IPV4TX   0x20
#define ID_IPV4RX   0xb0

typedef struct {
    uint8_t apiid;
    uint8_t frameid;
    uint8_t dstaddr[4];
    uint8_t dstport[2];
    uint8_t srcport[2];
    uint8_t protocol;
    uint8_t options;
    uint8_t data[1];
} IPV4TX_header;

typedef struct {
    uint8_t apiid;
    uint8_t srcaddr[4];
    uint8_t dstport[2];
    uint8_t srcport[2];
    uint8_t protocol;
    uint8_t status;
    uint8_t data[1];
} IPV4RX_header;

/* list of drivers we can use */
extern _Driver _FullDuplexSerialDriver;
_Driver *_driverlist[] = {
    &_FullDuplexSerialDriver,
    NULL
};

uint32_t ticks_per_ms;

void handle_ipv4_frame(XbeeFrame_t *mbox, uint8_t *frame, int length);
void show_frame(uint8_t *frame, int length);
int init_xbee(void);
void send(FdSerial_t *fds, char *str);
int expect(FdSerial_t *fds, char *str);
void wait(int ms);

int main(void)
{
    uint8_t frame[1024];
    XbeeFrameInit_t init;
    XbeeFrame_t mailbox;
    
    ticks_per_ms = CLKFREQ / 1000;
    
    if (!init_xbee()) {
        printf("Xbee initialization failed\n");
        return 1;
    }
    
    printf("Starting frame driver\n");
    if (XbeeFrame_start(&init, &mailbox, XBEE_RX, XBEE_TX, 0, XBEE_BAUD) < 0) {
        printf("failed to start frame driver\n");
        return 1;
    }
    
    printf("Getting our IP address\n");
    frame[0] = 0x08;
    frame[1] = 0x01;
    frame[2] = 'M';
    frame[3] = 'Y';
    XbeeFrame_sendframe(&mailbox, frame, 4);
    
    printf("Listen for packets\n");
    while (1) {
        uint8_t *frame;
        int length;
        if ((frame = XbeeFrame_recvframe(&mailbox, &length)) != NULL) {
        
            printf("[RX]");
            show_frame(frame, length);
            
            /* handle IPv4 packets received from the Xbee */
            if (frame[0] == ID_IPV4RX)
                handle_ipv4_frame(&mailbox, frame, length);
            XbeeFrame_release(&mailbox);
        }
    }
    
    return 0;
}

uint8_t *pkt_ptr;
int pkt_len;

int match(char *str)
{
    uint8_t *ptr = pkt_ptr;
    int len = pkt_len;
    while (*str) {
        if (--len < 0 || *ptr++ != *str++)
            return FALSE;
    }
    pkt_ptr = ptr;
    pkt_len = len;
    return TRUE;
}

void skip_spaces(void)
{
    while (pkt_len > 0 && *pkt_ptr == ' ') {
        --pkt_len;
        ++pkt_ptr;
    }
}

void send_response(XbeeFrame_t *mbox, IPV4RX_header *rxhdr, uint8_t *data, int length)
{
    uint8_t txframe[1024];
    IPV4TX_header *txhdr = (IPV4TX_header *)txframe;
    
    txhdr->apiid = ID_IPV4TX;
    txhdr->frameid = 0x42;
    memcpy(&txhdr->dstaddr, &rxhdr->srcaddr, 4);
    memcpy(&txhdr->dstport, &rxhdr->srcport, 2);
    memcpy(&txhdr->srcport, &rxhdr->dstport, 2);
    txhdr->protocol = rxhdr->protocol;
    txhdr->options = 0x00; // don't terminate after send
    memcpy(txhdr->data, data, length);
    length += sizeof(IPV4TX_header) - 1;

    printf("[TX]");
    show_frame(txframe, length);

    XbeeFrame_sendframe(mbox, txframe, length);
}

void show_frame(uint8_t *frame, int length)
{
    int i;
    for (i = 0; i < length; ++i)
        printf(" %02x", frame[i]);
    printf("\n     \"");
    for (i = 0; i < length; ++i) {
        if (frame[i] >= 0x20 && frame[i] <= 0x7e)
            putchar(frame[i]);
        else
            printf("<%02x>", frame[i]);
        if (frame[i] == '\n')
            putchar('\n');
    }
    printf("\"\n");
}

void handle_ipv4_frame(XbeeFrame_t *mbox, uint8_t *frame, int length)
{
    pkt_ptr = frame + (int)&((IPV4RX_header *)0)->data;
    pkt_len = length;
    
    if (match("XPOST")) {
        skip_spaces();
        if (match("/ld")) {
            printf("Got load request\n");
#define CANNED_RESPONSE "HTTP/1.1 200 OK\r\n\r\nGot ld request"
            send_response(mbox, (IPV4RX_header *)frame, (uint8_t *)CANNED_RESPONSE, sizeof(CANNED_RESPONSE) - 1);
        }
        else if (match("/tx")) {
            printf("Got tx request\n");
        }
        else if (match("/rx")) {
            printf("Got rx request\n");
        }
        else {
            // bad request
        }
    }
    else {
        // bad request
    }
}

char ssid_str[32];
char passwd_str[32];

struct {
    char *cmd;
    char *response;
    int retries;
    char *info;
} cmds[] = {
{   ssid_str,       "OK",   1,              "Set SSID"              },
{   passwd_str,     "OK",   1,              "Set password"          },
{   "ATEE2\r",      "OK",   1,              "Set WPA2 encryption"   },
{   "ATAC\r",       "OK",   1,              "Activate settings"     },
{   "ATAI\r",       "0",    MAX_RETRIES,    "Connect to AP"         },
// this doesn't work for exiting Soft AP mode
//{   "ATCE0\r",      "OK",   1,              "Disable Soft AP mode"  },
{   "ATC01F90\r",   "OK",   1,              "Set port to 8080"      },
{   "ATIP1\r",      "OK",   1,              "Set TCP mode"          },
{   "ATAP1\r",      "OK",   1,              "Set API mode"          },
{   "ATCN\r",       "OK",   1,              "Exit command mode"     },
{   0,              0,      0,              0,                      }
};

int init_xbee(void)
{
    int result = TRUE;
    FdSerial_t xbee;
    int i;

    /* open the xbee */
    if (FdSerial_start(&xbee, XBEE_RX, XBEE_TX, 0, XBEE_BAUD) < 0) {
        printf("failed to open xbee\n");
        return FALSE;
    }
    
    /* enter AT command mode */
    printf("Entering AT command mode\n");
    wait(1000);
    send(&xbee, "+++");
    wait(1000);
    if (!expect(&xbee, "OK")) {
        printf("failed to enter AT command mode\n");
        return FALSE;
    }
    
    /* setup the SSID and password strings */
    sprintf(ssid_str, "ATID%s\r", SSID);
    sprintf(passwd_str, "ATPK%s\r", PASSWD);

    /* initialize */
    for (i = 0; cmds[i].cmd != NULL; ++i) {
        int remaining;
        
        /* execute a command */
        printf("%s\n", cmds[i].info);
        remaining = cmds[i].retries;
        while (--remaining >= 0) {
        
            /* send the command */
            send(&xbee, cmds[i].cmd);
        
            /* receive the response */
            if (expect(&xbee, cmds[i].response))
                break;
            
            /* wait if another retry is required */
            printf("  waiting...\n");
            wait(500);
        }
        
        /* fail if the retry count expired */
        if (remaining < 0) {
            printf("  failed\n");
            result = FALSE;
            goto done;
        }
    }
        
done:
    printf("Closing Xbee\n");
    FdSerial_stop(&xbee);
    return result;
}

void send(FdSerial_t *fds, char *str)
{
    while (*str != '\0')
        FdSerial_tx(fds, *str++);
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
#ifdef DEBUG_AT
        printf(" %02x", ch); fflush(stdout);
#endif
        if (ch == '\r')
            break;
        buf[i++] = ch;
    }
    buf[i] = '\0';
#ifdef DEBUG_AT
    printf("\n got '%s'\n", buf);
#endif
    return strcmp(str, buf) == 0;
}

void wait(int ms)
{
    waitcnt(ms * ticks_per_ms + CNT);
}
