cmake_minimum_required(VERSION 3.15)
project(filedistance C)

set(CMAKE_C_STANDARD 11)

add_executable(Progetto_SO main.c fdistance.h fdistance.c feditseq.h feditseq.c filestruct.h filestruct.c)
