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

typedef struct
{
    struct sockaddr_in addr;
    int sockfd; 
} client_info;

#endif
