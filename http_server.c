/******************************************************************************
* http_server.c                                                               *
*                                                                             *
* Description: This file contains the C source code for the http server.      *
*                                                                             *
* Author: Shiyu Zhang <1181856726@qq.com>                                     *
*                                                                             *
*******************************************************************************/

#include "client.h"
#include "log.h"
#include "util.h"
#include "core.h"
#include "http.h"
#include "cgi.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

// close debug
#define NO_DEBUG

// arguments max size
#define ARG_MAX_SIZE 256

// whether daemonize
#define DAEMONIZED TRUE

// daemoize
extern int daemonize( char *lock_file );

// get arguments
int get_args(int argc, char *argv[], int *http, int *https, char *log, char *lock, char *www, char *cgi, char *prikey, char *certificate);

// shutdown http server and close all file descriptons
void shutdown_server( int r );

// get arguments
int get_args(int argc, char *argv[], int *http, int *https, char *log, char *lock, char *www, char *cgi, char *prikey, char *certificate)
{
    int size;
    size = 0;

    if ( argc != 9 )
    {
        fprintf(stderr, "usage: ./http_server <HTTP port> <HTTPS port> <log file> <lock file> <www folder> <CGI script path> <private key file> <certificate file>\n");
        return -1;
    }

    // http port
    if( is_uint( argv[1], strlen( argv[1] ) ) == FALSE )
    {
        LOG_ERROR( "HTTP PORT not UINT\n" );
        return -1;
    }
    *http = atoi(argv[1]);

    // https port
    if( is_uint( argv[2], strlen( argv[2] ) ) == FALSE )
    {
        LOG_ERROR( "HTTPS PORT not UINT\n" );
        return -1;
    }
    *https = atoi(argv[2]);
    
    // log file
    size = strlen( argv[3] ) + 1;
    if( size > ARG_MAX_SIZE )
    {
        LOG_ERROR( "LOG FILE too long\n" );
        return -1;
    }
    memcpy( log, argv[3], size );

    // lock file
    size = strlen( argv[4] ) + 1;
    if( size > ARG_MAX_SIZE )
    {
        LOG_ERROR( "LOCK FILE too long\n" );
        return -1;
    }
    memcpy( lock, argv[4], size );

    // www folder
    size = strlen( argv[5] ) + 1;
    if( size > ARG_MAX_SIZE )
    {
        LOG_ERROR( "WWW FOLDER too long\n" );
        return -1;
    }
    memcpy( www, argv[5], size );

    // cgi
    size = strlen( argv[6] ) + 1;
    if( size > ARG_MAX_SIZE )
    {
        LOG_ERROR( "CGI too long\n" );
        return -1;
    }
    memcpy( cgi, argv[6], size );

    // private ket file
    size = strlen( argv[7] ) + 1;
    if( size > ARG_MAX_SIZE )
    {
        LOG_ERROR( "PRIVATE KEY FILE too long\n" );
        return -1;
    }
    memcpy( prikey, argv[7], size );

    // certificate file
    size = strlen( argv[8] ) + 1;
    if( size > ARG_MAX_SIZE )
    {
        LOG_ERROR( "CERTIFICATE FILE too long\n" );
        return -1;
    }
    memcpy( certificate, argv[8], size );

    return 0; 
}

// shutdown http server and clean all file descriptions
void shutdown_server( int r )
{
    int i;

    for( i = getdtablesize(); i >= 0; --i )
        close( i );

    LOG_INFO( "Shutdown the server\n" );

    exit( r );
}

