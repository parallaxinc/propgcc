#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <propeller.h>

//#define PMC
#define DNA
//#define RAMPAGE2
//#define DEFAULTS

//#define FLASH_TEST
//#define BIG_FLASH_TEST
//#define RAM_TEST
#define BIG_RAM_TEST

#ifndef TESTDRIVER
#define TESTDRIVER      binary_spi_sram_xmem_dat_start
#endif

#ifdef PMC
#define START_PIN       0
#define SIO0_PIN        START_PIN
#define SD_CS_PIN       (START_PIN + 4)
#define FLASH_CS_PIN    (START_PIN + 5)
#define SRAM_CS_PIN     (START_PIN + 6)
#define SCK_PIN         (START_PIN + 7)
#define XMEM_CONFIG1    ((SIO0_PIN << 24) | (SCK_PIN << 16) | (FLASH_CS_PIN << 8) | SRAM_CS_PIN)
#define XMEM_CONFIG2    ((SD_CS_PIN << 24) | 1)
#define XMEM_CONFIG3    0
#define XMEM_CONFIG4    0
#endif

#ifdef DNA
#define START_PIN       22
#define SIO0_PIN        START_PIN
#define SCK_PIN         (START_PIN + 4)
#define SRAM_CS_PIN     (START_PIN + 5)
#define XMEM_CONFIG1    ((SIO0_PIN << 24) | (SCK_PIN << 8) | 0x01)
#define XMEM_CONFIG2    (SRAM_CS_PIN << 24)
#define XMEM_CONFIG3    0
#define XMEM_CONFIG4    0
#endif

#ifdef RAMPAGE2
#define START_PIN       0
#define SIO0_PIN        START_PIN
#define SCK_PIN         8
#define SRAM_CS_PIN     10
#define XMEM_CONFIG1    ((SIO0_PIN << 24) | (SCK_PIN << 8) | SRAM_CS_PIN)
#define XMEM_CONFIG2    0
#define XMEM_CONFIG3    0
#define XMEM_CONFIG4    0
#endif

#ifdef DEFAULTS
#define XMEM_CONFIG1    0
#define XMEM_CONFIG2    0
#define XMEM_CONFIG3    0
#define XMEM_CONFIG4    0
#endif

#define BUF_SIZE        1024
#define BUF_SIZE32      (BUF_SIZE / sizeof(uint32_t))
#define BUF_SIZE_CODE   XMEM_SIZE_1024
#define BUF_COUNT       (32768 / BUF_SIZE)

/* for the c3 */
#define LED_PIN         15
#define LED_MASK        (1 << LED_PIN)
#define LED_ON()        (OUTA &= ~LED_MASK)
#define LED_OFF()       (OUTA |= LED_MASK)

#define XMEM_WRITE      0x8
#define XMEM_SIZE_16    0x1
#define XMEM_SIZE_32    0x2
#define XMEM_SIZE_64    0x3
#define XMEM_SIZE_128   0x4
#define XMEM_SIZE_256   0x5
#define XMEM_SIZE_512   0x6
#define XMEM_SIZE_1024  0x7
#define XMEM_END        0x8

/* cache driver communications mailbox */
typedef struct {
    volatile uint32_t hubaddr;
    volatile uint32_t extaddr;
} xmem_mbox_t;

/* external memory base addresses */
#define RAM_BASE        0x20000000
#define ROM_BASE        0x30000000

/* globals */
static xmem_mbox_t xmem_mboxes[2];
static xmem_mbox_t  *xmem_mbox = &xmem_mboxes[0];
static uint8_t padded_buf[BUF_SIZE + 15];

int xmemStart(void *code, xmem_mbox_t *mboxes, int count, uint32_t config1, uint32_t config2, uint32_t config3, uint32_t config4);
void readBlock(uint32_t extaddr, void *buf, uint32_t count);
void writeBlock(uint32_t extaddr, void *buf, uint32_t count);

