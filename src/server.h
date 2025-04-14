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
    int   file_descriptor;
    char *request;
} Client;

typedef struct {
    char *status;
    char *body;
    char *response;
} Response;

typedef struct {
    char *request;
    char *path;
    char *user_agent;
} Request;

Server create_server(int port, int backlog);
Client accept_client(Server *server);

#endif // !SERVER_H
