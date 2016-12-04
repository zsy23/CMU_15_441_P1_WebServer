/******************************************************************************
* core.h                                                                      *
*                                                                             *
* Description: This file contains the C declaration code for the select()     * 
*              based core networking.                                         *
*                                                                             *
* Author: Shiyu Zhang <1181856726@qq.com>                                     *
*                                                                             *
*******************************************************************************/

#ifndef _CORE_H_
#define _CORE_H_

#include "client.h"
#include "util.h"

#define CLIENT_MAX_NUM ( FD_SETSIZE - 500 )
#define TIMEOUT_SEC 230

typedef struct
{
    int sockfd;
    struct sockaddr_in addr;
} sock_info;

define_list( sock_info )

// blocking listening with timeout and return client info
void listening(int ser_sock, client_info **cients, int *maxi, int *maxfd, fd_set *allset, struct timeval *timeout, list( sock_info ) *unacc_head);

#endif
