#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "editsequence.h"

void print_usage(){
    printf("usage: filedistance distance [file_from] [file_to] [file_out] (optional) \n"
           "       filedistance apply [file_from] [file_seq] [file_out] \n"
           "       filedistance search [file_path] [dir_path] \n"
           "       filedistance searchall [file_path] [dir_path] [limit]");
}

int main(int argc, char** argv) {
    if(argc<4){
        print_usage();
        return 1;
    }
    if(strcmp(argv[1],"distance")==0){
        if(argc==5){
            //savesequence
            savesequence(argv[2],argv[3],argv[4]);
            return 0;
        } else if(argc==4){
            //filedistance
            clock_t start = clock();
            long distance = filedistance(argv[2],argv[3]);
            clock_t end = clock();
            if(distance==-1){
                perror("DISTANCE");
                return 1;
            }
            double time = (double)(end-start)/CLOCKS_PER_SEC;
            printf("DISTANCE: %ld\nTIME: %f seconds",distance, time);
            return 0;
        } else{
            print_usage();
            return 1;
        }
    }
    if(strcmp(argv[1],"apply")==0){
        if(argc==5){
            //applysequence
            applysequence(argv[2],argv[3],argv[4]);
        } else print_usage();
        return 0;
    }
    if(strcmp(argv[1],"search")==0){
        if(argc==4){
            //search min
            searchmindistance(argv[2],argv[3]);
        }else{
            print_usage();
            return 1;
        }
        return 0;
    }
    if(strcmp(argv[1],"searchall")==0){
        if(argc==5){
            int limit=(int) strtol(argv[4],NULL,10);
            if(limit==0){
                printf("%s is not a valid limit\n",argv[4]);
                print_usage();
                return 0;
            }
            searchalldistance(argv[2],argv[3],limit);
        } else{
            print_usage();
            return 1;
        }
        return 0;
    }
    return 0;
}
