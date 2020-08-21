//
// Created by luca on 19/05/20.
//
#include "filestruct.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ftw.h>
#include <unistd.h>

#define ADD 1
#define DEL 2
#define SET 3
#define NONE 4
#define ENTRY_MAX 1024
#define PATH_MAX 4096

typedef struct row{
    unsigned int *buff;
    struct row *upper;
} row_t;

typedef struct seqtable{
    row_t *cur;
    long posx;
    long posy;
    int up;
    int left;
    row_t *last;
}seqtable_t;

typedef struct editblock{
    char type[4];
    unsigned int pos;
    unsigned char c;
}editblock_t;

seqtable_t *table;
file_t *file_to;
file_t *file_from;

unsigned int min(unsigned int val1, unsigned int val2, unsigned int val3){
    unsigned int min=val1;
    if(val2<min) min=val2;
    if(val3<min) min=val3;
    return min;
}

unsigned int get_upper(){
    if(!table->cur->upper) return table->posx+1;
    return table->cur->upper->buff[table->posx];
}

unsigned int get_left(){
    if(table->posx==0) return table->posy+1;
    return table->cur->buff[table->posx-1];
}

unsigned int get_upperleft(){
    if(!table->cur->upper) return table->posx;
    if(table->posx==0) return table->posy;
    return table->cur->upper->buff[table->posx-1];
}

unsigned int get_distance(){
    if(file_to->size==0)
        return file_from->size;
    if(file_from->size==0)
        return file_to->size;
    return table->last->buff[file_to->size - 1];
}

unsigned int get_curr(){
    return table->cur->buff[table->posx];
}

row_t *createrow() {
    row_t *row = malloc(sizeof(row_t));
    row->upper=NULL;
    row->buff=malloc(file_to->size * sizeof(unsigned int));
    for(int i=0; i < file_to->size; i++){
        row->buff[i]=0;
    }
    return row;
}

void destroy_row(row_t *row){
    free(row->buff);
    free(row);
}

unsigned int eval(){
    unsigned int upperleft=get_upperleft();
    if(table->left==table->up) return upperleft;
    unsigned int left=get_left();
    unsigned int up=get_upper();
    return min(upperleft,up,left)+1;
}

seqtable_t *createtable(){
    seqtable_t *st = malloc(sizeof(seqtable_t));
    st->left=0;
    st->up=0;
    st->cur=NULL;
    st->last=NULL;
    st->posx=0;
    st->posy=0;
    return st;
}

void destroy_table(){
    if(table->last) table->cur=table->last;
    while (table->cur){
        row_t *next = table->cur->upper;
        destroy_row(table->cur);
        table->cur=next;
    }
    free(table);
}

void completerow(){
    table->posx=0;
    while((table->up= next(file_to)) != EOF){
        table->cur->buff[table->posx]=eval();
        table->posx++;
    }
    start(file_to);
}

int completetable(){
    table = createtable();
    table->posy=0;
    while((table->left=next(file_from)) != EOF){
        if(table->left == RMS)
            return RMS;
        row_t *row = createrow();
        row->upper=table->cur;
        table->cur=row;
        completerow();
        table->posy++;
    }
    table->last=table->cur;
    return 0;
}

unsigned char* add_seq_chunk(editblock_t *block){
    unsigned char *buf = malloc(8);
    strcpy((char *)buf,block->type);
    for(unsigned char i=0;i<4;i++)
        buf[i+3]= (block->pos>>(3-i)*8u) & 0xffu;
    buf[7]=block->c;
    return buf;
}

int choose_next(){
    unsigned int up=get_upper();
    unsigned int left=get_left();
    unsigned int upperleft=get_upperleft();
    unsigned int m = min(up,left,upperleft);
    if(m==upperleft)
        if(upperleft==get_curr())
            return NONE;
        else return SET;
    else if (m==left)
        return ADD;
    else if(m==up)
        return DEL;
    return -1;
}

