#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"

static const char *default_status_response = "HTTP/1.1 %s\r\n"
                                             "content-type: text/plain\r\n"
                                             "content-length: %zu\r\n"
                                             "\r\n"
                                             "%s"
                                             "\r\n";

char *format_echo(char *s) {
    if ((s + strlen("/echo"))[0] != '\0') {
        return s + strlen("/echo") + 1;
    }
    return s + strlen("/echo");
}

void format_response(Response *res, int buffer_size) {
    int written_bytes;

    // body + the '\r\n' at the end of the status_response;
    int body_length = strlen(res->body) ? strlen(res->body) + 2 : 0;

    if (strcmp(res->status, "200 OK") == 0) {
        written_bytes = snprintf(res->response,
                                 buffer_size,
                                 default_status_response,
                                 res->status,
                                 body_length,
                                 res->body);
    } else if (strcmp(res->status, "404 Not Found") == 0) {
        written_bytes = snprintf(res->response,
                                 buffer_size,
                                 default_status_response,
                                 res->status,
                                 body_length,
                                 res->body);

    } else {
        perror("Status body not found! Something went wrong.\n");
        exit(EXIT_FAILURE);
    }

    if (written_bytes < 0) {
        perror("Error formating the response.\n");
        exit(EXIT_FAILURE);
    } else if (written_bytes >= buffer_size) {
        perror("Response larger than buffer size.\n");
        exit(EXIT_FAILURE);
    }
}

Response process_request(Request *req, int buffer_size) {
    Response res;
    res.response = calloc(buffer_size * sizeof(char), sizeof(char));
    res.status   = "";
    res.body     = "";

    if (strcmp(req->path, "/") == 0 || strcmp(req->path, "") == 0) {
        res.status = "200 OK";
        res.body   = "";
    } else if (strcmp(req->path, "/index.html") == 0) {
        res.status = "200 OK";
        res.body   = "";
    } else if ((strncmp(req->path, "/echo", strlen("/echo"))) == 0) {
        res.status = "200 OK";
        res.body   = format_echo(req->path);
    } else {
        res.status = "404 Not Found";
        res.body   = "";
    }

    format_response(&res, buffer_size);

    return res;
}

void send_response(Client *client, Response *response) {
    if ((send(client->file_descriptor, response->response, strlen(response->response), 0)) == -1) {
        perror("Sending response failed!");
        free(response->response);
        exit(EXIT_FAILURE);
    }
}
