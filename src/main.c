#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT            8080 // Port number for the server to listen on.
#define BACKLOG         16   // Maximum number of pending connections in the queue.
#define MAX_BUFFER_SIZE 1024 // Maximum size of the buffer for request/response handling.

/*
 * server.c
 */

static const int reuse_addr = 1;

typedef struct {
    int          file_descriptor;
    unsigned int port;
    unsigned int backlog;
} Server;

static void set_socket_options(Server *server) {
    int socket_options = setsockopt(
        server->file_descriptor, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));
    if (socket_options < 0) {
        perror("setsockopt failed !");
        exit(EXIT_FAILURE);
    }
}

static void bind_socket(Server *serv, struct sockaddr_in *address) {
    if (bind(serv->file_descriptor, (struct sockaddr *)address, sizeof(*address)) != 0) {
        perror("binding socket failed");
        exit(EXIT_FAILURE);
    }
}

static void listen_on_socket(Server *server) {
    if (listen(server->file_descriptor, server->backlog) != 0) {
        perror("listening failed");
        exit(EXIT_FAILURE);
    }
}

static void create_socket(Server *server, struct sockaddr_in *address) {
    if ((server->file_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    set_socket_options(server);

    bind_socket(server, address);
    printf("Binding to port %d successful.\n", server->port);

    listen_on_socket(server);
    printf("Listening with backlog %d.\n", server->backlog);
}

static struct sockaddr_in config_network_addr_structure(Server *server) {
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_port        = htons(server->port);
    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    return address;
}

Server create_server(int port, int backlog) {
    Server server;
    server.backlog = backlog;
    server.port    = port;

    struct sockaddr_in address = config_network_addr_structure(&server);
    create_socket(&server, &address);
    printf("Socket created successfully.\n");

    return server;
}

/*
 * Client
 */
typedef struct {
    int   file_descriptor;
    char *request;
} Client;

Client accept_client(Server *server) {
    Client             client;
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));

    unsigned int address_size = sizeof(address);

    client.file_descriptor =
        accept(server->file_descriptor, (struct sockaddr *)&address, &address_size);

    if (client.file_descriptor == -1) {
        perror("accept failed");
        client.file_descriptor = -1;
        return client;
    }

    printf("client accepted with file_descriptor: %d\n", client.file_descriptor);

    return client;
}

/*
 * Main
 * */

int main() {
    Server server = create_server(PORT, BACKLOG);

    while (1) {
        Client client = accept_client(&server);

        if (client.file_descriptor < 0) {
            continue;
        }

        shutdown(client.file_descriptor, SHUT_WR);
        printf("client shutdown gracefully.\n");
        close(client.file_descriptor);
        printf("client close gracefully.\n");
    }

    close(server.file_descriptor);
    printf("\nServer shutdown gracefully.\n");
    return EXIT_SUCCESS;
}
