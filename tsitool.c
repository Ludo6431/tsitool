#include <stdlib.h>
#include <stdio.h>

#include "tsidump.h"

int main(int argc, char *argv[]) {
    if(argc!=2)
        return EXIT_FAILURE;

    char *fname = argv[1];

    FILE *fd = fopen(fname, "rb+");
    if(!fd)
        return EXIT_FAILURE;

    fseek(fd, 0, SEEK_END);
    unsigned int fsize = ftell(fd);
    rewind(fd);
    printf("filesize=    %u\n", fsize);

    dumpsections(fd, NULL);

    fclose(fd);

    return EXIT_SUCCESS;
}

