#ifndef __SDIO_H__
#define __SDIO_H__

#include <stdint.h>

int SD_Init(volatile uint32_t *mbox, int retries);
int SD_ReadSector(uint32_t sector, void *buffer);

#endif
