#include "lt1500.h"

#include <string.h>
#include <stdio.h>
#include <strings.h> /* bzero */
#include <unistd.h> /* close (socket) */
#include <arpa/inet.h> /* inet_addr */

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
#ifdef LT_DEBUG
        printf("send packet : %s\n", packet);
#endif
        
        if(send(lth->s, packet, strlen(packet), 0) < 0) {
                fprintf(stderr, "send error");
                return LT_FAILED;
        }

        return LT_SUCCESS;
}

/**
 * receive a line. line must be terminated by '\n'
 */
static int
recv_line(int sock, char *line, int n)
{
        int i;
        for(i = 0; i < n-1; ++i) {
                if(recv(sock, &line[i], 1, 0) < 0)
                        return -1;

                if(line[i] == '\n') {
                        break;
                }
        }

        line[i] = '\0';

        return i;
}


/**
 * get return code
 * format :
 *  command
 *  OK/Err/Locked
 */
static int
lt_get_ret_code(struct lt_handle *lth)
{
        char line[80];

        /* skip command echo */
        recv_line(lth->s, line, sizeof(line));

        /* get return code */
        recv_line(lth->s, line, sizeof(line));
        if(!strcmp(line, "OK")) {
                return LT_SUCCESS;
        } else if(!strcmp(line, "Err")) {
                return LT_FAILED;
        } else if(!strcmp(line, "Locked")) {
                return LT_LOCKED;
        } else 
                return LT_FAILED;
}

static int
lt_get_pos(struct lt_handle *lth, struct lt_ports_status *pos)
{
        char line[80];

        /* skip command line */
        recv_line(lth->s, line, sizeof(line));

        recv_line(lth->s, line, sizeof(line));

#ifdef LT_DEBUG
        printf("pos %s\n", line);
#endif

        if(pos != NULL) {
                int p[4];
                if(sscanf(line, "POS=%d,%d,%d,%d", p, p+1, p+2, p+3) < 4) {
                        return LT_FAILED;
                }

                int i;
                for(i=0; i<4; ++i) {
                        pos->port[i].state = p[i] / 100;
                        pos->port[i].delay = p[i] - (p[i] / 100) * 100;
                }
        }

        return LT_SUCCESS;
}

int
lt_pon(struct lt_handle *lth, int port)
{
        int ret, ret2;

        if(!(port >= 1 && port <= 4) && port != 9) {
                fprintf(stderr, "invalid port : %d (port = 1,2,3,4,9)", port);
                return LT_INVALID_PORT;
        }

        if((ret = lt_cmd(lth, "PON%d", port)) != LT_SUCCESS) {
                fprintf(stderr, "command error");
                return ret;
        }

        ret = lt_get_ret_code(lth);
        printf("ret %d", ret);

        ret2 = lt_get_pos(lth, NULL);

        return LT_SUCCESS;
}

int
lt_pon_all(struct lt_handle *lth)
{
        return lt_pon(lth, 9);
}


int
lt_pof(struct lt_handle *lth, int port)
{
        int ret, ret2;

        if(!(port >= 1 && port <= 4) && port != 9) {
                fprintf(stderr, "invalid port : %d (port = 1,2,3,4,9)", port);
                return LT_INVALID_PORT;
        }

        if((ret = lt_cmd(lth, "POF%d", port)) != LT_SUCCESS) {
                fprintf(stderr, "command error");
                return ret;
        }

        ret = lt_get_ret_code(lth);
        printf("ret %d", ret);

        ret2 = lt_get_pos(lth, NULL);

        return LT_SUCCESS;
}

int
lt_pof_all(struct lt_handle *lth)
{
        return lt_pof(lth, 9);
}

/* get tap status */
int
lt_pos(struct lt_handle *lth, struct lt_ports_status *ports)
{
        int ret;
        if((ret = lt_cmd(lth, "POS")) != LT_SUCCESS) {
                fprintf(stderr, "command error");
                return ret;
        }

        return lt_get_pos(lth, ports);
}

void
lt_close(struct lt_handle *lth)
{
        close(lth->s);
}

/* debug */
#ifdef LT_DEBUG

int main()
{
        struct lt_handle lth;
        int res;

        /* open lantap */
        if((res = lt_open(&lth, "10.228.104.175", 33336, 0)) != LT_SUCCESS) {
                fprintf(stderr, "lt open error : %d", res);
                return 1;
        }

        /* power on : tap 1 */
        if((res = lt_pon(&lth, 1) != LT_SUCCESS)) {
                fprintf(stderr, "pon error : %d", res);
                return 1;
        }

        /* get tap status*/
        struct lt_ports_status ports;
        if((res = lt_pos(&lth, &ports) != LT_SUCCESS)) {
                fprintf(stderr, "pos error : %d", res);
                return 1;
        }
        
        /* show tap status */
        printf("state = %d,%d,%d,%d\n",
               ports.port[0].state,
               ports.port[1].state,
               ports.port[2].state,
               ports.port[3].state);

        /* close handle */
        lt_close(&lth);

        return 0;
}

#endif
