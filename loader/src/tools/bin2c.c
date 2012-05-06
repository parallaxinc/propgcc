#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    char base[100], opath[100], *name, *p;
    FILE *ifp, *ofp;
    int byte, cnt;

    if (argc < 2 || argc > 3) {
        fprintf(stderr, "usage: bin2c <infile> [ <outfile> ]\n");
        exit(1);
    }

    strcpy(base, argv[1]);
    if ((p = strrchr(base, '.')) != NULL)
        *p = '\0';

    if (argc >= 3)
        strcpy(opath, argv[2]);
    else {
        strcpy(opath, base);
        strcat(opath, ".c");
    }

    if ((name = strrchr(base, '/')) != NULL)
        ++name;
    else
        name = base;

    if (!(ifp = fopen(argv[1], "rb"))) {
        fprintf(stderr, "error: can't open: %s\n", argv[1]);
        exit(1);
    }

    if (!(ofp = fopen(opath, "wb"))) {
        fprintf(stderr, "error: can't create: %s\n", argv[2]);
        exit(1);
    }

    fprintf(ofp, "\
#include <stdint.h>\n\
\n\
uint8_t %s_array[] = {\n\
", name);

    cnt = 0;
    while ((byte = getc(ifp)) != EOF) {
        fprintf(ofp, " 0x%02x,", byte);
        if (++cnt == 8) {
            putc('\n', ofp);
            cnt = 0;
        }
    }

    if (cnt > 0)
        putc('\n', ofp);
    
    fprintf(ofp, "\
};\n\
\n\
int %s_size = sizeof(%s_array);\n\
", name, name);

    fclose(ifp);
    fclose(ofp);

    return 0;
}
