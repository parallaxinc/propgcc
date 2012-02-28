#ifndef _DIRENT_H
#define _DIRENT_H

#if defined(__cplusplus)
extern "C" {
#endif

#define ATTR_READ_ONLY  0x01
#define ATTR_HIDDEN     0x02
#define ATTR_SYSTEM     0x04
#define ATTR_VOLUME_ID  0x08
#define ATTR_DIRECTORY  0x10
#define ATTR_ARCHIVE    0x20

typedef struct DIRstruct {
    uint32_t currentcluster; // current cluster in dir
    uint8_t currentsector;   // current sector in cluster
    uint8_t currententry;    // current dir entry in sector
    uint8_t *scratch;        // ptr to user-supplied sector buffer
    uint8_t flags;           // internal DOSFS flags
} DIR;

struct dirent {
    uint8_t name[11];        // filename
    uint8_t attr;            // attributes
    uint8_t reserved;        // reserved, must be 0
    uint8_t crttimetenth;    // create time, 10ths of a second (0-199)
    uint8_t crttime_0;       // creation time byte 0
    uint8_t crttime_1;       // creation time byte 1
    uint8_t crtdate_0;       // creation date byte 0
    uint8_t crtdate_1;       // creation date byte 1
    uint8_t lstaccdate_l;    // last access date byte 0
    uint8_t lstaccdate_h;    // last access date byte 1
    uint8_t startcluster_2;  // first cluster, byte 2 (FAT32)
    uint8_t startcluster_3;  // first cluster, byte 3 (FAT32)
    uint8_t wrttime_0;       // last write time byte 0
    uint8_t wrttime_1;       // last write time byte 1
    uint8_t wrtdate_0;       // last write date byte 0
    uint8_t wrtdate_1;       // last write date byte 1
    uint8_t startcluster_0;  // first cluster, byte 0
    uint8_t startcluster_1;  // first cluster, byte 1
    uint8_t filesize_0;      // file size, byte 0
    uint8_t filesize_1;      // file size, byte 1
    uint8_t filesize_2;      // file size, byte 2
    uint8_t filesize_3;      // file size, byte 3
    char d_name[13];         // filename in file.ext format
};

DIR *opendir(const char *path);
struct dirent *readdir(DIR *dir);
int closedir(DIR *dir);

#if defined(__cplusplus)
}
#endif

#endif
