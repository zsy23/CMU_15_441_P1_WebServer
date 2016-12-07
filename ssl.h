/*****************************************************************************
 * ssl.h                                                                     *
 *                                                                           *
 * Description: This file contains the C declaration code for the ssl wrapper*
 *                                                                           *
 * Author: Shiyu Zhang <1181856726@qq.com>                                   *
 *                                                                           *
 *****************************************************************************/

#include <openssl/ssl.h>

// init ssl
int ssl_init( SSL_CTX **ssl_context, const char *prikey, const char *crt );

// wrap socket with ssl
int ssl_wrap_socket( SSL_CTX *ssl_context, int sockfd, SSL **sock_context );

// close ssl
void ssl_close( SSL_CTX *ssl_context, int sockfd, SSL *sock_context ); 

// ssl robust read
int ssl_read( SSL *ssl, void *buf, int len );

// ssl robust write
int ssl_write( SSL *ssl, const void *buf, int len );
