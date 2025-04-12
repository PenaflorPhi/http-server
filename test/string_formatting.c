#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFFER_SIZE 1024

int main(void) {
    const char  *OK_RESPONSE    = "HTTP/1.1 200 OK\r\n"
                                  "Content-Type: text/plain\r\n"
                                  "Content-Length: %d\r\n"
                                  "\r\n"
                                  "\%s";
    char        *response       = calloc(MAX_BUFFER_SIZE * sizeof(char), sizeof(char));
    char        *message        = "Hello World!\n";
    unsigned int content_length = strlen(message);

    int written_bytes = snprintf(response, MAX_BUFFER_SIZE, OK_RESPONSE, content_length, message);
    if (written_bytes < 0 || written_bytes > MAX_BUFFER_SIZE) {
        perror("Error formating the response or buffer is too small.\n");
        free(response);
    }

    printf("%s", response);
    free(response);

    return EXIT_SUCCESS;
}
