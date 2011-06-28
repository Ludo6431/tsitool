#include <stdlib.h>
#include <stdio.h>

#include "tsi.h"
#include "tools.h"

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
            dumpsections(fd, &s);   // recurse
            break;
        }
        case MAKEID('D', 'E', 'V', 'S'):    // devices
        {
            unsigned int nbdevs;

            if(fread(&nbdevs, sizeof(unsigned int), 1, fd)!=1)
                printf("wtff?\n fread");
            nbdevs = be2cpu32(nbdevs);

            for(i=0; i<level+2; i++)
                printf(" ");
            printf("nbdevs=%u\n", nbdevs);

            dumpsections(fd, &s);   // recurse
            break;
        }
        case MAKEID('D', 'E', 'V', 'I'):    // device item
        {
            for(i=0; i<level+2; i++)
                printf(" ");
            printf("devicename=%s\n", dumpname(fd));

            dumpsections(fd, &s);   // recurse
            break;
        }
        case MAKEID('D', 'D', 'A', 'T'):    // dev ice data
        {
            dumpsections(fd, &s);   // recurse
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
            dumpsections(fd, &s);   // recurse
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

            dumpsections(fd, &s);   // recurse
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

