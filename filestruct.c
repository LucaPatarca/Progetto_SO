//
// Created by luca on 03/07/20.
//

#include "filestruct.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

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

void destroyblock(fblock_t *block){
    free(block->buff);
    free(block);
}

void max_size_reached(file_t *file) {
    file->cur=NULL;
    errno=EOVERFLOW;
}

int next_block(file_t *file){
    if(file->fd!=-1){
        u_char *buf=malloc(BLOCK_MAX);
        uint n = read(file->fd,buf,BLOCK_MAX);
        if(n<1)file->cur=NULL;
        else {
            destroyblock(file->cur);
            file->cur=createhblock(buf,n);
            file->size+=n;
            if(file->size>MAX_SIZE) {
                max_size_reached(file);
                return -1;
            }
        }
    } else{
        file->cur=file->cur->next;
    }
    return 0;
}

file_t* create_file(const char *path){
    int fd=open(path,O_RDONLY);
    if(fd==-1)
        return NULL;
    __u_char *buff = malloc(BLOCK_MAX);
    uint n=read(fd,buff,BLOCK_MAX);
    file_t *file = malloc(sizeof(file_t));
    file->size=n;
    file->first=createhblock(buff, n);
    file->first->prev=NULL;
    fblock_t *prev=file->first;
    while(n==BLOCK_MAX){
        buff = malloc(BLOCK_MAX);
        n=read(fd,buff,BLOCK_MAX);
        file->size+=n;
        if(file->size>MAX_SIZE){
            errno=EOVERFLOW;
            return NULL;
        }
        prev->next=createhblock(buff,n);
        prev->next->prev=prev;
        prev=prev->next;
    }
    file->last=prev;
    if(file->last->size<1)file->last=file->last->prev;
    file->fd=-1;
    start(file);
    close(fd);
    return file;
}

file_t * create_file_volatile(const char *path){
    int fd=open(path,O_RDONLY);
    if(fd==-1)
        return NULL;
    __u_char *buff = malloc(BLOCK_MAX);
    uint n=read(fd,buff,BLOCK_MAX);
    file_t *file = malloc(sizeof(file_t));
    file->fd=fd;
    file->cur=createhblock(buff,n);
    file->first=NULL;
    file->last=NULL;
    file->pos=0;
    file->size=n;
    return file;
}

void close_file(file_t *file){
    if(file->fd!=-1 && file->cur){
        destroyblock(file->cur);
        close(file->fd);
    } else{
        file->cur=file->first;
        while(file->cur){
            fblock_t *temp=file->cur->next;
            destroyblock(file->cur);
            file->cur=temp;
        }
    }
    free(file);
}

__u_char get_current(file_t *file){
    return file->cur->buff[file->pos];
}

int next(file_t *file){
    if(!file->cur)
        return EOF;
    u_char toreturn=get_current(file);
    file->pos++;
    if(file->pos>=file->cur->size){
        file->pos=0;
        if(next_block(file)==-1)
            return RMS;
    }
    return toreturn;
}

int prev(file_t *file){
    if(file->fd!=-1){
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
