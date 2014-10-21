#ifndef MAIN_EVENT_H
#define MAIN_EVENT_H

void process_one_event(int epollfd, struct epoll_event *m_events, int timeout,
                       timer_link *timer);

#endif
