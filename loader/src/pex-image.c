#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "loadelf.h"
#include "loader.h"
#include "packet.h"
#include "PLoadLib.h"
#include "osint.h"
#include "pex.h"

int WriteExecutableFile(char *path, BoardConfig *config, ElfContext *c, char *outfile)
{
    ElfProgramHdr program_kernel;
    uint8_t *imagebuf, *buf;
    uint32_t loadAddress;
    PexeFileHdr hdr;
    int imageSize;
    FILE *fp;
    
    /* build the external image */
    if (!(imagebuf = BuildExternalImage(config, c, &loadAddress, &imageSize)))
        return FALSE;
        
    /* find the .xmmkernel segment */
    if (FindProgramSegment(c, ".xmmkernel", &program_kernel) < 0)
        return Error("can't find .xmmkernel segment");
    
    /* load the .kernel section */
    if (!(buf = LoadProgramSegment(c, &program_kernel)))
        return Error("can't load .xmmkernel section");

    memset(&hdr, 0, sizeof(hdr));
    strcpy(hdr.tag, PEXE_TAG);
    hdr.version = PEXE_VERSION;
    hdr.loadAddress = loadAddress;
    memcpy(hdr.kernel, buf, program_kernel.filesz);
    
    ConstructOutputName(outfile, path, ".pex");
    if (!(fp = fopen(outfile, "wb"))) {
        free(imagebuf);
        return Error("can't create '%s'", outfile);
    }
    
    /* write the header */
    if (fwrite(&hdr, 1, sizeof(hdr), fp) != sizeof(hdr)) {
        free(imagebuf);
        return Error("error writing '%s'", outfile);
    }
    
    /* write the image */
    if (fwrite(imagebuf, 1, imageSize, fp) != imageSize) {
        free(imagebuf);
        return Error("error writing '%s'", outfile);
    }
    
    fclose(fp);
    
    return TRUE;
}
