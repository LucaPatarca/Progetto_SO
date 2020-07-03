//
// Created by luca on 03/07/20.
//

#include "filestruct.h"
#include <stdlib.h>
#include <stdio.h>

void start(file_t *file){
    file->cur=file->first;
    file->pos=0;
}

fblock_t * createhblock(char *buff, uint size){
    fblock_t *block = malloc(sizeof(fblock_t));
    block->size=size;
    block->buff=buff;
    block->next=NULL;
    return block;
}

file_t* load(FILE *from){
    char *buff = malloc(BLOCK_MAX);
    uint n=fread(buff,1,BLOCK_MAX,from);
    file_t *file = malloc(sizeof(file_t));
    file->size+=n;
    file->first=createhblock(buff, n);
    file->first->prev=NULL;
    fblock_t *prev=file->first;
    while(n==BLOCK_MAX){
        buff = malloc(BLOCK_MAX);
        n=fread(buff,1,BLOCK_MAX,from);
        file->size+=n;
        prev->next=createhblock(buff,n);
        prev->next->prev=prev;
        prev=prev->next;
    }
    file->last=prev;
    start(file);
    return file;
}

char get_current(file_t *file){
    return file->cur->buff[file->pos];
}

char next(file_t *file){
    if(!file->cur) return EOF;
    char toreturn=get_current(file);
    file->pos++;
    if(file->pos>=file->cur->size){
        file->pos=0;
        file->cur=file->cur->next;
    }
    return toreturn;
}

char prev(file_t *file){
    if(!file->cur) return EOF;
    char toreturn=get_current(file);
    if(file->pos==0){
        file->cur=file->cur->prev;
        if(file->cur) file->pos=file->cur->size;
    }
    file->pos--;
    return toreturn;
}

void end(file_t *file){
    file->cur=file->last;
    file->pos=file->cur->size-1;
}
