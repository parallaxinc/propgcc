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
static int lineCount = 0;

static int FindLineN(int lineNumber, Line **pLine);
static int FindLine(int offset, Line **pLine);

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
        --lineCount;
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
        ++lineCount;
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
    --lineCount;

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

int BufGetLineN(int *pLineNumber, char *text)
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

int BufAddLine(int offset, const char *text)
{
    int newLength = sizeof(Line) + strlen(text);
    uint8_t *next;
    Line *line;

    /* make sure the length is a multiple of the word size */
    newLength = (newLength + ALIGN_MASK) & ~ALIGN_MASK;

    /* find the insertion point */
    FindLine(offset, &line);
    next = (uint8_t *)line;

    /* make sure there is enough space */
    if (bufferTop + newLength > bufferMax)
        return FALSE;

    /* make space for the new line */
    if (next < bufferTop)
        memmove(next + newLength, next, bufferTop - next);
    bufferTop += newLength;

    /* insert the new line */
    line->length = newLength;
    strcpy(line->text, text);
    
    /* increment the line count */
    ++lineCount;

    /* return successfully */
    return TRUE;
}

int BufDeleteLine(int offset)
{
    Line *line, *next;
    int spaceFreed;

    /* find the line to delete */
    if (!FindLine(offset, &line))
        return FALSE;

    /* get a pointer to the next line */
    next = (Line *)((uint8_t *)line + line->length);
    spaceFreed = line->length;

    /* remove the line to be deleted */
    if ((uint8_t *)next < bufferTop)
        memmove(line, next, bufferTop - (uint8_t *)next);
    bufferTop -= spaceFreed;
    --lineCount;

    /* return successfully */
    return TRUE;
}

int BufSeek(int offset)
{
    return FindLine(offset, &current);
}

int BufGetLine(char *text)
{
    /* check for the end of the buffer */
    if ((uint8_t *)current >= bufferTop)
        return FALSE;

    /* get the current line */
    strcpy(text, current->text);

    /* move ahead to the next line */
    current = (Line *)((char *)current + current->length);

    /* return successfully */
    return TRUE;
}

int BufGetLineCount(void)
{
    return lineCount;
}

static int FindLine(int offset, Line **pLine)
{
    uint8_t *p = buffer;
    while (p < bufferTop) {
        Line *line = (Line *)p;
        if (--offset < 0) {
            *pLine = line;
            return TRUE;
        }
        p = p + line->length;
    }
    *pLine = (Line *)p;
    return FALSE;
}
