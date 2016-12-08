/******************************************************************************
* util.h                                                                      *
*                                                                             *
* Description: This file contains the C declaration code for the utils.       *
*                                                                             *
* Author: Shiyu Zhang <1181856726@qq.com>                                     *
*                                                                             *
*******************************************************************************/

#ifndef _UTIL_H_
#define _UTIL_H_

#include "socket.h"
#include "ssl.h"

#include <string.h>
#include <time.h>
#include <netinet/in.h>

#define FALSE 0
#define TRUE  1

typedef unsigned char bool;

#define SP ' '
#define CR '\r'
#define LF '\n'

#define MIN( a, b ) ( a <= b ? a : b )
#define MAX( a, b ) ( a >= b ? a : b )

#define CLR_BUF( buf, size ) ( memset( buf, 0, size )  )

// generic list definition
#define define_list( type ) \
    typedef struct list_##type \
    { \
        type data; \
        struct list_##type *next; \
    } list_##type;

#define list( type ) \
    list_##type

// file type
typedef enum
{
    FT_BINARY,  // 0
    FT_ASCII,   // 1
    FT_UNKNOWN, // 2
} file_type;

// socket type
typedef enum
{
    SOCK_NONE,    // 0
    SOCK_HTTP,    // 1
    SOCK_HTTPS,   // 2
    SOCK_UNKNOWN, // 3 
} sock_type;

typedef struct
{
    sock_type type;
    struct sockaddr_in addr;
    int fd;
    SSL *context;
} sock_info;

// remove string's head and tail space
void strip( char *str, size_t len );

// convert upper alpha to lower in string
void to_lower( char *str, size_t len );

// convert lower alpha to upper in string
void to_upper( char *str, size_t len );

// rfc 2616 date format
void date_format( char *date, size_t len, time_t *t );

// check if string is uint
bool is_uint( char *str, size_t len );

// generic socket recv function
int generic_recv( const sock_info *sock, void *buf, size_t len, int flags );

// generic socket send function
int generic_send( const sock_info *sock, const void *buf, size_t len, int flags );

#endif
