#ifndef SOCKET_UTIL_H
#define SOCKET_UTIL_H

// set socket no block
int set_noblock(int sClient);

// set reused fd
int set_reused(int fd);

#endif
