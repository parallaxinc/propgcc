#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <propeller.h>

#ifndef TESTDRIVER
#define TESTDRIVER      binary_rampage2_xcache_dat_start
#endif

//#define FLASH_TEST
//#define BIG_FLASH_TEST
//#define RAM_TEST
#define BIG_RAM_TEST
//#define CACHE_TEST

#define CACHE_SIZE      (512 + 8192)
#define CACHE_CONFIG1   0       // cache geometry - use defaults

#ifdef PMC_PROTOTYPE
#define CACHE_CONFIG2   ((0 << 24) | (4 << 16) | (5 << 8) | 6)
#define CACHE_CONFIG3   ((7 << 24) | 1)
#define CACHE_CONFIG4   0
#endif

#ifdef PMC
#define CACHE_CONFIG2   ((0 << 24) | (7 << 16) | (5 << 8) | 6)
#define CACHE_CONFIG3   ((4 << 24) | 1)
#define CACHE_CONFIG4   0
#endif

#ifdef PMC_SQI_SRAM
#define CACHE_CONFIG2   ((0 << 24) | (7 << 8) | 0x01)
#define CACHE_CONFIG3   (6 << 24)
#define CACHE_CONFIG4   0
#endif

#ifdef PMC_SPI_SRAM24
#define CACHE_CONFIG2   ((0 << 24) | (1 << 16) | (7 << 8) | 0x21)
#define CACHE_CONFIG3   (6 << 24)
#define CACHE_CONFIG4   0
#endif

#ifdef RAMPAGE2_PMC
#define CACHE_CONFIG2   ((0 << 24) | (8 << 16) | (9 << 8) | 10)
#define CACHE_CONFIG3   0
#define CACHE_CONFIG4   0
#endif

#ifdef RAMPAGE2_SQI_SRAM
#define CACHE_CONFIG2   ((0 << 24) | (8 << 8) | 0x01)
#define CACHE_CONFIG3   (10 << 24)
#define CACHE_CONFIG4   0
#endif

#ifdef RAMPAGE2_SPI_SRAM24
#define CACHE_CONFIG2   ((0 << 24) | (1 << 16) | (8 << 8) | 0x21)
#define CACHE_CONFIG3   (10 << 24)
#define CACHE_CONFIG4   0
#endif

#ifdef RAMPAGE2
#define CACHE_CONFIG2   0
#define CACHE_CONFIG3   0
#define CACHE_CONFIG4   0
#endif

#define BUF_SIZE        32
#define BUF_COUNT       (32768 / sizeof(buf))

/* for the c3 */
#define LED_PIN         15
#define LED_MASK        (1 << LED_PIN)
#define LED_ON()        (OUTA &= ~LED_MASK)
#define LED_OFF()       (OUTA |= LED_MASK)

/* cache driver initialization structure */
typedef struct {
  void *mbox;               // mailbox
  void *cache;              // space for tags and cache lines
  uint32_t config1;         // cache geometry
  uint32_t config2;         // driver-specific parameter
  uint32_t config3;         // driver-specific parameter
  uint32_t config4;         // driver-specific parameter
} cache_init_t;

/* cache driver communications mailbox */
typedef struct {
  volatile uint32_t cmd;
  volatile uint32_t addr;
} cache_mbox_t;

/* commands */
#define EXTENDED_CMD        0x00000000
#define WRITE_CMD           0x00000002
#define READ_CMD            0x00000003

#define CMD_MASK            0x00000003

/* extended commands */
#define UNUSED_CMD          0x00000001
#define ERASE_BLOCK_CMD     0x00000005
#define WRITE_DATA_CMD      0x00000009
#define BLOCK_READ_CMD      0x0000000d
#define BLOCK_WRITE_CMD     0x00000011
#define UNUSED2_CMD         0x00000015
#define REINIT_CACHE_CMD    0x00000019
#define UNUSED3_CMD         0x0000001d

/* block i/o parameter structure */
typedef struct {
    void *hubaddr;
    uint32_t count;
    uint32_t extaddr;
} block_io_t;

/* external memory base addresses */
#define RAM_BASE            0x20000000
#define ROM_BASE            0x30000000
#define BLOCK_MASK_4K       0x00000fff

/* globals */
static cache_mbox_t cache_mbox;
static uint32_t cache_line_mask;
static uint8_t cache[CACHE_SIZE];
static uint32_t buf[BUF_SIZE];

uint32_t cacheStart(void *code, cache_mbox_t *mbox, uint8_t *cache, uint32_t config1, uint32_t config2, uint32_t config3, uint32_t config4);

uint8_t readByte(uint32_t addr);
void writeByte(uint32_t addr, uint8_t val);
uint32_t readLong(uint32_t addr);
void writeLong(uint32_t addr, uint32_t val);

