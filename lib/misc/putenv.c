/* functions for manipulating the environment */
/* written by Eric R. Smith and placed in the public domain */

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#ifndef _COMPILER_H
#include <compiler.h>
#endif

extern char **_environ;

static void del_env(const char *strng);

static void
del_env(const char *strng)
{
	char **var;
	char *name;
	size_t len = 0;

	if (!_environ) return;

/* find the length of "tag" in "tag=value" */
	for (name = (char *)strng; *name && (*name != '='); name++)
		len++;

/* find the tag in the environment */
	for (var = _environ; (name = *var) != NULL; var++) {
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

	if (!_environ)
		e = (char **) malloc(2*sizeof(char *));
	else {
		while(_environ[i]) i++ ;
		e = (char **) malloc((i+2)*sizeof(char *));
		if (!e) {
			return -1;
		}
		memcpy(e, _environ, (i+1)*sizeof(char *));
		free(_environ);
		_environ = e;
	}
	if (!e)
		return -1;

	_environ = e;
	_environ[i] = (char *)strng;
	_environ[i+1] = 0;
	return 0;
}

__weak_alias(putenv, _putenv);