int main(void)
{
    extern char TESTDRIVER[];
    uint32_t *buf = (uint32_t *)(((uint32_t)padded_buf + 15) & ~15);
    int cogn;
    
    cogn = xmemStart(TESTDRIVER, xmem_mboxes, 1, XMEM_CONFIG1, XMEM_CONFIG2, XMEM_CONFIG3, XMEM_CONFIG4);
    
    printf("mbox        %08x\n", (uint32_t)&xmem_mbox);
    printf("padded buf  %08x\n", (uint32_t)padded_buf);
    printf("buf         %08x\n", (uint32_t)buf);
    printf("cog         %d\n", cogn);
    
#if 0

{
    int i;

    readBlock(ROM_BASE, buf, XMEM_SIZE_128);
    for (i = 0; i < 32; ++i)
        printf("buf[%d] = %08x\n", i, buf[i]);
}

#endif

#ifdef FLASH_TEST

{
    int start, i, j;
    
    printf("Flash test\n");
    
    srand(CNT);
    start = rand();
    printf("Starting with %08x\n", start);
    for (i = 0, j = start; i < BUF_SIZE32; ++i)
        buf[i] = j++;
    writeBlock(0, buf, BUF_SIZE_CODE);
    memset(buf, 0, sizeof(buf));
    readBlock(ROM_BASE, buf, BUF_SIZE_CODE);
    for (i = 0, j = start; i < BUF_SIZE32; ++i) {
        if (buf[i] != j++)
            printf("%08x: expected %08x, found %08x\n", i, j - 1, buf[i]);
    }

    printf("done\n");
}

#endif

#ifdef BIG_FLASH_TEST

{
    uint32_t addr, startValue, value;
    int i, j;
    
    printf("Big flash test\n");

    addr = ROM_BASE;
    srand(CNT);
    startValue = value = addr + rand();
    printf("Start value %08x\n", startValue);
    printf("Filling flash\n");
    for (j = 0; j < BUF_COUNT; ++j) {
        uint32_t startAddr = addr;
        for (i = 0; i < BUF_SIZE32; ++i, addr += sizeof(uint32_t))
            buf[i] = value++;
        //printf("Writing %08x\n", ROM_BASE + startAddr);
        writeBlock(startAddr, buf, BUF_SIZE_CODE);
    }
    
    printf("Checking flash\n");
    addr = ROM_BASE;
    value = startValue;
    for (j = 0; j < BUF_COUNT; ++j) {
        //printf("Reading %08x\n", addr);
        readBlock(addr, buf, BUF_SIZE_CODE);
        for (i = 0; i < BUF_SIZE32; ++i, addr += sizeof(uint32_t))
            if (buf[i] != value++)
                printf("%08x: expected %08x, found %08x\n", addr, value - 1, buf[i]);
    }

    addr = ROM_BASE;
    value = startValue;
    printf("Filling flash inverted\n");
    for (j = 0; j < BUF_COUNT; ++j) {
        uint32_t startAddr = addr;
        for (i = 0; i < BUF_SIZE32; ++i, addr += sizeof(uint32_t))
            buf[i] = ~value++;
        //printf("Writing %08x\n", ROM_BASE + startAddr);
        writeBlock(startAddr, buf, BUF_SIZE_CODE);
    }
    
    addr = ROM_BASE;
    value = startValue;
    printf("Checking flash inverted\n");
    for (j = 0; j < BUF_COUNT; ++j) {
        //printf("Reading %08x\n", addr);
        readBlock(addr, buf, BUF_SIZE_CODE);
        for (i = 0; i < BUF_SIZE32; ++i, addr += sizeof(uint32_t))
            if (buf[i] != ~value++)
                printf("%08x: expected %08x, found %08x\n", addr, ~(value - 1), buf[i]);
    }

    printf("done\n");
}

#endif

#ifdef RAM_TEST

{
    int i;

    printf("RAM test\n");
    
    for (i = 0; i < BUF_SIZE32; ++i)
        buf[i] = 0xbeef0000 + i;
    writeBlock(RAM_BASE, buf, BUF_SIZE_CODE);
    
    memset(buf, 0, sizeof(buf));
    
    readBlock(RAM_BASE, buf, BUF_SIZE_CODE);
    for (i = 0; i < BUF_SIZE32; ++i)
        printf("buf[%d] = %08x\n", i, buf[i]);

    printf("done\n");
}

#endif
        
#ifdef BIG_RAM_TEST

{
    uint32_t addr, startValue, value;
    int i, j;
    
    printf("Big RAM test\n");

    addr = RAM_BASE;
    srand(CNT);
    startValue = value = addr + rand();
    printf("Start value %08x\n", startValue);
    printf("Filling RAM\n");
    for (j = 0; j < BUF_COUNT; ++j) {
        uint32_t startAddr = addr;
        for (i = 0; i < BUF_SIZE32; ++i, addr += sizeof(uint32_t))
            buf[i] = value++;
        //printf("Writing %08x\n", startAddr);
        writeBlock(startAddr, buf, BUF_SIZE_CODE);
    }
    
    printf("Checking RAM\n");
    addr = RAM_BASE;
    value = startValue;
    for (j = 0; j < BUF_COUNT; ++j) {
        //printf("Reading %08x\n", addr);
        readBlock(addr, buf, BUF_SIZE_CODE);
        for (i = 0; i < BUF_SIZE32; ++i, addr += sizeof(uint32_t))
            if (buf[i] != value++)
                printf("%08x: expected %08x, found %08x\n", addr, value - 1, buf[i]);
    }

    printf("Filling RAM inverted\n");
    addr = RAM_BASE;
    value = startValue;
    for (j = 0; j < BUF_COUNT; ++j) {
        uint32_t startAddr = addr;
        for (i = 0; i < BUF_SIZE32; ++i, addr += sizeof(uint32_t))
            buf[i] = ~value++;
        //printf("Writing %08x\n", startAddr);
        writeBlock(startAddr, buf, BUF_SIZE_CODE);
    }
    
    printf("Checking RAM inverted\n");
    addr = RAM_BASE;
    value = startValue;
    for (j = 0; j < BUF_COUNT; ++j) {
        //printf("Reading %08x\n", addr);
        readBlock(addr, buf, BUF_SIZE_CODE);
        for (i = 0; i < BUF_SIZE32; ++i, addr += sizeof(uint32_t))
            if (buf[i] != ~value++)
                printf("%08x: expected %08x, found %08x\n", addr, ~(value - 1), buf[i]);
    }

    printf("done\n");
}

#endif

    return 0;
}

