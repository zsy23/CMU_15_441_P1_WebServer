/******************************************************************************
* core.c                                                                      *
*                                                                             *
* Description: This file contains the C source code for the core networking.  *
*                                                                             *
* Author: Shiyu Zhang <1181856726@qq.com>                                     *
*                                                                             *
*******************************************************************************/

#include "log.h"
#include "socket.h"
#include "core.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

void listening(int ser_sock, client_info **clients, int *maxi, int *maxfd, fd_set *allset)
{
    fd_set rset;
    struct sockaddr_in cli_addr;
    int cli_sock;
    socklen_t addrlen;
    int i, nready;

    rset = *allset;

    nready = _select( *maxfd + 1, &rset, NULL, NULL, NULL );

    if ( FD_ISSET( ser_sock, &rset ) )
    {
        addrlen = sizeof( struct sockaddr_in ); 
        if( ( cli_sock = _accept( ser_sock, ( struct sockaddr * )&cli_addr, &addrlen ) ) != -1 )
        { 
            for ( i = 0; i < FD_SETSIZE; ++i )
            {
                if ( clients[i] == NULL )
                {
                    clients[i] = ( client_info * )malloc( sizeof( client_info ) );
                    clients[i]->addr = cli_addr;
                    clients[i]->sockfd = cli_sock;
                    clients[i]->meth = METHOD_NONE;
                    memset( clients[i]->uri, 0, 256 );
                    memset( clients[i]->version, 0, 10 );
                    clients[i]->conn = CONN_NONE;
                    memset( clients[i]->contype, 0, 128 );
                    clients[i]->conlen = -1;
                    memset( clients[i]->buf, 0, BUF_SIZE );
                    clients[i]->len = -1;
                    clients[i]->ready = FALSE; 
                    clients[i]->done = FALSE; 
                    break;
                }
            }
                
            if ( i == FD_SETSIZE )
            {
                _close( cli_sock );
                LOG_ERROR( "Failed accepting client because no fd available\n" );
            }
            else
            {
                LOG_INFO( "Connected by %s:%u\n", inet_ntoa( cli_addr.sin_addr ), ntohs( cli_addr.sin_port ) );

                if ( *maxi < i ) *maxi = i;
                if ( *maxfd < cli_sock ) *maxfd = cli_sock;
                FD_SET( cli_sock, allset );

                // workaround solution to situation where select() return while not actually be ready to read
                fcntl( cli_sock, F_SETFL, O_NONBLOCK ); 
            }            
        }

        if ( --nready <= 0 ) return;
    }        
         
    for ( i = 0; i <= *maxi; ++i )
    {
        if ( clients[i] != NULL && FD_ISSET( clients[i]->sockfd, &rset ) )
        {
            memset( clients[i]->buf, 0, BUF_SIZE );
            clients[i]->len = -1;            

            if( ( clients[i]->len = _recv( clients[i]->sockfd, clients[i]->buf, BUF_SIZE, 0 ) ) > 0 )
            {
                clients[i]->ready = TRUE;
            }
            else if( clients[i]->len == 0 )
            {
                LOG_INFO( "Connection to %s:%u closed\n", inet_ntoa( clients[i]->addr.sin_addr ), ntohs( clients[i]->addr.sin_port ) );
                _close( clients[i]->sockfd );
                FD_CLR( clients[i]->sockfd, allset );
                free( clients[i] );
                clients[i] = NULL;
            }

            if ( --nready <= 0 ) break;
        }
    }
}

