#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <propeller.h>
#include "xbee-server.h"

#define OPTION_RESPONSE "\
HTTP/1.1 200 OK\r\n\
Access-Control-Allow-Origin: *\r\n\
Access-Control-Allow-Methods: GET, POST, OPTIONS, XPING, XABS\r\n\
Access-Control-Allow-Headers: XPING, XABS\r\n\
Access-Control-Max-Age: 1000000\r\n\
Keep-Alive: timeout=1, max=100\r\n\
Connection: Keep-Alive\r\n\
Content-Type: text/plain\r\n\
Content-Length: 0\r\n\
\r\n"

#define CANNED_RESPONSE "\
HTTP/1.1 200 OK\r\n\
Access-Control-Allow-Origin: *\r\n\
Content-Length: 11\r\n\
\r\n\
Got request"

#define XPING_RESPONSE "\
HTTP/1.1 200 OK\r\n\
Access-Control-Allow-Origin: *\r\n\
Content-Length: 6\r\n\
\r\n\
Hello!"

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

void  skip_white(void)
{
    while (pkt_len > 0 && isspace(*pkt_ptr)) {
        --pkt_len;
        ++pkt_ptr;
    }
}

int token(char *str)
{
    int n = 0;
    char *s = (char*)pkt_ptr;
    skip_white();
    s = (char*)pkt_ptr;
    while(pkt_len > 0 && !isspace(*s)) {
        str[n] = *(s++);
        n++;
    }
    str[n] = 0;
    pkt_len -= n;
    pkt_ptr = (uint8_t*)s;
    return n;
}

static void handle_options_request(Socket_t *sock, int phase)
{
    if (phase == HP_CONTENT)
        send_response(sock, (uint8_t *)OPTION_RESPONSE, sizeof(OPTION_RESPONSE) - 1);
}

static void handle_xabs_request(Socket_t *sock, int phase)
{
    printf("Got XABS request: %d\n", phase);
    if (phase == HP_CONTENT) {
        int  num = 0;
        char str[80];
    
        pkt_ptr = sock->content;
        pkt_len = sock->length;
        
        skip_white();

        //
        // COMMAND: XABS PIN # [HIGH|LOW|TOGGLE|INPUT]
        //
        if (match("PIN"))
        { // got pin
            skip_spaces();
            if(token(str)) {
                num = atoi(str);
            }
            skip_spaces();
            if (match("HIGH")) {
                printf("PIN %d HIGH\n", num);
                DIRA |= 1 << num;
                OUTA |= 1 << num;
            }
            else if (match("LOW")) {
                printf("PIN %d LOW\n", num);
                DIRA |= 1 << num;
                OUTA &= ~(1 << num);
            }
            else if (match("TOGGLE")) {
                printf("PIN %d TOGGLE\n", num);
                DIRA |= 1 << num;
                OUTA ^= 1 << num;
            }
            else if (match("INPUT")) {
                printf("PIN %d INPUT\n", num);
                DIRA &= ~(1 << num);
            }
        }
        send_response(sock, (uint8_t *)CANNED_RESPONSE, sizeof(CANNED_RESPONSE) - 1);
    }
}

static void handle_xping_request(Socket_t *sock, int phase)
{
    printf("Got XPING request: %d\n", phase);
    if (phase == HP_CONTENT)
        send_response(sock, (uint8_t *)XPING_RESPONSE, sizeof(XPING_RESPONSE) - 1);
}

MethodBinding_t methodBindings[] = {
{   "OPTIONS",  handle_options_request  },
{   "XABS",     handle_xabs_request     },
{   "XPING",    handle_xping_request    },
{   NULL,       NULL                    }
};
