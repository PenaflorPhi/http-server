#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT            8080
#define BACKLOG         16
#define MAX_BUFFER_SIZE 1024

static int TRUE = 1;
// static int FALSE = 0;

static const char *OK_RESPONSE = "HTTP/1.1 200 OK\r\n"
                                 "Content-Type: text/plain\r\n"
                                 "Content-Length: %d\r\n"
                                 "\r\n"
                                 "%s";
static const char *NOT_FOUND   = "HTTP/1.1 404 Not Found\r\n\r\n";

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
        char   *requested_path = NULL;

        // Response information
        int          written_bytes   = 0;
        unsigned int response_status = 0;
        char        *response        = calloc(MAX_BUFFER_SIZE, sizeof(char));
        char        *message         = NULL;

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
        requested_path = strtok(request_buffer, " ");
        requested_path = strtok(NULL, " ");
        printf("requested_path: %s\n", requested_path);

        if (strcmp(requested_path, "/") == 0 || strcmp(requested_path, "/index.html") == 0) {
            response_status = 200;
            message         = "";
        } else if ((strstr(requested_path, "/echo/")) != NULL) {
            response_status = 200;
            message         = requested_path + strlen("/echo/");
            printf("message: %s\n", message);
        } else {
            response_status = 404;
        }

        if (response_status == 200) {
            written_bytes =
                snprintf(response, MAX_BUFFER_SIZE, OK_RESPONSE, strlen(message), message);
            printf("response: %s\n", response);

            if (written_bytes < 0 || written_bytes > MAX_BUFFER_SIZE) {
                perror("Error formating the response or buffer is too small.\n");
                free(response);
                exit(EXIT_FAILURE);
            }
        } else if (response_status == 404) {
            response = (char *)NOT_FOUND;
        }

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
