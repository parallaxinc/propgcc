#ifndef __P2_LOADIMAGE_H__
#define __P2_LOADIMAGE_H__

#include <stdint.h>

int p2_HardwareFound(int *pVersion);
int p2_InitLoader(int baudRate);
int p2_LoadImage(uint8_t *imageBuf, uint32_t addr, uint32_t size);
int p2_StartImage(uint32_t addr, uint32_t param);
int p2_StartCog(int id, uint32_t addr, uint32_t param);

#endif
