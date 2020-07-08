//
// Created by luca on 19/05/20.
//
#include "feditseq.h"
#include "filestruct.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

void destroy_row(row_t *row){
    free(row->buff);
    free(row);
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

unsigned char* add_seq_chunk(editblock_t *block){
    unsigned char *buf = malloc(8);
    strcpy((char *)buf,block->type);
    for(u_char i=0;i<4;i++)
        buf[i+3]= (block->pos>>(3-i)*8u) & 0xffu;
    buf[7]=block->c;
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

editblock_t *new_editblock(char type[4], uint pos, char c){
    editblock_t *new = malloc(sizeof(editblock_t));
    new->c=c;
    strcpy(new->type,type);
    new->pos=pos;
    return new;
}

editblock_t ** createsequence(uint size){
    editblock_t **sequence=malloc(sizeof(editblock_t*)*size);
    move_end();
    uint seq_pos=size-1;
    uint pos=table->posx;
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

void ordersequence(editblock_t ** seq, uint size){
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

void savesequence(FILE *from, FILE *to, FILE *out){
    file_to= create_file(to);
    file_from= create_file_volatile(from);
    completetable();
    uint distance = get_distance();
    editblock_t **seq=createsequence(distance);
    close_file(file_to);
    destroy_table();
    ordersequence(seq, distance);
    for (long i=0;i<distance;i++){
        u_char *out_buff=add_seq_chunk(seq[i]);
        fwrite(out_buff,1,8,out);
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
    for (u_char i=0;i<4; i++){
        seqblock->pos=(seqblock->pos<<8u)+buf[i+3];
    }
    seqblock->c=(char) buf[7];
    return seqblock;
}

editblock_t *next_editblock(FILE *file){
    u_char buff[8];
    uint n=fread(buff,8,1,file);
    if(n==1) return create_editblock(buff);
    else return NULL;
}

void applysequence(FILE *f_in, FILE *f_seq, FILE *f_out){
    u_char out[BLOCK_MAX];
    file_t *file=create_file_volatile(f_in);
    editblock_t *edit=next_editblock(f_seq);
    int pos=0, off=0, cur=next(file);
    while(cur!=EOF){
        if(pos>=BLOCK_MAX){
            fwrite(out,1,BLOCK_MAX,f_out);
            off+=pos;
            pos=0;
        }
        if(!edit || edit->pos>pos+off){
            out[pos++]=cur;
            cur=next(file);
        } else if(edit->pos==pos+off){
            //apply
            if(strcmp(edit->type,"ADD")==0){
                out[pos++]=edit->c;
            }else if(strcmp(edit->type,"DEL")==0){
                cur=next(file);
            } else if(strcmp(edit->type,"SET")==0){
                out[pos++]=edit->c;
                cur=next(file);
            }
            free(edit);
            edit=next_editblock(f_seq);
        }
    }
    if(pos>0){
        fwrite(out,1,pos,f_out);
    }
    close_file(file);
}

uint filedistance(FILE *file1, FILE *file2){
    file_to = create_file(file2);
    file_from = create_file_volatile(file1);
    completetable();
    uint distance = get_distance();
    destroy_table();
    close_file(file_from);
    close_file(file_to);
    return distance;
}