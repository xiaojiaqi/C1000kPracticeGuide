#include "network.h"

extern PFdProcess gFdProcess[MAX_FD];

uint64_t sumread = 0;

int client_readfun(int epoll, int fd, timer_link* timer) {
    char buff[1024];
    // Log << "read on call" << std::endl;
    while (true) {

        int len = read(fd, buff, sizeof(buff) - 1);
        if (len == -1) {
            buff[0] = 0;
            break;

        } else if (len == 0) {
            buff[len] = 0;
            gFdProcess[fd]->m_activeclose = true;
            return -1;
        }

        if (len > 0) {
            // Log << ".";
            sumread += len;
            //			Log << buff << std::endl;
        }
    }
    return 1;
}

int client_writefun(int epoll, int fd, timer_link* timer) { return 1; }

int client_closefun(int epoll, int fd, timer_link* timer) {
    gFdProcess[fd]->init();
    ::close(fd);
    return 1;
}
