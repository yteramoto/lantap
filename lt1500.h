#ifndef LANTAP_H
#define LANTAP_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdarg.h>

struct lt_handle {
        int s;
        char pass[5];
        uint16_t port;
};

#define LT_SUCCESS 0
#define LT_FAILED 1
#define LT_INVALID_PASSWORD 2
#define LT_CONNECTION_ERROR 3

#endif
