#include "network.h"
#include "socket_util.h"
#include "timers.h"

pthread_t t_all[MAX_CPU] = {0};
cpu_set_t mask[MAX_CPU];

// 命令管道
int sv[MAX_CPU][2] = {};
int cpu_cmd_map[MAX_CPU][2];
int cpu_num = 1;
bool close_fun(int fd) { return true; }

struct sendfdbuff order_list[MAX_CPU];

volatile int jobs = 0;

struct epoll_event *m_events;

int epollfd;

struct accepted_fd *g_fd;

pthread_mutex_t dispatch_mutex;
pthread_cond_t dispatch_cond;

extern PFdProcess gFdProcess[MAX_FD];
extern char senddata[2048];

int main() {

    g_fd = NULL;

    pthread_mutex_init(&dispatch_mutex, NULL);
    pthread_cond_init(&dispatch_cond, NULL);

    for (unsigned int i = 0; i < sizeof(senddata); ++i) {
        senddata[i] = i % 26 + 'a';
    }
    // pipe
    signal(SIGPIPE, SIG_IGN);  // sigpipe 信号屏蔽

    // bind
    m_events = (struct epoll_event *)malloc(MAXEPOLLEVENT *
                                            sizeof(struct epoll_event));
    int rt = 0;
    epollfd = epoll_create(MAXEPOLLEVENT);
    struct sockaddr_in servaddr;
    int listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listenfd <= 0) {
        perror("socket");
        return 0;
    }

    rt = set_reused(listenfd);
    if (rt < 0) {
        perror("setsockopt");
        exit(1);
    }
    for (int i = 0; i < MAX_FD; ++i) {
        PFdProcess p = new FdProcess();
        assert(p);
        p->fd = i;
        gFdProcess[i] = p;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(listenport);

    rt = bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (rt < 0) {
        perror("setsockopt");
        exit(1);
    }

    rt = listen(listenfd, 50000);
    if (rt < 0) {
        perror("listen");
        exit(1);
    }
    //
    struct epoll_event ev = {0};

    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listenfd;
    set_noblock(listenfd);

    gFdProcess[listenfd]->m_readfun = accept_readfun;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) == -1) {
        perror("epoll_ctl: listen_sock");
        exit(-1);
    }

    // 到Cpu 核心数目
    cpu_num = sysconf(_SC_NPROCESSORS_CONF) * 1;

    for (int i = 1; i < cpu_num; ++i) {
        Log << " cpu " << i << "  " << sv[i][0] << "  " << sv[i][1]
            << std::endl;
    }

    // 启动 对应的socketpair
    for (int i = 1; i < cpu_num; ++i) {
        CPU_ZERO(&mask[i]);
        CPU_SET(i, &mask[i]);
        struct worker_thread_arg *pagr = new worker_thread_arg();

        pagr->cpuid = i % (sysconf(_SC_NPROCESSORS_CONF) - 1) + 1;
        pagr->orderfd = sv[i][1];

        // 启动线程
        pthread_create(&t_all[i], NULL, worker_thread, (void *)pagr);
        //绑定亲元性
        pthread_setaffinity_np(t_all[i], sizeof(cpu_set_t),
                               (const cpu_set_t *)&(mask[i]));
    }

    pthread_t dispatch_t;

    pthread_create(&dispatch_t, NULL, dispatch_conn, NULL);

    // 启动主线程
    pthread_t t_main;
    cpu_set_t mask_main;
    // pthread_attr_t attr_main;

    CPU_ZERO(&mask_main);
    CPU_SET(0, &mask_main);

    // 启动线程
    pthread_create(&t_main, NULL, main_thread, NULL);
    //绑定亲元性
    pthread_setaffinity_np(t_main, sizeof(cpu_set_t),
                           (const cpu_set_t *)&mask_main);

    pthread_join(t_main, NULL);
    return 0;
}

void *main_thread(void *arg) {

    timer_link global_timer;
    int outime = 1000;
    while (true) {

        process_event(epollfd, m_events, outime, &global_timer);
        if (global_timer.get_arg_time_size() > 0) {
            outime = global_timer.get_mintimer();
        } else {
            outime = 1000;
        }
        if (jobs < 0) {
            free(m_events);
            return 0;
        }
    }
}
