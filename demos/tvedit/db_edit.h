#ifndef __EDIT_H__
#define __EDIT_H__

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#define MAX_PROG_NAME   32
#define EDITBUFSIZE	2048

/* edit buffer interface */
void BufInit(void);
int BufAddLineN(int lineNumber, const char *text);
int BufDeleteLineN(int lineNumber);
int BufSeekN(int lineNumber);
int BufGetLine(int *pLineNumber, char *text);

#endif

