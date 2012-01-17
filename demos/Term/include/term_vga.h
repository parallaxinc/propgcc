#ifndef __TERM_VGA_H__
#define __TERM_VGA_H__

#ifdef __cplusplus
extern "C" 
{
#endif

#include "term.h"

/**
 * VGA_Text column count
 */
#define  VGA_TEXT_COLS 30

/**
 * VGA_Text row count
 */
#define  VGA_TEXT_ROWS 14

/**
 * VGA_Text screensize count
 */
#define  VGA_TEXT_SCREENSIZE (VGA_TEXT_COLS * VGA_TEXT_ROWS)

/**
 * VGA_Text status enum
 */
typedef enum {
    VGA_TEXT_STAT_DISABLED,
    VGA_TEXT_STAT_INVISIBLE,
    VGA_TEXT_STAT_VISIBLE
} vgaTextStat_t;

/**
 * VGA_Text control struct
 */
typedef struct _vga_text_struct
{
    long status    ; // 0/1/2 = off/visible/invisible      read-only   (21 longs)
    long enable    ; // 0/non-0 = off/on                   write-only
    long pins      ; // %pppttt = pins                     write-only
    long mode      ; // %tihv = tile,interlace,hpol,vpol   write-only
    long screen    ; // pointer to screen (words)          write-only
    long colors    ; // pointer to colors (longs)          write-only            
    long ht        ; // horizontal tiles                   write-only
    long vt        ; // vertical tiles                     write-only
    long hx        ; // horizontal tile expansion          write-only
    long vx        ; // vertical tile expansion            write-only
    long ho        ; // horizontal offset                  write-only
    long vo        ; // vertical offset                    write-only
    long hd        ; // horizontal display ticks           write-only
    long hf        ; // horizontal front porch ticks       write-only
    long hs        ; // horizontal sync ticks              write-only
    long hb        ; // horizontal back porch ticks        write-only
    long vd        ; // vertical display lines             write-only
    long vf        ; // vertical front porch lines         write-only
    long vs        ; // vertical sync lines                write-only
    long vb        ; // vertical back porch lines          write-only
    long rate      ; // tick rate (Hz)                     write-only
} vgaText_t;

typedef struct {
	TERM term;
	vgaText_t control;
	uint16_t screen[VGA_TEXT_SCREENSIZE];
	uint32_t colors[TERM_COLORTABLE_SIZE];
	int cogid;
} TERM_VGA;

TERM *vgaTerm_start(TERM_VGA *vgaTerm, int basepin);
void vgaTerm_stop(TERM *term);

#ifdef __cplusplus
}
#endif

#endif
