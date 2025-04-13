#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "request.h"
#include "response.h"
#include "server.h"

#define PORT            8080
#define BACKLOG         16
#define MAX_BUFFER_SIZE 1024

static int TRUE = 1;

int main() {
    Server server;
    // Client client;

    // Client server information
    int                client_fd;
    struct sockaddr_in client_addr;
    unsigned int       client_addr_size = sizeof(client_addr);

    server = create_server(PORT, BACKLOG);

    while (TRUE) {
        Request  req;
        Response res;

        client_fd =
            accept(server.file_descriptor, (struct sockaddr *)&client_addr, &client_addr_size);
        if (client_fd == -1) {
            perror("Accepting incoming connection failed");
            exit(EXIT_FAILURE);
        }
        printf("client_fd: %d\n", client_fd);

        // 2. Parse request from the client.
        req = parse_request(client_fd, MAX_BUFFER_SIZE);

        // 3. Process request, create response.
        res = process_request(&req, MAX_BUFFER_SIZE);

        // 9. Send response to client

        printf("resp.response:\n%s\n", res.response);
        if ((send(client_fd, res.response, strlen(res.response), 0)) == -1) {
            perror("Sending response failed!");
            exit(EXIT_FAILURE);
        }

        free(req.request);
        free(res.response);

        shutdown(client_fd, SHUT_WR);
        close(client_fd);
    }
    close(server.file_descriptor);

    return EXIT_SUCCESS;
}
