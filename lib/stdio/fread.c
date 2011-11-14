/* nothing like the original from
 * from Dale Schumacher's dLibs
 */

/* 5/26/93 sb -- Modified for HSC to account for the possibility that
 * size * count >= 64K.
 */

#include <stddef.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

size_t	fread(void *_data, size_t size, size_t count, FILE *fp)
{
	register size_t n;
	register long l, cnt;
	register unsigned int f;
	unsigned char *data=_data;

	f = fp->_flag;
	if(f & _IORW) f = (fp->_flag |= _IOREAD);
	if(!(f & _IOREAD) || (f & (_IOERR | _IOEOF)))
	    return(0);

	l = 0;
	n = count * size;
#if 0
	if(fflush(fp))			/* re-sync file pointers */
		return 0;
#endif
    if(1 ) {
    again:	
	if((cnt = fp->_cnt) > 0)
	{
	    cnt = (cnt < n) ? cnt : n;
	    memcpy(data, fp->_ptr, cnt);
	    fp->_cnt -= cnt;
	    fp->_ptr += cnt;
	    l += cnt;
	    data = data + cnt;
	    n -= cnt;
	}
	/* n == how much more */
	if(n > 0)
	{
	    if(n < fp->_bsiz)
	    { /* read in fp->_bsiz bytes into fp->_base and do it again */
		fp->_ptr = fp->_base;
		if((cnt = fp->_drv->read(fp, fp->_base, (unsigned long)fp->_bsiz)) <= 0)
		{   /* EOF or error */
		    fp->_flag |= ((cnt == 0) ? _IOEOF : _IOERR);
		    goto ret;
		}
		fp->_cnt = cnt;
		goto again;
	    }
	    else
	    while (n > 0)
	    { /* read in n bytes into data */
		if((cnt = fp->_drv->read(fp, data, (unsigned long)n)) <= 0)
		{   /* EOF or error */
		    fp->_flag |= ((cnt == 0) ? _IOEOF : _IOERR);
		    goto ret;
		}
		l += cnt;
		data = data + cnt;
		n -= cnt;
	    }
	}
    }

    ret:
	return((l > 0) ? ((size_t)l / size) : 0);
}
