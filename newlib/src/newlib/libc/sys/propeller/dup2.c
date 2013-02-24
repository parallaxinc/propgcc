#include <unistd.h>
#include <errno.h>

int dup2(int oldfd, int newfd){
	(void)oldfd;
	(void)newfd;
	errno = EMFILE;
	return -1;
}

