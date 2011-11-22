#include <string.h>
#include "db_edit.h"

typedef struct {
    uint16_t lineNumber;
    uint16_t length;
    char text[1];
} Line;

static uint8_t buffer[EDITBUFSIZE];
static uint8_t *bufferMax = buffer + sizeof(buffer);
static uint8_t *bufferTop = buffer;
static Line *current = (Line *)buffer;

static int FindLineN(int16_t lineNumber, Line **pLine);

void BufInit(void)
{
    bufferTop = buffer;
    current = (Line *)buffer;
}

int BufAddLineN(int16_t lineNumber, const char *text)
{
    int newLength = sizeof(Line) + strlen(text);
    int spaceNeeded;
    uint8_t *next;
    Line *line;

    /* make sure the length is a multiple of the word size */
    if (newLength & 1)
        ++newLength;

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
        return VMFALSE;

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
    return VMTRUE;
}

int BufDeleteLineN(int16_t lineNumber)
{
    Line *line, *next;
    int spaceFreed;

    /* find the line to delete */
    if (!FindLineN(lineNumber, &line))
        return VMFALSE;

    /* get a pointer to the next line */
    next = (Line *)((uint8_t *)line + line->length);
    spaceFreed = line->length;

    /* remove the line to be deleted */
    if ((uint8_t *)next < bufferTop)
        memmove(line, next, bufferTop - (uint8_t *)next);
    bufferTop -= spaceFreed;

    /* return successfully */
    return VMTRUE;
}

int BufSeekN(int16_t lineNumber)
{
    /* if the line number is zero start at the first line */
    if (lineNumber == 0)
        current = (Line *)buffer;

    /* otherwise, start at the specified line */
    else if (!FindLineN(lineNumber, &current))
        return VMFALSE;

    /* return successfully */
    return VMTRUE;
}

int BufGetLine(int16_t *pLineNumber, char *text)
{
    /* check for the end of the buffer */
    if ((uint8_t *)current >= bufferTop)
        return VMFALSE;

    /* get the current line */
    *pLineNumber = current->lineNumber;
    strcpy(text, current->text);

    /* move ahead to the next line */
    current = (Line *)((char *)current + current->length);

    /* return successfully */
    return VMTRUE;
}

static int FindLineN(int16_t lineNumber, Line **pLine)
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
    return VMFALSE;
}
