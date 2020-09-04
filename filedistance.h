//
// Created by luca on 02/09/20.
//

#ifndef FILEDISTANCE_FILEDISTANCE_H
#define FILEDISTANCE_FILEDISTANCE_H

#include "filestruct.h"

typedef struct distanceTable {
    int upByte;
    int leftByte;
    unsigned int posX;
    unsigned int posY;
    unsigned int *upper;
    unsigned int *current;
    unsigned int sizeX;
}distanceTable_t;

unsigned int min(unsigned int val1, unsigned int val2, unsigned int val3);
int open_and_use_files(const char *path_file_from, const char *path_file_to);
void use_files(file_t *from, file_t *to);
int next_row(distanceTable_t *table);
distanceTable_t *create_table(unsigned int size);
void destroy_table(distanceTable_t *table);
long get_distance(distanceTable_t *table);

long file_distance(const char *path_file1, const char *path_file2);
void searchmindistance(const char *file_path, const char *dir_path);
void searchalldistance(const char *file_path, const char *dir_path, int limit);

#endif //FILEDISTANCE_FILEDISTANCE_H
