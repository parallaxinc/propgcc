#ifndef __DB_VMDEBUG_H__
#define __DB_VMDEBUG_H__

#include "db_types.h"

void DecodeFunction(uint16_t offset, const uint8_t *code, int len);
int DecodeInstruction(uint16_t base, const uint8_t *code, const uint8_t *lc);

#endif
