#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "server.h"

#define PORT            8080 // Port number for the server to listen on.
#define BACKLOG         16   // Maximum number of pending connections in the queue.
#define MAX_BUFFER_SIZE 1024 // Maximum size of the buffer for request/response handling.

#define MAX_METHOD_SIZE   8
#define MAX_URL_SIZE      1024
#define MAX_PROTOCOL_SIZE 16

/*
 * request.h
 */

static void receive_request(Client *client) {
    ssize_t bytes = recv(client->file_descriptor, client->request, client->buffer_size, 0);
    if (bytes > 0) {
        client->request[bytes] = '\0';
    } else {
        free(client->request);
        perror("failed while trying to receive request.");
    }
}

static void free_request(Request *request) {
    free(request->method);
    free(request->protocol);
    free(request->url);
    free(request->host);
    free(request->user_agent);
    free(request->connection);
    free(request->accept);
}

static void initialize_request(Request *request) {
    /* Request start line*/
    request->method   = calloc(MAX_METHOD_SIZE, sizeof(char));
    request->protocol = calloc(MAX_PROTOCOL_SIZE, sizeof(char));
    request->url      = calloc(MAX_URL_SIZE, sizeof(char));

    /* Request header */
    request->host       = calloc(MAX_URL_SIZE, sizeof(char));
    request->user_agent = calloc(MAX_URL_SIZE, sizeof(char));
    request->connection = calloc(MAX_URL_SIZE, sizeof(char));
    request->accept     = calloc(MAX_URL_SIZE, sizeof(char));
}

static Request parse_request_line(char *client_request) {
    printf("parsing request\n");
    Request request;
    initialize_request(&request);

    int response_size =
        sscanf(client_request, "%7s %1023s %15s", request.method, request.url, request.protocol);
    if (response_size == 3) {
        printf("method: %s\n", request.method);
        printf("url: %s\n", request.url);
        printf("protocol: %s\n", request.protocol);
    } else {
        free_request(&request);
        perror("Malformed request, could not find one of method, url, protocol");
    }

    return request;
}

static void parse_header(char *client_request, Request *request) {
    char *line = strtok(client_request, "\r\n");

    // Skip the request line.
    if (line != NULL)
        line = strtok(NULL, "\r\n");

    for (; line != NULL && line[0] != '\0'; line = strtok(NULL, "\r\n")) {
        char *header = strchr(line, ':');
        if (header != NULL) {
            header += 2; // Offset colon followed by space.
        }

        if (header != NULL) {
            if (strncmp(line, "User-Agent", strlen("User-Agent")) == 0) {
                request->user_agent = header;
            } else if (strncmp(line, "Host", strlen("Host")) == 0) {
                request->host = header;
            } else if (strncmp(line, "Accept", strlen("Accept")) == 0) {
                request->accept = header;
            }
        }
    }
    printf("`Host`: %s\n", request->host);
    printf("`User-Agent`: %s\n", request->user_agent);
    printf("`Accept`: %s\n", request->accept);
}

void request_handler(Client *client) {
    receive_request(client);
    char *client_request = strdup(client->request);
    printf("request:\n%s", client_request);

    Request request = parse_request_line(client_request);
    parse_header(client_request, &request);
}

/*
 * Main
 */

int main() {
    Server server = create_server(PORT, BACKLOG);

    while (1) {
        Client client = accept_client(&server, MAX_BUFFER_SIZE);

        if (client.file_descriptor < 0) {
            continue;
        }

        request_handler(&client);

        free(client.request);
        shutdown(client.file_descriptor, SHUT_WR);
        printf("client shutdown gracefully.\n");
        close(client.file_descriptor);
        printf("client close gracefully.\n");
    }

    close(server.file_descriptor);
    printf("\nServer shutdown gracefully.\n");
    return EXIT_SUCCESS;
}
