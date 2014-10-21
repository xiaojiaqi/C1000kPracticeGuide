#include "socket_util.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int set_noblock(int sClient) {

    int opts;

    opts = fcntl(sClient, F_GETFL);
    if (opts < 0) {
        perror("fcntl(sock,GETFL)");
        exit(1);
    }
    opts = opts | O_NONBLOCK;
    if (fcntl(sClient, F_SETFL, opts) < 0) {
        perror("fcntl(sock, SETFL, opts)");
        exit(1);
    }
    return 0;
}

int set_reused(int fd) {
    int on = 1;
    int rt = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    return rt;
}
