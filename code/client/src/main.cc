#include "network.h"
#include "socket_util.h"

#include "timers.h"

int listenport = 8888;

pthread_t t_all[MAX_CPU] = {0};
cpu_set_t mask[MAX_CPU];

extern PFdProcess gFdProcess[MAX_FD];
extern uint64_t sumread;
// 命令管道
// int sv[MAX_CPU][2] = { 0};
bool close_fun(int fd) { return true; }

volatile int jobs = 0;

struct epoll_event *m_events;

int epollfd;

int main(int argc, char *argv[]) {

    // pipe
    int num = 50000;
    if (argc >= 2) {
        num = atol(argv[1]);
    }
    if (argc >= 3) {
        listenport = atol(argv[2]);
    }
    signal(SIGPIPE, SIG_IGN);  // sigpipe 信号屏蔽

    // bind
    m_events = (struct epoll_event *)malloc(MAXEPOLLEVENT *
                                            sizeof(struct epoll_event));
    epollfd = epoll_create(MAXEPOLLEVENT);

    for (int i = 0; i < MAX_FD; ++i) {
        PFdProcess p = new FdProcess();
        assert(p);
        gFdProcess[i] = p;
    }
    for (int i = 0; i < num; ++i) {
        int fd = Simconnect("172.16.31.208", listenport);

        if (fd < 0) {
            // exit(-3);
            continue;
        }

        add_epoll(epollfd, fd);

        gFdProcess[fd]->m_readfun = client_readfun;
        gFdProcess[fd]->m_writefun = client_writefun;
        gFdProcess[fd]->m_closefun = client_closefun;
    }

    main_thread(NULL);
    /*
    while (1) {
            ::sleep(1000);
    }
    */
    return 0;
}

void *main_thread(void *arg) {

    timer_link global_timer;
    int outime = 1000;
    while (true) {
        process_event(epollfd, m_events, outime, &global_timer);
        outime = global_timer.get_mintimer();
        if (0 == outime) outime = 1000;

        if (jobs < 0) {
            free(m_events);
            return 0;
        }
    }
}
