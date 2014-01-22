#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <propeller.h>
#include "xbee-server.h"
#include "xbeeframe.h"
#include "xbeeload.h"

#define BAD_REQUEST_RESPONSE "\
HTTP/1.1 400 Bad Request\r\n\
\r\n"

/* Xbee pins */
#define XBEE_RX     13
#define XBEE_TX     12
#define XBEE_BAUD   9600

#define ID_IPV4TX   0x20
#define ID_IPV4RX   0xb0

#define MAX_SOCKETS 4

typedef struct {
    uint8_t apiid;
    uint8_t frameid;
    uint8_t dstaddr[4];
    uint8_t dstport[2];
    uint8_t srcport[2];
    uint8_t protocol;
    uint8_t options;
    uint8_t data[1];
} IPV4TX_header_t;

typedef struct {
    uint8_t apiid;
    uint8_t srcaddr[4];
    uint8_t dstport[2];
    uint8_t srcport[2];
    uint8_t protocol;
    uint8_t status;
    uint8_t data[1];
} IPV4RX_header_t;

#define HUBSIZE         (32 * 1024)
#define RESPONSESIZE    1024

extern MethodBinding_t methodBindings[];

/* place these data structures at the top of memory so the loader doesn't overwrite them */
XbeeFrame_t *mailbox = (XbeeFrame_t *)(HUBSIZE - sizeof(XbeeFrame_t));
uint8_t *response = (uint8_t *)HUBSIZE - sizeof(XbeeFrame_t) - RESPONSESIZE;

Socket_t sockets[MAX_SOCKETS];

/* prototypes */
static void handle_ipv4_frame(IPV4RX_header_t *frame, int length);
static void parse_request(Socket_t *sock);
static void parse_header(Socket_t *sock);
static void handle_content(Socket_t *sock);
static int prepare_response(Socket_t *sock, uint8_t *frame, uint8_t *data, int length);
static char *match(Socket_t *sock, char *str);
static char *skip_spaces(char *str);
static void show_frame(uint8_t *frame, int length);

/* main - the main routine */
int main(void)
{
    XbeeFrameInit_t init;
    uint8_t frame[1024];
    
    /* initialize the sockets */
    memset(sockets, 0, sizeof(sockets));
    
    printf("Starting frame driver\n");
    if (XbeeFrame_start(&init, mailbox, XBEE_RX, XBEE_TX, 0, XBEE_BAUD) < 0) {
        printf("failed to start frame driver\n");
        return 1;
    }
    
    printf("Getting our IP address\n");
    frame[0] = 0x08;
    frame[1] = 0x01;
    frame[2] = 'M';
    frame[3] = 'Y';
    XbeeFrame_sendframe(mailbox, frame, 4);
    
    printf("Listening for frames\n");
    while (1) {
        uint8_t *frame;
        int length;
        if ((frame = XbeeFrame_recvframe(mailbox, &length)) != NULL) {
        
            printf("[RX]");
            show_frame(frame, length);
            
            /* handle IPv4 packets received from the Xbee */
            if (frame[0] == ID_IPV4RX)
                handle_ipv4_frame((IPV4RX_header_t *)frame, length);
                
            XbeeFrame_release(mailbox);
        }
    }
    
    return 0;
}

