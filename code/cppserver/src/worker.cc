#include "network.h"

extern PFdProcess gFdProcess[MAX_FD];
int process_order(int epollfd, char *buff, int &buffindex,
                  timer_link *timerlink);

int order_readfun(int epollfd, int orderfd, timer_link *timerlink) {
    assert(epollfd != 0);
    assert(orderfd != 0);
    char buff[10 * 1024] = {0};
    int buffindex = 0;
    int canread = 0;
    while (true) {
        canread = sizeof(buff) - buffindex;

        int len = read(orderfd, buff + buffindex, canread);
        if (len > 0 && ((unsigned int)(buffindex + len) >= sizeof(int))) {
            buffindex += len;
            process_order(epollfd, buff, buffindex, timerlink);
        } else if (len == -1 && errno == EAGAIN) {
            break;
        } else {
        }
    }
    return 1;
}

int process_order(int epollfd, char *buff, int &buffindex,
                  timer_link *timerlink) {
    unsigned int processed = 0;
    struct epoll_event ev = {0};

    while (true) {

        time_value tnow = get_now();
        if (int(processed + sizeof(int)) <= buffindex) {
            int *p = (int *)(buff + processed);
            processed += sizeof(int);
            ev.events = EPOLLIN | EPOLLET;
            ev.data.fd = *p;
            set_noblock(*p);

            gFdProcess[*p]->m_readfun = client_readfun;
            gFdProcess[*p]->m_writefun = client_writefun;
            gFdProcess[*p]->m_closefun = client_closefun;
            gFdProcess[*p]->m_timeoutfun = client_timeoutfun;
            if (epoll_ctl(epollfd, EPOLL_CTL_ADD, *p, &ev) == -1) {
                perror("epoll_ctl: listen_sock");
                exit(-1);
            } else {
                timerlink->add_timer(gFdProcess[*p], tnow + 10 * 1000);
            }
        } else {
            break;
        }
    }

    if (int(processed) == buffindex) {
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
    }

    timer_link global_timer;
    time_value outtime = 1000;
    while (true) {
        process_event(epollfd, m_events, outtime, &global_timer);
        if (global_timer.get_arg_time_size() > 0) {
            outtime = global_timer.get_mintimer();
        } else {
            outtime = 1000;
        }
    }
}
