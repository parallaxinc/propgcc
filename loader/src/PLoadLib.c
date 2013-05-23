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
#include <unistd.h>

#include "PLoadLib.h"
#include "osint.h"

uint8_t LFSR = 80; // 'P'

int pload_verbose = 1;
int pload_delay = 0;

void psetverbose(int verbose)
{
    pload_verbose = verbose;
}

void psetdelay(int delay)
{
    pload_delay = delay;
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
    if (pload_delay == 0) {
        makelong(data, mybuf);
        return tx(mybuf, 11);
    }
    else {
        int n;
        makelong(data, mybuf);
        for(n = 0; n < 11; n++) {
            usleep(pload_delay);
            if(tx(&mybuf[n], 1) == 0)
                return 0;
        }
        return n-1;
    }
}

/**
 * hwfind ... find propeller using sync-up sequence.
 * @param hSerial - file handle to serial port
 * @returns zero on failure
 */
static int hwfind(int retry)
{
    int  n, ii, jj, rc, to;
    uint8_t mybuf[300];

    /* hwfind is recursive if we get a failure on th first try.
     * retry is set by caller and should never be more than one.
     */
    if(retry < 0)
        return 0;

    /* Do not pause after reset.
     * Propeller can give up if it does not see a response in 100ms of reset.
     */
    mybuf[0] = 0xF9;
    LFSR = 'P';  // P is for Propeller :)

    /* send the calibration pulse
     */
    if(tx(mybuf, 1) == 0)
        return 0;   // tx should never return 0, return error if it does.

    /* Send the magic propeller LFSR byte stream.
     */
    for(n = 0; n < 250; n++)
        mybuf[n] = iterate() | 0xfe;
    if(tx(mybuf, 250) == 0)
        return 0;   // tx should never return 0, return error if it does.

    n = 0;
    while((jj = rx_timeout(mybuf,10,50)) > -1)
        n += jj;
    if(n != 0)
        printf("Ignored %d bytes. \n", n);

    /* Send 258 0xF9 for LFSR and Version ID
     * These bytes clock the LSFR bits and ID from propeller back to us.
     */
    for(n = 0; n < 258; n++)
        mybuf[n] = 0xF9;
    if(tx(mybuf, 258) == 0)
        return 0;   // tx should never return 0, return error if it does.

    /*
     * Wait at least 100ms for the first response. Allow some margin.
     * Some chips may respond < 50ms, but there's no guarantee all will.
     * If we don't get it, we can assume the propeller is not there.
     */
    ii = getBit(&rc, 110);
    if(rc == 0) {
        //printf("Timeout waiting for first response bit. Propeller not found\n");
        return 0;
    }

    // wait for response so we know we have a Propeller
    for(n = 1; n < 250; n++) {

        jj = iterate();
        //printf("%d:%d ", ii, jj);
        //fflush(stdout);

        if(ii != jj) {
            /* if we get this far, we probably have a propeller chip
             * but the serial port is in a funny state. just retry.
             */
            //printf("Lost HW contact. %d %x ... retry.\n", n, *mybuf & 0xff);
            for(n = 0; (n < 300) && (rx_timeout(mybuf,1,10) > -1); n++);
            hwreset();
            return hwfind(--retry);
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
    version = hwfind(1); // retry once
    if(version && port) {
        if (pload_verbose)
            printf("Propeller Version %d on %s\n", version, port);
    }
    return version != 0 ? 0 : 1;
}

/*
 * forward declaration for pupload
 */
static int upload(const char* file, const uint8_t* dlbuf, int count, int type);

/**
 * pupload is a wrapper for findprop + upload
 * This function should be used so that we don't get timeouts between
 * findprop and upload that can be caused by slow PC/Linux/MAC hardware.
 * @param file - filename for status message
 * @param dlbuf - pointer to file buffer
 * @param count - number of bytes in image
 * @param type - type of upload
 * @returns non-zero on error
 */
static int pupload(const char* file, const uint8_t* dlbuf, int count, int type)
{
    int rc = -1;
    int version = 0;
    hwreset();
    version = hwfind(1); // retry once
    if(version) {
        rc = upload(file, dlbuf, count, type);
    }
    return rc;
}

/**
 * Upload file image to propeller on port.
 * A successful call to findprop must be completed before calling this function.
 * @param file - filename for status message
 * @param dlbuf - pointer to file buffer
 * @param count - number of bytes in image
 * @param type - type of upload
 * @returns non-zero on error
 */
static int upload(const char* file, const uint8_t* dlbuf, int count, int type)
{
    int  n  = 0;
    int  rc = 0;
    int  wv = 100;
    int  ack = 0;
    int  remaining = 0;
    uint32_t data = 0;
    uint8_t buf[1];

    int  longcount = count/4;

    // send type
    if(sendlong(type) == 0) {
        if (pload_verbose)
            printf("sendlong type failed\n");
        return 1;
    }

    // send count
    if(sendlong(longcount) == 0) {
        if (pload_verbose)
            printf("sendlong count failed\n");
        return 1;
    }

    if((type == 0) || (type >= DOWNLOAD_SHUTDOWN)) {
        if (pload_verbose)
            printf("Shutdown type %d sent\n", type);
        return 1;
    }

    if (pload_verbose) {
        if (file)
            printf("Loading %s",file);
        else
            printf("Loading");

        if(type == DOWNLOAD_RUN_BINARY)
            printf(" to hub memory\n");
        else
            printf(" to EEPROM via hub memory\n");
    }

    remaining = count;
    for(n = 0; n < count; n+=4) {
        if(n % 1024 == 0) {
            printf("%d bytes remaining             \r", remaining);
            fflush(stdout);
            remaining -= 1024;
        }
        data = dlbuf[n] | (dlbuf[n+1] << 8) | (dlbuf[n+2] << 16) | (dlbuf[n+3] << 24) ;
        if(sendlong(data) == 0) {
            if (pload_verbose)
                printf("sendlong error");
            return 1;
        }
    }

    printf("                               \r");
    printf("%d bytes sent\n", count);
    fflush(stdout);

    msleep(50);                 // give propeller time to calculate checksum match 32K/12M sec = 32ms
    buf[0] = 0xF9;              // need to send this to make Propeller send the ack

    /* 
     * Check for a RAM program complete signal
     */
    if (pload_verbose) {
        printf("Verifying RAM ... ");
        fflush(stdout);
    }
    for(n = 0; n < wv; n++) {
        if(tx(buf, 1) == 0) return 1;
        rc = getAck(&ack, 20);
        if(ack) break;
    }

    /* 
     * Check for a Timeout or Checksum Error
     */
    if(n >= wv) {
        if(pload_verbose)
            printf("Timeout Error!\n");
        return 1;
    }

    if(rc == 0) {
        if(pload_verbose)
            printf("OK\n");
    }
    else {
        if(pload_verbose)
            printf("Checksum Error!\n");
        return 1;
    }

    /* If type is DOWNLOAD_EEPROM or DOWNLOAD_RUN_EEPROM
     * Check for a complete signal, then check for verify signal
     * Otherwise the board will shutdown in an undesirable way.
     */
    if(type & DOWNLOAD_EEPROM) {
        wv = 500;

        /* send this to make Propeller send the ack
         */
        buf[0] = 0xF9;

        if (pload_verbose) {
            printf("Programming EEPROM ... ");
            fflush(stdout);
        }
        /* Check for EEPROM program finished
         */
        for(n = 0; n < wv; n++) {
            msleep(20);
            if(tx(buf, 1) == 0) return 1;
            rc = getAck(&ack, 20);
            if(ack) {
                if(rc != 0) {
                    if(pload_verbose)
                        printf("failed\n");
                    return 1;
                }
                break;
            }
        }
        if(n >= wv) {
            if(pload_verbose)
                printf("EEPROM programming timeout\n");
            return 1;
        }

        if (pload_verbose) {
            if(rc == 0)
                printf("OK\n");
        }

        if (pload_verbose) {
            printf("Verifying EEPROM ... ");
            fflush(stdout);
        }
        /* Check for EEPROM program verify
         */
        for(n = 0; n < wv; n++) {
            msleep(20);
            if(tx(buf, 1) == 0) return 1;
            rc = getAck(&ack, 20);
            if(ack) {
                if(rc != 0) {
                    if(pload_verbose)
                        printf("failed\n");
                    return 1;
                }
                break;
            }
        }
        if(n >= wv) {
            if(pload_verbose)
                printf("EEPROM verify timeout\n");
            return 1;
        }

        if (pload_verbose) {
            if(rc == 0)
                printf("OK\n");
        }
    }

    return 0;
}

int popenport(const char* port, int baud, int noreset)
{
    //
    // open the port
    //
    if (serial_init(port, baud) == 0)
        return PLOAD_STATUS_OPEN_FAILED;
    
    if (noreset)
      return PLOAD_STATUS_OK;

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
            printf("Can't open '%s' for download\n", file);
        return 4;
    }
    else {
        count = (int)fread(dlbuf, 1, 0x8000, fp);
        if (pload_verbose)
            if((type != 0) && (type < DOWNLOAD_SHUTDOWN))
                printf("Downloading %d bytes of '%s'\n", count, file);
        fclose(fp);
    }

    // if found and file, upload file
    //
    if(file) {
        rc = pupload(file, dlbuf, count, type);
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
        printf("Downloading %d bytes of '%s'\n", count, file);

    // if found and file, upload file
    //
    if(file) {
        rc = pupload(file, dlbuf, count, type);
    }

    return rc;
}

/**
 * find and load propeller with buffer - assumes port is already open
 * @returns non-zero on error.
 */
int ploadbuf(const char* file, const uint8_t* dlbuf, int count, int type)
{
    return pupload(file, dlbuf, count, type);
}

#ifdef MAIN
int main(int argc, char *argv[])
{
    int operation = DOWNLOAD_RUN_BINARY;
    if (argc < 3) {
        printf("usage: pload <port> <filename> [-pN]\n");
        return 1;
    }

    if(argv[3] != 0 && argv[3][0] == '-') {
        switch(argv[3][1]) {
            case 'p':
                operation = atoi(&argv[3][2]);
                break;
            default:
                break;
        }
    }
    if (popenport(argv[1], 115200, PLOAD_RESET_DEVICE)) {
        printf("Error opening port\n");
        return 1;
    }
    if (pload(argv[2], operation) != 0) {
        printf("Load failed\n");
        return 1;
    }
    return 0;
}
#endif
