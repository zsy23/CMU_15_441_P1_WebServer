/*****************************************************************************
 * ssl.c                                                                     *
 *                                                                           *
 * Description: This file contains the C source code for the ssl wrapper.    *
 *                                                                           *
 * Author: Shiyu Zhang <1181856726@qq.com>                                   *
 *                                                                           *
 *****************************************************************************/

#include "ssl.h"
#include "log.h"
#include "socket.h"

#include <openssl/err.h>

// init ssl
int ssl_init( SSL_CTX **ssl_context, const char *prikey, const char *crt )
{
    unsigned long e;
    char err_msg[1024] = { 0 }; 
    char *tmp = NULL;

    SSL_load_error_strings();
    SSL_library_init();

    // use TLSv1 only
    if( ( *ssl_context = SSL_CTX_new( TLSv1_server_method() ) ) == NULL )
    {
        e = ERR_get_error();
        tmp = ERR_error_string( e, err_msg );
        LOG_ERROR( "Error creating SSL context: %s\n", err_msg );
        return -1;
    }

    // register private key
    if( SSL_CTX_use_PrivateKey_file( *ssl_context, prikey, SSL_FILETYPE_PEM ) == 0 )
    {
        e = ERR_get_error();
        tmp = ERR_error_string( e, err_msg );
        LOG_ERROR( "Error associating private key: %s\n", err_msg );

        SSL_CTX_free( *ssl_context );

        return -1;
    }

    // registry public key( certificate )
    if( SSL_CTX_use_certificate_file( *ssl_context, crt, SSL_FILETYPE_PEM ) == 0 )
    {
        e = ERR_get_error();
        tmp = ERR_error_string( e, err_msg );
        LOG_ERROR( "Error associating certificate: %s\n", err_msg );

        SSL_CTX_free( *ssl_context );

        return -1;
    }

    return 0;
}

// wrap socket with ssl
int ssl_wrap_socket( SSL_CTX *ssl_context, int sockfd, SSL **sock_context )
{
    unsigned long e;
    char err_msg[1024] = { 0 }; 
    char *tmp = NULL;

    if( ( *sock_context = SSL_new( ssl_context ) ) == NULL )
    {
        e = ERR_get_error();
        tmp = ERR_error_string( e, err_msg );
        LOG_ERROR( "Error creating client SSL context: %s\n", err_msg );

        _close( sockfd );

        return -1;
    }

    if( SSL_set_fd( *sock_context, sockfd ) == 0 )
    {
        e = ERR_get_error();
        tmp = ERR_error_string( e, err_msg );
        LOG_ERROR( "Error creating client SSL context: %s\n", err_msg );

        _close( sockfd );
        SSL_free( *sock_context );

        return -1;
    }  

    if( SSL_accept( *sock_context ) <= 0 )
    {
        e = ERR_get_error();
        tmp = ERR_error_string( e, err_msg );
        LOG_ERROR( "Error accepting (handshake) client SSL context: %s\n", err_msg );

        _close( sockfd );
        SSL_free( *sock_context );

        return -1;
    }

    return 0;
}

// close ssl
void ssl_close( SSL_CTX *ssl_context, int sockfd, SSL *sock_context )
{
    if( sock_context != NULL )
    {
        SSL_shutdown( sock_context );
        SSL_free( sock_context );
    }
    if( sockfd >= 0 )
        _close(sockfd);
    if( ssl_context != NULL )
        SSL_CTX_free( ssl_context );
}

// ssl robust read
int ssl_read( SSL *ssl, void *buf, int len )
{
    int ret;
    unsigned long e;
    char err_msg[1024] = { 0 }; 
    char *tmp = NULL;

    ret = 0;
    ret = SSL_read( ssl, buf, len );

    if( ret < 0 )
    {
        e = ERR_get_error();
        tmp = ERR_error_string( e, err_msg );
        LOG_ERROR( "Error reading from sockets: %s\n", err_msg );
    }

    return ret;
}

// ssl robust write
int ssl_write( SSL *ssl, const void *buf, int len )
{
    int ret;
    unsigned long e;
    char err_msg[1024] = { 0 }; 
    char *tmp = NULL;

    ret = 0;
    ret = SSL_write( ssl, buf, len );

    if( ret < 0 )
    {
        e = ERR_get_error();
        tmp = ERR_error_string( e, err_msg );
        LOG_ERROR( "Error writing to socket: %s\n", err_msg );
    }

    return ret;
}
