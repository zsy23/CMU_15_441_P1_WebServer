/******************************************************************************
* socket.h                                                                    *
*                                                                             *
* Description: This file contains the C declaration code for the robust socket*
*                                                                             *
* Author: Shiyu Zhang <1181856726@qq.com>                                     *
*                                                                             *
*******************************************************************************/

#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <sys/socket.h>
#include <sys/select.h>

// robust socket()
int _socket( int domain, int type, int protocol );

// robust bind()
int _bind( int sockfd, const struct sockaddr *addr, socklen_t addrlen );

// robust listen()
int _listen( int sockfd, int backlog );

// robust accept()
int _accept( int sockfd, struct sockaddr *addr, socklen_t *addrlen );

// robust select()
int _select( int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout );

// robust recv()
ssize_t _recv( int sockfd, void *buf, size_t len, int flags);

// robust send()
ssize_t _send( int sockfd, const void *buf, size_t len, int flags);

// robust close()
int _close( int sockfd ); 

#endif
