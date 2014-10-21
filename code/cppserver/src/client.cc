#include "network.h"

extern PFdProcess gFdProcess[MAX_FD];

int client_readfun(int epoll, int fd, timer_link* timer) {
    char buff[1024];

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
            Log << "'" << buff << "'" << std::endl;
        }
    }
    return 1;
}

char senddata[2048];
const int presend = 500;
int client_writefun(int epoll, int fd, timer_link* timer) {

    int len = presend - gFdProcess[fd]->m_sended;
    len = write(fd, &(senddata[0]) + gFdProcess[fd]->m_sended, len);
    if (len == 0) {
        gFdProcess[fd]->m_activeclose = true;
    } else if (len > 0) {
        gFdProcess[fd]->m_sended += len;
        if (gFdProcess[fd]->m_sended == presend) {
            gFdProcess[fd]->m_sended = 0;
        }
    }
    return 1;
}

int client_closefun(int epoll, int fd, timer_link* timer) {
    timer->remote_timer(gFdProcess[fd]);

    gFdProcess[fd]->init();

    ::close(fd);
    return 1;
}
int client_timeoutfun(int epoll, int fd, timer_link* timers, time_value tnow) {

    int len = presend - gFdProcess[fd]->m_sended;
    len = write(fd, &(senddata[0]) + gFdProcess[fd]->m_sended, len);
    if (len == 0) {
        gFdProcess[fd]->m_activeclose = true;
    } else if (len > 0) {
        gFdProcess[fd]->m_sended += len;
        if (gFdProcess[fd]->m_sended == presend) {
            gFdProcess[fd]->m_sended = 0;
        }
    }
    timers->add_timer(gFdProcess[fd], tnow + 10 * 1000);
    return 0;
}
