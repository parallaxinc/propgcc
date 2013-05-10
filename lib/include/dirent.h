/**
 * @file include/dirent.h
 * @brief Provides directory operations API declarations.
 */
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

/**
 * @brief Defines the dirent.h DIR struct
 *
 * DIR fields:
 * @li uint32_t currentcluster; // current cluster in dir
 * @li uint8_t currentsector;   // current sector in cluster
 * @li uint8_t currententry;    // current dir entry in sector
 * @li uint8_t *scratch;        // ptr to user-supplied sector buffer
 * @li uint8_t flags;           // internal DOSFS flags
 */
typedef struct DIRstruct {
    uint32_t currentcluster; // current cluster in dir
    uint8_t currentsector;   // current sector in cluster
    uint8_t currententry;    // current dir entry in sector
    uint8_t *scratch;        // ptr to user-supplied sector buffer
    uint8_t flags;           // internal DOSFS flags
} DIR;

/**
 * @brief Defines the dirent.h dirent struct
 *
 * dirent fields:
 * @li uint8_t name[11];        // filename
 * @li uint8_t attr;            // attributes
 * @li uint8_t reserved;        // reserved, must be 0
 * @li uint8_t crttimetenth;    // create time, 10ths of a second (0-199)
 * @li uint8_t crttime_0;       // creation time byte 0
 * @li uint8_t crttime_1;       // creation time byte 1
 * @li uint8_t crtdate_0;       // creation date byte 0
 * @li uint8_t crtdate_1;       // creation date byte 1
 * @li uint8_t lstaccdate_l;    // last access date byte 0
 * @li uint8_t lstaccdate_h;    // last access date byte 1
 * @li uint8_t startcluster_2;  // first cluster, byte 2 (FAT32)
 * @li uint8_t startcluster_3;  // first cluster, byte 3 (FAT32)
 * @li uint8_t wrttime_0;       // last write time byte 0
 * @li uint8_t wrttime_1;       // last write time byte 1
 * @li uint8_t wrtdate_0;       // last write date byte 0
 * @li uint8_t wrtdate_1;       // last write date byte 1
 * @li uint8_t startcluster_0;  // first cluster, byte 0
 * @li uint8_t startcluster_1;  // first cluster, byte 1
 * @li uint8_t filesize_0;      // file size, byte 0
 * @li uint8_t filesize_1;      // file size, byte 1
 * @li uint8_t filesize_2;      // file size, byte 2
 * @li uint8_t filesize_3;      // file size, byte 3
 * @li char d_name[13];         // filename in file.ext format
 */
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

/**
 * Opens a directory.
 *
 * The opendir() function opens the directory named by dirname, associates a
 * directory stream with it, and returns a pointer to be used to identify
 * the directory stream in subsequent operations.  The pointer NULL is
 * returned if dirname cannot be accessed or if it cannot malloc(3) enough
 * memory to hold the whole thing.
 *
 * @param path The directory to be opened.
 * @returns DIR struct pointer to be used in subsequent operations. Returns NULL on error.
 */
DIR *opendir(const char *path);

/**
 * Returns pointer to the next directory entry.
 *
 * The readdir() function returns a pointer to the next directory entry.
 * It returns NULL upon reaching the end of the directory or detecting an
 * invalid seekdir() operation.
 *
 * Sample code searches a directory for entry "name":
 *
 * @code

  len = strlen(name);
  dirp = opendir(".");
  while ((dp = readdir(dirp)) != NULL) {
    if (!strcmp(dp->d_name, name)) {
      closedir(dirp);
      return FOUND;
    }
  }
  (void)closedir(dirp);
  return NOT_FOUND;

 * @endcode
 *
 * @pre Parameter dirp must be successfully opened by opendir.
 *
 * @param[in,out] dirp Pointer to the DIR struct.
 * @returns Pointer to the next directory entry, or NULL if end of directory.
 */
struct dirent *readdir(DIR *dirp);

/**
 * Closes a directory.
 *
 * The closedir() function closes the named directory stream and frees the
 * structure associated with the dirp pointer, returning 0 on success. On
 * failure, -1 is returned and the global variable errno is set to indicate
 * the error.
 *
 * @param dirp Pointer to directory to close.
 * @returns Zero on success or -1 on failure and sets errno.
 */
int closedir(DIR *dirp);

#if defined(__cplusplus)
}
#endif

#endif
