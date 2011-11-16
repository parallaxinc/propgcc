#ifndef __FATREAD_H__
#define __FATREAD_H__

#include <stdint.h>

typedef struct {
    int type;
    uint32_t bytesPerSector;
    uint32_t sectorsPerCluster;
    uint32_t rootDirectoryCluster;
    uint32_t firstRootDirectorySector;
    uint32_t rootDirectorySectorCount;
    uint32_t rootEntryCount;
    uint32_t firstFATSector;
    uint32_t firstDataSector;
    uint32_t clusterCount;
    uint32_t endOfClusterChain;
} VolumeInfo;

typedef struct {
    VolumeInfo *vinfo;
    uint32_t cluster;
    uint32_t sector;
    uint32_t sectorsRemainingInCluster;
    uint32_t bytesRemaining;
} FileInfo;

#define SECTOR_WIDTH	9
#define SECTOR_SIZE     (1 << SECTOR_WIDTH)

int MountFS(uint8_t *buffer, VolumeInfo *vinfo);
int FindFile(uint8_t *buffer, VolumeInfo *vinfo, const char *name, FileInfo *finfo);
int GetNextFileSector(FileInfo *finfo, uint8_t *buffer, uint32_t *pCount);
int GetFATEntry(VolumeInfo *vinfo, uint8_t *buffer, uint32_t cluster, uint32_t *pEntry);

#endif
