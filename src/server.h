#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>

typedef struct {
    int          file_descriptor;
    unsigned int port;
    unsigned int backlog;
} Server;

typedef struct {
    int           file_descriptor;
    unsigned long buffer_size;
    char         *request;
} Client;

typedef struct {
    char *method;
    char *url;
    char *protocol;
    char *host;
    char *user_agent;
    char *connection;
    char *accept;
} Request;

typedef struct {
    char         *status;
    char         *body;
    unsigned long size;
} Response;

Server create_server(int port, int backlog);
Client accept_client(Server *server, unsigned int buffer_size);

#endif // !SERVER_H
