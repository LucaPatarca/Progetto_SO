//
// Created by luca on 19/05/20.
//
#include "filestruct.h"
#include "filedistance.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define ADD 0
#define DEL 1
#define SET 2
#define NONE 3

typedef struct row{
    unsigned char *buff;
    struct row *upper;
} row_t;

typedef struct sequenceTable{
    row_t *cur;
    long posx;
    long posy;
}sequenceTable_t;

typedef struct editBlock{
    char type[4];
    unsigned int pos;
    unsigned char c;
}editBlock_t;

file_t *file_to;
file_t *file_from;

row_t *create_row(unsigned int size) {
    row_t *row = malloc(sizeof(row_t));
    row->upper=NULL;
    row->buff=malloc((size/4)+1);
    memset(row->buff,0,(size/4)+1);
    return row;
}

void destroy_row(row_t *row){
    free(row->buff);
    free(row);
}

sequenceTable_t *create_sequence_table(){
    sequenceTable_t *st = malloc(sizeof(sequenceTable_t));
    st->cur=create_row(file_to->size);
    st->posx=0;
    st->posy=0;
    st->cur->buff[0]=0xC0;
    return st;
}

void destroy_sequence_table(sequenceTable_t *sequenceTable){
    while (sequenceTable->cur){
        row_t *next = sequenceTable->cur->upper;
        destroy_row(sequenceTable->cur);
        sequenceTable->cur=next;
    }
    free(sequenceTable);
}

unsigned char choose_next_move(sequenceTable_t *sequenceTable, distanceTable_t *distanceTable){
    unsigned int up=distanceTable->upper[sequenceTable->posx];
    unsigned int left=distanceTable->current[sequenceTable->posx - 1];
    unsigned int upperleft=distanceTable->upper[sequenceTable->posx - 1];
    unsigned int m = min(up,left,upperleft);
    if(m==upperleft)
        if(upperleft == distanceTable->current[sequenceTable->posx])
            return NONE;
        else return SET;
    else if (m==left)
        return ADD;
    else if(m==up)
        return DEL;
    return -1;
}

void encode_current_row(sequenceTable_t *sequenceTable, distanceTable_t *distanceTable){
    sequenceTable->cur->buff[0]=0x40;
    for(sequenceTable->posx=1; sequenceTable->posx < distanceTable->sizeX; sequenceTable->posx++){
        unsigned int shift_by = (3-sequenceTable->posx%4)*2;
        unsigned char value = choose_next_move(sequenceTable, distanceTable);
        unsigned int pos = sequenceTable->posx/4;
        sequenceTable->cur->buff[pos]+=value<<shift_by;
    }
}

int complete_sequence_table(sequenceTable_t *sequenceTable, distanceTable_t *distanceTable){
    int result;
    while((result=next_row(distanceTable)) == 0){
        sequenceTable->posy++;
        row_t *tmp = sequenceTable->cur;
        sequenceTable->cur = create_row(file_to->size);
        sequenceTable->cur->upper=tmp;
        encode_current_row(sequenceTable, distanceTable);
    }
    return result;
}

unsigned char decode_current(sequenceTable_t *sequenceTable){
    unsigned int shifted_by = (3-sequenceTable->posx%4)*2;
    unsigned int pos = sequenceTable->posx/4;
    unsigned char value = sequenceTable->cur->buff[pos];
    unsigned char shiftedValue = value>>shifted_by;
    unsigned char maskedValue = shiftedValue & 0x3u;
    return maskedValue;
}

editBlock_t *new_edit_block(char *type, unsigned int pos, int c){
    if(c<0)
        return NULL;
    editBlock_t *new = malloc(sizeof(editBlock_t));
    new->c=c;
    strcpy(new->type,type);
    new->pos=pos;
    return new;
}

