#ifndef __P1IMAGE_H__
#define __P1IMAGE_H__

#include <stdint.h>
#include "config.h"

void p1_UpdateHeader(BoardConfig *config, uint8_t *imageBuf, uint32_t imageSize);
void p1_UpdateChecksum(uint8_t *imagebuf, int imageSize);
int p1_LoadImage(uint8_t *imageBuf, int imageSize, uint32_t cogImage, uint32_t stackTop, int baudRate);

#endif

