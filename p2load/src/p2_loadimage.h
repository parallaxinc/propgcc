#ifndef __P2_LOADIMAGE_H__
#define __P2_LOADIMAGE_H__

#include <stdint.h>

/* loader commands */
#define CMD_LOAD    1
#define CMD_START   2
#define CMD_COGINIT 3
#define CMD_FLASH   4

/* load command packet */
typedef struct {
    uint32_t cmd;   // CMD_LOAD
    uint32_t addr;
    uint32_t count;
} LoadCmd;

/* start or coginit command packet */
typedef struct {
    uint32_t cmd;   // CMD_START or CMD_COGINIT
    uint32_t addr;
    uint32_t param;
} StartCmd;

/* flash command packet */
typedef struct {
    uint32_t cmd;   // CMD_FLASH
    uint32_t flashaddr;
    uint32_t hubaddr;
} FlashCmd;

int p2_HardwareFound(int *pVersion);
int p2_InitLoader(int baudRate);
int p2_LoadImage(uint8_t *imageBuf, uint32_t addr, uint32_t size);
int p2_SendBuffer(uint8_t *imageBuf, uint32_t size);
int p2_StartImage(int id, uint32_t addr, uint32_t param);
int p2_StartCog(int id, uint32_t addr, uint32_t param);
int p2_Flash(uint32_t flashaddr, uint32_t hubaddr, uint32_t count);
int p2_FlashBooter(int baudRate);
int p2_FlashBuffer(uint8_t *buffer, int size, uint32_t flashaddr, int progress);
int p2_FlashFile(char *file, uint32_t flashaddr);

#endif
