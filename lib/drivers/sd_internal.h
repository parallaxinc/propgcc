#ifndef _SD_INTERNAL_H
#define _SD_INTERNAL_H

#include "dosfs.h"

extern int dfs_mountflag;
extern VOLINFO dfs_volinfo;
extern char dfs_currdir[MAX_PATH];
extern __attribute__((section(".hub"))) uint8_t dfs_scratch[512];

#endif