void readBlock(uint32_t extaddr, void *buf, uint32_t count);
void writeBlock(uint32_t extaddr, void *buf, uint32_t count);

void eraseFlashBlock(uint32_t extaddr);
void writeFlashBlock(uint32_t extaddr, void *buf, uint32_t count);

int main(void)
{
    extern char TESTDRIVER[];
    
    //LED_OFF();
    //DIRA |= LED_MASK;

    cache_line_mask = cacheStart(TESTDRIVER, &cache_mbox, cache, CACHE_CONFIG1, CACHE_CONFIG2, CACHE_CONFIG3, CACHE_CONFIG4);
    
    printf("mbox  %08x\n", (uint32_t)&cache_mbox);
    printf("cache %08x\n", (uint32_t)cache);
    printf("buf   %08x\n", (uint32_t)buf);
    
#ifdef FLASH_TEST

{
    int start, i, j;
    
    printf("Flash test\n");
    
    srand(CNT);
    start = rand();
    printf("Starting with %08x\n", start);
    for (i = 0, j = start; i < BUF_SIZE; ++i)
        buf[i] = j++;
    eraseFlashBlock(0);
    writeFlashBlock(0, buf, sizeof(buf));
    memset(buf, 0, sizeof(buf));
    readBlock(ROM_BASE, buf, sizeof(buf));
    for (i = 0, j = start; i < BUF_SIZE; ++i) {
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
        for (i = 0; i < BUF_SIZE; ++i, addr += sizeof(uint32_t))
            buf[i] = value++;
        if ((startAddr & BLOCK_MASK_4K) == 0) {
            //printf("Erasing %08x\n", ROM_BASE + startAddr);
            eraseFlashBlock(startAddr - ROM_BASE);
        }
        //printf("Writing %08x\n", ROM_BASE + startAddr);
        writeFlashBlock(startAddr - ROM_BASE, buf, sizeof(buf));
    }
    
    printf("Checking flash\n");
    addr = ROM_BASE;
    value = startValue;
    for (j = 0; j < BUF_COUNT; ++j) {
        //printf("Reading %08x\n", addr);
        readBlock(addr, buf, sizeof(buf));
        for (i = 0; i < BUF_SIZE; ++i, addr += sizeof(uint32_t))
            if (buf[i] != value++)
                printf("%08x: expected %08x, found %08x\n", addr, value - 1, buf[i]);
    }

    addr = ROM_BASE;
    value = startValue;
    printf("Filling flash inverted\n");
    for (j = 0; j < BUF_COUNT; ++j) {
        uint32_t startAddr = addr;
        for (i = 0; i < BUF_SIZE; ++i, addr += sizeof(uint32_t))
            buf[i] = ~value++;
        if ((startAddr & BLOCK_MASK_4K) == 0) {
            //printf("Erasing %08x\n", ROM_BASE + startAddr);
            eraseFlashBlock(startAddr - ROM_BASE);
        }
        //printf("Writing %08x\n", ROM_BASE + startAddr);
        writeFlashBlock(startAddr - ROM_BASE, buf, sizeof(buf));
    }
    
    addr = ROM_BASE;
    value = startValue;
    printf("Checking flash inverted\n");
    for (j = 0; j < BUF_COUNT; ++j) {
        //printf("Reading %08x\n", addr);
        readBlock(addr, buf, sizeof(buf));
        for (i = 0; i < BUF_SIZE; ++i, addr += sizeof(uint32_t))
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
    
    for (i = 0; i < BUF_SIZE; ++i)
        buf[i] = 0xbeef0000 + i;
    writeBlock(RAM_BASE, buf, sizeof(buf));
    
    memset(buf, 0, sizeof(buf));
    
    readBlock(RAM_BASE, buf, sizeof(buf));
    for (i = 0; i < BUF_SIZE; ++i)
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
        for (i = 0; i < BUF_SIZE; ++i, addr += sizeof(uint32_t))
            buf[i] = value++;
        //printf("Writing %08x\n", startAddr);
        writeBlock(startAddr, buf, sizeof(buf));
    }
    
    printf("Checking RAM\n");
    addr = RAM_BASE;
    value = startValue;
    for (j = 0; j < BUF_COUNT; ++j) {
        //printf("Reading %08x\n", addr);
        readBlock(addr, buf, sizeof(buf));
        for (i = 0; i < BUF_SIZE; ++i, addr += sizeof(uint32_t))
            if (buf[i] != value++)
                printf("%08x: expected %08x, found %08x\n", addr, value - 1, buf[i]);
    }

    printf("Filling RAM inverted\n");
    addr = RAM_BASE;
    value = startValue;
    for (j = 0; j < BUF_COUNT; ++j) {
        uint32_t startAddr = addr;
        for (i = 0; i < BUF_SIZE; ++i, addr += sizeof(uint32_t))
            buf[i] = ~value++;
        //printf("Writing %08x\n", startAddr);
        writeBlock(startAddr, buf, sizeof(buf));
    }
    
    printf("Checking RAM inverted\n");
    addr = RAM_BASE;
    value = startValue;
    for (j = 0; j < BUF_COUNT; ++j) {
        //printf("Reading %08x\n", addr);
        readBlock(addr, buf, sizeof(buf));
        for (i = 0; i < BUF_SIZE; ++i, addr += sizeof(uint32_t))
            if (buf[i] != ~value++)
                printf("%08x: expected %08x, found %08x\n", addr, ~(value - 1), buf[i]);
    }

    printf("done\n");
}

#endif

#ifdef CACHE_TEST

{
    int i;
    
    printf("cache test\n");
    
    for (i = 0; i < 32*1024; i += sizeof(uint32_t))
        writeLong(RAM_BASE + i, i);
        
    for (i = 0; i < 32*1024; i += sizeof(uint32_t)) {
        uint32_t data = readLong(RAM_BASE + i);
        if (data != i)
            printf("%08x: expected %08x, found %08x\n", RAM_BASE + i, i, data);
    }

    printf("done\n");
}
    
#endif

    return 0;
}

