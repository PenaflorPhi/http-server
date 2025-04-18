#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "read_file.h"
#include "server.h"
#include "utils.h"

static const char *default_response = "HTTP/1.1 %s\r\n"
                                      "content-type: %s\r\n"
                                      "content-length: %zu\r\n"
                                      "\r\n"
                                      "%s";

static Response process_request(Request *request) {
    Response response;
    if (strcmp(request->url, "/") == 0) {
        response.status       = "200 OK";
        response.body         = "";
        response.content_type = "text/plain";
    } else if (strcmp(request->url, "/index.html") == 0) {
        response.status       = "200 OK";
        response.body         = "";
        response.content_type = "text/plain";
    } else if (strcmp(request->url, "/user-agent") == 0) {
        response.status       = "200 OK";
        response.body         = request->user_agent;
        response.content_type = "text/plain";
    } else if (strncmp(request->url, "/echo", 5) == 0) {
        response.status          = "200 OK";
        const char *echo_content = request->url + 6;

        size_t length = strlen(echo_content) + 1;
        response.body = (char *)malloc(length);
        memcpy(response.body, echo_content, length);

        response.content_type = "text/plain";
    } else if (strncmp(request->url, "/files", 5) == 0) {
        response.status       = "200 OK";
        response.content_type = "application/octet-stream";

        const char *file_path = request->url + 1;

        response.body = read_file(file_path);
        if (response.body == NULL) {
            printf("File not found!\n");
            response.body = "";
        }
    } else {
        response.status = "404 Not Found";
        response.body   = NULL;
    }

    response.size = strlen(response.body);

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

void send_response(Request *request, Client *client) {
    Response response = process_request(request);
    char    *message  = format_response(&response, client->buffer_size);

    puts("--- send_response ----------------");
    printf("%s\n", message);
    puts("------------------------------------");

    if ((send(client->file_descriptor, message, strlen(message), 0)) == -1) {
        perror("Sending response failed!");
        return;
    }
}
