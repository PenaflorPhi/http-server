#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>

char *read_file(const char *file_path) {
    struct stat st;
    printf("file_path: %s\n", file_path);
    if (stat(file_path, &st) < 0) {
        perror("stat");
        return NULL;
    }

    size_t file_size = st.st_size;
    printf("file_size:\n%lu\n", file_size);
    FILE *fp = fopen(file_path, "rb");

    if (!fp) {
        perror("fopen");
        return NULL;
    }

    char *buffer = calloc(sizeof(char), file_size + 1);
    if (!buffer) {
        perror("calloc");
        fclose(fp);
        return NULL;
    }

    if (fread(buffer, 1, file_size, fp) != file_size) {
        perror("fread");
        free(buffer);
        buffer = NULL;
    }

    fclose(fp);

    return buffer;
}
