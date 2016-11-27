/******************************************************************************
* http_server.c                                                               *
*                                                                             *
* Description: This file contains the C source code for the http server.      *
*                                                                             *
* Author: Shiyu Zhang <1181856726@qq.com>                                     *
*                                                                             *
*******************************************************************************/

#include "global.h"
#include "log.h"
#include "socket.h"
#include "core.h"
#include "http.h"

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

int get_args(int argc, char *argv[], int *http, int *https, char *log, char *lock, char *www, char *cgi, char *prikey, char *certificate);

int get_args(int argc, char *argv[], int *http, int *https, char *log, char *lock, char *www, char *cgi, char *prikey, char *certificate)
{
    if ( argc != 9 )
    {
        fprintf(stderr, "usage: ./http_server <HTTP port> <HTTPS port> <log file> <lock file> <www folder> <CGI script path> <private key file> <certificate file>\n");
        return -1;
    }

    *http = atoi(argv[1]);
    *https = atoi(argv[2]);
    memcpy( log, argv[3], strlen( argv[3] ) + 1 );
    memcpy( lock, argv[4], strlen( argv[4] ) + 1 );
    memcpy( www, argv[5], strlen( argv[5] ) + 1 );
    memcpy( cgi, argv[6], strlen( argv[6] ) + 1 );
    memcpy( prikey, argv[7], strlen( argv[7] ) + 1 );
    memcpy( certificate, argv[8], strlen( argv[8] ) + 1 );

    return 0; 
}

int main( int argc, char *argv[] )
{
    int http_port, https_port;
    char log_file[256], lock_file[256], www_folder[256], cgi_path[256];
    char prikey_file[256], certificate_file[256];
    int sock;
    int maxi, maxfd;
    fd_set allset;
    client_info *clients[FD_SETSIZE];
    struct sockaddr_in addr;
    int i, r;

    // get eight arguments from input
    if( ( r = get_args(argc, argv, &http_port, &https_port, log_file, lock_file, www_folder, cgi_path, prikey_file, certificate_file) ) == -1 )    
        return -1;
    
    // init logging module
    log_init( LOG_LEVEL_DEBUG, log_file );    

    // create server socket
    if( ( sock = _socket( PF_INET, SOCK_STREAM, 0 ) ) == -1 )
        return -1;

    addr.sin_family = AF_INET;
    addr.sin_port = htons( http_port );
    addr.sin_addr.s_addr = INADDR_ANY;

   if( ( r = _bind( sock, ( struct sockaddr * ) &addr, sizeof( addr ) ) ) == -1 )
        return -1;     

    if( ( r = _listen( sock, FD_SETSIZE ) ) == -1 )
        return -1; 

    // init  select concerned data structure
    for ( i = 0; i < FD_SETSIZE; ++i ) clients[i] = NULL;
    maxi = -1;
    maxfd = sock;
    FD_ZERO( &allset );
    FD_SET( sock, &allset );

    // loop for accept and process
    while( 1 )
    {
        listening(sock, clients, &maxi, &maxfd, &allset); 
        for( i = 0; i <= maxi; ++i )
        {
            if( clients[i] != NULL && clients[i]->ready == TRUE )
            {
                fprintf( stdout, "Msg from %s:%u: %s", inet_ntoa( clients[i]->addr.sin_addr ), ntohs( clients[i]->addr.sin_port), clients[i]->buf );
                clients[i]->ready = FALSE;
            }
        }
    }

    // close server socket
    _close( sock );

    // close logging module
    log_close();

    return 0;
}
