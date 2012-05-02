#include <stdio.h>
#include <string.h>
#include "fatread.h"
#include "sdio.h"
#include "debug.h"

//#define FSDEBUG

#define MBR_PARTITIONS              0x1be
#define MBR_SIGNATURE               0x1fe
#define MAX_PARTITIONS              4

#define MBR_SIGNATURE_VALUE         0xaa55

#define PART_STATUS                 0x00
#define PART_TYPE                   0x04
#define PART_FIRST_SECTOR           0x08
#define PART_SECTOR_COUNT           0x0c
#define PART_SIZE                   16

#define BOOT_BYTES_PER_SECTOR       0x00b
#define BOOT_SECTORS_PER_CLUSTER    0x00d
#define BOOT_RESERVED_SECTOR_COUNT  0x00e
#define BOOT_NUMBER_OF_FATS         0x010
#define BOOT_ROOT_ENTRY_COUNT       0x011
#define BOOT_TOTAL_SECTOR_COUNT     0x013
#define BOOT_FAT_SIZE               0x016

#define BOOT_VOLUME_LABEL           0x02b
#define BOOT_FILE_SYSTEM_TYPE       0x036

#define BOOT_TOTAL_SECTOR_COUNT_32  0x020
#define BOOT_FAT_SIZE_32            0x024
#define BOOT_ROOT_CLUSTER_32        0x02c
#define BOOT_VOLUME_LABEL_32        0x047
#define BOOT_FILE_SYSTEM_TYPE_32    0x052

#define TYPE_UNKNOWN                0
#define TYPE_FAT12                  1
#define TYPE_FAT16                  2
#define TYPE_FAT32                  3

#define ENTRY_NAME                  0x00
#define ENTRY_ATTRIBUTES            0x0b
#define ENTRY_CLUSTER_HIGH          0x14
#define ENTRY_CLUSTER_LOW           0x1a
#define ENTRY_FILESIZE              0x1c
#define ENTRY_SIZE                  32

#define ATTR_READ_ONLY              0x01
#define ATTR_HIDDEN                 0x02
#define ATTR_SYSTEM                 0x04
#define ATTR_VOLUME_ID              0x08
#define ATTR_DIRECTORY              0x10
#define ATTR_ARCHIVE                0x20
#define ATTR_LONG_NAME              0x0f
#define ATTR_LONG_NAME_MASK         0x3f

#define NAME_SIZE                   11

static uint16_t GetU16(uint8_t *buffer, int offset);
static uint32_t GetU32(uint8_t *buffer, int offset);