int xmemStart(void *code, xmem_mbox_t *mboxes, int count, uint32_t config1, uint32_t config2, uint32_t config3, uint32_t config4)
{
    uint32_t *xmem = (uint32_t *)code;
    memset(mboxes, 0, sizeof(xmem_mbox_t) * count);
    mboxes[count].hubaddr = XMEM_END;
    xmem[1] = config1;
    xmem[2] = config2;
    xmem[3] = config3;
    xmem[4] = config4;
    return cognew(code, mboxes);
}

void readBlock(uint32_t extaddr, void *buf, uint32_t size)
{
    //printf("Reading %08x\n", extaddr);
    xmem_mbox->extaddr = extaddr;
    xmem_mbox->hubaddr = (uint32_t)buf | size;
    //LED_ON();
    while (xmem_mbox->hubaddr)
        ;
    //LED_OFF();
    //printf("read returned %08x\n", xmem_mbox->addr);
}

void writeBlock(uint32_t extaddr, void *buf, uint32_t size)
{
    //printf("Writing %08x\n", extaddr);
    xmem_mbox->extaddr = extaddr;
    xmem_mbox->hubaddr = (uint32_t)buf | XMEM_WRITE | size;
    //LED_ON();
    while (xmem_mbox->hubaddr)
        ;
    //LED_OFF();
    //printf("read returned %08x\n", xmem_mbox->addr);
}
