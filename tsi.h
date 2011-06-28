#ifndef _TSI_H
#define _TSI_H

typedef struct {
    union {
        char name[4];
        unsigned int id;
    };
    unsigned int size;
} tSECTION;

#endif

