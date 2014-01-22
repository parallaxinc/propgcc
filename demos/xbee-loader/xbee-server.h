#ifndef __XBEE_SERVER_H__
#define __XBEE_SERVER_H__

#include <stdint.h>

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

enum {
    SF_BUSY = 0x00000001
};

enum {
    SS_REQUEST,
    SS_HEADER,
    SS_CONTENT,
    SS_SKIP_NL,
    SS_CONTENT_NL
};

#define MAX_CONTENT 1024

typedef struct Socket_t Socket_t;
struct Socket_t {
    int flags;
    int state;
    struct {    // must be in the same order as in IPV4RX_header_t
        uint8_t srcaddr[4];
        uint8_t dstport[2];
        uint8_t srcport[2];
    } id;
    uint8_t protocol;
    void (*handler)(Socket_t *sock, int phase);
    uint8_t content[MAX_CONTENT + 1];
    int length;
    int i;
};

enum {
    HP_REQUEST,
    HP_HEADER,
    HP_CONTENT
};

typedef struct {
    char *method;
    void (*handler)(Socket_t *sock, int phase);
} MethodBinding_t;

void send_response(Socket_t *sock, uint8_t *data, int length);

#endif
