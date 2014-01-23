#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <propeller.h>
#include "xbee-server.h"
#include "xbeeframe.h"
#include "xbeeload.h"

#define MULTI_SOCKETS

//#define FRAME_DEBUG
//#define STATE_MACHINE_DEBUG
//#define SOCKET_DEBUG
//#define STATUS_DEBUG

#define BAD_REQUEST_RESPONSE "\
HTTP/1.1 400 Bad Request\r\n\
\r\n"

/* Xbee pins */
#define XBEE_RX     13
#define XBEE_TX     12
#define XBEE_BAUD   9600

#define ID_IPV4TX       0x20
#define ID_ATRESPONSE   0x88
#define ID_TXSTATUS     0x89
#define ID_IPV4RX       0xb0

#ifdef MULTI_SOCKETS
#define MAX_SOCKETS 4
#else
#define MAX_SOCKETS 1
#endif

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

typedef struct {
    uint8_t apiid;
    uint8_t frameid;
    uint8_t status;
} TXStatus_t;

typedef struct {
    uint8_t apiid;
    uint8_t frameid;
    uint8_t command[2];
    uint8_t status;
    uint8_t value[1];
} ATResponse_t;

#define HUBSIZE         (32 * 1024)
#define RESPONSESIZE    1024

extern MethodBinding_t methodBindings[];

/* place these data structures at the top of memory so the loader doesn't overwrite them */
XbeeFrame_t *mailbox = (XbeeFrame_t *)(HUBSIZE - sizeof(XbeeFrame_t));
uint8_t *response = (uint8_t *)HUBSIZE - sizeof(XbeeFrame_t) - RESPONSESIZE;

Socket_t sockets[MAX_SOCKETS];
uint8_t txframeid = 0x00;

/* prototypes */
static void handle_ipv4_frame(IPV4RX_header_t *frame, int length);
static void handle_txstatus_frame(TXStatus_t *frame, int length);
static void handle_atresponse_frame(ATResponse_t *frame, int length);
static void parse_request(Socket_t *sock);
static void parse_header(Socket_t *sock);
static void handle_content(Socket_t *sock);
static int prepare_response(Socket_t *sock, uint8_t *frame, uint8_t *data, int length);
static char *match(Socket_t *sock, char *str);
static char *skip_spaces(char *str);
#ifdef FRAME_DEBUG
static void show_frame(uint8_t *frame, int length);
#endif

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
    
    /* get our IP address */
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
        
#ifdef FRAME_DEBUG
            printf("\n\n[RX]");
            show_frame(frame, length);
#endif
            
            /* handle the frame */
            switch (frame[0]) {
            case ID_IPV4RX:
                handle_ipv4_frame((IPV4RX_header_t *)frame, length);
                break;
            case ID_ATRESPONSE:
                handle_atresponse_frame((ATResponse_t *)frame, length);
                break;
            case ID_TXSTATUS:
                handle_txstatus_frame((TXStatus_t *)frame, length);
                break;
            default:
                break;
            }
                
            XbeeFrame_release(mailbox);
        }
    }
    
    return 0;
}

