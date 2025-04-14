#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "server.h"

static char *get_path(char *request_buffer) {
    char *request_path = strtok(request_buffer, " ");
    request_path       = strtok(NULL, " ");

    if (request_path[strlen(request_path) - 1] == '/') {
        request_path[strlen(request_path) - 1] = '\0';
    }

    return request_path;
}

Request parse_request(int client_fd, ssize_t buffer_size) {
    Request req;
    req.request = calloc(buffer_size, sizeof(char));

    ssize_t bytes_read = recv(client_fd, req.request, buffer_size, 0);
    if (bytes_read > 0) {
        req.request[bytes_read] = '\0';
    } else {
        free(req.request);
        perror("recv failed!");
    }

    req.path = get_path(req.request);
    return req;
}
