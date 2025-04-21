#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "files.h"
#include "server.h"
#include "utils.h"

#define MAX_RESPONSE_SIZE 4096

static const char *default_response = "HTTP/1.1 %s\r\n"
                                      "content-type: %s\r\n"
                                      "content-length: %zu\r\n"
                                      "\r\n"
                                      "%s";

static void handle_post_file_request(Response *response, Request *request, Settings *settings) {
    if ((settings->directory) != NULL) {
        char       *file_path = calloc(1024, sizeof(char));
        const char *file_name = request->url + 6;

        int written_bytes = snprintf(file_path,           // output buffer
                                     1024,                // its size
                                     "%s%s",              // format
                                     settings->directory, // first %s
                                     file_name            // second %s
        );

        if (written_bytes < 0) {
            perror("Error formating the response.\n");
            free(file_path);
        } else if (written_bytes >= 1024) {
            perror("Response larger than buffer size.\n");
            free(file_path);
        }

        write_file(file_path, request->body);
    }

    response->status       = "201 Created";
    response->body         = "";
    response->content_type = "text/plain";
}

static void handle_get_file_request(Response *response, Request *request, Settings *settings) {
    if ((settings->directory) != NULL) {
        char       *file_path      = calloc(1024, sizeof(char));
        const char *requested_file = request->url + 6;

        int written_bytes = snprintf(file_path,           // output buffer
                                     1024,                // its size
                                     "%s%s",              // format
                                     settings->directory, // first %s
                                     requested_file       // second %s
        );

        if (written_bytes < 0) {
            perror("Error formating the response.\n");
            free(file_path);
        } else if (written_bytes >= 1024) {
            perror("Response larger than buffer size.\n");
            free(file_path);
        }
        printf("file_path: %s\n", file_path);

        response->body = read_file(file_path);

        if (response->body == NULL) {
            response->status       = "404 Not Found";
            response->body         = "";
            response->content_type = "application/octet-stream";
        } else {
            response->status       = "200 OK";
            response->content_type = "application/octet-stream";
        }
    } else {
        fprintf(stderr, "Directory was not set\n");
    };
}

static void initialize_response(Response *response) {
    response->body         = safe_calloc(MAX_RESPONSE_SIZE, sizeof(char));
    response->status       = safe_calloc(32, sizeof(char));
    response->content_type = safe_calloc(32, sizeof(char));
}

static void process_get_request(Response *response, Request *request, Settings *settings) {
    if (strcmp(request->url, "/") == 0) {
        response->status       = "200 OK";
        response->body         = "";
        response->content_type = "text/plain";
    } else if (strcmp(request->url, "/index.html") == 0) {
        response->status       = "200 OK";
        response->body         = "";
        response->content_type = "text/plain";
    } else if (strcmp(request->url, "/user-agent") == 0) {
        response->status       = "200 OK";
        response->body         = request->user_agent;
        response->content_type = "text/plain";
    } else if (strncmp(request->url, "/echo/", 6) == 0) {
        response->status         = "200 OK";
        const char *echo_content = request->url + 6;

        size_t length  = strlen(echo_content) + 1;
        response->body = (char *)malloc(length);
        memcpy(response->body, echo_content, length);

        response->content_type = "text/plain";
    } else if (strncmp(request->url, "/files/", 6) == 0) {
        handle_get_file_request(response, request, settings);
    } else {
        response->status = "404 Not Found";
    }

    response->size = strlen(response->body);
}

static void process_post_request(Response *response, Request *request, Settings *settings) {
    if (strncmp(request->url, "/files/", 6) == 0) {
        handle_post_file_request(response, request, settings);

    } else {
        response->status = "404 Not Found";
    }

    response->size = strlen(response->body);
}

static Response process_request(Request *request, Settings *settings) {
    Response response;
    initialize_response(&response);

    if (strcmp(request->method, "GET") == 0) {
        process_get_request(&response, request, settings);
    } else if (strcmp(request->method, "POST") == 0) {
        process_post_request(&response, request, settings);
    }
    return response;
}

static char *format_response(Response *response, int buffer_size) {
    char *formatted_response = safe_calloc(buffer_size, sizeof(char));
    int   written_bytes      = snprintf(formatted_response,
                                 buffer_size,
                                 default_response,
                                 response->status,
                                 response->content_type,
                                 response->size,
                                 response->body);

    if (written_bytes < 0) {
        perror("Error formating the response.\n");
        free(formatted_response);
        return NULL;
    } else if (written_bytes >= buffer_size) {
        perror("Response larger than buffer size.\n");
        free(formatted_response);
        return NULL;
    }

    return formatted_response;
}

void send_response(Request *request, Client *client, Settings *settings) {
    Response response = process_request(request, settings);
    char    *message  = format_response(&response, client->buffer_size);

    puts("--- send_response ----------------");
    printf("%s\n", message);
    puts("------------------------------------");

    if ((send(client->file_descriptor, message, strlen(message), 0)) == -1) {
        perror("Sending response failed!");
        return;
    }
}
