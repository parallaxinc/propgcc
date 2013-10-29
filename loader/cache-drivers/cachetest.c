#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <propeller.h>

#ifndef TESTDRIVER
#define TESTDRIVER      binary_rampage2_xcache_dat_start
#endif

#define WAY_WIDTH       2       // number of bits in the way offset (way count is 2^n)
#define INDEX_WIDTH     5       // number of bits in the index offset (index size is 2^n)
#define OFFSET_WIDTH    6       // number of bits in the line offset (line size is 2^n)

#define CACHE_SIZE      (512 + 8192)
#define CACHE_CONFIG1   0       // use defaults - ((WAY_WIDTH << 24) | (INDEX_WIDTH << 16) | (OFFSET_WIDTH << 8))
#define CACHE_CONFIG2   ((0 << 24) | (7 << 16) | (5 << 8) | 6)
#define CACHE_CONFIG3   0
#define CACHE_CONFIG4   0

#define BUF_SIZE        32

/*
  INIT_MBOX             = 0     ' cache line mask should be returned here
  INIT_CACHE            = 1		' base address in hub memory of tags and cache lines
  INIT_CONFIG_1         = 2     ' cache geometry
  INIT_CONFIG_2         = 3     ' driver specific configuration
  INIT_CONFIG_3         = 4     ' driver specific configuration
  INIT_CONFIG_4         = 5     ' driver specific configuration
  _INIT_SIZE            = 6
*/

typedef struct {
  void *mbox;
  void *cache;
  uint32_t config1;
  uint32_t config2;
  uint32_t config3;
  uint32_t config4;
} cache_init_t;

/*
  ' mailbox offsets
  MBOX_CMD              = 0
  MBOX_ADDR             = 1
  MBOX_EXTRA            = 2     ' extra space for debugging
  _MBOX_SIZE            = 2
*/

typedef struct {
  volatile uint32_t cmd;
  volatile uint32_t addr;
} cache_mbox_t;

/*
  ' cache access commands
  WRITE_CMD             = %10
  READ_CMD              = %11
*/

#define WRITE_CMD           0x00000002
#define READ_CMD            0x00000003

/*
  ' extended commands
  UNUSED_CMD            = %000_01  ' unused
  ERASE_BLOCK_CMD       = %001_01  ' only for flash
  WRITE_DATA_CMD        = %010_01  ' only for flash
  BLOCK_READ_CMD        = %011_01  ' only if BLOCK_IO is defined
  BLOCK_WRITE_CMD       = %100_01  ' only if BLOCK_IO is defined
  UNUSED2_CMD           = %101_01  ' unused
  REINIT_CACHE_CMD      = %110_01  ' only for SD cache driver to reinitialize the cache
  UNUSED3_CMD           = %111_01  ' unused
*/

#define UNUSED_CMD          0x00000001
#define ERASE_BLOCK_CMD     0x00000005
#define WRITE_DATA_CMD      0x00000009
#define BLOCK_READ_CMD      0x0000000d
#define BLOCK_WRITE_CMD     0x00000011
#define UNUSED2_CMD         0x00000015
#define REINIT_CACHE_CMD    0x00000019
#define UNUSED3_CMD         0x0000001d

/*
  CMD_MASK              = %11
*/

typedef struct {
    void *hubaddr;
    uint32_t count;
    uint32_t extaddr;
} block_io_t;

#define CMD_MASK            0x00000003

#define RAM_BASE            0x20000000
#define ROM_BASE            0x30000000

static cache_mbox_t cache_mbox;
static uint32_t cache_line_mask;
static uint8_t cache[CACHE_SIZE];
static uint32_t buf[BUF_SIZE];

uint32_t cacheStart(void *code, cache_mbox_t *mbox, uint8_t *cache, uint32_t config1, uint32_t config2, uint32_t config3, uint32_t config4);

uint8_t readByte(uint32_t addr);
uint32_t readLong(uint32_t addr);

void writeByte(uint32_t addr, uint8_t val);
void writeLong(uint32_t addr, uint32_t val);

void readBlock(uint32_t extaddr, void *buf, uint32_t count);
void writeBlock(uint32_t extaddr, void *buf, uint32_t count);

void eraseFlashBlock(uint32_t extaddr);
void writeFlashBlock(uint32_t extaddr, void *buf, uint32_t count);