void move_up(){
    table->cur=table->cur->upper;
    table->posy--;
}

void move_left(){
    table->posx--;
    table->up=prev(file_to);
}

void move_end(){
    end(file_to);
    table->cur=table->last;
    table->posx= file_to->size - 1;
    table->posy= file_from->size - 1;
    close_file(file_from);
    table->up=prev(file_to);
}

editblock_t *new_editblock(char type[4], unsigned int pos, int c){
    if(c<0)
        return NULL;
    editblock_t *new = malloc(sizeof(editblock_t));
    new->c=c;
    strcpy(new->type,type);
    new->pos=pos;
    return new;
}

editblock_t ** createsequence(unsigned int size){
    editblock_t **sequence=malloc(sizeof(editblock_t*)*size);
    move_end();
    unsigned int seq_pos=size-1;
    unsigned int pos=table->posx;
    while(table->posy>-1||table->posx>-1){
        int result = choose_next();
        switch (result){
            case DEL:
                sequence[seq_pos--]=new_editblock("DEL",pos+1,'\0');
                move_up();
                break;
            case ADD:
                sequence[seq_pos--]=new_editblock("ADD",pos,table->up);
                move_left();
                pos--;
                break;
            case SET:
                sequence[seq_pos--]=new_editblock("SET",pos,table->up);
            case NONE:
                move_left();
                move_up();
                pos--;
            default:
                break;
        }
    }
    return sequence;
}

void ordersequence(editblock_t ** seq, unsigned int size){
    for(int i=1;i<size;i++){
        editblock_t *temp = seq[i];
        int j=i-1;
        while(j>=0 && seq[j]->pos>temp->pos){
            seq[j+1]=seq[j];
            j--;
        }
        seq[j+1]=temp;
    }
}

void savesequence(const char *from, const char *to, const char *out){
    file_to = create_file(to);
    file_from = create_file_volatile(from);
    if(!file_to) {
        perror(to);
        return;
    }
    if(!file_from) {
        perror(from);
        return;
    }
    if(completetable()==RMS){
        perror(from);
        return;
    }
    unsigned int distance = get_distance();
    editblock_t **seq=createsequence(distance);
    close_file(file_to);
    destroy_table();
    ordersequence(seq, distance);
    int out_fd = creat(out,0644);
    for (long i=0;i<distance;i++){
        unsigned char *out_buff=add_seq_chunk(seq[i]);
        write(out_fd,out_buff,8);
        free(out_buff);
        free(seq[i]);
    }
    free(seq);
}

editblock_t *create_editblock(const unsigned char *buf){
    editblock_t *seqblock = malloc(sizeof(editblock_t));
    seqblock->pos=0;
    for (int i = 0; i < 3; i++) {
        seqblock->type[i] = (char) buf[i];
    }
    seqblock->type[3]='\0';
    for (unsigned char i=0;i<4; i++){
        seqblock->pos=(seqblock->pos<<8u)+buf[i+3];
    }
    seqblock->c=(char) buf[7];
    return seqblock;
}

editblock_t *next_editblock(int fd){
    unsigned char buff[8];
    unsigned int n=read(fd,buff,8);
    if(n==8) return create_editblock(buff);
    else return NULL;
}

int seq_pos;
int seq_off;
unsigned char seq_out[BLOCK_MAX];
int seq_in_cur;
file_t *seq_in_file;

int seq_init(const char *in_file_path){
    seq_in_file=create_file_volatile(in_file_path);
    if(!seq_in_file){
        perror(in_file_path);
        return -1;
    }
    seq_pos=0;
    seq_off=0;
    seq_in_cur=next(seq_in_file);
    return 0;
}

void apply_edit_block(editblock_t *edit){
    if(strcmp(edit->type,"ADD")==0){
        seq_out[seq_pos++]=edit->c;
    }else if(strcmp(edit->type,"DEL")==0){
        seq_in_cur=next(seq_in_file);
    } else if(strcmp(edit->type,"SET")==0){
        seq_out[seq_pos++]=edit->c;
        seq_in_cur=next(seq_in_file);
    }
}

