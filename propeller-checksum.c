/*
 * program to calculate and set up the checksum in a propeller binary
 * written by David Betz
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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

/* target checksum for a binary file */
#define SPIN_TARGET_CHECKSUM    0x14

int main(int argc, char *argv[])
{
    size_t fileSize;
    int chksum, byte;
    SpinHdr hdr;
    FILE *fp;

    /* check the arguments */
    if (argc != 2) {
        fprintf(stderr, "usage: fixchecksum <file>\n");
        return 1;
    }
    
    /* open the file to fix */
    if (!(fp = fopen(argv[1], "r+b"))) {
        fprintf(stderr, "error: can't open '%s'\n", argv[1]);
        return 1;
    }
    
    /* read the existing file header */
    if (fread(&hdr, 1, sizeof(hdr), fp) != sizeof(hdr)) {
        fprintf(stderr, "error: can't read file header\n");
        return 1;
    }
    
    /* if the checksum isn't zero it's already been fixed */
    if (hdr.chksum != 0) {
        fprintf(stderr, "error: file has already been fixed\n");
        return 1;
    }
    
    /* get the file size minus the trailer we will add later */
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    
    /* fixup the header to point past the spin bytecodes and generated PASM code */
    hdr.vbase = fileSize;
    hdr.dbase = fileSize + 2*sizeof(uint32_t); // stack markers
    hdr.dcurr = hdr.dbase + sizeof(uint32_t);
    
    /* rewrite the header so the checksum will come out right */
    fseek(fp, 0, SEEK_SET);
    if (fwrite(&hdr, 1, sizeof(hdr), fp) != sizeof(hdr)) {
        fprintf(stderr, "error: can't update file header\n");
        return 1;
    }
    
    /* compute the checksum */
    fseek(fp, 0, SEEK_SET);
    for (chksum = 0; (byte = getc(fp)) != EOF; chksum += byte)
        ;
        
    /* store the checksum in the header */
    hdr.chksum = SPIN_TARGET_CHECKSUM - chksum;
    
    /* write the updated header */
    fseek(fp, 0, SEEK_SET);
    if (fwrite(&hdr, 1, sizeof(hdr), fp) != sizeof(hdr)) {
        fprintf(stderr, "error: can't update file header\n");
        return 1;
    }
        
    /* close the file we're updating */
    fclose(fp);

#if 0
    /* show what we've done */
    printf("clkfreq: %08x\n", hdr.clkfreq);
    printf("clkmode: %02x\n", hdr.clkmode);
    printf("chksum:  %02x\n", hdr.chksum);
    printf("pbase:   %08x\n", hdr.pbase);
    printf("vbase:   %08x\n", hdr.vbase);
    printf("dbase:   %08x\n", hdr.dbase);
    printf("pcurr:   %08x\n", hdr.pcurr);
    printf("dcurr:   %08x\n", hdr.dcurr);
#endif   
    return 0;
}
