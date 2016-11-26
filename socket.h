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
#include <sys/time.h>

int _socket( int domain, int type, int protocol );
int _bind( int sockfd, const struct sockaddr *addr, socklen_t addrlen );
int _listen( int sockfd, int backlog );
int _accept( int sockfd, struct sockaddr *addr, socklen_t *addrlen );
int _select( int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout );
ssize_t _recv( int sockfd, void *buf, size_t len, int flags);
ssize_t _send( int sockfd, const void *buf, size_t len, int flags);
int _close( int sockfd ); 

#endif
