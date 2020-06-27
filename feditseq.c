//
// Created by luca on 19/05/20.
//
#include "feditseq.h"
#include <stdlib.h>
#include <stdio.h>

/**
 * Calcola la lungezza in numero di byte
 * @param pFile il file di cui calcolare la lunghezza
 * @return la lunghezza del file espressa in numero di byte
 */
unsigned int flen(FILE *pFile) {
    fseek(pFile,0,SEEK_END);
    unsigned int len=ftell(pFile);
    rewind(pFile);
    return len;
}

int get(mrow_t *r, int pos){
    if(pos<r->off||pos>r->off+r->size)
        return -1;
    else
        return r->buff[pos-r->off];
}

void set(mrow_t *r, int pos, int value){
    if(pos<r->off||pos>r->size+r->off)
        return;
    r->buff[pos-r->off]=value;
}

void printrow(mrow_t *row, unsigned int length){
    int i=0;
    while(i<row->off){
        printf("*");
        i++;
    }
    while(i<row->off+row->size){
        printf("%d",get(row,i));
        i++;
    }
    while(i<length){
        printf("*");
        i++;
    }
}

void debug(dmatrix_t *dmatrix){
    for(int i=0;i<dmatrix->rows; i++){
        printrow(dmatrix->M[i],dmatrix->cols);
        printf("\n");
    }
}

mrow_t *createrow(int offset, int size) {
    mrow_t *row = malloc(sizeof(mrow_t));
    row->size=size;
    row->off=offset;
    row->buff=malloc(size* sizeof(int));
    for(int i=0;i<size;i++){
        row->buff[i]=0;
    }
    return row;
}

unsigned int max(unsigned int x, unsigned int y) {
    if(x>y)
        return x;
    else return y;
}

void initfirstrow(dmatrix_t *m){
    mrow_t *row = m->M[0];
    for(int i=0;i<m->cols;i++){
        row->buff[i]=i;
    }
}

void initfirstcol(dmatrix_t *m){
    for(int i=0;i<m->rows;i++){
        if(m->M[i]->off==0)
            m->M[i]->buff[0]=i;
    }
}

void initmatrix(dmatrix_t *m){
    int exclude= max(m->cols, m->rows) / 2;
    for(int i=0;i<m->rows;i++){
        mrow_t *row =NULL;
        if(i<exclude){
            //above exclusion
            row=createrow(0,m->cols-(exclude-i));
        } else if(i>m->rows-exclude-1){
            //below exclusion
            int off = i-(m->rows-exclude)+1;
            row=createrow(off,m->cols-off);
        } else{
            //mid lines
            row=createrow(0,m->cols);
        }
        m->M[i]=row;
    }
}

dmatrix_t* creatematrix(unsigned int cols, unsigned int rows){
    dmatrix_t *dmatrix = malloc(sizeof(dmatrix_t));
    dmatrix->cols=cols;
    dmatrix->rows=rows;
    dmatrix->M = malloc(rows*sizeof(mrow_t*));
    initmatrix(dmatrix);
    initfirstrow(dmatrix);
    initfirstcol(dmatrix);
    return dmatrix;
}

void completerow(mrow_t *r){
    int start = r->off;
    if(start==0) start++;
    for(int i=start;i<r->off+r->size;i++){
        r->buff[i-r->off]=9;
    }
}

void completematrix(dmatrix_t *m){
    for(int i=1;i<m->rows;i++){
        completerow(m->M[i]);
    }
}

void savesequence(FILE *file1, FILE *file2, FILE *out){
    unsigned int len1=flen(file1);
    unsigned int len2=flen(file2);
    dmatrix_t *dmatrix = creatematrix(len1+1,len2+1);
    completematrix(dmatrix);
    debug(dmatrix);
}