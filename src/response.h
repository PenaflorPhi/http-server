#ifndef PROCESS_H
#define PROCESS_H

#include "server.h"

Response process_request(Request *req, int buffer_size);
void     send_response(Client *client, Response *response);

#endif // !PROCESS_H
