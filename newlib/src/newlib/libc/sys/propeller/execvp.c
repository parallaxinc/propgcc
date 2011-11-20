#include <unistd.h>
#include <errno.h>

int execvp( const char *file, char *const argv[]){
	(void)file;
	(void)argv;
	errno = EACCES;
	return -1;
}

