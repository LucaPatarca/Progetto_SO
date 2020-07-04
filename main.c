#include <stdio.h>
#include "fdistance.h"
#include "feditseq.h"

int main() {
    FILE *file1= fopen("/home/luca/Scrivania/prova1grossa","r");
    FILE *file2= fopen("/home/luca/Scrivania/prova2grossa","r");
    FILE *out= fopen("/home/luca/Scrivania/output","w");
    savesequence(file1,file2,out);
    return 0;
}
