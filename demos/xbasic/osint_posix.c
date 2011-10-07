#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include "db_vm.h"

void VM_sysinit(int argc, char *argv[])
{
}

void VM_getline(char *buf, int size)
{
#ifdef LINE_EDIT
    int i = 0;
    while (i < size - 1) {
        int ch = VM_getchar();
        if (ch == '\n') {
            buf[i++] = '\n';
            VM_putchar('\n');
            break;
        }
        else if (ch == '\b' || ch == 0x7f) {
            if (i > 0) {
                VM_putchar('\b');
                VM_putchar(' ');
                VM_putchar('\b');
                --i;
            }
        }
        else {
            buf[i++] = ch;
            VM_putchar(ch);
        }
    }
    buf[i] = '\0';
#else
    fgets(buf, size, stdin);
#endif
}

void VM_vprintf(const char *fmt, va_list ap)
{
    char buf[80], *p = buf;
    vsnprintf(buf, sizeof(buf), fmt, ap);
    while (*p != '\0')
        VM_putchar(*p++);
}

int VM_getchar(void)
{
    return getchar();
}

void VM_putchar(int ch)
{
    putchar(ch);
}

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
