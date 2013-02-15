#include <stdint.h>
#include "p1image.h"
#include "config.h"

/* target checksum for a binary file */
#define SPIN_TARGET_CHECKSUM    0x14

/* spin object file header */
typedef struct {
    uint32_t clkfreq;
    uint8_t clkmode;
    uint8_t chksum;
    uint16_t pbase;
    uint16_t vbase;
    uint16_t dbase;
    uint16_t pcurr;
    uint16_t dcurr;
} SpinHdr;

void p1_UpdateHeader(BoardConfig *config, uint8_t *imagebuf, uint32_t imageSize)
{
    SpinHdr *hdr;
    int ivalue;

    /* patch in header fields */
    hdr = (SpinHdr *)imagebuf;
    GetNumericConfigField(config, "clkfreq", &ivalue);
    hdr->clkfreq = ivalue;
    GetNumericConfigField(config, "clkmode", &ivalue);
    hdr->clkmode = ivalue;
    hdr->vbase = imageSize;
    hdr->dbase = imageSize + 2 * sizeof(uint32_t); // stack markers
    hdr->dcurr = hdr->dbase + sizeof(uint32_t);

    /* update the checksum */
    p1_UpdateChecksum(imagebuf, imageSize);
}

void p1_UpdateChecksum(uint8_t *imagebuf, int imageSize)
{
    SpinHdr *hdr = (SpinHdr *)imagebuf;
    uint32_t cnt;
    uint8_t *p;
    int chksum;
    
    /* first zero out the checksum */
    hdr->chksum = 0;
    
    /* compute the checksum */
    for (chksum = 0, p = imagebuf, cnt = imageSize; cnt > 0; --cnt)
        chksum += *p++;
        
    /* store the checksum in the header */
    hdr->chksum = SPIN_TARGET_CHECKSUM - chksum;
}
