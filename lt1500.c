#include "lt1500.h"

#include <string.h>
#include <strings.h>
#include <stdio.h>

static int
lt_init(struct lt_handle *lth, uint16_t port, short pass)
{
        /* invalid password */
        if(pass < 0 || pass > 9999) {
                fprintf(stderr, "invalid password %d", pass);
                return LT_INVALID_PASSWORD;
        }
        sprintf(lth->pass, "%04d", pass);

        lth->port = port;

        return LT_SUCCESS;
}

int
lt_open(struct lt_handle *lth, const char *host, uint16_t port, short pass)
{
        int res;
        struct sockaddr_in addr;

        /* init lth */
        if((res = lt_init(lth, port, pass)) != LT_SUCCESS)
                return res;

        /* create socket */
        if((lth->s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                fprintf(stderr, "socket error");
                return LT_FAILED;
        }

        /* connect */
        bzero((void*)&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(host);
        addr.sin_port = htons(port);

        if(connect(lth->s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                fprintf(stderr, "connect error : %s", host);
                return LT_CONNECTION_ERROR;
        }

        return LT_SUCCESS;
}

static int
lt_cmd(struct lt_handle *lth, const char *cmd, ...) {
        char command[256], packet[256];

        va_list ap;
        va_start(ap, cmd);
        vsprintf(command, cmd, ap);
        va_end(ap);

        /* create packet*/
        sprintf(packet, "*%s%s\r", lth->pass, command);

        /* send packet */
        printf("send packet : %s\n", packet);

        if(send(lth->s, packet, strlen(packet), 0) < 0) {
                fprintf(stderr, "send error");
                return LT_FAILED;
        }

        return LT_SUCCESS;
}

int
lt_pon(struct lt_handle *lth, int port)
{
        char cmd[20];
        if(!(port >= 1 && port <= 4) && port != 9) {
                fprintf(stderr, "invalid port : %d (port =1,2,3,4,9)", port);
        }

        return lt_cmd(lth, "PON%d", port);
}

int
lt_pof(struct lt_handle *lth, int port)
{
        char cmd[20];
        if(!(port >= 1 && port <= 4) && port != 9) {
                fprintf(stderr, "invalid port : %d (port =1,2,3,4,9)", port);
        }

        return lt_cmd(lth, "POF%d", port);
}

void
lt_close(struct lt_handle *lth)
{
        close(lth->s);
}

/* debug script */
#define LT_DEBUG
#ifdef LT_DEBUG

int main()
{
        struct lt_handle lth;
        int res;
        if((res = lt_open(&lth, "10.228.104.175", 33336, 0)) != LT_SUCCESS) {
                fprintf(stderr, "lt open error : %d", res);
                return 1;
        }

        if((res = lt_pof(&lth, 1) != LT_SUCCESS)) {
                fprintf(stderr, "pon error : %d", res);
        }

        lt_close(&lth);

        return 0;
}

#endif
