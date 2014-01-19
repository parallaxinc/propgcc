#include <stdio.h>
#include <stdint.h>
#include <propeller.h>
#include "fds.h"

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

#define DEBUG_AT
//#define SOFT_AP

/* assumes WPA2 encryption */
#ifdef SOFT_AP
#define SSID        "activityboard"
#else
#define SSID        "xlisper4"
#define PASSWD      "shaky2!raven"
#endif

/* Xbee pins */
#define XBEE_RX     13
#define XBEE_TX     12
#define XBEE_BAUD   9600

#define MAX_RETRIES 100000
#define BUFSIZE     1024

char ssid_str[32];
#ifndef SOFT_AP
char passwd_str[32];
#endif

struct {
    char *cmd;
    char *response;
    int retries;
    char *info;
} cmds[] = {
{   ssid_str,       "OK",   1,              "Set SSID"              },
#ifdef SOFT_AP
{   "ATCE1\r",      "OK",   1,              "Enable Soft AP mode"   },
{   "ATEE0\r",      "OK",   1,              "Disable encryption"    },
#else
{   "ATCE2\r",      "OK",   1,              "Disable Soft AP mode"  },
{   passwd_str,     "OK",   1,              "Set password"          },
{   "ATEE2\r",      "OK",   1,              "Set WPA2 encryption"   },
#endif
{   "ATDO0\r",      "OK",   1,              "Disable device cloud"  },
{   "ATC050\r",     "OK",   1,              "Set port to 80"        },
{   "ATIP1\r",      "OK",   1,              "Set TCP mode"          },
{   "ATAC\r",       "OK",   1,              "Activate settings"     },
{   "ATAI\r",       "0",    MAX_RETRIES,    "Connect to AP"         },
{   "ATAP1\r",      "OK",   1,              "Set API mode"          },
{   "ATWR\r",       "OK",   1,              "Write settings"        },
{   "ATCN\r",       "OK",   1,              "Exit command mode"     },
{   0,              0,      0,              0,                      }
};

uint32_t ticks_per_ms;

void send(FdSerial_t *fds, char *str);
int expect(FdSerial_t *fds, char *str);
void wait(int ms);

int main(void)
{
    int result = 1; // the glass is half empty
    FdSerial_t xbee;
    int i;

    /* get the number of clock ticks per millisecond */
    ticks_per_ms = CLKFREQ / 1000;
    
    /* open the xbee */
    if (FdSerial_start(&xbee, XBEE_RX, XBEE_TX, 0, XBEE_BAUD) < 0) {
        printf("failed to open xbee\n");
        return 1;
    }
    
    /* enter AT command mode */
    printf("Entering AT command mode\n");
    wait(1000);
    send(&xbee, "+++");
    wait(1000);
    if (!expect(&xbee, "OK")) {
        printf("failed to enter AT command mode\n");
        goto done;
    }
    
    /* setup the SSID and password strings */
    sprintf(ssid_str, "ATID%s\r", SSID);
#ifndef SOFT_AP
    sprintf(passwd_str, "ATPK%s\r", PASSWD);
#endif

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
            goto done;
        }
    }
    
    /* succeeded! */
    printf("Configuration successful!\n");
    result = TRUE;
        
done:
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
