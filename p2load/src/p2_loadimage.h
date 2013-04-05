#ifndef __P2_LOADIMAGE_H__
#define __P2_LOADIMAGE_H__

#include <stdint.h>

int p2_HardwareFound(int *pVersion);
int p2_LoadImage(uint8_t *imageBuf, int imageSize, uint32_t cogImage, uint32_t stackTop, int baudRate);

#endif
