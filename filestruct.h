//
// Created by luca on 03/07/20.
//

#ifndef PROGETTO_SO_FILELOADER_H
#define PROGETTO_SO_FILELOADER_H

#define BLOCK_MAX 256
#define MAX_SIZE 1000000
#define RMS -2

typedef struct fblock{
    unsigned int size;
    struct fblock *next;
    struct fblock *prev;
    unsigned char *buff;
} fblock_t;

typedef struct File{
    unsigned int size;
    fblock_t *first;
    fblock_t *last;
    fblock_t *cur;
    unsigned int pos;
    int fd;
} file_t;

file_t* create_file(const char *path);
file_t* create_file_volatile(const char *path);

void close_file(file_t *file);

int next(file_t *file);
int prev(file_t *file);

void start(file_t *file);
void end(file_t *file);

int check_file(int fd);
unsigned char get_current(file_t *file);

#endif //PROGETTO_SO_FILELOADER_H
