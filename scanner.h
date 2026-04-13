#ifndef SCANNER_H
#define SCANNER_H

typedef struct {
    char name[256];
    long size;
} FileInfo; // [cite: 14, 32]

FileInfo* scan_directory(const char* path, int* count); // [cite: 33]

#endif