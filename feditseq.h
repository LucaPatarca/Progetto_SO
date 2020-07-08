//
// Created by luca on 19/05/20.
//

#ifndef PROGETTO_SO_FEDITSEQ_H
#define PROGETTO_SO_FEDITSEQ_H

#include <bits/types/FILE.h>
#include <zconf.h>

typedef struct row{
    uint *buff;
    struct row *upper;
} row_t;

typedef struct seqtable{
    row_t *cur;
    long posx;
    long posy;
    int up;
    int left;
    row_t *last;
}seqtable_t;

typedef struct editblock{
    char type[4];
    uint pos;
    char c;
}editblock_t;

void savesequence(FILE *from, FILE *to, FILE *out);
void applysequence(FILE *file, FILE *seq, FILE *out);

uint filedistance(FILE *file1, FILE *file2);

#endif //PROGETTO_SO_FEDITSEQ_H
