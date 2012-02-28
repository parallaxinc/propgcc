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

int pload_verbose = 1;

void psetverbose(int verbose)
{
    pload_verbose = verbose;
}

static int iterate(void)
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
static int getBit(int* status, int timeout)
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
static int getAck(int* status, int timeout)
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
static void makelong(uint32_t data, uint8_t* buff)
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
static int sendlong(uint32_t data)
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
static int hwfind(void)
{
    int  n, ii, jj, rc, to;
    uint8_t mybuf[300];

    /* Do not pause after reset.
     * Propeller can give up if it does not see a response in 100ms of reset.
     */
    mybuf[0] = 0xF9;
    LFSR = 'P';  // P is for Propeller :)

    // set magic propeller byte stream
    for(n = 1; n < 251; n++)
        mybuf[n] = iterate() | 0xfe;
    if(tx(mybuf, 251) == 0)
        return 0;   // tx should never return 0, return error if it does.

    // send gobs of 0xF9 for id sync-up - these clock out the LSFR bits and the id
    for(n = 0; n < 258; n++)
        mybuf[n] = 0xF9;
    if(tx(mybuf, 258) == 0)
        return 0;   // tx should never return 0, return error if it does.

    //for(n = 0; n < 250; n++) printf("%d", iterate() & 1);
    //printf("\n\n");

    /*
     * Wait at least 100ms for the first response. Allow some margin.
     * Some chips may respond < 50ms, but there's no guarantee all will.
     * If we don't get it, we can assume the propeller is not there.
     */
    ii = getBit(&rc, 110);
    if(rc == 0) {
        //printf("Timeout waiting for first response bit. Propeller not found.\n");
        return 0;
    }

    // wait for response so we know we have a Propeller
    for(n = 1; n < 250; n++) {

        jj = iterate();
        //printf("%d:%d ", ii, jj);
        //fflush(stdout);

        if(ii != jj) {
            //printf("Lost HW contact. %d %x\n", n, *mybuf & 0xff);
            return 0;
        }

        to = 0;
        do {
            ii = getBit(&rc, 100);
        } while(rc == 0 && to++ < 100);
        //printf("%d", rc);
        if(to > 100) {
            //printf("Timeout waiting for response bit. Propeller Not Found!\n");
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
static int findprop(const char* port)
{
    int version = 0;
    hwreset();
    version = hwfind();
    if(version && port) {
        if (pload_verbose)
            fprintf(stderr, "Propeller Version %d on %s\n", version, port);
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
static int upload(const uint8_t* dlbuf, int count, int type)
{
    int  n  = 0;
    int  rc = 0;
    int  wv = 100;
    uint32_t data = 0;
    uint8_t buf[1];

    int  longcount = count/4;

    // send type
    if(sendlong(type) == 0) {
        if (pload_verbose)
            fprintf(stderr, "sendlong type failed.\n");
        return 1;
    }

    // send count
    if(sendlong(longcount) == 0) {
        if (pload_verbose)
            fprintf(stderr, "sendlong count failed.\n");
        return 1;
    }

    if (pload_verbose)
        fprintf(stderr, "Writing %d bytes to %s.\n",count,(type == DOWNLOAD_RUN_BINARY) ? "Propeller RAM":"EEPROM");
    msleep(100);

    for(n = 0; n < count; n+=4) {
        //printf("%d ",n);
        data = dlbuf[n] | (dlbuf[n+1] << 8) | (dlbuf[n+2] << 16) | (dlbuf[n+3] << 24) ;
        if(sendlong(data) == 0) {
            if (pload_verbose)
                fprintf(stderr, "sendlong error");
            return 1;
        }
        //if(n % 0x40 == 0) putchar('.');
    }

    msleep(150);                // wait for checksum calculation on Propeller ... 95ms is minimum
    buf[0] = 0xF9;              // need to send this to get Propeller to send the ack

    if (pload_verbose) {
        fprintf(stderr, "Verifying ... ");
        fflush(stderr);
    }

    for(n = 0; n < wv; n++) {
        tx(buf, 1);
        getAck(&rc, 100);
        if(rc) break;
    }
    if (pload_verbose) {
        if(n >= wv) fprintf(stderr, "Timeout Error!\n");
        else        fprintf(stderr, "Download OK!\n");
    }
    
    return 0;
}

int popenport(const char* port, int baud)
{
    //
    // open the port
    //
    if (serial_init(port, baud) == 0)
        return PLOAD_STATUS_OPEN_FAILED;
        
    //
    // find propeller
    //
    if (findprop(port) != 0) {
        serial_done();
        return PLOAD_STATUS_NO_PROPELLER;
    }

    return PLOAD_STATUS_OK;
}

int preset(void)
{
    return findprop(NULL);
}

/**
 * find and load propeller with file - assumes port is already open
 * @returns non-zero on error.
 */
int pload(const char* file, int type)
{
    int rc = 1;
    int count;
    uint8_t dlbuf[0x8000]; // 32K limit

    // read program if file parameter is available
    //
    FILE* fp = fopen(file, "rb");
    if(!fp) {
        if (pload_verbose)
            fprintf(stderr, "Can't open '%s' for download.\n", file);
        return 4;
    }
    else {
        count = (int)fread(dlbuf, 1, 0x8000, fp);
        if (pload_verbose)
            fprintf(stderr, "Downloading %d bytes of '%s'\n", count, file);
        fclose(fp);
    }

    // if found and file, upload file
    //
    if(file) {
        rc = upload(dlbuf, count, type);
    }

    return rc;
}

/**
 * find and load propeller with file - assumes port is already open
 * @returns non-zero on error.
 */
int ploadfp(const char* file, FILE *fp, int type)
{
    int rc = 1;
    int count;
    uint8_t dlbuf[0x8000]; // 32K limit

    // read program if file parameter is available
    //
    count = (int)fread(dlbuf, 1, 0x8000, fp);
    if (pload_verbose)
        fprintf(stderr, "Downloading %d bytes of '%s'\n", count, file);

    // if found and file, upload file
    //
    if(file) {
        rc = upload(dlbuf, count, type);
    }

    return rc;
}

/**
 * find and load propeller with buffer - assumes port is already open
 * @returns non-zero on error.
 */
int ploadbuf(const uint8_t* dlbuf, int count, int type)
{
    return upload(dlbuf, count, type);
}
