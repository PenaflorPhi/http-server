#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "args.h"
#include "request.h"
#include "response.h"
#include "server.h"
#include "utils.h"

#define PORT            4221
#define BACKLOG         16
#define MAX_BUFFER_SIZE 1024

/* ----------------------------------------------------
 * ------ Threading -----------------------------------
 * ---------------------------------------------------- */

typedef struct {
    Client   *client;
    Settings *settings;
} ThreadArgs;

void *thread_request_handler(void *args) {
    ThreadArgs *args_local = (ThreadArgs *)args;
    Client     *client     = (Client *)args_local->client;
    Settings   *settings   = (Settings *)args_local->settings;

    Request request = request_handler(client);
    send_response(&request, client, settings);

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

int main(int argc, char **argv) {
    Settings settings = parse_arguments(argc, argv);
    printf("directory: %s\n", settings.directory);

    Server server = create_server(PORT, BACKLOG);

    while (1) {
        pthread_t thread_id;
        Client    client       = accept_client(&server, MAX_BUFFER_SIZE);
        Client   *client_ptr   = safe_malloc(sizeof(Client));
        Settings *settings_ptr = safe_malloc(sizeof(Settings));

        if (client.file_descriptor < 0) {
            continue;
        }

        ThreadArgs *args = malloc(sizeof(*args));
        *client_ptr      = client;
        args->client     = client_ptr;
        *settings_ptr    = settings;
        args->settings   = settings_ptr;

        if (pthread_create(&thread_id, NULL, thread_request_handler, (void *)args) != 0) {
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
