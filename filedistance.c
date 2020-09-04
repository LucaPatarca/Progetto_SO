//
// Created by luca on 02/09/20.
//
#include "filestruct.h"
#include "filedistance.h"
#include <stdlib.h>
#include <stdio.h>
#include <ftw.h>
#include <string.h>

#define PATH_MAX 4096
#define ENTRY_MAX 1024

file_t *file_from;
file_t *file_to;


unsigned int min(unsigned int val1, unsigned int val2, unsigned int val3){
    unsigned int min=val1;
    if(val2<min) min=val2;
    if(val3<min) min=val3;
    return min;
}

distanceTable_t *create_table(unsigned int size){
    distanceTable_t *table = malloc(sizeof(distanceTable_t));
    table->upByte=0;
    table->leftByte=0;
    table->posX=0;
    table->posY=0;
    table->sizeX=size+1;
    table->upper=malloc(table->sizeX * sizeof(unsigned int));
    table->current=malloc(table->sizeX * sizeof(unsigned int));
    for(int i=0; i<size+1;i++)
        table->current[i]=i;
    return table;
}

void destroy_table(distanceTable_t *table){
    free(table->upper);
    free(table->current);
    free(table);
}

/**
 * Computa la prossima cella, le celle up, left e upperleft devono essere gia definite
 * @param table
 */
void next_cell(distanceTable_t *table){
    unsigned int upperleft=table->upper[table->posX-1];
    if(table->leftByte==table->upByte) {
        table->current[table->posX] = upperleft;
    }else{
        unsigned int left=table->current[table->posX-1];
        unsigned int up=table->upper[table->posX];
        table->current[table->posX] = min(upperleft,up,left)+1;
    }
    table->posX++;
    table->upByte=next(file_to);
}

int next_row(distanceTable_t *table){
    int newLeftByte = next(file_from);
    if(newLeftByte == EOF) return EOF;
    if(newLeftByte == RMS) return RMS;
    unsigned int *tmp = table->upper;
    table->upper = table->current;
    table->current = tmp;
    table->posY++;
    table->leftByte= newLeftByte;
    table->current[0] = table->posY;
    table->posX=1;
    start(file_to);
    table->upByte=next(file_to);
    while(table->upByte != EOF){
        next_cell(table);
    }
    return 0;
}

int complete_table(distanceTable_t *table){
    int result = 0;
    while(result == 0){
        result = next_row(table);
    }
    return result;
}

int open_and_use_files(const char *path_file_from, const char *path_file_to){
    file_to = create_file(path_file_to);
    file_from = create_file_volatile(path_file_from);
    if(file_to == NULL){
        perror(path_file_to);
        return -1;
    }
    if(file_from == NULL){
        perror(path_file_from);
        return -1;
    }
    start(file_to);
    return 0;
}

void use_files(file_t *from, file_t *to){
    file_from = from;
    file_to = to;
    start(file_to);
    start(file_from);
}

long get_distance(distanceTable_t *table){
    return table->current[table->sizeX-1];
}

long file_distance(const char *path_file1, const char *path_file2){
    if(open_and_use_files(path_file1, path_file2) != 0)
        return -1;
    distanceTable_t *table = create_table(file_to->size);
    long distance;
    if(complete_table(table)==RMS){
        perror(path_file2);
        destroy_table(table);
        close_file(file_to);
        close_file(file_from);
        return -2;
    }else{
        distance=get_distance(table);
    }
    destroy_table(table);
    close_file(file_to);
    close_file(file_from);
    return distance;
}

typedef struct Entry{
    long distance;
    char path[PATH_MAX];
} entry_t;

entry_t entries[ENTRY_MAX];
char filepath[PATH_MAX];
long entries_limit=0;
int entries_size=0;

void clear_entries(){
    for(int i=0;i<ENTRY_MAX;i++)
        entries[i].distance=-1;
    entries_size=0;
}

int add_entry(long distance, const char path[]){
    entries[entries_size].distance=distance;
    strcpy(entries[entries_size].path, path);
    entries_size++;
    if(entries_size>=ENTRY_MAX)
        return 1;
    return 0;
}

int is_dir(const char *path){
    struct stat status;
    stat(path,&status);
    if(S_ISDIR(status.st_mode)!=0)
        return 1;
    else
        return 0;
}

int find_min(const char *fpath, const struct stat *sb, int typeflag){
    if(typeflag==FTW_F){
        if(strcmp(filepath,fpath)==0)
            return 0;
        long distance= file_distance(fpath, filepath);
        if(distance==-1)
            return 0;
        if (distance == -2)
            return 2;
        if(distance<entries_limit){
            clear_entries();
            entries_limit=distance;
        }
        if(distance==entries_limit){
            return add_entry(distance,fpath);
        }
    }
    return 0;
}


void searchmindistance(const char *file_path, const char *dir_path){
    if(!is_dir(dir_path)){
        printf("%s is not a directory\n",dir_path);
        return;
    }
    clear_entries();
    entries_limit=0xFFFFFFFF;
    realpath(file_path, filepath);
    char real_dir_path[PATH_MAX];
    realpath(dir_path,real_dir_path);
    int ret = ftw(real_dir_path,find_min,50);
    if(ret==-1){
        perror(dir_path);
        return;
    }
    int i=0;
    while(entries[i].distance!=-1 && i<ENTRY_MAX){
        printf("%s:\t%ld\n",entries[i].path,entries[i].distance);
        i++;
    }
    if(ret==1) printf("found more than %d files with the same minimum length but only the first %d are shown", ENTRY_MAX, ENTRY_MAX);
}

void order_entries(entry_t *pEntries){
    int i=0;
    while(i<ENTRY_MAX && pEntries[i].distance != -1){
        entry_t temp = pEntries[i];
        int j=i-1;
        while(j>=0 && pEntries[j].distance > temp.distance){
            pEntries[j + 1]=pEntries[j];
            j--;
        }
        pEntries[j + 1]=temp;
        i++;
    }
}

int find_all(const char *fpath, const struct stat *sb, int typeflag){
    if(strcmp(filepath,fpath)==0)
        return 0;
    if(typeflag==FTW_F){
        long distance = file_distance(filepath, fpath);
        if(distance==-2)
            return 2;
        if(distance!=-1 && distance<=entries_limit) {
            return add_entry(distance,fpath);
        }
    }
    return 0;
}

void searchalldistance(const char *file_path, const char *dir_path, int limit){
    if(!is_dir(dir_path)){
        printf("%s is not a directory\n",dir_path);
        return;
    }
    clear_entries();
    realpath(file_path, filepath);
    entries_limit=limit;
    char real_dir_path[PATH_MAX];
    realpath(dir_path,real_dir_path);
    int ret = ftw(real_dir_path, find_all, 50);
    if(ret==-1){
        perror(dir_path);
        return;
    }
    order_entries(entries);
    int i=0;
    while(entries[i].distance!=-1 && i<ENTRY_MAX){
        printf("%-7ld\t%s\n",entries[i].distance,entries[i].path);
        i++;
    }
    if(ret==1) printf("found more than %d files but only the first %d are shown", ENTRY_MAX,ENTRY_MAX);
}