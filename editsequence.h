//
// Created by luca on 19/05/20.
//

#ifndef PROGETTO_SO_FEDITSEQ_H
#define PROGETTO_SO_FEDITSEQ_H

void savesequence(const char *from, const char *to, const char *out);
void applysequence(const char *file, const char *seq, const char *out);

long filedistance(const char *path_file1, const char *path_file2);
void searchmindistance(const char *file_path, const char *dir_path);
void searchalldistance(const char *file_path, const char *dir_path, int limit);
#endif //PROGETTO_SO_FEDITSEQ_H
