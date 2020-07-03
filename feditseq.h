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

void savesequence(FILE *file1, FILE *file2, FILE *out);
void applysequence(FILE *file, FILE *seq);

#endif //PROGETTO_SO_FEDITSEQ_H
