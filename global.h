/******************************************************************************
* global.h                                                                    *
*                                                                             *
* Description: This file contains the C declaration code for the              * 
*              global definition.                                             *
*                                                                             *
* Author: Shiyu Zhang <1181856726@qq.com>                                     *
*                                                                             *
*******************************************************************************/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <netinet/in.h>

#define BUF_SIZE 4096
#define FALSE 0
#define TRUE  1

typedef unsigned char bool;

typedef enum
{
    METHOD_NONE, // 0
    METHOD_HEAD, // 1
    METHOD_GET,  // 2
    METHOD_POST, // 3
} method;

typedef enum
{
    CONN_NONE,       // 0
    CONN_KEEP_ALIVE, // 1
    CONN_CLOSE,      // 2
} connection;

typedef struct
{
    // socket
    struct sockaddr_in addr;
    int sockfd; 

    // request
    method meth;
    char uri[256];
    char version[10];

    // general header
    connection conn; 

    // entity header
    char contype[128];
    int conlen;

    // message
    char buf[BUF_SIZE]; 
    ssize_t len;
    bool ready;
    bool done;
} client_info;

#endif
