/* from Dale Schumacher's dLibs */

/* 5/26/93 sb -- Modified for HSC to account for the possibility that
 * size * count >= 64K.
 */

#include <stddef.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <sys/thread.h>

size_t
fwrite(const void *vdata, size_t size, size_t count, FILE *fp)
{
        unsigned char *data=(unsigned char *)vdata;
	register size_t n, m;
	register long l = 0;
	long space;
	unsigned int f = fp->_flag;

	if(f & _IORW)
	{
	    fp->_flag |= _IOWRT;
	    f = (fp->_flag &= ~(_IOREAD | _IOEOF));
	}

	if(!(f & _IOWRT)			/* not opened for write? */
	|| (f & (_IOERR | _IOEOF)))		/* error/eof conditions? */
		return(0);

	__lock(&fp->_lock);

	n =  count * size;
	space = fp->_bsiz - fp->_cnt;
	while(n > 0)
	  {
	    m = (n > space)? space: n;
	    memcpy(fp->_ptr, data, m);
	    fp->_ptr += m;
	    fp->_cnt += m;
	    space -= m;
	    if(space == 0)
	      {
		__unlock(&fp->_lock);
		if(fflush(fp)) {
		  return 0;
		}
		__lock(&fp->_lock);
		space = fp->_bsiz;
		if(f & _IORW)
		  fp->_flag |= _IOWRT; /* fflush resets this */
	      }
	    l += m;
	    data += m;
	    n -= m;
	    if(n < space)
	      continue;
	    if((m = (*fp->_drv->write)(fp, data, (unsigned long)n )) != (long)n)
	      {
		fp->_flag |= _IOERR;
		__unlock(&fp->_lock);
		return 0;
	      }
	    l += m;
	    break;
	  }

	__unlock(&fp->_lock);
	return((l > 0) ? ((size_t)l / size) : 0);
}
