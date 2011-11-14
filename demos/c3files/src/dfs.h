#ifndef __DFS_H__
#define __DFS_H__

#include "dosfs.h"

// Basic file I/O functions
char *dfs_getcd(void);
uint32_t dfs_mount(void);
int dfs_mkdir(const char *path);
int dfs_chdir(const char *path1);
PDIRINFO dfs_opendir(char *path);
void dfs_closedir(PDIRINFO dirinfo);
PDIRENT dfs_readdir(PDIRINFO dirinfo);

#endif
