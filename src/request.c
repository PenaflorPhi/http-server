#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "server.h"

// Extracts a specific header value from the request by looking for "\r\n<Header>: "
// and returning a dynamically allocated string with the header's value.
static char *get_header(const char *request, const char *header, int buffer_size) {
    char *header_prefix = "\r\n%s: ";

    // Allocate a buffer to hold the header key. This buffer will be used
    // to search within the request string.
    char *request_header = calloc(buffer_size, sizeof(char));
    char *header_start, *header_end;

    // Write the header key into request_header using the provided header string.
    int bytes = snprintf(request_header, buffer_size, header_prefix, header);
    if (bytes < 0) {
        perror("Error getting header\n");
        exit(EXIT_FAILURE);
    } else if (bytes >= buffer_size) {
        perror("Request larger than buffer size.\n");
        exit(EXIT_FAILURE);
    }

    // Search the request for the header key
    if (!(header_start = strstr(request, request_header))) {
        free(request_header);
        return NULL;
    }
    // Advance the pointer past the header key to get to the header value.
    header_start += strlen(request_header);

    // Look for the end of the header value.
    if (!(header_end = strstr(header_start, "\r\n"))) {
        header_end = request_header + strlen(request_header);
    }

    memcpy(request_header, header_start, header_end - header_start);
    request_header[header_end - header_start] = '\0';

    return request_header;
}

// Extracts the request path from the request buffer by finding the method and the path tokens.
static char *get_path(const char *request_buffer) {
    const char *first_space, *second_space;
    if (!(first_space = strchr(request_buffer, ' '))) {
        if (!first_space) {
            return NULL;
        }
    }
    first_space += 1;

    if (!(second_space = strchr(first_space, ' '))) {
        return NULL;
    }

    size_t path_len = second_space - first_space;
    // Allocate memory for the path substring plus a null terminator.
    char *path = malloc(path_len);
    if (!path) {
        perror("malloc failed!");
        return NULL;
    }

    memcpy(path, first_space, path_len);
    path[path_len] = '\0';
    return path;
}

// Parses an incoming request from a client socket by reading the request buffer
// and extracting relevant parts.
Request parse_request(int client_fd, ssize_t buffer_size) {
    Request req;
    // Allocate memory for storing the full HTTP request.
    req.request = calloc(buffer_size, sizeof(char));

    // Receive data from the client socket.
    // recv returns the number of bytes read from the socket.
    ssize_t bytes_read = recv(client_fd, req.request, buffer_size, 0);
    if (bytes_read > 0) {
        // Null-terminate the received data to create a proper C string.
        req.request[bytes_read] = '\0';
    } else {
        // In case of an error, free the allocated memory and print an error.
        free(req.request);
        perror("recv failed!");
    }

    req.path       = get_path(req.request);
    req.user_agent = get_header(req.request, "User-Agent", buffer_size);

    return req;
}
