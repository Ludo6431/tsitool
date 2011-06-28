#include <stdlib.h>
#include <stdio.h>

typedef struct {
    union {
        char name[4];
        unsigned int id;
    };
    unsigned int size;
} tSECTION;

typedef struct {    // devices
    unsigned int nbitems;
} tDEVS;

#define be2cpu16(u) (((((u)>>0)&0xFF) << 8) | ((((u)>>8)&0xFF) << 0))
#define be2cpu32(u) (((((u)>>0)&0xFF) << 24) | ((((u)>>8)&0xFF) << 16) | ((((u)>>16)&0xFF) << 8) | ((((u)>>24)&0xFF) << 0))
#define MAKEID(a, b, c, d) be2cpu32((((a)&0xFF)<<24) | (((b)&0xFF)<<16) | (((c)&0xFF)<<8) | (((d)&0xFF)<<0))

char *dumpname(FILE *fd) {
    unsigned int len, i;
    static char mbname[1024];

    if(fread(&len, sizeof(unsigned int), 1, fd)!=1)
        printf("wtff? fread\n");
    len = be2cpu32(len);

    unsigned short *wcname = calloc(sizeof(unsigned short), len);
    if(!wcname)
        printf("wtff? malloc\n");

    if(fread(wcname, sizeof(unsigned short), len, fd)!=len)
        printf("wtff? fread\n");

    for(i=0; i<len; i++)
        wcname[i] = be2cpu16(wcname[i]);

    for(i=0; i<len; i++)
        mbname[i] = ((char *)(&wcname[i]))[0];
    mbname[i]='\0';

    return mbname;
}

void dumpsections(FILE *fd, tSECTION *sec) {
    tSECTION s;
    unsigned int i, size = 0;
    static unsigned int level = 0;
    level++;

    while(!sec || (sec && size<sec->size)) {
        if(fread((void *)&s, sizeof(tSECTION), 1, fd)!=1)
            break;
        s.size = be2cpu32(s.size);

        for(i=0; i<level; i++)
            printf("#");

        printf(" section %c%c%c%c (%u)\n", s.name[0], s.name[1], s.name[2], s.name[3], s.size);
        size+=sizeof(tSECTION) + s.size;

        switch(s.id) {
        case MAKEID('D', 'I', 'O', 'M'):
        {
            dumpsections(fd, &s);
            break;
        }
        case MAKEID('D', 'E', 'V', 'S'):    // devices
        {
            tDEVS devs;

            if(fread(&devs, sizeof(tDEVS), 1, fd)!=1)
                printf("wtff?\n fread");
            devs.nbitems = be2cpu32(devs.nbitems);

            for(i=0; i<level+2; i++)
                printf(" ");
            printf("nbitems=%u\n", devs.nbitems);

            dumpsections(fd, &s);
            break;
        }
        case MAKEID('D', 'E', 'V', 'I'):    // device item
        {
            for(i=0; i<level+2; i++)
                printf(" ");
            printf("devicename=%s\n", dumpname(fd));

            dumpsections(fd, &s);
            break;
        }
        case MAKEID('D', 'D', 'A', 'T'):    // dev ice data
        {
            dumpsections(fd, &s);
            break;
        }
        case MAKEID('D', 'D', 'I', 'V'):    // device data interpreter? version
        {
            for(i=0; i<level+2; i++)
                printf(" ");
            printf("deviceversion=%s\n", dumpname(fd));

            unsigned int version;
            if(fread(&version, sizeof(unsigned int), 1, fd)!=1)
                printf("wtff? fread\n");
            version = be2cpu32(version);
            for(i=0; i<level+2; i++)
                printf(" ");
            printf("version=      %u\n", version);
            break;
        }
        case MAKEID('D', 'D', 'I', 'C'):    // device data interpreter? comment
        {
            for(i=0; i<level+2; i++)
                printf(" ");
            printf("devicecomment=%s\n", dumpname(fd));
            break;
        }
        case MAKEID('D', 'D', 'P', 'T'):    // device data interpreter? t...?
        {
            for(i=0; i<level+2; i++)
                printf(" ");
            printf("devicet...=%s\n", dumpname(fd));

            for(i=0; i<level+2; i++)
                printf(" ");
            printf("devicet...=%s\n", dumpname(fd));
            break;
        }
        case MAKEID('D', 'D', 'D', 'C'):    // device data
        {
            dumpsections(fd, &s);
            break;
        }
        case MAKEID('D', 'D', 'C', 'I'):    // device data controller input
        case MAKEID('D', 'D', 'C', 'O'):    // device data controller input
        {
            unsigned int number;
            if(fread(&number, sizeof(unsigned int), 1, fd)!=1)
                printf("wtff? fread\n");
            number = be2cpu32(number);
            for(i=0; i<level+2; i++)
                printf(" ");
            printf("number=      %u\n", number);

            dumpsections(fd, &s);
            break;
        }
        case MAKEID('D', 'C', 'D', 'T'):    // device data controller
        {
            for(i=0; i<level+2; i++)
                printf(" ");
            printf("linkname=%s\n", dumpname(fd));

            char data[20];
            if(fread(data, sizeof(char), 20, fd)!=20)
                printf("wtff? fread\n");
            break;
        }
        default:
            fseek(fd, s.size, SEEK_CUR);
            break;
        }
    }

    level--;
}

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

#if 0
    char *fdata = malloc(fsize);
    if(!fdata)
        return EXIT_SUCCESS;

    if(fread(fdata, fsize, 1, fd)!=1)
        return EXIT_FAILURE;
    fclose(fd);

    // read header
    tSECTION *hdr = (tSECTION *)fdata;
    hdr->size = be2cpu(hdr->size);

    printf("magic=       %c%c%c%c\n", hdr->name[0], hdr->name[1], hdr->name[2], hdr->name[3]);
    printf("contentsize= %u\n", hdr->size);

    // read each section
    tSECTION *sec = NULL;
    for(i=0; fread(&sec, sizeof(tSECTION); i++) {
        
    }

    free(fdata);
#else
    dumpsections(fd, NULL);

    fclose(fd);
#endif

    return EXIT_SUCCESS;
}


