#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "scanner.h"

/* * Scans the specified directory for files ending in .txt.
 * Stores metadata in a dynamically allocated array.
 */
FileInfo* scan_directory(const char* path, int* count) {
    DIR *d = opendir(path);
    if (!d) return NULL;

    struct dirent *dir;
    FileInfo *list = NULL;
    *count = 0;

    /* Iterate through all items in the directory to filter for target files */
    while ((dir = readdir(d)) != NULL) {
        if (strstr(dir->d_name, ".txt")) { 
            /* Resize the array dynamically to accommodate a new file entry */
            list = realloc(list, sizeof(FileInfo) * (*count + 1)); 
            strncpy(list[*count].name, dir->d_name, 256);

            struct stat st;
            char full_path[512];
            sprintf(full_path, "%s/%s", path, dir->d_name);
            stat(full_path, &st);
            list[*count].size = st.st_size; // Store file size from inode data

            (*count)++;
        }
    }
    closedir(d);
    return list;
}