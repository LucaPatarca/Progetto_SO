#include <stdio.h>
#include <string.h>
#include "fdistance.h"
#include "feditseq.h"

void print_usage(){

}

int main(int argc, char** argv) {
    if(argc<3){
        print_usage();
    } else{
        if(strcmp(argv[1],"distance")==0){
            FILE *file_from = fopen(argv[2],"r");
            FILE *file_to = fopen(argv[3],"r");
            FILE *file_out = fopen(argv[4],"w");
            savesequence(file_to,file_from,file_out);
        } else if(strcmp(argv[1],"apply")==0){
            FILE *file_from = fopen(argv[2],"r");
            FILE *file_seq = fopen(argv[3],"r");
            FILE *file_out = fopen(argv[4],"w");
            applysequence(file_from,file_seq,file_out);
        }
    }
    return 0;
}