int main( int argc, char *argv[] )
{
    int http_port, https_port;
    char log_file[ARG_MAX_SIZE] = { 0 };
    char lock_file[ARG_MAX_SIZE] = { 0 };
    char www_folder[ARG_MAX_SIZE] = { 0 };
    char cgi_path[ARG_MAX_SIZE] = { 0 };
    char prikey_file[ARG_MAX_SIZE] = { 0 };
    char certificate_file[ARG_MAX_SIZE] = { 0 };
    int http_sock, https_sock;
    int maxi, maxfd;
    fd_set allset;
    struct timeval timeout;
    client_info *clients[CLIENT_MAX_NUM];
    struct sockaddr_in http_addr, https_addr;
    list( sock_info ) unacc_head, *unacc_sock, *unacc_tmp;
    time_t now;
    char date[DATE_SIZE] = { 0 };
    SSL_CTX *ssl_context;
    int i, j, r;

    // get eight arguments from input
    if( ( r = get_args(argc, argv, &http_port, &https_port, log_file, lock_file, www_folder, cgi_path, prikey_file, certificate_file) ) < 0 )    
        return -1;
    
    // init daemonize
    if( DAEMONIZED == TRUE )
        daemonize( lock_file );

    // init logging module
    log_init( LOG_LEVEL_DEBUG, log_file );    

    // set www folder
    if( set_www( www_folder ) < 0 )
        return -1;

    // set cgi path
    if( set_cgi( cgi_path, http_port, https_port ) < 0 )
        return -1;

    // setup http socket
    if( ( http_sock = _socket( PF_INET, SOCK_STREAM, 0 ) ) < 0 )
        return -1;

    http_addr.sin_family = AF_INET;
    http_addr.sin_port = htons( http_port );
    http_addr.sin_addr.s_addr = INADDR_ANY;

   if( ( r = _bind( http_sock, ( struct sockaddr * ) &http_addr, sizeof( http_addr ) ) ) < 0 )
        return -1;     

    if( ( r = _listen( http_sock, FD_SETSIZE ) ) < 0 )
        return -1; 

    // init ssl
    ssl_context = NULL;
    if( ssl_init( &ssl_context, prikey_file, certificate_file ) < 0 )
        return -1;

    // setup https socket
    if( ( https_sock = _socket( PF_INET, SOCK_STREAM, 0 ) ) < 0 )
        return -1;

    https_addr.sin_family = AF_INET;
    https_addr.sin_port = htons( https_port );
    https_addr.sin_addr.s_addr = INADDR_ANY;

    if( ( r = _bind( https_sock, ( struct sockaddr * ) &https_addr, sizeof( https_addr ) ) ) < 0 )
        return -1;

    if( ( r = _listen( https_sock, FD_SETSIZE ) ) < 0 )
        return -1;

    // init  select concerned data structure
    for ( i = 0; i < CLIENT_MAX_NUM; ++i ) clients[i] = NULL;
    maxi = -1;
    maxfd = MAX( http_sock, https_sock );
    FD_ZERO( &allset );
    FD_SET( http_sock, &allset );
    FD_SET( https_sock, &allset );
    timeout.tv_sec = TIMEOUT_SEC;
    timeout.tv_usec = 0;

    // loop for accept and process
    while( 1 )
    {
        unacc_head.next =  NULL;

        // block to wait for inboound io
        listening(http_sock, https_sock, ssl_context, clients, &maxi, &maxfd, &allset, &timeout, &unacc_head); 

        // process unable to served clients
        unacc_sock = unacc_head.next;
        while( unacc_sock != NULL )
        {
            CLR_BUF( date, DATE_SIZE );
            time( &now );
            date_format( date, DATE_SIZE, &now );

            respond( &unacc_sock->data, "HTTP/1.1", STATUS_503, "close", date, "Liso/1.0", NULL );

            _close( unacc_sock->data.fd );
            LOG_ERROR( "Unable to accept anymore connection\n" );
            unacc_tmp = unacc_sock;
            unacc_sock = unacc_sock->next;
            free( unacc_tmp );
        }

        // process client resquest
        for( i = 0; i <= maxi; ++i )
        {
            // find one cgi output
            if( clients[i] != NULL && clients[i]->cgi_done == TRUE )
            {
                int size = 0;
                char conn[CONN_MAX_SIZE] = { 0 };

                if( clients[i]->header[HDR_CONNECTION] != NULL )
                    size = strlen( clients[i]->header[HDR_CONNECTION] );
                if( size == 0 )
                    snprintf( conn, CONN_MAX_SIZE, "keep-alive" );
                else if( strncmp( clients[i]->header[HDR_CONNECTION], "close", MAX( size, 5 ) ) == 0 )
                    snprintf( conn, CONN_MAX_SIZE, "close" );
                else if( strncmp( clients[i]->header[HDR_CONNECTION], "keep-alive", MAX( size, 10 ) ) == 0 )
                    snprintf( conn, CONN_MAX_SIZE, "keep-alive" );
                else
                    snprintf( conn, CONN_MAX_SIZE, "keep-alive" );
        
                // if output too long
                if( clients[i]->output_too_long == TRUE )
                {
                    CLR_BUF( date, DATE_SIZE );
                    time( &now );
                    date_format( date, DATE_SIZE, &now );

                    respond( &clients[i]->sock, "HTTP/1.1", STATUS_500, conn, date, "Liso/1.0", NULL );
                }
                else
                {
                    LOG_INFO( "Response To %s:%u( %d ):\n%s\n",
                          inet_ntoa( clients[i]->sock.addr.sin_addr ), ntohs( clients[i]->sock.addr.sin_port ), 
                          clients[i]->output_len, clients[i]->output );

                    respond_directly( &clients[i]->sock, clients[i]->output, clients[i]->output_len );
                }

                // close piped fd
                if( clients[i]->piped_fd >= 0 )
                {
                    FD_CLR( clients[i]->piped_fd, &allset );
                    _close( clients[i]->piped_fd );
                }
                
                if( strncmp( conn, "close", MAX( strlen( conn ), 5 ) ) == 0  )
                {
                    LOG_INFO( "Connection to %s:%u closed normally\n", inet_ntoa( clients[i]->sock.addr.sin_addr ), ntohs( clients[i]->sock.addr.sin_port ) );
                    _close( clients[i]->sock.fd );
                    if( clients[i]->sock.context != NULL )
                        ssl_close( NULL, -1, clients[i]->sock.context );
                    for( j = 0; j < HDR_SIZE; ++j )
                        if( clients[i]->header[j] != NULL )
                            free( clients[i]->header[j] );
                    FD_CLR( clients[i]->sock.fd, &allset );
                    free( clients[i] );
                    clients[i] = NULL;
                }
                else
                {
                    clients[i]->piped_fd = -1;
                    clients[i]->output_len = 0;
                    CLR_BUF( clients[i]->output, OUTPUT_SIZE );
                    clients[i]->output_too_long = FALSE;
                    clients[i]->cgi_done = FALSE;
                }
            }

            // find one request
            if( clients[i] != NULL && clients[i]->ready == TRUE )
            {
                LOG_INFO( "Request From %s:%u( %ld ):\n%s\n",
                          inet_ntoa( clients[i]->sock.addr.sin_addr ), ntohs( clients[i]->sock.addr.sin_port ), 
                          clients[i]->len, clients[i]->buf );

                // parse HTTP/1.1 request
                parse( clients[i] );  

                LOG_DEBUG( "Client(%s:%u) meth: %d, uri: %s, version: %s, done: %d, state: %d, hdr_len: %d, token:%s\nleft: %d msg: %s\n", 
                        inet_ntoa( clients[i]->sock.addr.sin_addr ), ntohs( clients[i]->sock.addr.sin_port ), 
                        clients[i]->meth, clients[i]->uri, clients[i]->version,
                        clients[i]->done, clients[i]->state, clients[i]->hdr_len, clients[i]->token, 
                        clients[i]->left, clients[i]->msg );

                // if client's parse end, process request
                if( clients[i]->done == TRUE )
                {                    
                    process( clients[i] );
                    
                    if( clients[i]->piped_fd >= 0 )
                    {
                        if( maxfd < clients[i]->piped_fd ) 
                            maxfd = clients[i]->piped_fd;
                        FD_SET( clients[i]->piped_fd, &allset );
                    }

                    // persistent connection
                    if( clients[i]->piped_fd < 0 && strncmp( clients[i]->header[HDR_CONNECTION], "close", MAX( strlen( clients[i]->header[HDR_CONNECTION] ), 5 ) ) == 0  )
                    {
                        LOG_INFO( "Connection to %s:%u closed normally\n", inet_ntoa( clients[i]->sock.addr.sin_addr ), ntohs( clients[i]->sock.addr.sin_port ) );
                        _close( clients[i]->sock.fd );
                        if( clients[i]->piped_fd >= 0 )
                            _close( clients[i]->piped_fd );
                        if( clients[i]->sock.context != NULL )
                            ssl_close( NULL, -1, clients[i]->sock.context );
                        for( j = 0; j < HDR_SIZE; ++j )
                            if( clients[i]->header[j] != NULL )
                                free( clients[i]->header[j] );
                        FD_CLR( clients[i]->sock.fd, &allset );
                        free( clients[i] );
                        clients[i] = NULL;
                    }
                    else
                    {
                        CLR_BUF( clients[i]->buf, BUF_SIZE );
                        clients[i]->len = -1;
                        clients[i]->ready = FALSE; 
                        clients[i]->timeout = time(NULL) + TIMEOUT_SEC;
                        clients[i]->meth = METHOD_NONE;
                        CLR_BUF( clients[i]->uri, URI_SIZE );
                        CLR_BUF( clients[i]->version, VERSION_SIZE );
                        for( j = 0; j < HDR_SIZE; ++j )
                            if( clients[i]->header[j] != NULL )
                            {
                                free( clients[i]->header[j] );
                                clients[i]->header[j] = NULL;
                            }
                        clients[i]->left = -1;
                        CLR_BUF( clients[i]->msg, MSG_SIZE );
                        clients[i]->state = STATE_START;
                        clients[i]->hdr_len = 0;
                        CLR_BUF( clients[i]->token, TOKEN_SIZE );
                        clients[i]->done = FALSE; 
                    }
                }  
                else
                    // reset client receive buf state
                    clients[i]->ready = FALSE;
            }
        }
    }

    // close server HTTP socket
    _close( http_sock );

    // close server HTTPS socket
    ssl_close( ssl_context, https_sock, NULL );

    // close logging module
    log_close();

    return 0;
}
