/* from Henry Spencer's stringlib */
/* modified to use thread local storage by
 * Eric Smith, Total Spectrum Software Inc.
 */
#include <string.h>
#include <sys/thread.h>

/*
 * Get next token from string s (NULL on 2nd, 3rd, etc. calls),
 * where tokens are nonempty strings separated by runs of
 * chars from delim.  Writes NULs into s to end tokens.  delim need not
 * remain constant from call to call.
 */

char *				/* NULL if no token left */
strtok(char *s, const char *delim)
{
	register char *scan;
	char *tok;
	register const char *dscan;

	if (s == NULL && _TLS->strtok_scanpoint == NULL)
		return(NULL);
	if (s != NULL)
		scan = s;
	else
		scan = _TLS->strtok_scanpoint;

	/*
	 * Scan leading delimiters.
	 */
	for (; *scan != '\0'; scan++) {
		for (dscan = delim; *dscan != '\0'; dscan++)
			if (*scan == *dscan)
				break;
		if (*dscan == '\0')
			break;
	}
	if (*scan == '\0') {
		_TLS->strtok_scanpoint = NULL;
		return(NULL);
	}

	tok = scan;

	/*
	 * Scan token.
	 */
	for (; *scan != '\0'; scan++) {
		for (dscan = delim; *dscan != '\0';)	/* ++ moved down. */
			if (*scan == *dscan++) {
				_TLS->strtok_scanpoint = scan+1;
				*scan = '\0';
				return(tok);
			}
	}

	/*
	 * Reached end of string.
	 */
	_TLS->strtok_scanpoint = NULL;
	return(tok);
}
