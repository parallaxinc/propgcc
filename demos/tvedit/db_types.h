/* db_types.h - type definitions for a simple virtual machine
 *
 * Copyright (c) 2009-2012 by David Michael Betz.  All rights reserved.
 *
 */

#ifndef __DB_TYPES_H__
#define __DB_TYPES_H__

/**********/
/* Common */
/**********/

#define VMTRUE      1
#define VMFALSE     0

typedef struct VMDIR VMDIR;
typedef struct VMDIRENT VMDIRENT;

/*******/
/* MAC */
/*******/

#ifdef MAC

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* sizeof(long) == sizeof(void *) on the Mac */
typedef int32_t VMVALUE;
typedef uint32_t VMUVALUE;
typedef double VMFLOAT;
typedef void **VMHANDLE;

#define ALIGN_MASK              3

#define FLASH_SPACE
#define DATA_SPACE

#define VMCODEBYTE(p)           *(uint8_t *)(p)

#define ANSI_FILE_IO

#endif  // MAC

/*********/
/* LINUX */
/*********/

#ifdef LINUX

#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef int32_t VMVALUE;
typedef uint32_t VMUVALUE;
typedef double VMFLOAT;
typedef void **VMHANDLE;

#define ALIGN_MASK              3

#define FLASH_SPACE
#define DATA_SPACE

#define VMCODEBYTE(p)           *(uint8_t *)(p)

#define ANSI_FILE_IO

#endif  // LINUX

/**********/
/* XGSPIC */
/**********/

#ifdef XGSPIC

//#include "FSIO.h"

#define XGS_COMMON
#define PIC_COMMON

#endif  // XGSPIC

/****************/
/* CHAMELEONPIC */
/****************/

#ifdef CHAMELEONPIC

#define PIC_COMMON

#endif  // CHAMELEONPIC

/**************/
/* PIC common */
/**************/

#ifdef PIC_COMMON

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <libpic30.h>
//#include "ebasic_pic24/MDD File System/FSIO.h"

typedef int16_t VMVALUE;
typedef uint16_t VMUVALUE;
typedef float VMFLOAT;
typedef void **VMHANDLE;

typedef void VMFILE;

#if 0
typedef FSFILE VMFILE;

struct VMDIR {
    SearchRec rec;
    int first;
};

struct VMDIRENT {
    char name[FILE_NAME_SIZE_8P3 + 2];
};
#endif

#ifdef PIC24
#include <p24hxxxx.h>
#endif
#ifdef dsPIC33
#include <p33fxxxx.h>
#endif

#define ALIGN_MASK              1

#define FLASH_SPACE             const
#define DATA_SPACE              __attribute__((far))
#define VMCODEBYTE(p)           *(p)

#define snprintf    __simple_snprintf
#define vsnprintf   __simple_vsnprintf

#endif  // PIC_COMMON

/*****************/
/* PROPELLER_GCC */
/*****************/

#ifdef PROPELLER_GCC

#include <string.h>
#include <stdint.h>

int strcasecmp(const char *s1, const char *s2);

#define FLASH_SPACE
#define DATA_SPACE

#define VMCODEBYTE(p)           *(uint8_t *)(p)

#define PROPELLER
#define ANSI_FILE_IO

#endif  // PROPELLER_GCC

/*************/
/* PROPELLER */
/*************/

#ifdef PROPELLER

typedef int32_t VMVALUE;
typedef uint32_t VMUVALUE;
typedef float VMFLOAT;
typedef void **VMHANDLE;

#define ALIGN_MASK              3

#endif

/****************/
/* ANSI_FILE_IO */
/****************/

#ifdef ANSI_FILE_IO

#include <stdio.h>
#include <dirent.h>

typedef FILE VMFILE;

#define VM_fopen	fopen
#define VM_fclose	fclose
#define VM_fgets	fgets
#define VM_fputs	fputs

struct VMDIR {
    DIR *dirp;
};

struct VMDIRENT {
    char name[FILENAME_MAX];
};

#endif // ANSI_FILE_IO

/************/
/* DEFAULTS */
/************/

/* workspace size */
#ifndef WORKSPACESIZE
#define WORKSPACESIZE       (8 * 1024)
#endif

/* runtime heap size */
#ifndef HEAPSIZE
#define HEAPSIZE            (4 * 1024)
#endif

/* maximum number of runtime objects */
#ifndef MAXOBJECTS
#define MAXOBJECTS          128
#endif

/* compiler code buffer size */
#ifndef MAXCODE
#define MAXCODE             1024
#endif

#endif
