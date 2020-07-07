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
    char up;
    char left;
    row_t *last;
}seqtable_t;

typedef struct editblock{
    char *type;
    uint pos;
    char c;
}editblock_t;

void savesequence(FILE *to, FILE *from, FILE *out);
void applysequence(FILE *file, FILE *seq, FILE *out);

#endif //PROGETTO_SO_FEDITSEQ_H
