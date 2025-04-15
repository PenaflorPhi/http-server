#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "server.h"

static const int reuse_addr = 1;

/*========================================================================
 *
 * Server
 *
========================================================================== */
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

/*========================================================================
 *
 * Client
 *
========================================================================== */

Client accept_client(Server *server, unsigned int buffer_size) {
    Client client;
    client.buffer_size = buffer_size;
    client.request     = calloc(client.buffer_size, sizeof(char));

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
