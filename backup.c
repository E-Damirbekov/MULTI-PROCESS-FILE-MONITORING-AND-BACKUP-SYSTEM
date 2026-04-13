#include <stdio.h>
#include "backup.h"

/* * Performs a binary copy of the source file to the destination folder.
 */
int perform_backup(const char* source_dir, const char* filename) {
    char src_path[512], dst_path[512];
    sprintf(src_path, "%s/%s", source_dir, filename);
    sprintf(dst_path, "backup/%s", filename);

    FILE *src = fopen(src_path, "rb");
    FILE *dst = fopen(dst_path, "wb");

    if (!src || !dst) {
        if (src) fclose(src);
        return -1;
    }

    char buffer[4096];
    size_t bytes;
    /* Read and write in blocks to efficiently handle large file data */
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dst);
    }

    fclose(src);
    fclose(dst);
    return 0;
}