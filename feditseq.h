//
// Created by luca on 19/05/20.
//

#ifndef PROGETTO_SO_FEDITSEQ_H
#define PROGETTO_SO_FEDITSEQ_H

#include <bits/types/FILE.h>
#include <zconf.h>

typedef struct hblock{
    uint size;
    struct hblock *next;
    char *buff;
} hblock_t;

typedef struct vblock{
    uint size;
    char *buff;
} vblock_t;

typedef struct tblock{
    uint xsize;
    uint ysize;
    struct tblock *up;
    struct tblock *left;
    uint **M;
} tblock_t;

void savesequence(FILE *file1, FILE *file2, FILE *out);
void applysequence(FILE *file, FILE *seq);

#endif //PROGETTO_SO_FEDITSEQ_H
