//
// Created by luca on 19/05/20.
//

#ifndef PROGETTO_SO_FEDITSEQ_H
#define PROGETTO_SO_FEDITSEQ_H

#include <bits/types/FILE.h>

typedef struct mrow {
    int *buff;
    int size;
    int off;
} mrow_t;

typedef struct dmatrix {
    mrow_t **M;
    unsigned int cols;
    unsigned int rows;
}dmatrix_t;

void savesequence(FILE *file1, FILE *file2, FILE *out);
void applysequence(FILE *file, FILE *seq);

#endif //PROGETTO_SO_FEDITSEQ_H
