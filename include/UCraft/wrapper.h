#ifndef WRAPPER_H
#define WRAPPER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/select.h>
#include <signal.h>
#include <netdb.h>
#include <fcntl.h>

// Misc functions
static inline int U_fctl(int fd, int cmd, int arg)
{
    return fcntl(fd, cmd, arg);
}
static inline void U_usleep(useconds_t usec)
{
    usleep(usec);
}
static inline uint64_t micros()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;
}
// Networking functions
static inline int U_socket(int domain, int type, int protocol)
{
    return socket(domain, type, protocol);
}

static inline int U_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
    return setsockopt(sockfd, level, optname, optval, optlen);
}

static inline int U_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    return bind(sockfd, addr, addrlen);
}

static inline int U_listen(int sockfd, int backlog)
{
    return listen(sockfd, backlog);
}

static inline int U_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    return select(nfds, readfds, writefds, exceptfds, timeout);
}

static inline int U_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    return accept(sockfd, addr, addrlen);
}

static inline int U_getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    return getpeername(sockfd, addr, addrlen);
}

static inline struct hostent *U_gethostbyname(const char *name)
{
    return gethostbyname(name);
}

static inline char *U_inet_ntoa(struct in_addr in)
{
    return inet_ntoa(in);
}

static inline ssize_t U_read(int fd, void *buf, size_t count)
{
    return read(fd, buf, count);
}
static inline ssize_t U_recv(int sockfd, void *buf, size_t len, int flags)
{
    return recv(sockfd, buf, len, flags);
}

static inline ssize_t U_send(int sockfd, const void *buf, size_t len, int flags)
{
    return send(sockfd, buf, len, flags);
}

static inline int U_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    return connect(sockfd, addr, addrlen);
}

static inline int U_close(int fd)
{
    return close(fd);
}

static inline int U_shutdown(int sockfd, int how)
{
    return shutdown(sockfd, how);
}
// Memory function
static inline void *U_malloc(size_t size)
{
    return malloc(size);
}

static inline void *U_calloc(size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

static inline void *U_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

static inline void U_free(void *ptr)
{
    free(ptr);
}
#endif