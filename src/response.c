#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int format_response(char *response_to_send, int buff_size, const char *response,
                    const char *response_status, const char *message) {
    int written_bytes;
    written_bytes =
        snprintf(response_to_send, buff_size, response, response_status, strlen(message), message);

    if (written_bytes < 0) {
        perror("Error formating the response.\n");
        free(response_to_send);
        exit(EXIT_FAILURE);
    } else if (written_bytes > buff_size) {
        perror("Response larger than buffer size.\n");
        free(response_to_send);
        exit(EXIT_FAILURE);
    }

    return written_bytes;
}

char *format_echo(char *s) {
    if ((s + strlen("/echo"))[0]) {
        return s + strlen("/echo") + 1;
    }
    return s + strlen("/echo");
}
