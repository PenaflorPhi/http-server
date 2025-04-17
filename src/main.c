#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "request.h"
#include "response.h"
#include "server.h"
#include "utils.h"

#define PORT            8080
#define BACKLOG         16
#define MAX_BUFFER_SIZE 1024

/* ----------------------------------------------------
 * ------ Threading -----------------------------------
 * ---------------------------------------------------- */
void *thread_request_handler(void *arg) {
    Client *client = (Client *)arg;

    Request request = request_handler(client);
    send_response(&request, client);

    free(client->request);
    shutdown(client->file_descriptor, SHUT_WR);
    printf("client shutdown gracefully.\n");
    close(client->file_descriptor);
    printf("client closed gracefully.\n");

    free(client); // allocated memory for Client struct itself
    printf("free memory\n");
    pthread_exit(NULL);
}

/* ----------------------------------------------------
 * ------ Main ----------------------------------------
 * ---------------------------------------------------- */

int main() {
    Server server = create_server(PORT, BACKLOG);

    while (1) {
        pthread_t thread_id;
        Client    client     = accept_client(&server, MAX_BUFFER_SIZE);
        Client   *client_ptr = safe_malloc(sizeof(Client));

        if (client.file_descriptor < 0) {
            continue;
        }

        *client_ptr = client;
        if (pthread_create(&thread_id, NULL, thread_request_handler, (void *)client_ptr) != 0) {
            perror("Failed to create thread");
            free(client_ptr);
            continue;
        }
        printf("detaching thread: %lu\n", thread_id);
        pthread_detach(thread_id);
    }

    close(server.file_descriptor);
    printf("\nServer shutdown gracefully.\n");
    return EXIT_SUCCESS;
}
