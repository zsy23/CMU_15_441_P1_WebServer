/******************************************************************************
* core.h                                                                      *
*                                                                             *
* Description: This file contains the C declaration code for the core         * 
*              networking.                                                    *
*                                                                             *
* Author: Shiyu Zhang <1181856726@qq.com>                                     *
*                                                                             *
*******************************************************************************/

#ifndef _CORE_H_
#define _CORE_H_

#include "global.h"

void listening(int ser_sock, client_info **cients, int *maxi, int *maxfd, fd_set *allset);

#endif
