#ifndef __PROPDEV_H__
#define __PROPDEV_H__

#include <stdint.h>
#include "dosfs.h"

#ifdef __cplusplus
extern "C" 
{
#endif

#define coginit(id, code, param)    __builtin_coginit(                                  \
                                            (((uint32_t)(param) << 16) & 0xfffc0000)    \
                                        |   (((uint32_t)(code)  <<  2) & 0x0003fff0)    \
                                        |   (((id)                   ) & 0x0000000f))
                                    
#define cognew(code, param)         coginit(0x8, (code), (param))
                                    
#define cogstop(a)                  __builtin_cogstop((a))

#define waitcnt(a)                  __builtin_waitcnt((a), _CNT)

#define TERM_IN		0x01
#define TERM_OUT	0x02
#define TERM_IO		(TERM_IN | TERM_OUT)

int InitSerialTerm(int rxpin, int txpin, int mode, int baudrate, int flags);
int InitKeyboardTerm(int dpin, int cpin);
int InitTvTerm(int tvpin);
int InitTv2Term(int tvpin);
int InitFileIO(void);
int InitC3FileIO(int retries);

int _inbyte(void);
int _outbyte(int ch);

_ssize_t _term_read(void *buf, size_t bytes);
_ssize_t _term_write(const void *buf, size_t bytes);

extern int (*_term_getc_p)(void);
extern int (*_term_putc_p)(int ch);
extern int (*_error_putc_p)(int ch);

extern _ssize_t (*_term_read_p)(void *buf, size_t bytes);
extern _ssize_t (*_term_write_p)(const void *buf, size_t bytes);
extern _ssize_t (*_error_write_p)(const void *buf, size_t bytes);
extern _ssize_t (*_file_read_p)(int fd, void *buf, size_t bytes);
extern _ssize_t (*_file_write_p)(int fd, const void *buf, size_t bytes);

#define _FS_MAX_FILES   4
#define _FS_BASE_FD     3

typedef struct {
    FILEINFO file;
    int inUse;
} _fs_file_t;

typedef struct {
    int initialized;
    VOLINFO vinfo;
    _fs_file_t files[_FS_MAX_FILES];
    char scratch[SECTOR_SIZE];
} _fs_state_t;

#ifdef __cplusplus
}
#endif

#endif
