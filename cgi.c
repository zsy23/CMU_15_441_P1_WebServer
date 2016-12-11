/******************************************************************************
* cgi.c                                                                       *
*                                                                             *
* Description: This file contains the C source code for the CGI.              *
*                                                                             *
* Author: Shiyu Zhang <1181856726@qq.com>                                     *
*                                                                             *
*******************************************************************************/

#include "cgi.h"
#include "log.h"
#include "util.h"

#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

// cgi script path
char cgi_script[CGI_SIZE] = { 0 };

// server port
int http_port = -1, https_port = -1;

// set cgi path
int set_cgi( const char *cgi_path, int http, int https )
{
    if( strlen( cgi_path ) > ( CGI_SIZE - 1 ) )
    {
        LOG_ERROR( "CGI path too log\n" );
        return -1;
    }
    snprintf( cgi_script, CGI_SIZE, "%s", cgi_path );

    http_port = http;
    https_port = https;

    return 0;
}

// set cgi's meta-variables
int set_meta_variables( client_info *client, char **meta )
{
    int size = 0;
    // /cgi/arg1/arg2?test=value
    //     ^         ^
    //    pos0      pos1( if no ?, then = len
    int pos0 = 4, pos1 = -1;
    for( pos1 = pos0 + 1; pos1 < strlen( client->uri ) && client->uri[pos1] != '?'; ++pos1 );
        
    size = 0;
    if( client->header[HDR_CONTENT_LENGTH] != NULL )
        size += strlen( client->header[HDR_CONTENT_LENGTH] );
    if( size > ( META_SIZE - 1 ) )
    {
        LOG_ERROR( "Content-Length too long\n" );
        return REQUEST_ERROR;
    }
    size += 16;
    meta[META_CONTENT_LENGTH] = ( char * )malloc( size );
    snprintf( meta[META_CONTENT_LENGTH], size, "CONTENT_LENGTH=%s", client->header[HDR_CONTENT_LENGTH] );
        
    size = 0;
    if( client->header[HDR_CONTENT_TYPE] != NULL )
        size += strlen( client->header[HDR_CONTENT_TYPE] );
    if( size > ( META_SIZE - 1 ) )
    {
        LOG_ERROR( "Content-Type too long\n" );
        return REQUEST_ERROR;
    }
    size += 14;
    meta[META_CONTENT_TYPE] = ( char * )malloc( size );
    snprintf( meta[META_CONTENT_TYPE], size, "CONTENT_TYPE=%s", client->header[HDR_CONTENT_TYPE] );
        
    size = 26;
    meta[META_GATEWAY_INTERFACE] = ( char * )malloc( size );
    snprintf( meta[META_GATEWAY_INTERFACE], size, "GATEWAY_INTERFACE=CGI/1.1" );
        
    size = pos1 - pos0;
    if( size > ( META_SIZE - 1 ) )
    {
        LOG_ERROR( "Path Info too long\n" );
        return REQUEST_ERROR;
    }
    size += 11;
    meta[META_PATH_INFO] = ( char * )malloc( size );
    snprintf( meta[META_PATH_INFO], size, "PATH_INFO=" );
    strncpy( meta[META_PATH_INFO] + 10, client->uri + pos0, pos1 - pos0 );
    meta[META_PATH_INFO][size - 1] = 0;
        
    size = strlen( client->uri ) - pos1 - 1;
    if( size > ( META_SIZE - 1 ) )
    {
        LOG_ERROR( "Query string too long\n" );
        return REQUEST_ERROR;
    }
    size += 14;
    meta[META_QUERY_STRING] = ( char * )malloc( size );
    snprintf( meta[META_QUERY_STRING], size, "QUERY_STRING=" );
    if( size > 0 )
        snprintf( meta[META_QUERY_STRING] + 13, size - 13, "%s", client->uri + pos1 + 1 );
        
    size = 13 + strlen( inet_ntoa( client->sock.addr.sin_addr ) );
    meta[META_REMOTE_ADDR] = ( char * )malloc( size );
    snprintf( meta[META_REMOTE_ADDR], size, "REMOTE_ADDR=%s", inet_ntoa( client->sock.addr.sin_addr ) );
        
    meta[META_REMOTE_HOST] = ( char * )malloc( size );
    snprintf( meta[META_REMOTE_HOST], size, "REMOTE_HOST=%s", inet_ntoa( client->sock.addr.sin_addr ) );
        
    size = 20;
    meta[META_REQUEST_METHOD] = ( char * )malloc( size );
    if( client->meth == METHOD_HEAD )
        snprintf( meta[META_REQUEST_METHOD], size, "REQUEST_METHOD=HEAD" );
    else if( client->meth == METHOD_GET )
        snprintf( meta[META_REQUEST_METHOD], size, "REQUEST_METHOD=GET" );
    else if( client->meth == METHOD_POST )
        snprintf( meta[META_REQUEST_METHOD], size, "REQUEST_METHOD=POST" );
        
    size = pos1;
    if( size > ( META_SIZE - 1 ) )
    {
        LOG_ERROR( "Request uri too long\n" );
        return REQUEST_ERROR;
    }
    size += 13;
    meta[META_REQUEST_URI] = ( char * )malloc( size );
    snprintf( meta[META_REQUEST_URI], size, "REQUEST_URI=" );
    strncpy( meta[META_REQUEST_URI] + 12, client->uri, pos1 );
    meta[META_REQUEST_URI][size - 1] = 0;
        
    size = 14;
    meta[META_SCRIPT_NAME] = ( char * )malloc( size );
    snprintf( meta[META_SCRIPT_NAME], size, "SCRIPT_NAME=." );
        
    size = 0;
    if( client->header[HDR_HOST] != NULL )
        size = strlen( client->header[HDR_HOST] );
    if( size > ( META_SIZE - 1 ) )
    {
        LOG_ERROR( "HOST too long\n" );
        return REQUEST_ERROR;
    }
    size += 11;
    meta[META_HOST_NAME] = ( char * )malloc( size );
    meta[META_HTTP_HOST] = ( char * )malloc( size );
    snprintf( meta[META_HOST_NAME], size, "HOST_NAME=%s", client->header[HDR_HOST] );
    snprintf( meta[META_HTTP_HOST], size, "HTTP_HOST=%s", client->header[HDR_HOST] );
        
    size = 20;
    meta[META_SERVER_PORT] = ( char * )malloc( size );
    if( client->sock.type == SOCK_HTTP )
        snprintf( meta[META_SERVER_PORT], size, "SERVER_PORT=%d", http_port );
    else if( client->sock.type == SOCK_HTTPS )
        snprintf( meta[META_SERVER_PORT], size, "SERVER_PORT=%d", https_port );
        
    size = 25;
    meta[META_SERVER_PROTOCOL] = ( char * )malloc( size);
    snprintf( meta[META_SERVER_PROTOCOL], size, "SERVER_PROTOCOL=HTTP/1.1" );
        
    size = 25;
    meta[META_SERVER_SOFTWARE] = ( char * )malloc( size );
    snprintf( meta[META_SERVER_SOFTWARE], size, "SERVER_SOFTWARE=Liso/1.0" );
        
    size = 0;
    if( client->header[HDR_ACCEPT] != NULL )
        size = strlen( client->header[HDR_ACCEPT] );
    if( size > ( META_SIZE - 1 ) )
    {
        LOG_ERROR( "Accept too long\n" );
        return REQUEST_ERROR;
    }
    size += 13;
    meta[META_HTTP_ACCEPT] = ( char * )malloc( size );
    snprintf( meta[META_HTTP_ACCEPT], size, "HTTP_ACCEPT=%s", client->header[HDR_ACCEPT] );
        
    size = 0;
    if( client->header[HDR_REFERER] != NULL )
        size = strlen( client->header[HDR_REFERER] );
    if( size > ( META_SIZE - 1 ) )
    {
        LOG_ERROR( "Referer too long\n" );
        return REQUEST_ERROR;
    }
    size += 14;
    meta[META_HTTP_REFERER] = ( char * )malloc( size );
    snprintf( meta[META_HTTP_REFERER], size, "HTTP_REFERER=%s", client->header[HDR_REFERER] );
        
    size = 0;
    if( client->header[HDR_ACCEPT_ENCODING] != NULL )
        size = strlen( client->header[HDR_ACCEPT_ENCODING] );
    if( size > ( META_SIZE - 1 ) )
    {
        LOG_ERROR( "Accept-Encoding too long\n" );
        return REQUEST_ERROR;
    }
    size += 22;
    meta[META_HTTP_ACCEPT_ENCODING] = ( char * )malloc( size );
    snprintf( meta[META_HTTP_ACCEPT_ENCODING], size, "HTTP_ACCEPT_ENCODING=%s", client->header[HDR_ACCEPT_ENCODING] );
        
    size = 0;
    if( client->header[HDR_ACCEPT_LANGUAGE] != NULL )
        size = strlen( client->header[HDR_ACCEPT_LANGUAGE] );
    if( size > ( META_SIZE - 1 ) )
    {
        LOG_ERROR( "Accept-Language too long\n" );
        return REQUEST_ERROR;
    }
    size += 22;
    meta[META_HTTP_ACCEPT_LANGUAGE] = ( char * )malloc( size );
    snprintf( meta[META_HTTP_ACCEPT_LANGUAGE], size, "HTTP_ACCEPT_LANGUAGE=%s", client->header[HDR_ACCEPT_LANGUAGE] );
        
    size = 0;
    if( client->header[HDR_ACCEPT_CHARSET] != NULL )
        size = strlen( client->header[HDR_ACCEPT_CHARSET] );
    if( size > ( META_SIZE - 1 ) )
    {
        LOG_ERROR( "Accept-Charset too long\n" );
        return REQUEST_ERROR;
    }
    size += 21;
    meta[META_HTTP_ACCEPT_CHARSET] = ( char * )malloc( size );
    snprintf( meta[META_HTTP_ACCEPT_CHARSET], size, "ACCEPT_CHARSET=%s", client->header[HDR_ACCEPT_CHARSET] );
        
    size = 0;
    if( client->header[HDR_COOKIE] != NULL )
        size = strlen( client->header[HDR_COOKIE] );
    if( size > ( META_SIZE - 1 ) )
    {
        LOG_ERROR( "Cookie too long\n" );
        return REQUEST_ERROR;
    }
    size += 13;
    meta[META_HTTP_COOKIE] = ( char * )malloc( size );
    snprintf( meta[META_HTTP_COOKIE], size, "HTTP_COOKIE=%s", client->header[HDR_COOKIE] );
        
    size = 0;
    if( client->header[HDR_USER_AGENT] != NULL )
        size = strlen( client->header[HDR_USER_AGENT] );
    if( size > ( META_SIZE - 1 ) )
    {
        LOG_ERROR( "User-Agent too long\n" );
        return REQUEST_ERROR;
    }
    size += 17;
    meta[META_HTTP_USER_AGENT] = ( char * )malloc( size );
    snprintf( meta[META_HTTP_USER_AGENT], size, "HTTP_USER_AGENT=%s", client->header[HDR_USER_AGENT] );
        
    size = 0;
    if( client->header[HDR_CONNECTION] != NULL )
        size = strlen( client->header[HDR_CONNECTION] );
    if( size > ( META_SIZE - 1 ) )
    {
        LOG_ERROR( "Connection too long\n" );
        return REQUEST_ERROR;
    }
    size += 17;
    meta[META_HTTP_CONNECTION] = ( char * )malloc( size );
    snprintf( meta[META_HTTP_CONNECTION], size, "HTTP_CONNECTION=%s", client->header[HDR_CONNECTION] );
        
    meta[META_NUM] = NULL;

    return 0;
}

