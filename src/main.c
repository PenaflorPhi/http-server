#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "request.h"
#include "server.h"

#define PORT            8080
#define BACKLOG         16
#define MAX_BUFFER_SIZE 1024

/* ----------------------------------------------------
 * ------ Main ----------------------------------------
 * ---------------------------------------------------- */

int main() {
    Server server = create_server(PORT, BACKLOG);

    while (1) {
        Client client = accept_client(&server, MAX_BUFFER_SIZE);

        if (client.file_descriptor < 0) {
            continue;
        }

        request_handler(&client);

        free(client.request);
        shutdown(client.file_descriptor, SHUT_WR);
        printf("client shutdown gracefully.\n");
        close(client.file_descriptor);
        printf("client close gracefully.\n");
    }

    close(server.file_descriptor);
    printf("\nServer shutdown gracefully.\n");
    return EXIT_SUCCESS;
}
