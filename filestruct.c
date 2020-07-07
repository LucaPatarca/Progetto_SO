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

fblock_t * createhblock(__u_char *buff, uint size){
    fblock_t *block = malloc(sizeof(fblock_t));
    block->size=size;
    block->buff=buff;
    block->next=NULL;
    block->prev=NULL;
    return block;
}

void next_block(file_t *file){
    if(file->real_file){
        u_char *buf=malloc(BLOCK_MAX);
        uint n = fread(buf,1,BLOCK_MAX,file->real_file);
        if(n<1)file->cur=NULL;
        else {
            file->cur=createhblock(buf,n);
            file->size+=n;
        }
    } else{
        file->cur=file->cur->next;
    }
}

file_t* create_file(FILE *from){
    __u_char *buff = malloc(BLOCK_MAX);
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
    file->real_file=NULL;
    start(file);
    return file;
}

file_t * create_file_volatile(FILE *from){
    __u_char *buff = malloc(BLOCK_MAX);
    uint n=fread(buff,1,BLOCK_MAX,from);
    file_t *file = malloc(sizeof(file_t));
    file->real_file=from;
    file->cur=createhblock(buff,n);
    file->first=NULL;
    file->last=NULL;
    file->pos=0;
    file->size=n;
    return file;
}

__u_char get_current(file_t *file){
    return file->cur->buff[file->pos];
}

int next(file_t *file){
    if(!file->cur) return EOF;
    char toreturn=get_current(file);
    file->pos++;
    if(file->pos>=file->cur->size){
        file->pos=0;
        next_block(file);
    }
    return toreturn;
}

int prev(file_t *file){
    if(file->real_file){
        perror("cannot call prev() on a volatile file");
        return -1;
    }
    if(!file->cur) return EOF;
    __u_char toreturn=get_current(file);
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
