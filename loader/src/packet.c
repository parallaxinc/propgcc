/* packet.c - an elf and spin binary loader for the Parallax Propeller microcontroller

Copyright (c) 2011 David Michael Betz

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <stdio.h>
#include <string.h>
#include "packet.h"
#include "osint.h"

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

/* timeouts for waiting for ACK/NAK */
#define INITIAL_TIMEOUT     10000   // 10 seconds
#define PACKET_TIMEOUT      10000   // 10 seconds - this is long because SD cards may take a file to scan the FAT

/* packet format: SOH pkt# type length-lo length-hi hdrchk length*data crc1 crc2 */
#define HDR_SOH     0
#define HDR_TYPE    1
#define HDR_LEN_HI  2
#define HDR_LEN_LO  3
#define HDR_CHK     4

/* packet header and crc lengths */
#define PKTHDRLEN   5
#define PKTCRCLEN   2

/* maximum length of a frame */
#define FRAMELEN    (PKTHDRLEN + PKTMAXLEN + PKTCRCLEN)

/* protocol characters */
#define SOH     0x01    /* start of a packet */
#define ACK     0x06    /* positive acknowledgement */
#define NAK     0x15    /* negative acknowledgement */
#define ESC     0x1b    /* escape from terminal mode */

#define updcrc(crc, ch) (crctab[((crc) >> 8) & 0xff] ^ ((crc) << 8) ^ (ch))

static const uint16_t crctab[256] = {
    0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
    0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
    0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
    0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
    0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
    0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
    0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
    0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
    0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
    0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
    0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
    0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
    0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
    0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
    0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
    0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
    0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
    0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
    0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
    0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
    0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
    0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
    0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
    0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
    0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
    0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
    0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
    0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
    0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
    0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
    0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
    0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0
};

static int WaitForAckNak(int timeout);

int WaitForInitialAck(void)
{
    return WaitForAckNak(INITIAL_TIMEOUT) == ACK;
}

int SendPacket(int type, uint8_t *buf, int len)
{
    uint8_t hdr[PKTHDRLEN], crc[PKTCRCLEN], *p;
    uint16_t crc16 = 0;
    int cnt, ch;

    /* setup the frame header */
    hdr[HDR_SOH] = SOH;                                 /* SOH */
    hdr[HDR_TYPE] = type;                               /* type type */
    hdr[HDR_LEN_HI] = (uint8_t)(len >> 8);              /* data length - high byte */
    hdr[HDR_LEN_LO] = (uint8_t)len;                     /* data length - low byte */
    hdr[HDR_CHK] = hdr[1] + hdr[2] + hdr[3];            /* header checksum */

    /* compute the crc */
    for (p = buf, cnt = len; --cnt >= 0; ++p)
        crc16 = updcrc(crc16, *p);
    crc16 = updcrc(crc16, '\0');
    crc16 = updcrc(crc16, '\0');

    /* add the crc to the frame */
    crc[0] = (uint8_t)(crc16 >> 8);
    crc[1] = (uint8_t)crc16;

    /* send the packet */
    tx(hdr, PKTHDRLEN);
    if (len > 0)
        tx(buf, len);
    tx(crc, PKTCRCLEN);

    /* wait for an ACK/NAK */
    if ((ch = WaitForAckNak(PACKET_TIMEOUT)) < 0) {
        printf("Timeout waiting for ACK/NAK\n");
        ch = NAK;
    }

    /* return status */
    return ch == ACK;
}

int ReceivePacket(int *pType, uint8_t *buf, int len)
{
    uint8_t hdr[PKTHDRLEN], crc[PKTCRCLEN];
    int actual_len, chk;
    uint16_t crc16 = 0;

    /* look for start of packet */
    do {
        rx(&hdr[HDR_SOH], 1);
    } while (hdr[HDR_SOH] != SOH);

    /* receive the rest of the header */
    rx(&hdr[HDR_TYPE], PKTHDRLEN - 1);

    /* check the header checksum */
    chk = (hdr[1] + hdr[2] + hdr[3]) & 0xff;
    if (hdr[HDR_CHK] != chk)
        return -1;

    /* make sure the buffer is big enough for the payload */
    actual_len = hdr[HDR_LEN_HI] << 8 | hdr[HDR_LEN_LO];
    if (actual_len > len)
        return -1;
    
    /* receive the packet payload */
    rx(buf, actual_len);

    /* compute the crc */
    for (len = actual_len; --len >= 0; )
        crc16 = updcrc(crc16, *buf++);

    /* receive the crc */
    rx(crc, PKTCRCLEN);

    /* check the crc */
    crc16 = updcrc(crc16, crc[0]);
    crc16 = updcrc(crc16, crc[1]);
    if (crc16 != 0)
        return -1;

    /* return packet type and the length of the payload */
    *pType = hdr[HDR_TYPE];
    return actual_len;
}

static int WaitForAckNak(int timeout)
{
    uint8_t buf[1];
    return rx_timeout(buf, 1, timeout) == 1 ? buf[0] : -1;
}
