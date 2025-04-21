#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LIST_SIZE    32
#define MAX_ELEMENT_SIZE 32

int main(void) {
    // 1) Use a 2D char array for token storage
    char list[MAX_LIST_SIZE][MAX_ELEMENT_SIZE] = {{0}};

    // 2) Copy the literal into a mutable array
    char parameters[] = "1, 2, 3, 4";

    // 3) Tokenize: first call with the buffer, then NULL
    const char *delim = ", "; // split on comma and/or space
    char       *token = strtok(parameters, delim);

    size_t count = 0;
    while (token && count < MAX_LIST_SIZE) {
        // 4) Copy token into our list storage
        strncpy(list[count], token, MAX_ELEMENT_SIZE - 1);
        list[count][MAX_ELEMENT_SIZE - 1] = '\0';

        count++;
        token = strtok(NULL, delim);
    }

    // 5) Print out what we captured
    for (size_t i = 0; i < count; i++) {
        printf("%zu\t%s\n", i, list[i]);
    }

    return 0;
}
