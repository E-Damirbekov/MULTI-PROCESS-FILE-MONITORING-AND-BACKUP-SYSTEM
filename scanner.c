#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "scanner.h"

FileInfo* scan_directory(const char* path, int* count) {
    DIR *d = opendir(path);
    if (!d) return NULL; // [cite: 45]

    struct dirent *dir;
    FileInfo *list = NULL;
    *count = 0;

    while ((dir = readdir(d)) != NULL) {
        if (strstr(dir->d_name, ".txt")) { // [cite: 50]
            list = realloc(list, sizeof(FileInfo) * (*count + 1)); // [cite: 51]
            strncpy(list[*count].name, dir->d_name, 256); // [cite: 52]

            struct stat st;
            char full_path[512];
            sprintf(full_path, "%s/%s", path, dir->d_name);
            stat(full_path, &st);
            list[*count].size = st.st_size; // [cite: 57]

            (*count)++;
        }
    }
    closedir(d);
    return list; // [cite: 62]
}