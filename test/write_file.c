#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc == 3) {
        const char *path    = argv[1];
        const char *content = argv[2];

        printf("%s\n", path);
        printf("%s\n", content);

        FILE *fp = fopen(path, "w");
        if (!fp) {
            perror("fopen");
            return EXIT_FAILURE;
        }

        // 2a) Write formatted text
        if (fprintf(fp, "%s", content) < 0) {
            perror("fprintf");
            fclose(fp);
            return EXIT_FAILURE;
        }

    } else {
        printf("fuck you\n");
    }

    return EXIT_SUCCESS;
}
