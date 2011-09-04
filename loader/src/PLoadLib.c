/**
 * @file PLoadLib.c
 *
 * Downloads an image to Propeller using Windows32 API.
 * Ya, it's been done before, but some programs like Propellent and
 * PropTool do not recognize virtual serial ports. Hence, this program.
 *
 * Copyright (c) 2009 by John Steven Denson
 * Modified in 2011 by David Michael Betz
 *
 * MIT License                                                           
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "PLoadLib.h"
#include "osint.h"

uint8_t LFSR = 80; // 'P'

int iterate(void)
{
    int bit = LFSR & 1;
    LFSR = (uint8_t)((LFSR << 1) | (((LFSR >> 7) ^ (LFSR >> 5) ^ (LFSR >> 4) ^ (LFSR >> 1)) & 1));
    return bit;
}

/**
 * getBit ... get bit from serial stream
 * @param hSerial - file handle to serial port
 * @param status - pointer to transaction status 0 on error
 * @returns bit state 1 or 0
 */
int getBit(int* status, int timeout)
{
    uint8_t mybuf[2];
    int rc = rx_timeout(mybuf, 1, timeout);
    if(status)
        *status = rc <= 0 ? 0 : 1;
    return *mybuf & 1;
}

/**
 * getAck ... get ack from serial stream
 * @param hSerial - file handle to serial port
 * @param status - pointer to transaction status 0 on error
 * @returns bit state 1 or 0
 */
int getAck(int* status, int timeout)
{
    uint8_t mybuf[2];
    int rc = rx_timeout(mybuf, 1, timeout);
    if(status)
        *status = rc <= 0 ? 0 : 1;
    return *mybuf & 1;
}

/**
 * makelong ... make an encoded long word to string
 * @param data - value to send
 * @param buff - uint8_t buffer
 * @returns nada
 */
void makelong(uint32_t data, uint8_t* buff)
{
    int n = 0;
    //printf("\n0x%08x: ", data);
    for( ; n < 10; n++) {
        buff[n] = (uint8_t)(0x92 | (data & 1) | ((data & 2) << 2) | ((data & 4) << 4));
        data >>= 3;
    }
    buff[n] = (0xf2 | (data & 1) | ((data & 2) << 2));
    //for(n = 0; n < 11; n++) printf("0x%02x ",buff[n]);
    //decodelong(buff,11);
}

/**
 * sendlong ... transmit an encoded long word to propeller
 * @param hSerial - file handle to serial port
 * @param data - value to send
 * @returns number of bytes sent
 */
int sendlong(uint32_t data)
{
    uint8_t mybuf[12];
    makelong(data, mybuf);
    return tx(mybuf, 11);
}

/**
 * hwfind ... find propeller using sync-up sequence.
 * @param hSerial - file handle to serial port
 * @returns zero on failure
 */
int hwfind(void)
{
    int  n, ii, jj, rc, to;
    uint8_t mybuf[300];

    msleep(50); // pause after reset - 100ms is too long
    mybuf[0] = 0xF9;
    LFSR = 'P';  // P is for Propeller :)

    // set magic propeller byte stream
    for(n = 1; n < 251; n++)
        mybuf[n] = iterate() | 0xfe;
    tx(mybuf, 251);

    // send gobs of 0xF9 for id sync-up - these clock out the LSFR bits and the id
    for(n = 0; n < 258; n++)
        mybuf[n] = 0xF9;
    tx(mybuf, 258);

    //for(n = 0; n < 250; n++) printf("%d", iterate() & 1);
    //printf("\n\n");

    msleep(100);
    
    // wait for response so we know we have a Propeller
    for(n = 0; n < 250; n++) {
        to = 0;
        do {
            ii = getBit(&rc, 100);
        } while(rc == 0 && to++ < 100);
        //printf("%d", rc);
        if(to > 100) {
            printf("Timeout waiting for response bit. Propeller Not Found!\n");
            return 0;
        }
        jj = iterate();
        //printf("%d:%d ", ii, jj);
        if(ii != jj) {
            printf("Lost HW contact. %d %x\n", n, *mybuf & 0xff);
            return 0;
        }
    }
    
    //printf("Propeller Version ... ");
    rc = 0;
    for(n = 0; n < 8; n++) {
        rc >>= 1;
        rc += getBit(0, 100) ? 0x80 : 0;
    }
    //printf("%d\n",rc);
    return rc;
}

