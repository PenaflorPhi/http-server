#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "utils.h"

#define MAX_METHOD_SIZE   8
#define MAX_PROTOCOL_SIZE 16
#define MAX_URL_SIZE      1024

#define MAX_HOST_SIZE       256
#define MAX_USER_AGENT_SIZE 512
#define MAX_CONNECTION_SIZE 128
#define MAX_ACCEPT_SIZE     256

// ------------------------------------------------------------------
// ---------------- Request Struct ----------------------------------
// ------------------------------------------------------------------

static void initialize_request(Request *request) {
    /* Request start line*/
    request->method   = safe_calloc(MAX_METHOD_SIZE, sizeof(char));
    request->protocol = safe_calloc(MAX_PROTOCOL_SIZE, sizeof(char));
    request->url      = safe_calloc(MAX_URL_SIZE, sizeof(char));

    /* Request header */
    request->host       = safe_calloc(MAX_HOST_SIZE, sizeof(char));
    request->user_agent = safe_calloc(MAX_USER_AGENT_SIZE, sizeof(char));
    request->connection = safe_calloc(MAX_CONNECTION_SIZE, sizeof(char));
    request->accept     = safe_calloc(MAX_ACCEPT_SIZE, sizeof(char));
}

static void free_request(Request *request) {
    free(request->method);
    free(request->protocol);
    free(request->url);
    free(request->host);
    free(request->user_agent);
    free(request->connection);
    free(request->accept);
}

// ------------------------------------------------------------------
// ---------------- Request Parsing ---------------------------------
// ------------------------------------------------------------------

static void parse_header(char *client_request, Request *request) {
    char *line = strtok(client_request, "\r\n");

    // Skip the request line.
    if (line != NULL)
        line = strtok(NULL, "\r\n");

    for (; line != NULL && line[0] != '\0'; line = strtok(NULL, "\r\n")) {
        char *header_value = strchr(line, ':');
        if (!header_value)
            continue;

        *header_value  = '\0'; // Temporarily end header name
        header_value  += 2;    // Skip ': ' to the value

        if (strcmp(line, "User-Agent") == 0) {
            strncpy(request->user_agent, header_value, MAX_USER_AGENT_SIZE - 1);
            request->user_agent[MAX_USER_AGENT_SIZE - 1] = '\0';
        } else if (strcmp(line, "Host") == 0) {
            strncpy(request->host, header_value, MAX_HOST_SIZE - 1);
            request->host[MAX_HOST_SIZE - 1] = '\0';
        } else if (strcmp(line, "Accept") == 0) {
            strncpy(request->accept, header_value, MAX_ACCEPT_SIZE - 1);
            request->accept[MAX_ACCEPT_SIZE - 1] = '\0';
        } else if (strcmp(line, "Connection") == 0) {
            strncpy(request->connection, header_value, MAX_CONNECTION_SIZE - 1);
            request->connection[MAX_CONNECTION_SIZE - 1] = '\0';
        }
    }

    // printf("`Connection`: %s\n", request->connection);
    // printf("`User-Agent`: %s\n", request->user_agent);
    // printf("`Accept`: %s\n", request->accept);
    // printf("`Connection`: %s\n", request->connection);

    string_realloc(&request->host);
    string_realloc(&request->user_agent);
    string_realloc(&request->accept);
    string_realloc(&request->connection);

    // printf("`Host`: %s\n", request->host);
    // printf("`User-Agent`: %s\n", request->user_agent);
    // printf("`Accept`: %s\n", request->accept);
    // printf("`Connection`: %s\n", request->connection);
}

static Request parse_request_line(char *client_request) {
    printf("parsing request\n");
    Request request;
    initialize_request(&request);

    int response_size =
        sscanf(client_request, "%7s %1023s %15s", request.method, request.url, request.protocol);
    if (response_size == 3) {
        // printf("method: %s\n", request.method);
        // printf("url: %s\n", request.url);
        // printf("protocol: %s\n", request.protocol);

        string_realloc(&request.method);
        string_realloc(&request.url);
        string_realloc(&request.protocol);
        //
        // // printf("method: %s\n", request.method);
        // // printf("url: %s\n", request.url);
        // // printf("protocol: %s\n", request.protocol);
    } else {
        free_request(&request);
        perror("Malformed request, could not find one of method, url, protocol");
    }

    return request;
}

// ------------------------------------------------------------------
// ---------------- Receive Request ---------------------------------
// ------------------------------------------------------------------
static void receive_request(Client *client) {
    ssize_t bytes = recv(client->file_descriptor, client->request, client->buffer_size, 0);
    if (bytes > 0) {
        client->request[bytes] = '\0';
    } else {
        free(client->request);
        perror("failed while trying to receive request.");
    }
}

// ------------------------------------------------------------------
// ---------------- Request Handler ---------------------------------
// ------------------------------------------------------------------
void request_handler(Client *client) {
    receive_request(client);
    char *client_request = strdup(client->request);
    // printf("request:\\n\n%s", client_request);
    // printf("client_request:\\n\n%s", client_request);

    Request request = parse_request_line(client_request);
    parse_header(client_request, &request);

    puts("----------------------------------");
    printf("`method`: %s\n", request.method);
    printf("`url`: %s\n", request.url);
    printf("`protocol`: %s\n", request.protocol);

    printf("`Host`: %s\n", request.host);
    printf("`User-Agent`: %s\n", request.user_agent);
    printf("`Accept`: %s\n", request.accept);
    printf("`Connection`: %s\n", request.connection);
    puts("----------------------------------");
}
