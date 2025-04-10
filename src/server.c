#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT             8080
#define BACKLOG          16
#define MAX_REQUEST_SIZE 1024

int main() {
    const char *OK_RESPONSE = "HTTP/1.1 200 OK\r\n\r\n";
    const char *NOT_FOUND   = "HTTP/1.1 404 Not Found\r\n\r\n";

    int TRUE = 1;

    int                server_fd, client_fd = -1;
    struct sockaddr_in server_addr, client_addr;
    unsigned int       client_addr_size = sizeof(client_addr);

    ssize_t bytes_read;
    char   *request_buffer = calloc(MAX_REQUEST_SIZE * sizeof(char), sizeof(char));

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
        // 5. Accept incoming connection
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_size);
        if (client_fd == -1) {
            perror("Accepting incoming connection failed");
            exit(EXIT_FAILURE);
        }

        // 6. Read and print client's request
        bytes_read = recv(client_fd, request_buffer, MAX_REQUEST_SIZE, 0);
        if (bytes_read > 0) {
            request_buffer[bytes_read] = '\0';
        } else {
            perror("recv failed!");
        }
        printf("bytes_read: %ld\n", bytes_read);
        printf("request_buffer:\n---\n%s\n---\n", request_buffer);

        // 7. Send response to the client
        if (strstr(request_buffer, "GET /index.html HTTP/1.1\r\n") != NULL) {
            if ((send(client_fd, OK_RESPONSE, strlen(OK_RESPONSE), 0)) == -1) {
                perror("Sending response failed!");
                exit(EXIT_FAILURE);
            }
        } else {
            if ((send(client_fd, NOT_FOUND, strlen(NOT_FOUND), 0)) == -1) {
                perror("Sending response failed!");
                exit(EXIT_FAILURE);
            }
        }

        shutdown(client_fd, SHUT_WR);
        close(client_fd);
    }
    close(server_fd);

    return EXIT_SUCCESS;
}