void write_seq_buffer(int out_fd){
    write(out_fd, seq_out, BLOCK_MAX);
    seq_off+=seq_pos;
    seq_pos=0;
}

int check_editblock(editblock_t *edit){
    if(edit==NULL) return 0;
    if(strcmp(edit->type,"ADD")==0 ||
       strcmp(edit->type,"SET")==0 ||
       strcmp(edit->type,"DEL")==0){
        return 0;
    }
    return 1;
}

void applysequence(const char *in_path, const char *seq_path, const char *out_path){
    if(seq_init(in_path)!=0)
        return;
    int seq_fd = open(seq_path,O_RDONLY);
    if(check_file(seq_fd)!=0){
        perror(seq_path);
        return;
    }
    int out_fd = creat(out_path,0644);
    if(out_fd==-1){
        perror(out_path);
        return;
    }
    if(seq_fd==-1){
        perror(in_path);
        return;
    }
    editblock_t *edit=next_editblock(seq_fd);
    if(check_editblock(edit)!=0){
        printf("%s is not a valid edit sequence file or it is corrupted.\n",seq_path);
        return;
    }
    while(seq_in_cur != EOF){
        if(seq_pos >= BLOCK_MAX){
            write_seq_buffer(out_fd);
        }
        if(!edit || edit->pos > seq_pos + seq_off){
            seq_out[seq_pos++]=seq_in_cur;
            seq_in_cur=next(seq_in_file);
        } else if(edit->pos == seq_pos + seq_off){
            apply_edit_block(edit);
            free(edit);
            edit=next_editblock(seq_fd);
            if(check_editblock(edit)!=0){
                printf("%s is not a valid edit sequence file or it is corrupted.\n",seq_path);
                return;
            }
        }
    }
    if(seq_pos > 0){
        write(out_fd, seq_out, seq_pos);
    }
    close_file(seq_in_file);
}

char search_interrupt=0;

long filedistance(const char *path_file1, const char *path_file2){
    file_to = create_file(path_file1);
    file_from = create_file_volatile(path_file2);
    if(!file_from){
        search_interrupt=1;
        perror(path_file2);
        return -1;
    }
    if(!file_to){
        perror(path_file1);
        return -1;
    }
    long distance = -1;
    if(completetable()==RMS){
        search_interrupt=1;
        perror(path_file2);
    }else{
        distance=get_distance();
    }
    destroy_table();
    close_file(file_from);
    close_file(file_to);
    return distance;
}

typedef struct Entry{
    long distance;
    char path[PATH_MAX];
} entry_t;

entry_t entries[ENTRY_MAX];
const char *filepath;
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

int find_min(const char *fpath, const struct stat *sb, int typeflag){
    if(typeflag==FTW_F){
        long distance=filedistance(fpath,filepath);
        if(distance==-1){
            if(search_interrupt)
                return 2;
            else return 0;
        }
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

int is_dir(const char *path){
    struct stat status;
    stat(path,&status);
    if(S_ISDIR(status.st_mode)!=0)
        return 1;
    else
        return 0;
}

void searchmindistance(const char *file_path, const char *dir_path){
    if(!is_dir(dir_path)){
        printf("%s is not a directory\n",dir_path);
        return;
    }
    clear_entries();
    entries_limit=0xFFFFFFFF;
    filepath=file_path;
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
    if(typeflag==FTW_F){
        long distance = filedistance(filepath,fpath);
        if(distance!=-1 && distance<=entries_limit) {
            return add_entry(distance,fpath);
        } else if(search_interrupt)
            return 2;
    }
    return 0;
}

void searchalldistance(const char *file_path, const char *dir_path, int limit){
    if(!is_dir(dir_path)){
        printf("%s is not a directory\n",dir_path);
        return;
    }
    clear_entries();
    filepath=file_path;
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