int MountFS(uint8_t *buffer, VolumeInfo *vinfo)
{
    uint8_t  sectorsPerCluster, numberOfFATs;
    uint16_t bytesPerSector, reservedSectorCount, rootEntryCount;
    uint32_t start, firstFATSector, firstRootDirectorySector, rootDirectorySectorCount;
    uint32_t firstDataSector, dataSectorCount, clusterCount, totalSectorCount, FATSize;
    uint32_t endOfClusterChain;
#ifdef FSDEBUG
    char *volumeLabel;
#endif
    int type;
       
    if (SD_ReadSector(0, buffer) != 0) {
        DPRINTF("SD_ReadSector 0 failed\n");
        return -1;
    }
    
    // check to see if there is a master boot record
    if (strncmp((char *)&buffer[BOOT_FILE_SYSTEM_TYPE], "FAT12", 5) != 0
    &&  strncmp((char *)&buffer[BOOT_FILE_SYSTEM_TYPE], "FAT16", 5) != 0
    &&  strncmp((char *)&buffer[BOOT_FILE_SYSTEM_TYPE_32], "FAT32", 5) != 0) {

        // get the first partition information
#ifdef FSDEBUG
        uint8_t status = buffer[MBR_PARTITIONS + PART_STATUS];
        uint32_t size = GetU32(buffer, MBR_PARTITIONS + PART_SECTOR_COUNT);
#endif
        start = GetU32(buffer, MBR_PARTITIONS + PART_FIRST_SECTOR);
#ifdef FSDEBUG
        DPRINTF("status: %02x, start: %08x, size %08x\n", status, start, size);
#endif            
        // get the boot sector of the first partition
        if (SD_ReadSector(start, buffer) != 0) {
            DPRINTF("SD_ReadSector %d failed\n", start);
            return -1;
        }
    }
    
    // no master boot record
    else
        start = 0;
    
    bytesPerSector = GetU16(buffer, BOOT_BYTES_PER_SECTOR);
    sectorsPerCluster = buffer[BOOT_SECTORS_PER_CLUSTER];
    reservedSectorCount = GetU16(buffer, BOOT_RESERVED_SECTOR_COUNT);
    numberOfFATs = buffer[BOOT_NUMBER_OF_FATS];
    rootEntryCount = GetU16(buffer, BOOT_ROOT_ENTRY_COUNT);
    if ((totalSectorCount = GetU16(buffer, BOOT_TOTAL_SECTOR_COUNT)) == 0)
        totalSectorCount = GetU32(buffer, BOOT_TOTAL_SECTOR_COUNT_32);
    if ((FATSize = GetU16(buffer, BOOT_FAT_SIZE)) == 0)
        FATSize = GetU32(buffer, BOOT_FAT_SIZE_32);
    firstFATSector = start + reservedSectorCount;
    firstRootDirectorySector = firstFATSector + numberOfFATs * FATSize; // recomputed for FAT32 later
    rootDirectorySectorCount = (rootEntryCount * ENTRY_SIZE + bytesPerSector - 1) / bytesPerSector;
    firstDataSector = firstRootDirectorySector + rootDirectorySectorCount;
    dataSectorCount = totalSectorCount - reservedSectorCount - numberOfFATs * FATSize - rootDirectorySectorCount;
    clusterCount = dataSectorCount / sectorsPerCluster;
    
    // these magic numbers come from a Microsoft paper on the FAT32 File System
    if (clusterCount < 4085) {
        type = TYPE_FAT12;
        endOfClusterChain = 0x00000ff8;
    }
    else if (clusterCount < 65525) {
        type = TYPE_FAT16;
        endOfClusterChain = 0x0000fff8;
    }
    else {
        type = TYPE_FAT32;
        endOfClusterChain = 0xfffffff8;
    }
        
    switch (type) {
    case TYPE_FAT12:
    case TYPE_FAT16:
#ifdef FSDEBUG
        volumeLabel = (char *)&buffer[BOOT_VOLUME_LABEL];
#endif
        vinfo->rootDirectoryCluster = 0;
        break;
    case TYPE_FAT32:
#ifdef FSDEBUG
        volumeLabel = (char *)&buffer[BOOT_VOLUME_LABEL_32];
#endif
        vinfo->rootDirectoryCluster = GetU32(buffer, BOOT_ROOT_CLUSTER_32);
        firstRootDirectorySector = firstDataSector + (vinfo->rootDirectoryCluster - 2) * sectorsPerCluster;
        rootDirectorySectorCount = sectorsPerCluster;
        break;
    }
    
    vinfo->type = type;
    vinfo->bytesPerSector = bytesPerSector;
    vinfo->sectorsPerCluster = sectorsPerCluster;
    vinfo->firstRootDirectorySector = firstRootDirectorySector;
    vinfo->rootDirectorySectorCount = rootDirectorySectorCount;
    vinfo->rootEntryCount = rootEntryCount;
    vinfo->firstFATSector = firstFATSector;
    vinfo->firstDataSector = firstDataSector;
    vinfo->clusterCount = clusterCount;
    vinfo->endOfClusterChain = endOfClusterChain;
    
#ifdef FSDEBUG
    DPRINTF("label:                    %-11.11s\n", volumeLabel);
    DPRINTF("type:                     ");
    switch (type) {
    case TYPE_FAT12:
        DPRINTF("FAT12\n");
        break;
    case TYPE_FAT16:
        DPRINTF("FAT16\n");
        break;
    case TYPE_FAT32:
        DPRINTF("FAT32\n");
        break;
    default:
        DPRINTF("UNKNOWN\n");
        break;
    }
    DPRINTF("bytesPerSector:           %d\n", bytesPerSector);
    DPRINTF("sectorsPerCluster:        %d\n", sectorsPerCluster);
    DPRINTF("reservedSectorCount:      %d\n", reservedSectorCount);
    DPRINTF("numberOfFATs:             %d\n", numberOfFATs);
    DPRINTF("rootEntryCount:           %d\n", rootEntryCount);
    DPRINTF("totalSectorCount:         %d\n", totalSectorCount);
    DPRINTF("FATSize:                  %d\n", FATSize);
    DPRINTF("firstRootDirectorySector: %d\n", firstRootDirectorySector);
    DPRINTF("rootDirectorySectorCount: %d\n", rootDirectorySectorCount);
    DPRINTF("firstFATSector:           %d\n", firstFATSector);
    DPRINTF("firstDataSector:          %d\n", firstDataSector);
    DPRINTF("dataSectorCount:          %d\n", dataSectorCount);
    DPRINTF("clusterCount:             %d\n", clusterCount);
#endif
    
    return 0;
}

