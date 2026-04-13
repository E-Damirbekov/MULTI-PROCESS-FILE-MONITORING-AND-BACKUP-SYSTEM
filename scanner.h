#ifndef SCANNER_H
#define SCANNER_H

/* Structure to hold metadata for discovered files */
typedef struct {
    char name[256];
    long size;
} FileInfo;

/* Function to traverse directory and find .txt files */
FileInfo* scan_directory(const char* path, int* count);

#endif