static void handle_ipv4_frame(IPV4RX_header_t *frame, int length)
{
    Socket_t *sock = sockets;
    //Socket_t *free = NULL;
    int remaining, len, i;
    uint8_t *ptr;
    
    printf("ip %d.%d.%d.%d, dst %d, src %d\n", 
            frame->srcaddr[0],
            frame->srcaddr[1],
            frame->srcaddr[2],
            frame->srcaddr[3],
            (frame->dstport[0] << 8) | frame->dstport[1],
            (frame->srcport[0] << 8) | frame->srcport[1]);
    
    /* find a socket for this connection */
    for (i = 0; i < MAX_SOCKETS; ++i) {
        if (sock->flags & SF_BUSY) {
            if (memcmp(&sock->id, &frame->srcaddr, sizeof(sock->id)) == 0)
                break;
        }
        //else if (!free)
        //    free = sock;
        ++sock;
    }
    
    /* check for needing to open a new socket */
    if (i >= MAX_SOCKETS) {
        //if (!(sock = free)) {
        //    printf("No free sockets\n");
        //    return; // no sockets available, ignore frame
        //}
        sock = &sockets[0];
        sock->flags |= SF_BUSY;
        memcpy(&sock->id, frame->srcaddr, sizeof(sock->id));
        sock->protocol = frame->protocol;
        sock->state = SS_REQUEST;
        sock->handler = NULL;
        sock->length = 0;
        sock->i = 0;
    }
    printf("Using socket %d\n", sock - sockets);
    
    /* setup the frame parsing variables */
    ptr = frame->data;
    len = length - (int)&((IPV4RX_header_t *)0)->data;
    
    /* process the data in this frame */
    while (len > 0) {
        switch (sock->state) {
        case SS_REQUEST:
        case SS_HEADER:
            while (len > 0 && *ptr != '\r' && *ptr != '\n') {
                if (sock->i < MAX_CONTENT)
                    sock->content[sock->i++] = *ptr;
                ++ptr;
                --len;
            }
            if (len > 0) {
                int term = *ptr;
                sock->content[sock->i] = '\0';
                ++ptr;
                --len;
                if (sock->content[0] == '\0') {
                    printf("Found content\n");
                    sock->state = (term == '\r' ? SS_CONTENT_NL : SS_CONTENT);
                }
                else {
                    printf("Found %s: %s\n", sock->state == SS_REQUEST ? "request" : "header", sock->content);
                    switch (sock->state) {
                    case SS_REQUEST:
                        parse_request(sock);
                        break;
                    case SS_HEADER:
                        parse_header(sock);
                        break;
                    }
                    sock->state = (term == '\r' ? SS_SKIP_NL : SS_HEADER);
                }
                sock->i = 0;
            }
            break;
        case SS_SKIP_NL:
        case SS_CONTENT_NL:
            sock->state = (sock->state == SS_SKIP_NL ? SS_HEADER : SS_CONTENT);
            if (*ptr == '\n') {
                ++ptr;
                --len;
            }
            break;
        case SS_CONTENT:
            remaining = sock->length - sock->i;
            printf("length %d, remaining: %d\n", sock->length, remaining);
            if (len > remaining)
                len = remaining;
            memcpy(&sock->content[sock->i], ptr, len);
            sock->i += len;
            if (sock->i >= sock->length)
                handle_content(sock);
            break;
        }
    }
    
    if (sock->i >= sock->length)
        handle_content(sock);
}

static void parse_request(Socket_t *sock)
{
    MethodBinding_t *binding = NULL;
    printf("Request: %s\n", sock->content);
    for (binding = methodBindings; binding->method != NULL; ++binding) {
        if (match(sock, binding->method)) {
            printf(" -- found handler for %s\n", binding->method);
            sock->handler = binding->handler;
            (*sock->handler)(sock, HP_REQUEST);
            break;
        }
    }
}

static void parse_header(Socket_t *sock)
{
    char *p;
    if ((p = match(sock, "CONTENT-LENGTH:")) != NULL) {
        sock->length = atoi(skip_spaces(p));
        printf("Length: %d\n", sock->length);
    }
    if (sock->handler)
        (*sock->handler)(sock, HP_HEADER);
}

static void handle_content(Socket_t *sock)
{
    printf("Collected content:\n");
    show_frame(sock->content, sock->length);
    if (sock->handler)
        (*sock->handler)(sock, HP_CONTENT);
    else
        send_response(sock, (uint8_t *)BAD_REQUEST_RESPONSE, sizeof(BAD_REQUEST_RESPONSE) - 1);
    sock->state = SS_REQUEST;
    sock->handler = NULL;
    sock->length = 0;
    sock->i = 0;
}

void send_response(Socket_t *sock, uint8_t *data, int length)
{
    uint8_t frame[1024];
    length = prepare_response(sock, frame, data, length);
    XbeeFrame_sendframe(mailbox, frame, length);
}

static int prepare_response(Socket_t *sock, uint8_t *frame, uint8_t *data, int length)
{
    IPV4TX_header_t *txhdr = (IPV4TX_header_t *)frame;

    txhdr->apiid = ID_IPV4TX;
    txhdr->frameid = 0x42;
    memcpy(&txhdr->dstaddr, &sock->id.srcaddr, 4);
    memcpy(&txhdr->dstport, &sock->id.srcport, 2);
    memcpy(&txhdr->srcport, &sock->id.dstport, 2);
    txhdr->protocol = sock->protocol;
    txhdr->options = 0x00; // don't terminate after send
    //txhdr->options = 0x01; // terminate after send
    memcpy(txhdr->data, data, length);
    length += (int)&((IPV4TX_header_t *)0)->data;

    printf("[TX]");
    show_frame(frame, length);
    
    return length;
}

static char *match(Socket_t *sock, char *str)
{
    char *p = (char *)sock->content;
    while (*str != '\0' && *p != '\0') {
        if (*str != toupper(*p))
            return NULL;
        ++str;
        ++p;
    }
    return p;
}

static char *skip_spaces(char *str)
{
    while (*str != '\0' && isspace(*str))
        ++str;
    return str;
}

static void show_frame(uint8_t *frame, int length)
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
