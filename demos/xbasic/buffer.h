#include "db_types.h"

void BufInit(void);
int BufAddLineN(int16_t lineNumber, const char *text);
int BufDeleteLineN(int16_t lineNumber);
int BufSeekN(int16_t lineNumber);
int BufGetLine(int16_t *pLineNumber, char *text);

int BufWriteWords(int offset, const int16_t *buf, int size);
int BufReadWords(int offset, int16_t *buf, int size);
