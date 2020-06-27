//
// Created by luca on 18/05/20.
//
#ifndef PROGETTO_SO_FDISTANCE_H
#define PROGETTO_SO_FDISTANCE_H

typedef struct dtable {
    __u_int *buff;
    __u_int upperleft;
    __u_int size;
    __u_int pos;
    __u_int rownum;
} dtable_t;

int calcfiledistance(FILE *file1, FILE *file2);
void searchmindistance(FILE *file, FILE *path);

#endif //PROGETTO_SO_FDISTANCE_H