// main cgi function
int cgi( client_info *client )
{
    pid_t pid;
    int stdin_pipe[2];
    int stdout_pipe[2];
    char *ARGV[] = {
        cgi_script,
        NULL,
    };
    char *ENVP[META_NUM + 1] = { 0 };

    // set envp
    if( set_meta_variables( client, ENVP ) < 0 )
        return REQUEST_ERROR;

    // pipe stdin and stdout
    if( pipe( stdin_pipe ) < 0 )
    {
        LOG_ERROR( "Error piping for stdin\n" );
        return SERVER_ERROR;
    }
    if( pipe( stdout_pipe ) < 0 )
    {
        LOG_ERROR( "Error piping for stdout\n" );
        return SERVER_ERROR;
    }

    // fork child
    pid = fork();
    if( pid < 0 )
    {
        LOG_ERROR( "Error forking\n" );
        return SERVER_ERROR;
    }

    // child process, setup environment, execve
    if( pid == 0 )
    {
        // pipe stdin and stdout
        _close( stdout_pipe[0] );
        _close( stdin_pipe[1] );
        dup2( stdout_pipe[1], fileno( stdout ) );
        dup2( stdin_pipe[0], fileno( stdin ) );

        // execve
        if( execve( cgi_script, ARGV, ENVP ) )
        {
            LOG_ERROR( "Error execve: %s\n", strerror( errno ) );
            return SERVER_ERROR;
        }
    }

    // parent process
    if( pid > 0 )
    {
        // pipe 
        _close( stdout_pipe[1] );
        _close( stdin_pipe[0] );

        // send msg body to cgi script
        if( strlen( client->msg ) > 0 && write( stdin_pipe[1], client->msg, strlen( client->msg ) ) < 0 )
            return SERVER_ERROR;
        _close( stdin_pipe[1] );

        client->piped_fd = stdout_pipe[0];

        return 0;
    }

    LOG_ERROR( "CGI Error\n" );
    return SERVER_ERROR;
}
