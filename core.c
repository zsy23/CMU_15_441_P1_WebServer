/******************************************************************************
* core.c                                                                      *
*                                                                             *
* Description: This file contains the C source code for the select() based    *
*              core networking.                                               *
*                                                                             *
* Author: Shiyu Zhang <1181856726@qq.com>                                     *
*                                                                             *
*******************************************************************************/

#include "log.h"
#include "socket.h"
#include "core.h"

#include <fcntl.h>
#include <stdlib.h>
#include <arpa/inet.h>

// blocking listening with timeout and return client info
void listening(int ser_sock, client_info **clients, int *maxi, int *maxfd, fd_set *allset, struct timeval *timeout, list( sock_info ) *unacc_head)
{
    fd_set rset;
    struct sockaddr_in cli_addr;
    int cli_sock;
    socklen_t addrlen;
    int i, nready;

    rset = *allset;

    // wait for inbound io
    nready = _select( *maxfd + 1, &rset, NULL, NULL, timeout );

    if( nready <= 0 )
    {
        // all clients must be timeout
        for( i = 0; i <= *maxi; ++i )
        {
            if( clients[i] != NULL )
            {
                LOG_INFO( "Connection to %s:%u closed due to timeout\n", inet_ntoa( clients[i]->addr.sin_addr ), ntohs( clients[i]->addr.sin_port ) );
                _close( clients[i]->sockfd );
                free( clients[i] );
                clients[i] = NULL;
            }
        }

        // reset data concerned
        FD_ZERO( &allset );
        *maxi = -1;
        *maxfd = ser_sock;
    }
    else
    {
        // process inbound io
        if( FD_ISSET( ser_sock, &rset ) )
        {
            // new connection from client
            addrlen = sizeof( struct sockaddr_in ); 
            if( ( cli_sock = _accept( ser_sock, ( struct sockaddr * )&cli_addr, &addrlen ) ) != -1 )
            { 
                // search for available client slot
                for( i = 0; i < CLIENT_MAX_NUM; ++i )
                {
                    if( clients[i] == NULL )
                    {
                        // find one and init client_info structure
                        clients[i] = ( client_info * )malloc( sizeof( client_info ) );
                        clients[i]->addr = cli_addr;
                        clients[i]->sockfd = cli_sock;
                        CLR_BUF( clients[i]->buf, BUF_SIZE );
                        clients[i]->len = -1;
                        clients[i]->ready = FALSE; 
                        clients[i]->timeout = time(NULL) + TIMEOUT_SEC;
                        clients[i]->meth = METHOD_NONE;
                        CLR_BUF( clients[i]->uri, URI_SIZE );
                        CLR_BUF( clients[i]->version, VERSION_SIZE );
                        clients[i]->conn = CONN_NONE;
                        CLR_BUF( clients[i]->contype, CONTYPE_SIZE );
                        clients[i]->conlen = -1;
                        clients[i]->left = -1;
                        CLR_BUF( clients[i]->msg, MSG_SIZE );
                        clients[i]->state = STATE_START;
                        clients[i]->hdr_len = 0;
                        CLR_BUF( clients[i]->token, TOKEN_SIZE );
                        clients[i]->done = FALSE; 
                        break;
                    }
                }
                
                // there's no available client slot
                if( i == CLIENT_MAX_NUM )
                {
                    // add unable served client socket to list
                    list( sock_info ) *unacc_sock = ( list( sock_info ) * )malloc( sizeof( list( sock_info ) ) );
                    unacc_sock->data.sockfd = cli_sock;
                    unacc_sock->data.addr = cli_addr;
                    unacc_sock->next = unacc_head->next;
                    unacc_head->next = unacc_sock;
                }
                else
                {
                    // already find one client slot and inited, then update data concerned
                    LOG_INFO( "Connected by %s:%u\n", inet_ntoa( cli_addr.sin_addr ), ntohs( cli_addr.sin_port ) );

                    if( *maxi < i ) *maxi = i;
                    if( *maxfd < cli_sock ) *maxfd = cli_sock;
                    FD_SET( cli_sock, allset );

                    // workaround solution to situation where select() return while not actually be ready to read
                    fcntl( cli_sock, F_SETFL, O_NONBLOCK ); 
                }            
            }

            --nready;
        }           
         
        if( nready > 0 )
        {
            // there is unprocessed io
            for( i = 0; i <= *maxi; ++i )
            {
                if( clients[i] != NULL && FD_ISSET( clients[i]->sockfd, &rset ) )
                {
                    CLR_BUF( clients[i]->buf, BUF_SIZE );
                    clients[i]->len = -1;            

                    // receive client's message
                    if( ( clients[i]->len = _recv( clients[i]->sockfd, clients[i]->buf, BUF_SIZE, 0 ) ) > 0 )
                    {
                        clients[i]->ready = TRUE;
                        clients[i]->timeout = time(NULL) + TIMEOUT_SEC;
                    }
                    else if( clients[i]->len == 0 )
                    {
                        // remote client close the connection and update data concerned
                        LOG_INFO( "Connection to %s:%u closed normally\n", inet_ntoa( clients[i]->addr.sin_addr ), ntohs( clients[i]->addr.sin_port ) );
                        _close( clients[i]->sockfd );
                        FD_CLR( clients[i]->sockfd, allset );
                        free( clients[i] );
                        clients[i] = NULL;
                    } 

                    if( --nready <= 0 ) break;
                }
            }
        }

        // check if there is client timeout
        for ( i = 0; i <= *maxi; ++i )
        {
            if( clients[i] != NULL && clients[i]->timeout <= time(NULL) )
            {
                LOG_INFO( "Connection to %s:%u closed due to timeout\n", inet_ntoa( clients[i]->addr.sin_addr ), ntohs( clients[i]->addr.sin_port ) );
                _close( clients[i]->sockfd );
                FD_CLR( clients[i]->sockfd, allset );
                free( clients[i] );
                clients[i] = NULL;
            }
        }
    }
}
