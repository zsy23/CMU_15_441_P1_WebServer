/******************************************************************************
* socket.c                                                                    *
*                                                                             *
* Description: This file contains the C source code for the robust socket.    *
*                                                                             *
* Author: Shiyu Zhang <1181856726@qq.com>                                     *
*                                                                             *
*******************************************************************************/

#include "socket.h"
#include "log.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>

int _socket( int domain, int type, int protocol)
{
    int sockfd;
    if( ( sockfd = socket( domain, type, protocol ) ) == -1 )
    {
        LOG_ERROR( "Failed creating socket: %s\n", strerror( errno ) );
        return -1;
    }
    return sockfd;
}

int _bind( int sockfd, const struct sockaddr *addr, socklen_t addrlen )
{
    if ( bind( sockfd, addr, addrlen ) )
    {
        _close( sockfd );
        LOG_ERROR( "Failed binding socket: %s\n", strerror( errno ) );
        return -1;
    }
    return 0;
}

int _listen( int sockfd, int backlog )
{
    if ( listen( sockfd, backlog ) )
    {
        _close( sockfd );
        LOG_ERROR( "Failed listening on socket: %s\n", strerror( errno ) );
        return -1;
    }
    return 0;
}

int _accept( int sockfd, struct sockaddr *addr, socklen_t *addrlen )
{
    int client_sockfd;
    if ( ( client_sockfd = accept( sockfd, addr, addrlen ) ) == -1 )
    {
        LOG_ERROR( "Failed accepting connection: %s\n", strerror( errno ) );
        return -1;
    }

    return client_sockfd;
}

int _select( int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout )
{
    int nready;
    nready = select( nfds, readfds, writefds, exceptfds, timeout );
    return nready;
}

int _close( int sockfd ) 
{
    if ( close( sockfd ) )
    {
        LOG_ERROR( "Failed closing socket: %s\n", strerror( errno ) );
        return -1;
    }
    return 0;
}

ssize_t _recv( int sockfd, void *buf, size_t len, int flags)
{
    ssize_t recv_len;
    if( ( recv_len = recv( sockfd, buf, len, flags ) ) < 0 )
        LOG_ERROR( "Failed receiving message: %s\n", strerror( errno ) );
    
    return recv_len;
}

ssize_t _send( int sockfd, const void *buf, size_t len, int flags)
{
    ssize_t send_len;
    if( ( send_len = send( sockfd, buf, len, flags ) ) < 0 )
        LOG_ERROR( "Failed sending message: %s\n", strerror( errno ) );

    return send_len;   
}
