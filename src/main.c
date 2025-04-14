#include <stdio.h>
#include <unistd.h>

#include "request.h"
#include "response.h"
#include "server.h"

#define PORT            8080 // Port number for the server to listen on.
#define BACKLOG         16   // Maximum number of pending connections in the queue.
#define MAX_BUFFER_SIZE 1024 // Maximum size of the buffer for request/response handling.

int main() {
    // Initialize the server and a temporary client structure.
    Server server;
    Client client;

    /*
     * Create the server with the following steps:
     *  1. Create a socket.
     *  2. Set socket options (e.g., enabling reuse).
     *  3. Define the server address struct.
     *  4. Bind the socket to the defined address.
     *  5. Start listening for incoming connections (with a maximum queue of BACKLOG).
     */
    server = create_server(PORT, BACKLOG);

    while (1) {
        Request  request;  // Structure to store the client's request.
        Response response; // Structure to store the server's response.

        /* Accept a new client connection. */
        client = accept_client(&server);

        /*
         * Read the client's HTTP request into a buffer with a maximum size of MAX_BUFFER_SIZE,
         * and parse it to determine the requested resource.
         */
        request = parse_request(client.file_descriptor, MAX_BUFFER_SIZE);

        /*
         * Process the parsed request to create the appropriate response.
         */
        response = process_request(&request, MAX_BUFFER_SIZE);

        /*
         * Send the generated response back to the client.
         */
        printf("Response:\n");
        printf("%s", response.response);
        send_response(&client, &response);

        // Free memory.
        free(request.request);
        free(response.response);

        // Properly shutdown the connection and then close the file descriptor.
        shutdown(client.file_descriptor, SHUT_WR);
        close(client.file_descriptor);
    }

    // Close the server's file descriptor before exiting.
    close(server.file_descriptor);

    return EXIT_SUCCESS;
}
