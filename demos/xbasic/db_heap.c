#include "db_compiler.h"

static uint8_t *heap;
static uint8_t *heapFree;
static uint8_t *heapTop;

void HeapInit(uint8_t *p, size_t size)
{
    heap = p;
    heapTop = p + size;
}

void HeapReset(void)
{
    heapFree = heap;
}

uint8_t *HeapAlloc(size_t size)
{
    uint8_t *p = heapFree;
    if (heapFree + size > heapTop)
        return NULL;
    heapFree += size;
    return p;
}