editBlock_t ** create_edit_sequence(sequenceTable_t *sequenceTable, unsigned int size){
    editBlock_t **sequence=malloc(sizeof(editBlock_t*)*size);

    sequenceTable->posx = file_to->size;
    sequenceTable->posy = file_from->size;
    end(file_to);
    unsigned char editChar=prev(file_to);

    unsigned int seq_pos=size-1;
    unsigned int pos=sequenceTable->posx-1;
    while(sequenceTable->posy>-1||sequenceTable->posx>-1){
        int result = decode_current(sequenceTable);
        switch (result){
            case DEL:
                sequence[seq_pos--]= new_edit_block("DEL", pos + 1, '\0');

                //move up
                sequenceTable->cur=sequenceTable->cur->upper;
                sequenceTable->posy--;

                break;
            case ADD:
                sequence[seq_pos--]= new_edit_block("ADD", pos, editChar);

                //move left
                sequenceTable->posx--;
                editChar=prev(file_to);

                pos--;
                break;
            case SET:
                sequence[seq_pos--]= new_edit_block("SET", pos, editChar);
            case NONE:
                //move left
                sequenceTable->posx--;
                editChar=prev(file_to);
                //move up
                sequenceTable->cur=sequenceTable->cur->upper;
                sequenceTable->posy--;

                pos--;
            default:
                break;
        }
    }
    return sequence;
}

void order_edit_sequence(editBlock_t ** seq, unsigned int size){
    for(int i=1;i<size;i++){
        editBlock_t *temp = seq[i];
        int j=i-1;
        while(j>=0 && seq[j]->pos>temp->pos){
            seq[j+1]=seq[j];
            j--;
        }
        seq[j+1]=temp;
    }
}

void edit_block_to_buffer(editBlock_t *block, unsigned char *buffer){
    strcpy((char *)buffer,block->type);
    for(unsigned char i=0;i<4;i++)
        buffer[i+3]= (block->pos>>(3-i)*8u) & 0xffu;
    buffer[7]=block->c;
}

void save_sequence(const char *from, const char *to, const char *out){
    file_from = create_file_volatile(from);
    file_to = create_file(to);
    if(file_to == NULL){
        perror(to);
        return;
    }
    if(file_from == NULL){
        perror(from);
        return;
    }

    sequenceTable_t *sequenceTable=create_sequence_table();

    use_files(file_from,file_to);
    distanceTable_t *distanceTable = create_table(file_to->size);

    complete_sequence_table(sequenceTable, distanceTable);

    /*while(sequenceTable->cur) {
        for (sequenceTable->posx = 0; sequenceTable->posx < distanceTable->sizeX; sequenceTable->posx++) {
            char toPrint = 0;
            switch (decode_current(sequenceTable)) {
                case 0:
                    toPrint = 'A';
                    break;
                case 1:
                    toPrint = 'D';
                    break;
                case 2:
                    toPrint = 'S';
                    break;
                case 3:
                    toPrint = 'N';
                    break;
            }
            printf("%4c", toPrint);
        }
        printf("\n");
        sequenceTable->cur=sequenceTable->cur->upper;
    }*/

    long distance = get_distance(distanceTable);
    editBlock_t **sequence = create_edit_sequence(sequenceTable, distance);
    order_edit_sequence(sequence, distance);

    int outputFd = creat(out,0644);
    unsigned char outputBuffer[8];
    for(long i = 0; i< distance; i++){
        //printf("%s  %d  %c\n",sequence[i]->type, sequence[i]->pos, sequence[i]->c);
        edit_block_to_buffer(sequence[i],outputBuffer);
        write(outputFd,outputBuffer,8);
        free(sequence[i]);
    }
    free(sequence);

    destroy_table(distanceTable);
    destroy_sequence_table(sequenceTable);
}

editBlock_t *create_editblock(const unsigned char *buf){
    editBlock_t *seqblock = malloc(sizeof(editBlock_t));
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

editBlock_t *next_editblock(int fd){
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

void apply_edit_block(editBlock_t *edit){
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

int check_editblock(editBlock_t *edit){
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
    editBlock_t *edit=next_editblock(seq_fd);
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