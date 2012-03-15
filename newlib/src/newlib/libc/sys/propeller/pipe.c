#include <unistd.h>
#include <errno.h>

int pipe(int filedes[2]){
	(void)filedes;
	errno = EMFILE;
	return -1;
}

