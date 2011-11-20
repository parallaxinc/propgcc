#include <propeller.h>
#include "term_tv.h"

/**
 * This is the TV palette.
 */
static char default_palette[TERM_COLORTABLE_SIZE] =     
{// fgRGB bgRGB
    0x07, 0x0a,     // 0    white / dark blue
    0x07, 0xbb,     // 1   yellow / brown
    0x9e, 0x9b,     // 2  magenta / black
    0x04, 0x07,     // 3     grey / white
    0x3d, 0x3b,     // 4     cyan / dark cyan
    0x6b, 0x6e,     // 5    green / gray-green
    0xbb, 0xce,     // 6      red / pink
    0x3e, 0x0a      // 7     cyan / blue
};

static int vblank(TERM *term);

static TERM_OPS ops = {
	Term_out,
	vblank
};

/*
 * TV_Text start function starts TV on a cog
 * See header file for more details.
 */
TERM *tvTerm_start(TERM_TV *tvTerm, int basepin)
{
    TERM *term = &tvTerm->term;
    TvText_t *tvText = &tvTerm->control;
    extern uint32_t binary_TV_dat_start[];

#if 0
	term->ops = &ops;
    term->screen = tvTerm->screen;
    term->colors = tvTerm->colors;
    term->screensize = TV_TEXT_SCREENSIZE;
    term->lastrow = term->screensize - TV_TEXT_COLS;
    term->col   = 0; // init vars
    term->row   = 0;
    term->rows = TV_TEXT_ROWS;
    term->cols = TV_TEXT_COLS;
    term->color = 0;
    term->flag = 0;

    tvText->status = 0;
    tvText->enable = 1;
    tvText->pins   = ((basepin & 0x38) << 1) | (((basepin & 4) == 4) ? 0x5 : 0);
    tvText->mode   = 0x12;
    tvText->colors = tvTerm->colors;
    tvText->screen = tvTerm->screen;
    tvText->ht = TV_TEXT_COLS;
    tvText->vt = TV_TEXT_ROWS;
    tvText->hx = 4;
    tvText->vx = 1;
    tvText->ho = 0;
    tvText->vo = -5;
    tvText->broadcast = 0;
    tvText->auralcog  = 0;

    // start new cog from external memory using pasm and tvText
    tvTerm->cogid = cognew((uint32_t)binary_TV_dat_start, (uint32_t)tvText) + 1;

    // set main fg/bg color
    Term_setColorPalette(term, default_palette);

    // blank the screen
    Term_clearScreen(term);
#endif

    return term;
}

/**
 * stop stops the cog running the native assembly driver 
 */
void tvTerm_stop(TERM *term)
{
	TERM_TV *tvTerm = (TERM_TV *)term;
    if(tvTerm->cogid) {
        cogstop(tvTerm->cogid);
    }
}

static int vblank(TERM *term)
{
	TERM_TV *tvTerm = (TERM_TV *)term;
    return tvTerm->control.status == TV_TEXT_STAT_INVISIBLE;
}
