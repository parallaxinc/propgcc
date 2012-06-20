#include "term.h"

static void printc(TERM *term, int c);
static void newline(TERM *term);
static void wordfill(uint16_t *dst, uint16_t val, int len);
static void wordmove(uint16_t *dst, uint16_t *src, int len);

/*
 * Term clearScreen function clears the screen.
 * See header file for more details.
 */
void Term_clearScreen(TERM *term)
{
    wordfill(term->screen, TERM_BLANK, term->screensize);
}

/*
 * Term setcolors function sets the palette to that defined by pointer.
 * See header file for more details.
 */
void Term_setColorPalette(TERM *term, const char* ptr)
{
    int  ii = 0;
    uint8_t  fg = 0;
    uint8_t  bg = 0;

    for(ii = 0; ii < TERM_COLORTABLE_SIZE; ii += 2)
    {
        fg = (uint8_t) ptr[ii];
        bg = (uint8_t) ptr[ii+1];
        term->colors[ii]     = fg << 24 | bg << 16 | fg << 8 | bg;
        term->colors[ii+1]   = fg << 24 | fg << 16 | bg << 8 | bg;
   }        
}

/*
 * Term getTile gets tile data from x,y position
 * See header file for more details.
 */
int Term_getTile(TERM *term, int x, int y)
{
    if(x >= term->cols)
        return 0; 
    if(y >= term->rows)
        return 0;
    return term->screen[y * term->cols + x];
}

/*
 * Term setTileColor sets tile data color at x,y position
 * See header file for more details.
 */
int Term_getTileColor(TERM *term, int x, int y)
{
    int shift  = 11;
    int   mask = ((TERM_COLORS-1) << shift);
    int   ndx  = y * term->cols + x;
    int   color = 0;
    
    if(x >= term->cols)
        return 0; 
    if(y >= term->rows)
        return 0;
    color = term->screen[ndx] & mask;
    color >>= shift;
    return color;
}


/*
 * Term setTile sets tile data at x,y position
 * See header file for more details.
 */
void Term_setTile(TERM *term, int x, int y, int tile)
{
    if(x >= term->cols)
        return; 
    if(y >= term->rows)
        return;
    term->screen[y * term->cols + x] = tile;
}

/*
 * Term setTileColor sets tile data color at x,y position
 * See header file for more details.
 */
void Term_setTileColor(TERM *term, int x, int y, int color)
{
    uint16_t tile = 0;
    int shift  = 11;
    int   mask = ((TERM_COLORS-1) << shift);
    int   ndx  = y * term->cols + x;
    
    while(!(*term->ops->vblank)(term))
        ;
        
    if(x >= term->cols)
        return; 
    if(y >= term->rows)
        return;

    color <<= shift; 
    tile = term->screen[ndx];
    tile = tile & ~mask;
    tile = tile | color;
    term->screen[ndx] = tile;
}

/*
 * Term str function prints a string at current position
 * See header file for more details.
 */
void Term_str(TERM *term, const char* sptr)
{
    while(*sptr) {
        (*term->ops->putch)(term, *(sptr++));
    }
}

/*
 * Term dec function prints a decimal number at current position
 * See header file for more details.
 */
void Term_dec(TERM *term, int value)
{
    int n = value;
    int len = 10;
    int result = 0;

    if(value < 0) {
        value = ~value;
        (*term->ops->putch)(term, '-');
    }

    n = 1000000000;

    while(--len > -1) {
        if(value >= n) {
            (*term->ops->putch)(term, value / n + '0');
            value %= n;
            result++;
        }
        else if(result || n == 1) {
            (*term->ops->putch)(term, '0');
        }
        n /= 10;
    }
}

/*
 * Term hex function prints a hexadecimal number at current position
 * See header file for more details.
 */
void Term_hex(TERM *term, int value, int digits)
{
    int ndx;
    char hexlookup[] =
    {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };
    while(digits-- > 0) {
        ndx = (value >> (digits<<2)) & 0xf;
        (*term->ops->putch)(term, hexlookup[ndx]);
    }   
}


/*
 * Term bin function prints a binary number at current position
 * See header file for more details.
 */
void Term_bin(TERM *term, int value, int digits)
{
    int bit = 0;
    while(digits-- > 0) {
        bit = (value >> digits) & 1;
        (*term->ops->putch)(term, bit + '0');
    }   
}

/*
 * Term out function prints a character at current position or performs
 * a screen function.
 * See header file for more details.
 */
