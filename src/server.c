#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "response.h"

#define PORT            8080
#define BACKLOG         16
#define MAX_BUFFER_SIZE 1024

static int TRUE = 1;
// static int FALSE = 0;

static const char *RESPONSE = "HTTP/1.1 %s\r\n"
                              "Content-Type: text/plain\r\n"
                              "Content-Length: %zu\r\n"
                              "\r\n"
                              "%s";

int main() {
    // Client server information
    int                server_fd, client_fd = -1;
    struct sockaddr_in server_addr, client_addr;
    unsigned int       client_addr_size = sizeof(client_addr);

    // 1. Create a socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation faild!");
        exit(EXIT_FAILURE);
    }

    // 2. Set socket options.
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &TRUE, sizeof(TRUE)) < 0) {
        perror("setsockopt failed !");
        exit(EXIT_FAILURE);
    }

    // 3. Bind the socket to an addr (Any available) and port (8080)
    // The server_addr stores an IPv4 addr and port
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        perror("Binding failed!");
        exit(EXIT_FAILURE);
    }

    // 4. Listen on the socket for incoming connections
    if (listen(server_fd, BACKLOG) != 0) {
        perror("Listeing failed!");
        exit(EXIT_FAILURE);
    }

    while (TRUE) {
        // Buffer information
        ssize_t bytes_read;
        char   *request_buffer = calloc(MAX_BUFFER_SIZE, sizeof(char));
        char   *request_path   = NULL;

        // Response information
        char *response        = calloc(MAX_BUFFER_SIZE, sizeof(char));
        char *response_status = NULL;
        char *message         = NULL;

        // 5. Accept incoming connection
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_size);
        if (client_fd == -1) {
            perror("Accepting incoming connection failed");
            exit(EXIT_FAILURE);
        }

        // 6. Read client's request
        bytes_read = recv(client_fd, request_buffer, MAX_BUFFER_SIZE, 0);
        if (bytes_read > 0) {
            request_buffer[bytes_read] = '\0';
        } else {
            perror("recv failed!");
        }

        // 7. Get path from clients request and validate
        request_path = strtok(request_buffer, " ");
        request_path = strtok(NULL, " ");

        if (strcmp(request_path, "/") == 0 || strcmp(request_path, "/index.html") == 0) {
            response_status = "200 OK";
            message         = "";
        } else if ((strstr(request_path, "/echo")) != NULL) {
            response_status = "200 OK";
            message         = format_echo(request_path);
        } else {
            response_status = "404 Not Found";
            message         = "";
        }

        format_response(response, MAX_BUFFER_SIZE, RESPONSE, response_status, message);

        // 9. Send response to client
        if ((send(client_fd, response, strlen(response), 0)) == -1) {
            perror("Sending response failed!");
            exit(EXIT_FAILURE);
        }

        free(request_buffer);
        free(response);

        shutdown(client_fd, SHUT_WR);
        close(client_fd);
    }
    close(server_fd);

    return EXIT_SUCCESS;
}
