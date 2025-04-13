#include <stdio.h>
#include <string.h>

static void show_chars(char *s) {
    for (unsigned int i = 0; i < strlen(s); i++) {
        if (s[i] == '\t') {
            printf("\\t");
        } else if (s[i] == '\n') {
            printf("\\n\n");
        } else if (s[i] == '\r') {
            printf("\\r");
        } else if (s[i] == '\0') {
            printf("\\0");
        } else {
            printf("%c", s[i]);
        }
    }
    printf("\n");
}
