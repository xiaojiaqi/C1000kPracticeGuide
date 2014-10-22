#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <string>

int Simconnect(std::string host, int port);

int add_epoll(int epollfd, int fd, bool r = true, bool w = true,
              bool add_or_mod = true);

#endif
