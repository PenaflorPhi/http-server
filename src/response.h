#ifndef RESPONSE_H
#define RESPONSE_H

int format_response(char *response_to_send, unsigned long buff_size, const char *response,
                    const char *response_status, const char *message);

char *format_echo(const char *str);

#endif /* RESPONSE_H */