uint32_t cacheStart(void *code, cache_mbox_t *mbox, uint8_t *cache, uint32_t config1, uint32_t config2, uint32_t config3, uint32_t config4)
{
    cache_init_t params;
    params.mbox = mbox;
    params.cache = cache;
    params.config1 = config1;
    params.config2 = config2;
    params.config3 = config3;
    params.config4 = config4;
    mbox->cmd = 1;
    cognew(code, &params);
    while (mbox->cmd)
        ;
    return (uint32_t)params.mbox;
}

static void *getReadAddr(uint32_t addr)
{
    cache_mbox.cmd = (addr & ~CMD_MASK) | READ_CMD;
    while (cache_mbox.cmd)
        ;
    addr &= cache_line_mask;
    return (void *)(cache_mbox.addr + addr);
}

uint8_t readByte(uint32_t addr)
{
    return *(uint8_t *)getReadAddr(addr);
}

uint32_t readLong(uint32_t addr)
{
    return *(uint32_t *)getReadAddr(addr);
}

static void *getWriteAddr(uint32_t addr)
{
    cache_mbox.cmd = (addr & ~CMD_MASK) | WRITE_CMD;
    while (cache_mbox.cmd)
        ;
    addr &= cache_line_mask;
    return (void *)(cache_mbox.addr + addr);
}

void writeByte(uint32_t addr, uint8_t val)
{
    *(uint8_t *)getWriteAddr(addr) = val;
}

void writeLong(uint32_t addr, uint32_t val)
{
    *(uint32_t *)getWriteAddr(addr) = val;
}

void readBlock(uint32_t extaddr, void *buf, uint32_t count)
{
    block_io_t block_io;
    //printf("read %08x %08x %d\n", extaddr, (uint32_t)buf, count);
    block_io.hubaddr = buf;
    block_io.extaddr = extaddr;
    block_io.count = count;
    cache_mbox.cmd = ((uint32_t)&block_io << 8) | BLOCK_READ_CMD;
    //LED_ON();
    while (cache_mbox.cmd)
        ;
    //LED_OFF();
    //printf("read returned %08x\n", cache_mbox.addr);
    //usleep(1);
}

void writeBlock(uint32_t extaddr, void *buf, uint32_t count)
{
    block_io_t block_io;
    //printf("write %08x %08x %d\n", extaddr, (uint32_t)buf, count);
    block_io.hubaddr = buf;
    block_io.extaddr = extaddr;
    block_io.count = count;
    cache_mbox.cmd = ((uint32_t)&block_io << 8) | BLOCK_WRITE_CMD;
    //LED_ON();
    while (cache_mbox.cmd)
        ;
    //LED_OFF();
    //printf("write returned %08x\n", cache_mbox.addr);
    //usleep(1);
}

/* offset is the byte offset from the start of flash */
void eraseFlashBlock(uint32_t offset)
{
    //printf("erase %08x\n", offset);
    cache_mbox.cmd = (offset << 8) | ERASE_BLOCK_CMD;
    while (cache_mbox.cmd)
        ;
    //printf("erase returned %08x\n", cache_mbox.addr);
}

/* offset is the byte offset from the start of flash */
void writeFlashBlock(uint32_t offset, void *buf, uint32_t count)
{
    block_io_t block_io;
    //printf("flash %08x %08x %d\n", offset, (uint32_t)buf, count);
    block_io.hubaddr = buf;
    block_io.extaddr = offset;
    block_io.count = count;
    cache_mbox.cmd = ((uint32_t)&block_io << 8) | WRITE_DATA_CMD;
    while (cache_mbox.cmd)
        ;
    //printf("flash returned %08x\n", cache_mbox.addr);
}

