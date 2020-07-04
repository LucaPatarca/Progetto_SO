//
// Created by luca on 19/05/20.
//
#include "feditseq.h"
#include "filestruct.h"
#include <stdlib.h>
#include <stdio.h>

#define ADD 1
#define DEL 2
#define SET 3
#define NONE 4

seqtable_t *table;
file_t *file_to;
file_t *file_from;

void debug(){
    row_t *cur=table->last;
    while(cur){
        for(int i=0; i < file_to->size; i++){
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

uint get_upper(){
    if(!table->cur->upper) return table->posx+1;
    return table->cur->upper->buff[table->posx];
}

uint get_left(){
    if(table->posx==0) return table->posy+1;
    return table->cur->buff[table->posx-1];
}

uint get_upperleft(){
    if(!table->cur->upper) return table->posx;
    if(table->posx==0) return table->posy;
    return table->cur->upper->buff[table->posx-1];
}

uint get_distance(){
    return table->last->buff[file_to->size - 1];
}

uint get_curr(){
    return table->cur->buff[table->posx];
}

row_t *createrow() {
    row_t *row = malloc(sizeof(row_t));
    row->upper=NULL;
    row->buff=malloc(file_to->size * sizeof(uint));
    for(int i=0; i < file_to->size; i++){
        row->buff[i]=0;
    }
    return row;
}

uint eval(){
    uint upperleft=get_upperleft();
    if(table->left==table->up) return upperleft;
    uint left=get_left();
    uint up=get_upper();
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

void completerow(){
    table->posx=0;
    while((table->up= next(file_to)) != EOF){
        table->cur->buff[table->posx]=eval();
        table->posx++;
    }
    start(file_to);
}

void completetable(){
    table = createtable();
    table->posy=0;
    while((table->left=next(file_from)) != EOF){
        row_t *row = createrow();
        row->upper=table->cur;
        table->cur=row;
        completerow();
        table->posy++;
    }
    table->last=table->cur;
}

unsigned char* create_add(){
    unsigned char *buf = malloc(8);
    buf[0]='A';
    buf[1]='D';
    buf[2]='D';
    buf[3]=table->posx>>24;
    buf[4]=table->posx>>16;
    buf[5]=table->posx>>8;
    buf[6]=table->posx;
    buf[7]=table->up;

    return buf;
}

u_char * create_del(){
    __u_char *buf=malloc(7);
    buf[0]='D';
    buf[1]='E';
    buf[2]='L';
    buf[3]=table->posy>>24;
    buf[4]=table->posy>>16;
    buf[5]=table->posy>>8;
    buf[6]=table->posy;
    return buf;
}

__u_char * create_set(){
    unsigned char *buf = malloc(8);
    buf[0]='S';
    buf[1]='E';
    buf[2]='T';
    buf[3]=table->posy>>24;
    buf[4]=table->posy>>16;
    buf[5]=table->posy>>8;
    buf[6]=table->posy;
    buf[7]=table->up;
    return buf;
}

int choose_next(){
    uint up=get_upper();
    uint left=get_left();
    uint upperleft=get_upperleft();
    uint m = min(up,left,upperleft);
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
    table->left=prev(file_from);
}

void move_left(){
    table->posx--;
    table->up=prev(file_to);
}

void move_end(){
    end(file_to);
    end(file_from);
    table->cur=table->last;
    table->posx= file_to->size - 1;
    table->posy= file_from->size - 1;
    table->up=prev(file_to);
    table->left=prev(file_from);
}

__u_char ** createsequence(){
    uint distance = get_distance();
    __u_char **sequence=malloc(sizeof(char*)*distance);
    move_end();
    int seq_pos=0;
    while(table->up!=EOF&&table->left!=EOF){
        int result = choose_next();
        switch (result){
            case DEL:
                sequence[seq_pos++]=create_del();
                move_up();
                break;
            case ADD:
                sequence[seq_pos++]=create_add();
                move_left();
                break;
            case SET:
                sequence[seq_pos++]=create_set();
            case NONE:
                move_left();
                move_up();
            default:
                break;
        }
    }
    return sequence;
}

void savesequence(FILE *to, FILE *from, FILE *out){
    file_to=load(to);
    file_from=load(from);
    completetable();
    debug();
    __u_char ** seq=createsequence();
    for (int i=get_distance()-1;i>=0;i--){
        fwrite(seq[i],1,8,out);
    }
}