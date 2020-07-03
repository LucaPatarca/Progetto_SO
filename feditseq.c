//
// Created by luca on 19/05/20.
//
#include "feditseq.h"
#include "filestruct.h"
#include <stdlib.h>
#include <stdio.h>

row_t *last=NULL;
file_t *file1;
file_t *file2;

void debug(){
    row_t *cur=last;
    while(cur){
        for(int i=0;i<file1->size;i++){
            printf("|%3d|",cur->buff[i]);
        }
        printf("\n");
        cur=cur->upper;
    }
}

uint min(uint val1, uint val2, uint val3){
    uint min=val1;
    if(val2<min) min=val2;
    if(val3<min) min=val3;
    return min;
}

uint get_upper(row_t *row, uint posx){
    if(!row->upper) return posx+1;
    return row->upper->buff[posx];
}

uint get_left(row_t *row, uint posy, uint posx){
    if(posx==0) return posy+1;
    return row->buff[posx-1];
}

uint get_upperleft(row_t *row, uint posy, uint posx){
    if(!row->upper) return posx;
    if(posx==0) return posy;
    return row->upper->buff[posx-1];
}

row_t *createrow() {
    row_t *row = malloc(sizeof(row_t));
    row->upper=NULL;
    row->buff=malloc(file1->size* sizeof(uint));
    for(int i=0;i<file1->size;i++){
        row->buff[i]=0;
    }
    return row;
}

uint eval(row_t *row, uint posy, uint posx, char l, char u){
    uint upperleft=get_upperleft(row,posy, posx);
    if(l==u) return upperleft;
    uint left=get_left(row,posy,posx);
    uint up=get_upper(row,posx);
    return min(upperleft,up,left)+1;
}

void completerow(row_t *row, char left_char, uint row_num){
    char up_char;
    uint pos=0;
    while((up_char= next(file1)) != EOF){
        row->buff[pos]=eval(row,row_num,pos,left_char,up_char);
        pos++;
    }
    start(file1);
}

void completetable(){
    uint row_num=0;
    char cur;
    while((cur=next(file2))!=EOF){
        row_t *row = createrow();
        row->upper=last;
        completerow(row,cur,row_num);
        last=row;
        row_num++;
    }
}

void savesequence(FILE *source1, FILE *source2, FILE *out){
    file1=load(source1);
    file2=load(source2);
    completetable();
    debug();
}