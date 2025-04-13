#ifndef PARSER_H
#define PARSER_H
#include "server.h"
#include <stdlib.h>

Request parse_request(int client_fd, ssize_t buffer_size);

#endif
