#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>

typedef struct {
    int                file_descriptor;
    struct sockaddr_in address;
} Server;

typedef struct {
    int                file_descriptor;
    unsigned int       address_size;
    struct sockaddr_in address;
} Client;

typedef struct {
    char *status;
    char *body;
    char *response;
} Response;

typedef struct {
    char *request;
    char *path;
} Request;

Server create_server(int port, int backlog);
Client create_client(Server *serv);

#endif // !SERVER_H