/**
 * find a propeller on port
 * @param hSerial - file handle to serial port
 * @param sparm - pointer to DCB serial control struct
 * @param port - pointer to com port name
 * @returns non-zero on error
 */
int findprop(char* port)
{
    int version = 0;
    hwreset();
    version = hwfind();
    if(version) {
        printf("Propeller Version %d on %s\n", version, port);
    }
    return version != 0 ? 0 : 1;
}

/**
 * Upload file image to propeller on port.
 * A successful call to findprop must be completed before calling this function.
 * @param hSerial - file handle to serial port
 * @param dlbuf - pointer to file buffer
 * @param count - number of bytes in image
 * @param type - type of upload
 * @returns non-zero on error
 */
int upload(uint8_t* dlbuf, int count, int type)
{
    int  n  = 0;
    int  rc = 0;
    int  wv = 100;
    uint32_t data = 0;
    uint8_t buf[1];

    int  longcount = count/4;

    // send type
    if(sendlong(type) == 0) {
        printf("sendlong type failed.\n");
        return 1;
    }

    // send count
    if(sendlong(longcount) == 0) {
        printf("sendlong count failed.\n");
        return 1;
    }

    printf("Writing %d bytes to %s.\n",count,(type == DOWNLOAD_RUN_BINARY) ? "Propeller RAM":"EEPROM");
    msleep(100);

    for(n = 0; n < count; n+=4) {
        //printf("%d ",n);
        data = dlbuf[n] | (dlbuf[n+1] << 8) | (dlbuf[n+2] << 16) | (dlbuf[n+3] << 24) ;
        if(sendlong(data) == 0) {
            printf("sendlong error");
            return 1;
        }
        //if(n % 0x40 == 0) putchar('.');
    }

    msleep(150);                // wait for checksum calculation on Propeller ... 95ms is minimum
    buf[0] = 0xF9;              // need to send this to get Propeller to send the ack

    printf("Verifying ... ");
    fflush(stdout);

    for(n = 0; n < wv; n++) {
        tx(buf, 1);
        getAck(&rc, 100);
        if(rc) break;
    }
    if(n >= wv) printf("Upload Timeout Error!\n");
    else        printf("Upload OK!\n");

    return 0;
}

/**
 * find and load propeller with file - assumes port is already open
 * @returns non-zero on error.
 */
int pload(char* file, char* port, int type)
{
    int rc = 0;
    int count;
    uint8_t dlbuf[0x8000]; // 32K limit

    // read program if file parameter is available
    //
    FILE* fp = fopen(file, "rb");
    if(!fp) {
        printf("Can't open '%s' for upload.\n", file);
        return 4;
    }
    else {
        count = fread(dlbuf, 1, 0x8000, fp);
        printf("Downloading %d bytes of '%s'\n", count, file);
        fclose(fp);
    }

    //
    // find propeller
    //
    rc = findprop(port);

    // if found and file, upload file
    //
    if(!rc && file) {
        rc = upload(dlbuf, count, type);
    }

    return rc;
}

/**
 * find and load propeller with file - assumes port is already open
 * @returns non-zero on error.
 */
int ploadfp(char* file, FILE *fp, char* port, int type)
{
    int rc = 0;
    int count;
    uint8_t dlbuf[0x8000]; // 32K limit

    // read program if file parameter is available
    //
    count = fread(dlbuf, 1, 0x8000, fp);
    printf("Downloading %d bytes of '%s'\n", count, file);
    fclose(fp);

    //
    // find propeller
    //
    rc = findprop(port);

    // if found and file, upload file
    //
    if(!rc && file) {
        rc = upload(dlbuf, count, type);
    }

    return rc;
}

/**
 * find and load propeller with buffer - assumes port is already open
 * @returns non-zero on error.
 */
int ploadbuf(uint8_t* dlbuf, int count, char* port, int type)
{
    int rc = 0;

    //
    // find propeller
    //
    rc = findprop(port);

    // if found, upload file
    //
    if(!rc) {
        rc = upload(dlbuf, count, type);
    }

    return rc;
}