static void handle_ipv4_frame(IPV4RX_header_t *frame, int length)
{
    Socket_t *sock = sockets;
#ifdef MULTI_SOCKETS
    Socket_t *free = NULL;
#endif
    int len, cnt, i;
    uint8_t *ptr;
    
#ifdef SOCKET_DEBUG
    printf("ip %d.%d.%d.%d, dst %d, src %d, protocol %d\n", 
            frame->srcaddr[0],
            frame->srcaddr[1],
            frame->srcaddr[2],
            frame->srcaddr[3],
            (frame->dstport[0] << 8) | frame->dstport[1],
            (frame->srcport[0] << 8) | frame->srcport[1],
            frame->protocol);
#endif
    
    /* find a socket for this connection */
    for (i = 0; i < MAX_SOCKETS; ++i) {
        if (sock->flags & SF_BUSY) {
            if (memcmp(&sock->id, &frame->srcaddr, sizeof(sock->id)) == 0)
                break;
        }
#ifdef MULTI_SOCKETS
        else if (!free)
            free = sock;
#endif
        ++sock;
    }
    
    /* check for needing to open a new socket */
    if (i >= MAX_SOCKETS) {
#ifdef MULTI_SOCKETS
        if (!(sock = free)) {
            printf("No free sockets\n");
            return; // no sockets available, ignore frame
        }
#else
        sock = &sockets[0];
#endif
        sock->flags |= SF_BUSY;
        memcpy(&sock->id, frame->srcaddr, sizeof(sock->id));
        sock->protocol = frame->protocol;
        sock->state = SS_REQUEST;
        sock->handler = NULL;
        sock->length = 0;
        sock->i = 0;
    }
#ifdef SOCKET_DEBUG
    printf("Using socket %d\n", sock - sockets);
#endif
    
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
                if (sock->i == 0) {
                    if (sock->state == SS_REQUEST) {
                        printf("Missing request line\n");
                    }
                    else {
#ifdef STATE_MACHINE_DEBUG
                        printf("Found content\n");
#endif
                        sock->state = (term == '\r' ? SS_CONTENT_NL : SS_CONTENT);
                    }
                }
                else {
#ifdef STATE_MACHINE_DEBUG
                    printf("Found %s: %s\n", sock->state == SS_REQUEST ? "request" : "header", sock->content);
#endif
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
            cnt = sock->length - sock->i;
#ifdef STATE_MACHINE_DEBUG
            printf("length %d, remaining: %d\n", sock->length, cnt);
#endif
            if (len < cnt)
                cnt = len;
            memcpy(&sock->content[sock->i], ptr, cnt);
            sock->i += len;
            ptr += cnt;
            len -= cnt;
            if (len > 0 && sock->i >= sock->length)
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
    printf("\nRequest: %s\n", sock->content);
    for (binding = methodBindings; binding->method != NULL; ++binding) {
        if (match(sock, binding->method)) {
#ifdef REQUEST_DEBUG
            printf(" -- found handler for %s\n", binding->method);
#endif
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
#ifdef REQUEST_DEBUG
        printf("Length: %d\n", sock->length);
#endif
    }
    if (sock->handler)
        (*sock->handler)(sock, HP_HEADER);
}

static void handle_content(Socket_t *sock)
{
#ifdef STATE_MACHINE_DEBUG
    printf("Collected content:\n");
#endif
#ifdef FRAME_DEBUG
    show_frame(sock->content, sock->length);
#endif
    if (sock->handler)
        (*sock->handler)(sock, HP_CONTENT);
    else
        send_response(sock, (uint8_t *)BAD_REQUEST_RESPONSE, sizeof(BAD_REQUEST_RESPONSE) - 1);
    sock->state = SS_REQUEST;
    sock->handler = NULL;
    sock->length = 0;
    sock->i = 0;
}

static void handle_atresponse_frame(ATResponse_t *frame, int length)
{
    if (frame->status == 0x00 && strncmp((char *)frame->command, "MY", 2) == 0)
        printf("IP Address: %d.%d.%d.%d\n", frame->value[0], frame->value[1], frame->value[2], frame->value[3]);
}

static void handle_txstatus_frame(TXStatus_t *frame, int length)
{
#ifndef STATUS_DEBUG
    if (frame->status != 0x00)
#endif
        printf("TX Status: Frame %02x, ", frame->frameid);
    switch (frame->status) {
#ifdef STATUS_DEBUG
    case 0x00:  printf("Success\n"); break;
#else
    case 0x00:  break;
#endif
    case 0x03:  printf("Transmission purged\n"); break;
    case 0x04:  printf("Physical error\n"); break;
    case 0x21:  printf("TX64 transmission timed out\n"); break;
    case 0x32:  printf("Resource error\n"); break;
    case 0x74:  printf("Message too long\n"); break;
    case 0x76:  printf("Attempt to create client socket failed\n"); break;
    case 0x77:  printf("TCP connection does not exist\n"); break;
    case 0x78:  printf("Source port on UDP transmission doesn't match listening port\n"); break;
    default:    printf("Unknown status %02x\n", frame->status); break;
    }
}

void send_response(Socket_t *sock, uint8_t *data, int length)
{
    uint8_t frame[1024];
    length = prepare_response(sock, frame, data, length);
    XbeeFrame_sendframe(mailbox, frame, length);
    sock->flags &= ~SF_BUSY;
}

static int prepare_response(Socket_t *sock, uint8_t *frame, uint8_t *data, int length)
{
    IPV4TX_header_t *txhdr = (IPV4TX_header_t *)frame;
    char *ptr = (char *)data;
    int len = length;
    
    printf("Response: ");
    while (--len >= 0 && *ptr != '\r')
        putchar(*ptr++);
    putchar('\n');

    txhdr->apiid = ID_IPV4TX;
    txhdr->frameid = ++txframeid;
    memcpy(&txhdr->dstaddr, &sock->id.srcaddr, 4);
    memcpy(&txhdr->dstport, &sock->id.srcport, 2);
    memcpy(&txhdr->srcport, &sock->id.dstport, 2);
    txhdr->protocol = sock->protocol;
    //txhdr->options = 0x00; // don't terminate after send
    txhdr->options = 0x01; // terminate after send
    memcpy(txhdr->data, data, length);
    length += (int)&((IPV4TX_header_t *)0)->data;

#ifdef FRAME_DEBUG
    printf("[TX %02x]", txframeid);
    show_frame(frame, length);
#endif
    
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

#ifdef FRAME_DEBUG
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
#endif
