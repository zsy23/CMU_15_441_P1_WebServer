/******************************************************************************
* cgi.h                                                                       *
*                                                                             *
* Description: This file contains the C declaration code for the CGI.         *
*                                                                             *
* Author: Shiyu Zhang <1181856726@qq.com>                                     *
*                                                                             *
*******************************************************************************/

#ifndef _CGI_H_
#define _CGI_H_

#include "client.h"

#define CGI_SIZE 256
#define META_SIZE 256
#define META_NUM 23
#define SERVER_ERROR -1
#define REQUEST_ERROR -2

typedef enum
{
    META_CONTENT_LENGTH,       //  0
    META_CONTENT_TYPE,         //  1
    META_GATEWAY_INTERFACE,    //  2
    META_PATH_INFO,            //  3
    META_QUERY_STRING,         //  4
    META_REMOTE_ADDR,          //  5
    META_REMOTE_HOST,          //  6
    META_REQUEST_METHOD,       //  7
    META_REQUEST_URI,          //  8
    META_SCRIPT_NAME,          //  9
    META_HOST_NAME,            // 10
    META_SERVER_PORT,          // 11
    META_SERVER_PROTOCOL,      // 12
    META_SERVER_SOFTWARE,      // 13
    META_HTTP_ACCEPT,          // 14
    META_HTTP_REFERER,         // 15
    META_HTTP_ACCEPT_ENCODING, // 16
    META_HTTP_ACCEPT_LANGUAGE, // 17
    META_HTTP_ACCEPT_CHARSET,  // 18
    META_HTTP_HOST,            // 19
    META_HTTP_COOKIE,          // 20
    META_HTTP_USER_AGENT,      // 21
    META_HTTP_CONNECTION,      // 22
} meta_variables;

// set cgi path
int set_cgi( const char *cgi_path, int http, int https );

// set cgi's meta-variables
int set_meta_variables( client_info *client, char **meta );

// main cgi function
int cgi( client_info *client );

#endif
