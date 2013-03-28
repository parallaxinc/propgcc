#ifndef __P2IMAGE_H__
#define __P2IMAGE_H__

#include <stdint.h>
#include "config.h"

int p2_HardwareFound(int *pVersion);
void p2_UpdateHeader(BoardConfig *config, uint8_t *imageBuf, uint32_t imageSize);
void p2_UpdateChecksum(uint8_t *imagebuf, int imageSize);
int p2_LoadImage(uint8_t *imageBuf, int imageSize, uint32_t cogImage, uint32_t stackTop, int baudRate);
int p2_FlashImage(uint8_t *imageBuf, int imageSize, uint32_t cogImage, uint32_t stackTop, int baudRate);

#endif
