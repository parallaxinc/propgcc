#include <stdlib.h>
#include <reent.h>
#include "bget.h"

/*
 * the "reentrant" versions of malloc, etc.
 */

void *_malloc_r(struct _reent *r, size_t s)
{
    return malloc(s);
}

void _free_r(struct _reent *r, void *ptr){
  brel(ptr);
}

void *_realloc_r(struct _reent *r, void *ptr, size_t size){
  return bgetr(ptr, size);
}

void *_calloc_r(struct _reent *r, size_t nmemb, size_t size)
{
    size = size * nmemb;  /* get total size in bytes */
    return bgetz(size);
}
