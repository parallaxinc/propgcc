#include <stdlib.h>

/* current maximum number of multibyte characters needed for a wide character */
/* if the character encoding is ASCII this is 1; if UTF-8, it is 4 */
int _mb_cur_max = 1;
