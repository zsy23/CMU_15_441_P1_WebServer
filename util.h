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

#include <string.h>
#include <time.h>

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

#endif
