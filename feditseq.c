//
// Created by luca on 19/05/20.
//
#include "feditseq.h"
#include <stdlib.h>
#include <stdio.h>

#define BLOCK_MAX 32

hblock_t *first=NULL;
vblock_t *cur=NULL;

uint min(uint val1, uint val2, uint val3){
    uint min=val1;
    if(val2<min) min=val2;
    if(val3<min) min=val3;
    return min;
}

uint eval(uint upperleft, uint up, uint left, char l, char u){
    if(l==u) return upperleft+1;
    return min(upperleft,up,left)+1;
}

hblock_t * createhblock(char *buff, uint size){
    hblock_t *hblock = malloc(sizeof(hblock_t));
    hblock->size=size;
    hblock->buff=buff;
    hblock->next=NULL;
    return hblock;
}

void loadfile1(FILE *file){
    //TESTED
    char *buff = malloc(BLOCK_MAX);
    uint n=fread(buff,1,BLOCK_MAX,file);
    first=createhblock(buff,n);
    hblock_t *prev=first;
    while(n==BLOCK_MAX){
        buff = malloc(BLOCK_MAX);
        n=fread(buff,1,BLOCK_MAX,file);
        prev->next=createhblock(buff,n);
        prev=prev->next;
    }
}

vblock_t *createvblock(char *buff, uint size){
    if(size==0) return NULL;
    vblock_t *vblock = malloc(sizeof(vblock_t));
    vblock->size=size;
    vblock->buff=buff;
    return vblock;
}

void advancefile2(FILE *file2){
    //TESTED
    char *buff = malloc(BLOCK_MAX);
    uint n=fread(buff,1,BLOCK_MAX,file2);
    cur=createvblock(buff,n);
}

uint get_upper_left(tblock_t *block, uint globalx, uint globaly){
    if(!block->up&&!block->left)
        return 0;
    if(!block->up)
        return globalx;
    if(!block->left)
        return globaly;
    tblock_t *ulblock = block->up->left;
    return ulblock->M[ulblock->ysize-1][ulblock->xsize-1];
}

uint get_upper(tblock_t *tblock, uint x, uint globalx){
    tblock_t *ublock = tblock->up;
    if(ublock==NULL){
        return globalx+1;
    }
    return ublock->M[ublock->ysize-1][x];
}

uint get_left(tblock_t *tblock, uint y, uint globaly){
    tblock_t *left = tblock->left;
    if(left==NULL){
        return globaly+1;
    }
    return left->M[y][left->xsize-1];
}

void inittblock(tblock_t *tblock, vblock_t *vblock, hblock_t *hblock, uint xoff, uint yoff){
    if (tblock->xsize!=hblock->size) perror("wrong dimentions");
    if(tblock->ysize!=vblock->size) perror("wrong dimentions");
    uint upperleft=get_upper_left(tblock,xoff,yoff);
    uint left=get_left(tblock,0,yoff);
    uint up=get_upper(tblock,0,xoff);
    for(int y=0;y<tblock->ysize;y++){
        for (int x = 0; x < tblock->xsize; x++) {
            
        }
    }
}

tblock_t *createtblock(uint xsize, uint ysize){
    tblock_t *tblock = malloc(sizeof(tblock_t));
    tblock->xsize=xsize;
    tblock->ysize=ysize;
    tblock->M=malloc(xsize*ysize);
    tblock->left=NULL;
    tblock->up=NULL;
    return tblock;
}

void savesequence(FILE *file1, FILE *file2, FILE *out){
    loadfile1(file1);

}