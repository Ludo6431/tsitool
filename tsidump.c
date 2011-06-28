#include <stdlib.h>
#include <stdio.h>

#include "tsi.h"
#include "tools.h"

char *dumpname(FILE *fd, unsigned int *rsize) {
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

    if(rsize)
        (*rsize) -= sizeof(unsigned short)*len;

    return mbname;
}

void dumpsections(FILE *fd, unsigned int secsize) {
    tSECTION s;
    unsigned int i, size = 0, rsize;
    static unsigned int level = 0;
    level++;

    while(size < secsize) {
        if(fread((void *)&s, sizeof(tSECTION), 1, fd)!=1)
            break;
        s.size = be2cpu32(s.size);
        rsize = s.size;

        for(i=0; i<level; i++)
            printf("#");

        printf(" section %c%c%c%c (%u)\n", s.name[0], s.name[1], s.name[2], s.name[3], s.size);
        size+=sizeof(tSECTION) + s.size;

        switch(s.id) {
        case MAKEID('D', 'I', 'O', 'M'):
        {
            dumpsections(fd, rsize);   // recurse
            break;
        }
        case MAKEID('D', 'E', 'V', 'S'):    // devices
        {
            // number of devices
            unsigned int nbdevs;
            if(fread(&nbdevs, sizeof(unsigned int), 1, fd)!=1)
                printf("wtff?\n fread");
            nbdevs = be2cpu32(nbdevs);
            for(i=0; i<level+2; i++)
                printf(" ");
            printf("nbdevs=%u\n", nbdevs);
            rsize -= sizeof(unsigned int);

            dumpsections(fd, rsize);   // recurse
            break;
        }
        case MAKEID('D', 'E', 'V', 'I'):    // device item
        {
            // device name
            for(i=0; i<level+2; i++)
                printf(" ");
            printf("devicename=%s\n", dumpname(fd, &rsize));

            dumpsections(fd, rsize);   // recurse
            break;
        }
        case MAKEID('D', 'D', 'A', 'T'):    // dev ice data
        {
            dumpsections(fd, rsize);   // recurse
            break;
        }
        case MAKEID('D', 'D', 'I', 'V'):    // device data interpreter? version
        {
            for(i=0; i<level+2; i++)
                printf(" ");
            printf("deviceversion=%s\n", dumpname(fd, &rsize));

            unsigned int version;
            if(fread(&version, sizeof(unsigned int), 1, fd)!=1)
                printf("wtff? fread\n");
            version = be2cpu32(version);
            for(i=0; i<level+2; i++)
                printf(" ");
            printf("version=      %u\n", version);
            rsize -= sizeof(unsigned int);
            break;
        }
        case MAKEID('D', 'D', 'I', 'C'):    // device data interpreter? comment
        {
            for(i=0; i<level+2; i++)
                printf(" ");
            printf("devicecomment=%s\n", dumpname(fd, &rsize));
            break;
        }
        case MAKEID('D', 'D', 'P', 'T'):    // device data interpreter? t...?
        {
            for(i=0; i<level+2; i++)
                printf(" ");
            printf("devicet...=%s\n", dumpname(fd, &rsize));

            for(i=0; i<level+2; i++)
                printf(" ");
            printf("devicet...=%s\n", dumpname(fd, &rsize));
            break;
        }
        case MAKEID('D', 'D', 'D', 'C'):    // device data
        {
            dumpsections(fd, rsize);   // recurse
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
            rsize -= sizeof(unsigned int);

            dumpsections(fd, rsize);   // recurse
            break;
        }
        case MAKEID('D', 'C', 'D', 'T'):    // device data controller
        {
            for(i=0; i<level+2; i++)
                printf(" ");
            printf("linkname=%s\n", dumpname(fd, &rsize));

            char data[20];
            if(fread(data, sizeof(char), 20, fd)!=20)
                printf("wtff? fread\n");
            rsize -= 20;
            break;
        }
        default:
            fseek(fd, rsize, SEEK_CUR);
            break;
        }
    }

    level--;
}

