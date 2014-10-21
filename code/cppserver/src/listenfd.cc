#include "network.h"

extern PFdProcess gFdProcess[MAX_FD];
extern int sv[MAX_CPU][2];
extern struct sendfdbuff order_list[MAX_CPU];
extern int cpu_num;
extern pthread_mutex_t dispatch_mutex;
extern pthread_cond_t dispatch_cond;

extern struct accepted_fd *g_fd;

int movefddata(int fd, char *buff, int len);

int sendbuff(int fd);
int sumaccept = 0;
// listen fd, 当有可读事件发生时调用
int accept_readfun(int epollfd, int listenfd, timer_link *timerlink) {
    struct sockaddr_in servaddr;
    int len = sizeof(servaddr);
    int newacceptfd = 0;

    char buff[MAXACCEPTSIZE * 4] = {0};
    int acceptindex = 0;
    int *pbuff = (int *)buff;

    while (true) {
        memset(&servaddr, 0, sizeof(servaddr));
        newacceptfd =
            accept(listenfd, (struct sockaddr *)&servaddr, (socklen_t *)&len);
        if (newacceptfd > 0) {            // accept new fd
            if (newacceptfd >= MAX_FD) {  // 太多链接 系统无法接受
                Log << "accept fd  close fd:" << newacceptfd << std::endl;
                close(newacceptfd);
                continue;
            }
            *(pbuff + acceptindex) = newacceptfd;
            acceptindex++;
            if (acceptindex < MAXACCEPTSIZE) {
                continue;
            }

            struct accepted_fd *p = new accepted_fd();
            p->len = acceptindex * 4;
            memcpy(p->buff, buff, p->len);
            p->next = NULL;
            pthread_mutex_lock(&dispatch_mutex);

            if (g_fd) {
                p->next = g_fd;
                g_fd = p;
            } else {
                g_fd = p;
            }
            pthread_cond_broadcast(&dispatch_cond);
            pthread_mutex_unlock(&dispatch_mutex);
            acceptindex = 0;

        } else if (newacceptfd == -1) {
            if (errno != EAGAIN && errno != ECONNABORTED && errno != EPROTO &&
                errno != EINTR) {
                Log << "errno ==" << errno << std::endl;
                perror("read error1 errno");
                exit(0);
            } else {

                if (acceptindex) {
                    struct accepted_fd *p = new accepted_fd();
                    p->len = acceptindex * 4;
                    memcpy(p->buff, buff, p->len);
                    p->next = NULL;
                    pthread_mutex_lock(&dispatch_mutex);

                    if (g_fd) {
                        p->next = g_fd;
                        g_fd = p;
                    } else {
                        g_fd = p;
                    }
                    pthread_cond_broadcast(&dispatch_cond);
                    pthread_mutex_unlock(&dispatch_mutex);
                    acceptindex = 0;
                }
                break;
            }
        } else {
            Log << "errno 3==" << errno << std::endl;
            exit(1);
            break;
        }
    }
    return 1;
}

int accept_write(int fd) { return 1; }

int fdsend_writefun(int epollfd, int sendfd, timer_link *timerlink) {
    Log << "call fdsend_writefun" << std::endl;
    return sendbuff(sendfd);
}

int movefddata(int fd, char *buff, int len) {

    memmove(order_list[fd].buff + order_list[fd].len, buff, len);
    order_list[fd].len += len;
    return order_list[fd].len;
}

int sendbuff(int fd) {

    int needsend = order_list[fd].len;
    int sended = 0;
    int sendlen = 0;
    char *buff = order_list[fd].buff;
    while (true) {
        if (needsend - sended > 0) {
            sendlen = send(fd, buff + sended, needsend - sended, 0);
            if (sendlen > 0) {
                sended += sendlen;
            } else {
                break;
            }
        } else {
            break;
        }
    }
    if (needsend == sended) {
    } else {
        memmove(buff, buff + sended, needsend - sended);
    }
    order_list[fd].len = needsend - sended;
    return sended;
}

void *dispatch_conn(void *arg) {
    static long sum = 0;
    struct accepted_fd *paccept = NULL;
    for (;;) {
        pthread_mutex_lock(&dispatch_mutex);
        if (g_fd) {
            paccept = g_fd;
            g_fd = NULL;

        } else {
            pthread_cond_wait(&dispatch_cond, &dispatch_mutex);
            paccept = g_fd;
            g_fd = NULL;
        }

        pthread_mutex_unlock(&dispatch_mutex);
        while (paccept) {
            int index = sum % (cpu_num - 1) + 1;
            int sendfd = sv[index][0];
            movefddata(sendfd, paccept->buff, paccept->len);
            sendbuff(sendfd);
            ++sum;
            struct accepted_fd *next = paccept->next;
            delete paccept;
            paccept = next;
        }
    }
    return NULL;
}
