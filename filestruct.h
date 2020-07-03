//
// Created by luca on 03/07/20.
//

#include <bits/types/FILE.h>
#include <zconf.h>

#ifndef PROGETTO_SO_FILELOADER_H
#define PROGETTO_SO_FILELOADER_H

#define BLOCK_MAX 32

typedef struct fblock{
    uint size;
    struct fblock *next;
    struct fblock *prev;
    char *buff;
} fblock_t;

typedef struct File{
    uint size;
    fblock_t *first;
    fblock_t *last;
    fblock_t *cur;
    uint pos;
} file_t;

file_t* load(FILE *file);

char next(file_t *file);
char prev(file_t *file);

void start(file_t *file);
void end(file_t *file);

#endif //PROGETTO_SO_FILELOADER_H