int FindFile(uint8_t *buffer, VolumeInfo *vinfo, const char *name, FileInfo *finfo)
{
    uint32_t cluster, sector, count;
    int i, j;
    
    // start with the first root directory sector
    cluster = vinfo->rootDirectoryCluster;
    sector = vinfo->firstRootDirectorySector;
    count = vinfo->rootDirectorySectorCount;
    
    // loop through all directory clusters
    for (;;) {
    
        // loop through each sector of the current cluster
        for (j = 0; j < count; ++j) {
        
            // get the next sector in this cluster
            if (SD_ReadSector(sector + j, buffer) != 0) {
                DPRINTF("SD_ReadSector %d failed\n", sector + j);
                return -1;
            }

            // find a file in this directory sector
            for (i = 0; i < SECTOR_SIZE; i += ENTRY_SIZE) {
                uint8_t flag = buffer[i + ENTRY_NAME];
                uint32_t firstCluster;
                uint8_t attr;
                switch (flag) {
                case 0xe5:
                case 0x00:
                    break;
                default:
                    attr = buffer[i + ENTRY_ATTRIBUTES];
                    firstCluster = (GetU16(buffer, i + ENTRY_CLUSTER_HIGH) << 16) | GetU16(buffer, i + ENTRY_CLUSTER_LOW);
                    if ((attr & ATTR_LONG_NAME_MASK) != ATTR_LONG_NAME
                    && !(attr & ATTR_HIDDEN)
                    && !(attr & ATTR_VOLUME_ID)) {
                        if (strncmp(name, (char *)&buffer[i + ENTRY_NAME], NAME_SIZE) == 0) {
                            finfo->vinfo = vinfo;
                            finfo->cluster = firstCluster;
                            finfo->sector = vinfo->firstDataSector + (firstCluster - 2) * vinfo->sectorsPerCluster;
                            finfo->sectorsRemainingInCluster = vinfo->sectorsPerCluster;
                            finfo->bytesRemaining = GetU32(buffer, i + ENTRY_FILESIZE);
                            return 0;
                        }
                    }
                    break;
                }
                
                // check for the end of the directory
                if (flag == 0x00)
                    return -1;
            }
        }
        
        // move ahead to the next cluster
        if (!cluster)
            break;
            
        // get the next cluster number
        if (GetFATEntry(vinfo, buffer, cluster, &cluster) != 0)
            return -1;
            
        // check for the end of the cluster chain
        if (cluster < 2)
            break;
            
        // setup to process the next cluster
        sector = vinfo->firstDataSector + (cluster - 2) * vinfo->sectorsPerCluster;
        count = vinfo->sectorsPerCluster;
    }
    
    return -1;
}

int GetNextFileSector(FileInfo *finfo, uint8_t *buffer, uint32_t *pCount)
{
    // check to see if we're at the end of the file
    if (finfo->bytesRemaining == 0)
        return -1;
    
    // check to see if we're at the end of the cluster
    if (finfo->sectorsRemainingInCluster == 0) {
        VolumeInfo *vinfo = finfo->vinfo;
        uint32_t next;
        
        // check to see if this is the last cluster
        if (finfo->cluster == 0)
            return -1;
            
        // get the next cluster number
        if (GetFATEntry(vinfo, buffer, finfo->cluster, &next) != 0) {
            DPRINTF("GetFATEntry %d failed\n", finfo->cluster);
            return -1;
        }
            
        // check for the end of the cluster chain
        if (next >= vinfo->endOfClusterChain) {
            finfo->cluster = 0;
            return -1;
        }
            
        // setup the next cluster
        finfo->cluster = next;
        finfo->sector = vinfo->firstDataSector + (next - 2) * vinfo->sectorsPerCluster;
        finfo->sectorsRemainingInCluster = vinfo->sectorsPerCluster;
    }
        
    // get the next sector of the file
    if (SD_ReadSector(finfo->sector, buffer) != 0) {
        DPRINTF("SD_ReadSector %d failed\n", finfo->sector);
        return 1;
    }
    
    // move to the next sector in the cluster
    ++finfo->sector;
    --finfo->sectorsRemainingInCluster;
    
    // return the current count and update the number of bytes remaining
    if ((*pCount = finfo->bytesRemaining) > SECTOR_SIZE)
        *pCount = SECTOR_SIZE;
    finfo->bytesRemaining -= *pCount;
    
    return 0;
}

int GetFATEntry(VolumeInfo *vinfo, uint8_t *buffer, uint32_t cluster, uint32_t *pEntry)
{
    uint32_t sector, offset;
    
    // determine that offset based on the fat type
    switch (vinfo->type) {
    case TYPE_FAT12:
        return -1; // BUG: fix this
        break;
    case TYPE_FAT16:
        offset = cluster * sizeof(uint16_t);
        break;
    case TYPE_FAT32:
        offset = cluster * sizeof(uint32_t);
        break;
    default:
        return -1;
    }
    
    // determine the sector number and offset
    sector = vinfo->firstFATSector + (offset / vinfo->bytesPerSector);
    offset %= vinfo->bytesPerSector;
    
    // get the sector containing the fat entry
    if (SD_ReadSector(sector, buffer) != 0) {
        DPRINTF("SD_ReadSector %d failed\n", sector);
        return -1;
    }
    
    // get the fat entry
    switch (vinfo->type) {
    case TYPE_FAT12:
        // BUG: fix this!
        break;
    case TYPE_FAT16:
        *pEntry = *(uint16_t *)(buffer + offset);
        break;
    case TYPE_FAT32:
        *pEntry = *(uint32_t *)(buffer + offset) & 0x0fffffff;
        break;
    }
    
    return 0;
}

static uint16_t GetU16(uint8_t *buffer, int offset)
{
    return (buffer[offset + 1] << 8)
         |  buffer[offset];
}

static uint32_t GetU32(uint8_t *buffer, int offset)
{
    return (buffer[offset + 3] << 24)
         | (buffer[offset + 2] << 16)
         | (buffer[offset + 1] << 8)
         |  buffer[offset];
}
