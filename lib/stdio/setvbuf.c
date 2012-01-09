/* from Dale Schumacher's dLibs library */

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

int setvbuf(FILE *fp, char *bp, int bmode, size_t size)
{
    if(fp->_flag & _IOFREEBUF)
	free(fp->_base);
    fp->_flag &= ~(_IOFBF | _IOLBF | _IONBF | _IOFREEBUF);
    fp->_flag |= bmode;
    fp->_cnt = 0;
    if(bmode == _IONBF)				/* unbuffered */
    {
	fp->_base = fp->_chbuf;			/* use tiny buffer */
	fp->_bsiz = 1;
    }
    
    else if (size > (size_t) LONG_MAX)              /* not likely! */
	return -1;
    
    else						/* full buffering */
    {
	if(bp != NULL)
	{
	    fp->_base = (unsigned char *) bp;
	}
	else
	{
	    if ((fp->_base = (unsigned char *) malloc(size)) != NULL)
	    {
		fp->_flag |= _IOFREEBUF;
	    }
	    else
	    {
		return -1;
	    }
	}
	fp->_bsiz = size;
    }
    fp->_ptr = fp->_base;
    return 0;
}
