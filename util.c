/******************************************************************************
* util.c                                                                      *
*                                                                             *
* Description: This file contains the C source code for the utils.            *
*                                                                             *
* Author: Shiyu Zhang <1181856726@qq.com>                                     *
*                                                                             *
*******************************************************************************/

#include "util.h"
#include "log.h"

// remove string's head and tail space
void strip( char *str, size_t len )
{
    int i, start, end;
    
    for( start = 0  ; start < len && str[start]   == ' '; ++start ); 
    for( end   = len; end   > 0   && str[end - 1] == ' '; --end   );
    if( end > start )
    {
        for( i = 0; i < end - start; ++i) str[i] = str[start + i];
        str[end - start] = 0;
    }
    else
        str[0] = 0;
}

// convert upper alpha to lower in string
void to_lower( char *str, size_t len )
{
    int i;

    for( i = 0; i < len; ++i )
        if( str[i] >= 'A' && str[i] <= 'Z' ) str[i] = 'a' + str[i] - 'A';
}

// convert lower alpha to upper in string
void to_upper( char *str, size_t len )
{
    int i;

    for( i = 0; i < len; ++i )
        if( str[i] >= 'a' && str[i] <= 'z' ) str[i] = 'A' + str[i] - 'a';
}

// rfc 2616 date format
void date_format( char *date, size_t len, time_t *t )
{ 
    strftime( date, len, "%a, %e %b %Y %T GMT", gmtime( t ) );
}

// check if string is uint
bool is_uint( char *str, size_t len )
{
    int i;

    for( i = 0; i < len && str[i] >= '0' && str[i] <= '9'; ++i );

    if( i >= len )
        return TRUE;
    else
        return FALSE;
}

// generic socket recv function
int generic_recv( const sock_info *sock, void *buf, size_t len, int flags )
{
    if( sock->type == SOCK_HTTP )
        return _recv( sock->fd, buf, len, flags );
    else if( sock->type == SOCK_HTTPS )
        return ssl_read( sock->context, buf, len );
    else
        LOG_ERROR( "Unknown recv socket type.\n" );

    return -1;
}

// generic socket send function
int generic_send( const sock_info *sock, const void *buf, size_t len, int flags )
{
    if( sock->type == SOCK_HTTP )
        return _send( sock->fd, buf, len, flags );
    else if( sock->type == SOCK_HTTPS )
        return ssl_write( sock->context, buf, len );
    else
        LOG_ERROR( "Unknown send socket type.\n" );

    return -1;
}