int main(void)
{
    extern char TESTDRIVER[];
    uint32_t addr;
    int i, j;
    
    cache_line_mask = cacheStart(TESTDRIVER, &cache_mbox, cache, CACHE_CONFIG1, CACHE_CONFIG2, CACHE_CONFIG3, CACHE_CONFIG4);
    
    printf("mbox %08x\n", (uint32_t)&cache_mbox);
    printf("buf %08x\n", (uint32_t)buf);
    
#if 0

    srand(CNT);
    j = rand();
    printf("Starting with %08x\n", j);
    for (i = 0; i < BUF_SIZE; ++i)
        buf[i] = j++;
    eraseFlashBlock(0);
    writeFlashBlock(0, buf, sizeof(buf));
    memset(buf, 0, sizeof(buf));
    readBlock(ROM_BASE, buf, sizeof(buf));
    for (i = 0; i < BUF_SIZE; ++i)
        printf("buf[%d] = %08x\n", i, buf[i]);

#endif

#if 1

    for (i = 0; i < BUF_SIZE; ++i)
        buf[i] = i;
    writeBlock(RAM_BASE, buf, sizeof(buf));
    memset(buf, 0, sizeof(buf));
    readBlock(RAM_BASE, buf, sizeof(buf));
    for (i = 0; i < BUF_SIZE; ++i)
        printf("buf[%d] = %08x\n", i, buf[i]);

#endif
        
#if 0

    printf("Filling RAM\n");
    addr = RAM_BASE;
    for (j = 0; j < 8; ++j) {
        uint32_t startAddr = addr;
        for (i = 0; i < BUF_SIZE; ++i, addr += sizeof(uint32_t))
            buf[i] = addr;
        printf("Writing %08x\n", startAddr);
        writeBlock(startAddr, buf, sizeof(buf));
    }
    
    printf("Checking RAM\n");
    addr = RAM_BASE;
    for (j = 0; j < 8; ++j) {
        printf("Reading %08x\n", addr);
        readBlock(addr, buf, sizeof(buf));
        for (i = 0; i < BUF_SIZE; ++i, addr += sizeof(uint32_t))
            if (buf[i] != addr)
                printf("%08x: found %08x, expected %08x\n", addr, buf[i], addr);
    }

#endif

#if 0

    for (i = 0; i < 32*1024; i += sizeof(uint32_t))
        writeLong(RAM_BASE + i, i);
        
    for (i = 0; i < 32*1024; i += sizeof(uint32_t)) {
        uint32_t data = readLong(RAM_BASE + i);
        if (data != i)
            printf("%08x: expected %08x, found %08x\n", RAM_BASE + i, i, data);
    }
    
#endif

    printf("done\n");
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
    mbox->cmd = 0xffffffff;
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
    OUTA &= 1<<15;
    DIRA |= 1<<15;
    while (cache_mbox.cmd)
        ;
    OUTA |= 1<<15;
    //printf("read returned %08x\n", cache_mbox.addr);
    usleep(1);
}

void writeBlock(uint32_t extaddr, void *buf, uint32_t count)
{
    block_io_t block_io;
    //printf("write %08x %08x %d\n", extaddr, (uint32_t)buf, count);
    block_io.hubaddr = buf;
    block_io.extaddr = extaddr;
    block_io.count = count;
    cache_mbox.cmd = ((uint32_t)&block_io << 8) | BLOCK_WRITE_CMD;
    OUTA &= 1<<15;
    DIRA |= 1<<15;
    while (cache_mbox.cmd)
        ;
    OUTA |= 1<<15;
    //printf("write returned %08x\n", cache_mbox.addr);
    usleep(1);
}

/* offset is the byte offset from the start of flash */
void eraseFlashBlock(uint32_t offset)
{
    printf("erase %08x\n", offset);
    cache_mbox.cmd = (offset << 8) | ERASE_BLOCK_CMD;
    while (cache_mbox.cmd)
        ;
    printf("erase returned %08x\n", cache_mbox.addr);
}

/* offset is the byte offset from the start of flash */
void writeFlashBlock(uint32_t offset, void *buf, uint32_t count)
{
    block_io_t block_io;
    printf("flash %08x %08x %d\n", offset, (uint32_t)buf, count);
    block_io.hubaddr = buf;
    block_io.extaddr = offset;
    block_io.count = count;
    cache_mbox.cmd = ((uint32_t)&block_io << 8) | WRITE_DATA_CMD;
    while (cache_mbox.cmd)
        ;
    printf("flash returned %08x\n", cache_mbox.addr);
}

