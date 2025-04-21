#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "utils.h"

#define MAX_METHOD_SIZE   8
#define MAX_PROTOCOL_SIZE 16
#define MAX_URL_SIZE      1024

#define MAX_REQUEST_SIZE 4096

#define MAX_HOST_SIZE       256
#define MAX_USER_AGENT_SIZE 512
#define MAX_CONNECTION_SIZE 128
#define MAX_ACCEPT_SIZE     256
#define MAX_CONTENT_TYPE    128
#define MAX_BODY_SIZE       4096

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

    request->content_type = safe_calloc(MAX_CONTENT_TYPE, sizeof(char));
    request->body         = safe_calloc(MAX_REQUEST_SIZE, sizeof(char));

    request->content_length = 0;

    request->count_accept_encodings = 0;
    memset(request->accept_encoding, 0, sizeof(request->accept_encoding));
}

static void free_request(Request *request) {
    free(request->method);
    free(request->protocol);
    free(request->url);
    free(request->host);
    free(request->user_agent);
    free(request->connection);
    free(request->accept);
    free(request->body);
    free(request->content_type);
}

// ------------------------------------------------------------------
// ---------------- Encoding Parsing --------------------------------
// ------------------------------------------------------------------
void parse_encoding(char *header_value, Request *request) {
    // 1) Make a mutable copy of the header *value*
    char *enc_copy = strdup(header_value);
    if (!enc_copy) {
        perror("strdup");
        return;
    }

    size_t count = 0;

    // 2) First token: strtok on the copy, delim = ","
    char *token = strtok(enc_copy, ",");
    while (token && count < MAX_ENCODINGS) {
        // 3) Trim leading spaces
        char *start = token;
        while (*start == ' ') start++;

        // 4) Trim trailing spaces / newlines
        char *end = start + strlen(start) - 1;
        while (end > start && (*end == ' ' || *end == '\r' || *end == '\n')) {
            *end = '\0';
            end--;
        }

        // 5) Copy into your struct’s 2D array
        strncpy(request->accept_encoding[count], start, MAX_ENCODING_SIZE - 1);
        request->accept_encoding[count][MAX_ENCODING_SIZE - 1] = '\0';

        count++;
        token = strtok(NULL, ",");
    }

    request->count_accept_encodings = (int)count;

    free(enc_copy);
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
        } else if (strcmp(line, "Content-Type") == 0) {
            strncpy(request->content_type, header_value, MAX_CONNECTION_SIZE - 1);
            request->content_type[MAX_CONTENT_TYPE - 1] = '\0';
        } else if (strcmp(line, "Content-Length") == 0) {
            char *endptr;
            long  val = strtol(header_value, &endptr, 10);
            if (endptr != header_value && val >= 0 && val <= MAX_ACCEPT_SIZE) {
                request->content_length = (int)val;
            } else {
                request->content_length = 0;
            }
        } else if (strcmp(line, "Accept-Encoding") == 0) {
            parse_encoding(header_value, request);
        } else {
            strncpy(request->body, header_value, MAX_ACCEPT_SIZE - 1);
            request->body[MAX_CONNECTION_SIZE - 1] = '\0';
        }
    }

    string_realloc(&request->host);
    string_realloc(&request->user_agent);
    string_realloc(&request->accept);
    string_realloc(&request->connection);
    string_realloc(&request->body);
}

static void parse_body(char *client_request, Request *request) {
    char *body_start = strstr(client_request, "\r\n\r\n");
    if (body_start) {
        body_start += 4; // skip past the "\r\n\r\n"
    }

    if (body_start) {
        size_t to_copy = strlen(body_start);
        if (to_copy > MAX_BODY_SIZE - 1)
            to_copy = MAX_BODY_SIZE - 1;
        memcpy(request->body, body_start, to_copy);
        request->body[to_copy] = '\0';
    }

    string_realloc(&request->body);
}

static Request parse_request_line(char *client_request) {
    Request request;
    initialize_request(&request);

    int response_size =
        sscanf(client_request, "%7s %1023s %15s", request.method, request.url, request.protocol);
    if (response_size == 3) {
        string_realloc(&request.method);
        string_realloc(&request.url);
        string_realloc(&request.protocol);
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
Request request_handler(Client *client) {
    receive_request(client);
    char *client_request = strdup(client->request);
    puts("--Original's client request---------");
    printf("%s\n", client_request);
    puts("------------------------------------");

    Request request = parse_request_line(client_request);
    parse_body(client_request, &request);
    parse_header(client_request, &request);

    puts("--- request_handler ----------------");
    printf("`method`: %s\n", request.method);
    printf("`url`: %s\n", request.url);
    printf("`protocol`: %s\n", request.protocol);

    printf("`Host`: %s\n", request.host);
    printf("`User-Agent`: %s\n", request.user_agent);
    printf("`Accept`: %s\n", request.accept);
    printf("`Connection`: %s\n", request.connection);

    printf("`Accept-Encoding`:\n");
    for (int i = 0; i < request.count_accept_encodings; ++i) {
        printf("\t- %s\n", request.accept_encoding[i]);
    }

    printf("`Content-Type`: %s\n", request.content_type);
    printf("`Content-Length`: %d\n", request.content_length);
    printf("`Body`: %s\n", request.body);
    puts("------------------------------------");

    return request;
}
