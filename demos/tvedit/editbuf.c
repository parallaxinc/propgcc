#include <stdint.h>
#include <string.h>
#include "db_edit.h"
#include "db_types.h"

typedef struct {
    int lineNumber;
    int length;
    char text[1];
} Line;

static uint8_t buffer[EDITBUFSIZE];
static uint8_t *bufferMax = buffer + sizeof(buffer);
static uint8_t *bufferTop = buffer;
static Line *current = (Line *)buffer;

static int FindLineN(int lineNumber, Line **pLine);

void BufInit(void)
{
    bufferTop = buffer;
    current = (Line *)buffer;
}

int BufAddLineN(int lineNumber, const char *text)
{
    int newLength = sizeof(Line) + strlen(text);
    int spaceNeeded;
    uint8_t *next;
    Line *line;

    /* make sure the length is a multiple of the word size */
    newLength = (newLength + ALIGN_MASK) & ~ALIGN_MASK;

    /* replace an existing line */
    if (FindLineN(lineNumber, &line)) {
        next = (uint8_t *)line + line->length;
        spaceNeeded = newLength - line->length;
    }

    /* insert a new line */
    else {
        next = (uint8_t *)line;
        spaceNeeded = newLength;
    }

    /* make sure there is enough space */
    if (bufferTop + spaceNeeded > bufferMax)
        return FALSE;

    /* make space for the new line */
    if (next < bufferTop && spaceNeeded != 0)
        memmove(next + spaceNeeded, next, bufferTop - next);
    bufferTop += spaceNeeded;

    /* insert the new line */
    if (newLength > 0) {
        line->lineNumber = lineNumber;
        line->length = newLength;
        strcpy(line->text, text);
    }

    /* return successfully */
    return TRUE;
}

int BufDeleteLineN(int lineNumber)
{
    Line *line, *next;
    int spaceFreed;

    /* find the line to delete */
    if (!FindLineN(lineNumber, &line))
        return FALSE;

    /* get a pointer to the next line */
    next = (Line *)((uint8_t *)line + line->length);
    spaceFreed = line->length;

    /* remove the line to be deleted */
    if ((uint8_t *)next < bufferTop)
        memmove(line, next, bufferTop - (uint8_t *)next);
    bufferTop -= spaceFreed;

    /* return successfully */
    return TRUE;
}

int BufSeekN(int lineNumber)
{
    /* if the line number is zero start at the first line */
    if (lineNumber == 0)
        current = (Line *)buffer;

    /* otherwise, start at the specified line */
    else if (!FindLineN(lineNumber, &current))
        return FALSE;

    /* return successfully */
    return TRUE;
}

int BufGetLine(int *pLineNumber, char *text)
{
    /* check for the end of the buffer */
    if ((uint8_t *)current >= bufferTop)
        return FALSE;

    /* get the current line */
    *pLineNumber = current->lineNumber;
    strcpy(text, current->text);

    /* move ahead to the next line */
    current = (Line *)((char *)current + current->length);

    /* return successfully */
    return TRUE;
}

static int FindLineN(int lineNumber, Line **pLine)
{
    uint8_t *p = buffer;
    while (p < bufferTop) {
        Line *line = (Line *)p;
        if (lineNumber <= line->lineNumber) {
            *pLine = line;
            return lineNumber == line->lineNumber;
        }
        p = p + line->length;
    }
    *pLine = (Line *)p;
    return FALSE;
}
