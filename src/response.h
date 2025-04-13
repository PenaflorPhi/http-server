#ifndef PROCESS_H
#define PROCESS_H

#include "server.h"

Response process_request(Request *req, int buffer_size);

#endif // !PROCESS_H
