#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "xbee-server.h"
#include "xbeeframe.h"
#include "xbeeload.h"

#define OPTIONS_RESPONSE "\
HTTP/1.1 200 OK\r\n\
Access-Control-Allow-Origin: *\r\n\
Access-Control-Allow-Methods: GET, POST, OPTIONS, XPOST, XLOAD\r\n\
Access-Control-Allow-Headers: XPOST, XLOAD\r\n\
Access-Control-Max-Age: 1000000\r\n\
Keep-Alive: timeout=1, max=100\r\n\
Connection: Keep-Alive\r\n\
Content-Type: text/plain\r\n\
Content-Length: 0\r\n\
\r\n"

#define LD_RESPONSE "\
HTTP/1.1 200 OK\r\n\
Access-Control-Allow-Origin: *\r\n\
Content-Length: 20\r\n\
\r\n\
ld request succeeded"

#define TX_RESPONSE "\
HTTP/1.1 200 OK\r\n\
Access-Control-Allow-Origin: *\r\n\
Content-Length: 14\r\n\
\r\n\
Got tx request"

#define RX_RESPONSE "\
HTTP/1.1 200 OK\r\n\
Access-Control-Allow-Origin: *\r\n\
Content-Length: 14\r\n\
\r\n\
Got rx request"

static void handle_options_request(Socket_t *sock, int phase)
{
    if (phase == HP_CONTENT)
        send_response(sock, (uint8_t *)OPTIONS_RESPONSE, sizeof(OPTIONS_RESPONSE) - 1);
}

static void handle_xpost_ld_request(Socket_t *sock, int phase)
{
    if (phase == HP_CONTENT_START) {
        XbeeLoadInit_t init;
        init.mailbox = mailbox;
        init.ldbuf = sock->frame_ptr;
        init.ldcount = sock->frame_len;
        init.ldaddr = 0;
        init.ldtotal = sock->length;
        init.response = response;
        init.rcount = prepare_response(sock, response, (uint8_t *)LD_RESPONSE, sizeof(LD_RESPONSE) - 1);
        XbeeLoad_start(&init);
        printf("Should never get here\n");
    }
}

static void handle_xpost_tx_request(Socket_t *sock, int phase)
{
    if (phase == HP_CONTENT)
        send_response(sock, (uint8_t *)TX_RESPONSE, sizeof(TX_RESPONSE) - 1);
}

static void handle_xpost_rx_request(Socket_t *sock, int phase)
{
    if (phase == HP_CONTENT)
        send_response(sock, (uint8_t *)RX_RESPONSE, sizeof(RX_RESPONSE) - 1);
}

static void handle_xpost_request(Socket_t *sock, int phase)
{
    char *uri = (char *)sock->content;
    char *p;
    
    /* skip over the method */
    while (*uri && !isspace(*uri))
        ++uri;
    
    /* skip space between the method name and the uri */
    while (*uri && isspace(*uri))
        ++uri;
    
    /* find the end of the uri */
    for (p = uri; *p && !isspace(*p); ++p)
        ;
    *p = '\0';
    
    if (strcasecmp(uri, "/ld") == 0)
        sock->handler = handle_xpost_ld_request;
    else if (strcasecmp(uri, "/tx") == 0)
        sock->handler = handle_xpost_tx_request;
    else if (strcasecmp(uri, "/rx") == 0)
        sock->handler = handle_xpost_rx_request;
    else
        sock->handler = NULL;
}

MethodBinding_t methodBindings[] = {
{   "OPTIONS",  handle_options_request  },
{   "XPOST",    handle_xpost_request    },
{   NULL,       NULL                    }
};
