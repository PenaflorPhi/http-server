#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *safe_calloc(unsigned int nmemb, unsigned int size) {
    void *buffer = calloc(nmemb, size);
    if (buffer == NULL) {
        perror("Error while callocating memory");
        return NULL;
    }
    return buffer;
}

void string_realloc(char **str) {
    size_t len  = strlen(*str);
    char  *temp = realloc(*str, len + 1);
    if (!temp) {
        perror("Error while reallocating memory");
        return; // leave the original pointer untouched
    }
    *str = temp;
}
