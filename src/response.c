#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "utils.h"

static const char *default_response = "HTTP/1.1 %s\r\n"
                                      "content-type: text/plain\r\n"
                                      "content-length: %zu\r\n"
                                      "\r\n"
                                      "%s";

static Response process_request(Request *request) {
    Response response;
    if (strcmp(request->url, "/") == 0) {
        response.status = "200 OK";
        response.body   = "";
    } else if (strcmp(request->url, "/index.html") == 0) {
        response.status = "200 OK";
        response.body   = "";
    } else if (strncmp(request->url, "/echo", 5) == 0) {
        response.status          = "200 OK";
        const char *echo_content = request->url + 6;

        size_t length = strlen(echo_content) + 1;
        response.body = (char *)malloc(length);
        memcpy(response.body, echo_content, length);
    } else if (strcmp(request->url, "/user-agent") == 0) {
        response.status = "200 OK";
        response.body   = request->user_agent;
    } else {
        response.status = "404 Not Found";
        response.body   = "";
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
