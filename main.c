#include <stdio.h>
#include <string.h>
#include "feditseq.h"

void print_usage(){
    printf("usage: filedistance distance file_from file_to [file_out] \n"
           "       filedistance apply file_from file_seq file_out \n");
}
//TODO controllare i file prima di usarli.
int main(int argc, char** argv) {
    if(argc<4){
        print_usage();
        return 0;
    }
    if(strcmp(argv[1],"distance")==0){
        if(argc==5){
            //savesequence
            FILE *file_from = fopen(argv[2],"r");
            FILE *file_to = fopen(argv[3],"r");
            FILE *file_seq = fopen(argv[4],"w");
            savesequence(file_from,file_to,file_seq);
            fclose(file_from);
            fclose(file_to);
            fclose(file_seq);
        } else if(argc==4){
            //filedistance
            FILE *file_from = fopen(argv[2],"r");
            FILE *file_to = fopen(argv[3],"r");
            uint distance = filedistance(file_from,file_to);
            fclose(file_from);
            fclose(file_to);
            printf("DISTANCE: %d\n",distance);
        } else{
            print_usage();
        }
        return 0;
    }
    if(strcmp(argv[1],"apply")==0){
        if(argc==5){
            //applysequence
            FILE *file_from = fopen(argv[2],"r");
            FILE *file_seq = fopen(argv[3],"r");
            FILE *file_out = fopen(argv[4],"w");
            applysequence(file_from,file_seq,file_out);
            fclose(file_from);
            fclose(file_seq);
            fclose(file_out);
        } else print_usage();
        return 0;
    }
    return 0;
}
