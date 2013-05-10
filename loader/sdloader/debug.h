#ifndef __DEBUG_H__
#define __DEBUG_H__

#define DEBUG

#ifdef DEBUG
#define DPRINTF(...)    __simple_printf(__VA_ARGS__)
#else
#define DPRINTF(...)
#endif

#endif
