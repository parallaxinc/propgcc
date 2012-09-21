#include <stdio.h>
#include <stdarg.h>

void printdec(FILE *fp, int val)
{
    int scale;
    int zeroflag = 0;

    if (val < 0)
    {
        fputc('-', fp);
        val = -val;
    }

    for (scale = 1000000000; scale > 1; scale /= 10)
    {
        if (val >= scale)
        {
            fputc('0' + val/scale, fp);
            val %= scale;
            zeroflag = 1;
        }
        else if (zeroflag)
            fputc('0', fp);
        else
            fputc(' ', fp);
    }

    fputc('0'+val, fp);
}

void printhex(FILE *fp, int val)
{
    int i;
    static char hexdigits[] = "0123456789abcdef";

    for (i = 0; i < 8; i++)
    {
        fputc(hexdigits[(val >> 28)&15], fp);
        val <<= 4;
    }
}

void _doprint(FILE *fp, const char *fmt, va_list args )
{
    int val;
    int state = 0;

    while (*fmt)
    {
        val = *fmt++;
        if (state)
        {
            if (val == 's')
            {
                state = 0;
	        val = va_arg(args, int);
                fputs((char *)val, fp);
            }
            else if (val == 'd')
            {
                state = 0;
	        val = va_arg(args, int);
                printdec(fp, val);
            }
            else if (val == 'x')
            {
                state = 0;
	        val = va_arg(args, int);
                printhex(fp, val);
            }
        }
        else
        {
            if (val == '%') state = 1;
            else fputc(val, fp);
        }
    }
}

int printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    _doprint(stdout, fmt, args);
    va_end(args);
    return 0;
}

int fprintf(FILE *fp, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    _doprint(fp, fmt, args);
    va_end(args);
    return 0;
}
