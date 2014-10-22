#ifndef NETWORK_H
#define NETWORK_H

#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <assert.h>
#include "timers.h"

#include <string>
#include <iostream>
#include <sched.h>
#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include "socket_util.h"
#include "http_client.h"
/*
//buffer 部分，主要存储原始数据
//除了http head以外，这个服务器绝大多数的数据应该由这个buff来存储
struct buffer {
        char *buffs; //实际数据存储的地方
        int datalen;  // 存在的数据长度，方便计算
        int startid;  // 开始id
        int endid;   //结束id

        // 关系是这样的 如果存在100 个空间 存储了50个数据， 那么 startid =0，
endid = 50
        // buffmaxlen = 100；endid  应该《 buffmaxlen 才对

        int buffmaxlen;  //buffmaxlen 再长就有问题
        struct buffer *next; //下一个
        struct buffer *last;  // 链表的最后一个
};
*/
const int K = 1024;

// 60 万连接数
const int MAX_FD = 6 * 10 * K;

// 每次处理event 数目
const int MAXEPOLLEVENT = 10000;

//#define maxlen   65000

#define MAX_CPU 128

typedef int (*readfun)(int epoll, int fd, timer_link *);
typedef int (*writefun)(int epoll, int fd, timer_link *);
typedef int (*closefun)(int epoll, int fd, timer_link *);
typedef int (*timeoutfun)(int epoll, int fd, timer_link *, time_t tnow);

int accept_readfun(int epoll, int listenfd, timer_link *);
int accept_write(int epoll, int listenfd, timer_link *);

int client_readfun(int epoll, int fd, timer_link *timer);
int client_writefun(int epoll, int fd, timer_link *timer);
int client_closefun(int epoll, int fd, timer_link *timer);

int order_readfun(int epoll, int listenfd, timer_link *);

struct worker_thread_arg {
    int orderfd;  // listen 线程会用这个发送fd过来
    int cpuid;
};

struct FdProcess {

    readfun m_readfun;
    writefun m_writefun;
    closefun m_closefun;
    timeoutfun m_timeoutfun;
    time_t m_lastread;
    time_t m_timeout;  // timeout的时间
    bool m_activeclose;

    FdProcess() { init(); }
    void init() {
        m_readfun = NULL;
        m_writefun = NULL;
        m_closefun = NULL;
        m_timeoutfun = NULL;
        m_lastread = time(0);

        m_lastread = 0;

        m_activeclose = false;
    }
};

typedef struct FdProcess FdProcess;
typedef struct FdProcess *PFdProcess;
#define Log std::cerr

bool connect(int &fd, std::string host, int port);

// int listenport = 8888;

bool addtimer(timer_link *, int fd, time_t);

bool checkontimer(timer_link *, int *fd, time_t *);

bool stoptimer(timer_link *, int fd, time_t);

time_t getnexttimer(timer_link *);

void process_event(int epollfd, struct epoll_event *m_events, int timeout,
                   timer_link *timers);

void *worker_thread(void *arg);

void *main_thread(void *arg);
#endif
