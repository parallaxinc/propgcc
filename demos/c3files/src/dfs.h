#ifndef __DFS_H__
#define __DFS_H__

#include "dosfs.h"

// Device Driver Functions
int File_fopen(FILE *fp, const char *str, const char *mode);
int File_fclose(FILE *fp);
int File_write(FILE *fp, unsigned char *buf, int count);
int File_read(FILE *fp, unsigned char *buf, int count);
int File_fseek(FILE *fp, long int offset, int origin);
int File_remove(const char *str);

// Basic file I/O functions
uint32_t dfs_read(PFILEINFO fileinfo, uint8_t *buffer, uint32_t num);
uint32_t dfs_write(PFILEINFO fileinfo, uint8_t *buffer, uint32_t num);
uint32_t dfs_mount(uint8_t *parms);
PFILEINFO dfs_open(const char *fname, const char *mode);
PDIRINFO dfs_opendir(char *path);
void dfs_closedir(PDIRINFO dirinfo);
PDIRENT dfs_readdir(PDIRINFO dirinfo);
int dfs_close(PFILEINFO fileinfo);
int dfs_remove(char *fname);
int dfs_mkdir(const char *path);
void dfs_perror(int errnum, char *str);
int dfs_chdir(const char *path1);
char *dfs_getcd(void);

#endif
