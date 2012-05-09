#ifndef LANTAP_H
#define LANTAP_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdarg.h>

struct lt_handle {
        int s; /* socket handle */
        char pass[5]; /* password */
        uint16_t port; /* connection port */
};

/**
 * port status, for details look manual
 */
enum lt_port_state {
        LT_PORT_OFF = 0,
        LT_PORT_ON,
        LT_PORT_SHUTDOWN,
        LT_PORT_RESET,
        LT_PORT_WATCH_DOG
};

/**
 * status of a port
 */
struct lt_port_status {
        enum lt_port_state state;
        int delay;
};

/**
 * status of ports
 */
struct lt_ports_status {
        struct lt_port_status port[4];
};

/* return codes */
#define LT_SUCCESS 0
#define LT_FAILED 1
#define LT_INVALID_PASSWORD 2
#define LT_CONNECTION_ERROR 3
#define LT_INVALID_PORT 4
#define LT_LOCKED 5

/* connect to lantap */
int lt_open(struct lt_handle *lth, const char *host, uint16_t port, short pass);

/**
 * power on port
 * port = 1,2,3,4,9
 * power on all port if port = 9
 */
int lt_pon(struct lt_handle *lth, int port);

/**
 * power on all port
 * same as lt_pon(port = 9)
 */
int lt_pon_all(struct lt_handle *lth);

/**
 * power off port
 * port = 1,2,3,4,9
 * power off all port if port = 9
 */
int lt_pof(struct lt_handle *lth, int port);

/**
 * power off all port
 */
int lt_pof_all(struct lt_handle *lth);

/**
 * get port status
 * see also : enum lt_port_state, struct lt_ports_status, struct lt_ports_status
 */
int lt_pos(struct lt_handle *lth, struct lt_ports_status *ports);

/**
 * disconnect lantap
 */
void lt_close(struct lt_handle *lth);

#endif
