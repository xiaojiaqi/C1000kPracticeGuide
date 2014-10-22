#include "network.h"

extern PFdProcess gFdProcess[MAX_FD];

extern volatile int jobs;

int Simconnect(std::string host, int port) {
    struct sockaddr_in addr;
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd <= 0) {
        return -1;
    }
    printf(" socket new fd %d \n", clientfd);
    if (clientfd >= MAX_FD) {  // 太多链接 系统无法接受
        close(clientfd);
        return -2;
    }

    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host.c_str());
    addr.sin_port = htons(port);

    set_noblock(clientfd);

    int i = connect(clientfd, (struct sockaddr *)&addr, sizeof(addr));
    if (i < 0 && errno != EINPROGRESS) {
        Log << "errno " << errno << " " << strerror(errno) << std::endl;
        close(clientfd);
        return -1;
    } else {

        return clientfd;
    }
}

int add_epoll(int epollfd, int fd, bool r, bool w, bool add_or_mod) {

    struct epoll_event ev = {0};
    ev.events |= EPOLLRDHUP;
    if (r) ev.events |= EPOLLIN | EPOLLET;
    if (w) ev.events |= EPOLLOUT | EPOLLET;

    ev.data.fd = fd;
    if (add_or_mod) {
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
            perror("epoll_ctl: fd ");
            exit(-1);
        }
    } else {
        if (epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
            perror("epoll_ctl: fd ");
            exit(-1);
        }
    }
    return 0;
}
