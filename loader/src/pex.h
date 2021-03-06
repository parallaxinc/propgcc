#ifndef __PEX_H__
#define __PEX_H__

/* this header takes up 16 longs and is followed by the xmm kernel which takes 496 longs */
#define PEXE_TAG        "PEXE"
#define PEXE_VERSION1   0x0100  // cache drivers
#define PEXE_VERSION2   0x0200  // cache tag handling in kernel, xmem drivers
#define PEXE_VERSION    PEXE_VERSION2
#define PEXE_HDR_SIZE   (16 * sizeof(uint32_t))

typedef struct {
    char tag[4];
    uint16_t version;
    uint16_t unused;
    uint32_t loadAddress;
    uint8_t reserved[PEXE_HDR_SIZE - 12];
    uint32_t kernel[496];
} PexeFileHdr;

#endif
