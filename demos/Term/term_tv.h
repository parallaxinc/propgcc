#ifndef __TERM_TV_H__
#define __TERM_TV_H__

#ifdef __cplusplus
extern "C" 
{
#endif

#include "term.h"

/**
 * TV_Text column count
 */
#define  TV_TEXT_COLS 42

/**
 * TV_Text row count
 */
#define  TV_TEXT_ROWS 14

/**
 * TV_Text screensize count
 */
#define  TV_TEXT_SCREENSIZE (TV_TEXT_COLS * TV_TEXT_ROWS)

/**
 * TV_Text status enum
 */
typedef enum {
    TV_TEXT_STAT_DISABLED,
    TV_TEXT_STAT_INVISIBLE,
    TV_TEXT_STAT_VISIBLE
} tvTextStat_t;

/**
 * TV_Text control struct
 */
typedef struct _tv_text_struct
{
    uint32_t status     ; //0/1/2 = off/invisible/visible              read-only
    uint32_t enable     ; //0/non-0 = off/on                           write-only
    uint32_t pins       ; //%pppmmmm = pin group, pin group mode       write-only
    uint32_t mode       ; //%tccip = tile,chroma,interlace,ntsc/pal    write-only
    uint16_t *screen    ; //pointer to screen (words)                  write-only      
    uint32_t *colors    ; //pointer to colors (longs)                  write-only
    uint32_t ht         ; //horizontal tiles                           write-only
    uint32_t vt         ; //vertical tiles                             write-only
    uint32_t hx         ; //horizontal tile expansion                  write-only
    uint32_t vx         ; //vertical tile expansion                    write-only
    uint32_t ho         ; //horizontal offset                          write-only
    uint32_t vo         ; //vertical offset                            write-only
    uint32_t broadcast  ; //broadcast frequency (Hz)                   write-only
    uint32_t auralcog   ; //aural fm cog                               write-only      
}   TvText_t;

typedef struct {
	TERM term;
	TvText_t control;
	uint16_t screen[TV_TEXT_SCREENSIZE];
	uint32_t colors[TERM_COLORTABLE_SIZE];
	int cogid;
} TERM_TV;

TERM *tvTerm_start(TERM_TV *tvTerm, int basepin);
void tvTerm_stop(TERM *term);

#ifdef __cplusplus
}
#endif

#endif
