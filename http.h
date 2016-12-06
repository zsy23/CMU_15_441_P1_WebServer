/******************************************************************************
* http.h                                                                      *
*                                                                             *
* Description: This file contains the C declaration code about HTTP/1.1       *
*                                                                             *
* Author: Shiyu Zhang <1181856726@qq.com>                                     *
*                                                                             *
*******************************************************************************/

#ifndef _HTTP_H_
#define _HTTP_H_

#include "client.h"
#include "util.h"

// path separate concerned with OS
#define SEPARATE '/'

// max www folder path length
#define WWW_SIZE 256

// main web page
#define INDEX_HTML "index.html"

// max header length
#define HDR_MAX_SIZE 8192

// max resource path length
#define RESOURCE_SIZE 256

// other field max length
#define STA_REA_MAX_SIZE 32
#define CONN_MAX_SIZE 16
#define CONLEN_MAX_SIZE 16
#define SUFFIX_MAX_SIZE 16

// file type
typedef enum
{
    FT_BINARY,  // 0
    FT_ASCII,   // 1
    FT_UNKNOWN, // 2
} file_type;

// status code
typedef enum
{
    STATUS_200, // 0
    STATUS_400, // 1
    STATUS_404, // 2
    STATUS_411, // 3
    STATUS_500, // 4
    STATUS_501, // 5
    STATUS_503, // 6
    STATUS_505, // 7
} status_code;

// set www folder
void set_www( const char *folder );

// check Content-Type
bool check_contype( const char *str, size_t len );

// parse request
void parse( client_info *client );

// process request
void process( client_info *client );

// respond request
void respond( const sock_info *sock, const char *version, const int status, const connection conn, const char *date, const char *server, const msg_info *mi );

// head method
int do_head( const char *uri, msg_info *mi );

// get method
int do_get( const char *uri, msg_info *mi );

// post method
int do_post( const char *uri, const char *msg, msg_info *mi );

// get resource defined by URI
int get_resource( const char *uri, msg_info *mi, bool msg_needed );

#endif
