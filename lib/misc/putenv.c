/* functions for manipulating the environment */
/* written by Eric R. Smith and placed in the public domain */

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#ifndef _COMPILER_H
#include <compiler.h>
#endif

char **_environ_ptr;
extern char *_environ[];

static void del_env(const char *strng);

static void
del_env(const char *strng)
{
	char **var;
	char *name;
	size_t len = 0;

	if (!_environ_ptr) {
	  _environ_ptr = _environ;
	}
/* find the length of "tag" in "tag=value" */
	for (name = (char *)strng; *name && (*name != '='); name++)
		len++;

/* find the tag in the environment */
	for (var = _environ_ptr; (name = *var) != NULL; var++) {
		if (!strncmp(name, strng, len) && name[len] == '=')
			break;
	}

/* if it's found, move all the other environment variables down by 1 to
   delete it
 */
	if (name) {
		while (name) {
			name = var[1];
			*var++ = name;
		}
	}
}

int
_putenv(const char *strng)
{
	int i = 0;
	char **e;

	del_env(strng);

	if (!_environ_ptr)
	  _environ_ptr = _environ;

	{
		while(_environ_ptr[i]) i++ ;
		e = (char **) malloc((i+2)*sizeof(char *));
		if (!e) {
			return -1;
		}
		memcpy(e, _environ_ptr, (i+1)*sizeof(char *));
		free(_environ_ptr);
		_environ_ptr = e;
	}
	if (!e)
		return -1;

	_environ_ptr = e;
	_environ_ptr[i] = (char *)strng;
	_environ_ptr[i+1] = 0;
	return 0;
}

__weak_alias(putenv, _putenv);
