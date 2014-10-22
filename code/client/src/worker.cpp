#include "network.h"

extern PFdProcess gFdProcess[MAX_FD];
int process_order(int epollfd, char *buff, int &buffindex);

int order_readfun(int epollfd, int orderfd, timer_link *timerlink) {

    char buff[4 * 1024] = {0};
    int buffindex = 0;
    int canread = 0;
    while (true) {
        canread = sizeof(buff) - buffindex;
        int len = read(orderfd, buff + buffindex, canread);
        if (len > 0 && ((buffindex + len) >= sizeof(int))) {
            buffindex += len;
            process_order(epollfd, buff, buffindex);
        } else if (len == -1 && errno == EAGAIN) {
            break;
        } else {
        }
    }
    return 1;
}

int process_order(int epollfd, char *buff, int &buffindex) {
    unsigned int processed = 0;
    struct epoll_event ev = {0};
    while (true) {
        if (processed + sizeof(int) <= (unsigned int)buffindex) {
            int *p = (int *)(buff + processed);
            Log << "recv fd " << *p << " from main" << std::endl;
            processed += sizeof(int);

            ev.events = EPOLLIN | EPOLLET;
            ev.data.fd = *p;
            set_noblock(*p);

            gFdProcess[*p]->m_readfun = client_readfun;
            gFdProcess[*p]->m_writefun = client_writefun;
            gFdProcess[*p]->m_closefun = client_closefun;

            if (epoll_ctl(epollfd, EPOLL_CTL_ADD, *p, &ev) == -1) {
                perror("epoll_ctl: listen_sock");
                exit(-1);
            } else {
                Log << "add client fd " << *p << std::endl;
            }

        } else {
            break;
        }
    }

    if (processed == (unsigned int)buffindex) {
        buffindex = 0;
    } else {
        memmove(buff + processed, buff, buffindex - processed);
        buffindex = processed;
    }
    return 0;
}

void *worker_thread(void *arg) {

    struct worker_thread_arg *pagr = (struct worker_thread_arg *)arg;

    struct epoll_event *m_events;

    int epollfd;
    assert(pagr);
    struct epoll_event ev = {0};

    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = pagr->orderfd;
    set_noblock(pagr->orderfd);

    m_events = (struct epoll_event *)malloc(MAXEPOLLEVENT *
                                            sizeof(struct epoll_event));
    epollfd = epoll_create(MAXEPOLLEVENT);
    gFdProcess[pagr->orderfd]->m_readfun = order_readfun;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, pagr->orderfd, &ev) == -1) {
        perror("epoll_ctl: listen_sock");
        exit(-1);
    } else {

        Log << "thread " << pagr->cpuid << " add fd  to epoll " << pagr->orderfd
            << std::endl;
    }

    timer_link global_timer;
    int outime = 1000;
    while (true) {

        process_event(epollfd, m_events, outime, &global_timer);
        outime = global_timer.get_mintimer();
        if (0 == outime) outime = 1000;
    }
}