int Term_out(TERM *term, int c)
{
if(term->flag == 0)
    {
        switch(c)
        {
            case 0:
                wordfill(&term->screen[0], term->color << 11 | TERM_BLANK, term->screensize);
                term->col = 0;
                term->row = 0;
                break;
            case 1:
                term->col = 0;
                term->row = 0;
                break;
            case 8:
                if (term->col)
                    term->col--;
                break;
            case 9:
                do {
                    printc(term, ' ');
                } while(term->col & 7);
                break;
            case 0xA:   // fall though
            case 0xB:   // fall though
            case 0xC:   // fall though
                term->flag = c;
                return c;
            case 0xD:
                newline(term);
                break;
            default:
                printc(term, c);
                break;
        }
    }
    else
    if (term->flag == 0xA) {
        term->col = c % term->cols;
    }
    else
    if (term->flag == 0xB) {
        term->row = c % term->rows;
    }
    else
    if (term->flag == 0xC) {
        term->color = c & 0xf;
    }
    term->flag = 0;
    return c;
}

/*
 * Term Term_print null terminated char* to screen with normal stdio definitions
 * See header file for more details.
 */
void Term_print(TERM *term, const char* s)
{
    while(*s) {
        Term_putchar(term, *(s++));
    }
}

/*
 * Term Term_putchar print char to screen with normal stdio definitions
 * See header file for more details.
 */
int Term_putchar(TERM *term, int c)
{
    switch(c)
    {
        case '\b':
            if (term->col)
                term->col--;
            break;
        case '\t':
            do {
                printc(term, ' ');
            } while(term->col & 7);
            break;
        case '\n':
            newline(term);
            break;
        case '\r':
            term->col = 0;
            break;
        default:
            printc(term, c);
            break;
    }
    return (int)c;
}

/*
 * Term setCurPosition function sets position to x,y.
 * See header file for more details.
 */
void Term_setCurPosition(TERM *term, int x, int y)
{
    term->col = x;
    term->row = y;
}

/*
 * Term setCoordPosition function sets position to Cartesian x,y.
 * See header file for more details.
 */
void Term_setCoordPosition(TERM *term, int x, int y)
{
    term->col = x;
    term->row = term->rows-y-1;
}

/*
 * Term setXY function sets position to x,y.
 * See header file for more details.
 */
void Term_setXY(TERM *term, int x, int y)
{
    term->col = x;
    term->row = y;
}

/*
 * Term setX function sets column position value
 * See header file for more details.
 */
void Term_setX(TERM *term, int value)
{
    term->col = value;
}

/*
 * Term setY function sets row position value
 * See header file for more details.
 */
void Term_setY(TERM *term, int value)
{
    term->row = value;
}

/*
 * Term getX function gets column position
 * See header file for more details.
 */
int Term_getX(TERM *term)
{
    return term->col;
}

/*
 * Term getY function gets row position
 * See header file for more details.
 */
int Term_getY(TERM *term)
{
    return term->row;
}

/*
 * Term setColors function sets palette color set index
 * See header file for more details.
 */
void Term_setColors(TERM *term, int value)
{
    term->color = value % TERM_COLORS;
}

/*
 * Term getColors function gets palette color set index
 * See header file for more details.
 */
int Term_getColors(TERM *term)
{
    return term->color % TERM_COLORS;
}

/*
 * Term getWidth function gets screen width.
 * See header file for more details.
 */
int Term_getColumns(TERM *term)
{
    return term->cols;
}

/*
 * Term getHeight function gets screen height.
 * See header file for more details.
 */
int Term_getRows(TERM *term)
{
    return term->rows;
}

/*
 * print a character
 */
static void printc(TERM *term, int c)
{
    int   ndx = term->row * term->cols + term->col;
    short val = 0;
    
    // I can't seem to get the palette colors to work correctly.
    // If you want just one screen fg/bg color, use setColorPalette(&gcolors[N]);
    // It seems the color would be wrong for every "even" char.
    // Fine but remove the (c & 1) and the character is wrong.
    
    val  = ((term->color << 1) | (c & 1)) << 10;
    val += 0x200 + (c & 0xFE);

    // Driver updates during invisible. Need some delay so screen updates right.
    // For flicker-free once per scan update, you can wait for status != invisible.
    while (!(*term->ops->vblank)(term))
    	;
    
    term->screen[ndx] = val; // works
    //term->screen[term->row * term->cols + term->col] = val; // fails ... don't know why

    if (++term->col == term->cols) {
        newline(term);
    }
}

/*
 * print a new line
 */
static void newline(TERM *term)
{
    uint16_t* sp = term->screen;
    term->col = 0;
    if (++term->row == term->rows) {
        term->row--;
        wordmove(sp, &sp[term->cols], term->lastrow); // scroll
        wordfill(&sp[term->lastrow], TERM_BLANK, term->cols); // clear new line
    }
}

static void wordfill(uint16_t *dst, uint16_t val, int len)
{
    while(--len > -1) {
        *dst = val;
        dst++;
    }
}

static void wordmove(uint16_t *dst, uint16_t *src, int len)
{
    while(--len > -1) {
        *dst = *src;
        dst++;
        src++;
    }
}
