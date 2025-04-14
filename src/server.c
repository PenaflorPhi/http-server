#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "server.h"

static const int TRUE = 1;

/*========================================================================
 *
 * Server
 *
========================================================================== */
static void create_socket(Server *serv) {
    if ((serv->file_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation faild!");
        exit(EXIT_FAILURE);
    }
}

static void set_socket_options(Server *serv) {
    if (setsockopt(serv->file_descriptor, SOL_SOCKET, SO_REUSEADDR, &TRUE, sizeof(TRUE)) < 0) {
        perror("setsockopt failed !");
        exit(EXIT_FAILURE);
    }
}

static void set_server_address(struct sockaddr_in *server_address, int port) {
    server_address->sin_family      = AF_INET;
    server_address->sin_addr.s_addr = INADDR_ANY;
    server_address->sin_port        = htons(port);
}

static void bind_socket(Server *serv, struct sockaddr_in address) {
    if (bind(serv->file_descriptor, (struct sockaddr *)&address, sizeof(address)) != 0) {
        perror("Binding failed!");
        exit(EXIT_FAILURE);
    }
}

static void listen_on_socket(Server *serv, int backlog) {
    if (listen(serv->file_descriptor, backlog) != 0) {
        perror("Listeing failed!");
        exit(EXIT_FAILURE);
    }
}

Server create_server(int port, int backlog) {
    Server server;

    struct sockaddr_in address;

    create_socket(&server);
    set_socket_options(&server);
    set_server_address(&address, port);
    bind_socket(&server, address);
    server.address = address;

    listen_on_socket(&server, backlog);
    return server;
}

/*========================================================================
 *
 * Client
 *
========================================================================== */

Client accept_client(Server *server) {
    Client client;
    client.address_size = sizeof(client.address);

    client.file_descriptor =
        accept(server->file_descriptor, (struct sockaddr *)&client.address, &client.address_size);

    if (client.file_descriptor == -1) {
        perror("Accepting incoming connection failed");
        exit(EXIT_FAILURE);
    }

    return client;
}
