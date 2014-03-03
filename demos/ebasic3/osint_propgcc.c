#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <ctype.h>
#include <dirent.h>
#include "db_vm.h"

#if defined(USE_FDS) || defined(LOAD_SAVE)

/* list of drivers we can use */
#ifdef USE_FDS
extern _Driver _FullDuplexSerialDriver;
#endif
#ifdef LOAD_SAVE
extern _Driver _FileDriver;
#endif
_Driver *_driverlist[] = {
#ifdef USE_FDS
    &_FullDuplexSerialDriver,
#endif
#ifdef LOAD_SAVE
    &_FileDriver,
#endif
    NULL
};

#endif

void VM_sysinit(int argc, char *argv[])
{
#ifdef LOAD_SAVE
#ifdef __PROPELLER_LMM__
    LoadSDDriver(0);
#endif
    dfs_mount();
#endif
}

void VM_sysexit(void)
{
    exit(0);
}

char *VM_getline(char *buf, int size)
{
    return fgets(buf, size, stdin);
}

void VM_vprintf(const char *fmt, va_list ap)
{
    char buf[80], *p = buf;
    vsnprintf(buf, sizeof(buf), fmt, ap);
    while (*p != '\0')
        VM_putchar(*p++);
}

void VM_flush(void)
{
    fflush(stdout);
}

int VM_getchar(void)
{
    return getchar();
}

void VM_putchar(int ch)
{
    putchar(ch);
}

#ifdef LOAD_SAVE

int VM_opendir(const char *path, VMDIR *dir)
{
    if (!(dir->dirp = opendir(path)))
        return -1;
    return 0;
}

int VM_readdir(VMDIR *dir, VMDIRENT *entry)
{
    struct dirent *propgcc_entry;
    char *src, *dst;
    
    if (!(propgcc_entry = readdir(dir->dirp)))
        return -1;
    
    src = propgcc_entry->d_name;
    dst = entry->name;
    while ((*dst++ = tolower(*src++)) != '\0')
        ;
    
    return 0;
}

void VM_closedir(VMDIR *dir)
{
    closedir(dir->dirp);
}

#endif

void LOG_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    VM_vprintf(fmt, ap);
    va_end(ap);
}

int strcasecmp(const char *s1, const char *s2)
{
    while (*s1 != '\0' && (tolower(*s1) == tolower(*s2))) {
        ++s1;
        ++s2;
    }
    return tolower((unsigned char) *s1) - tolower((unsigned char) *s2);
}
