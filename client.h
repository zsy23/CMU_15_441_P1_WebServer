/******************************************************************************
* client.h                                                                    *
*                                                                             *
* Description: This file contains the C declaration code for the client info. * 
*                                                                             *
* Author: Shiyu Zhang <1181856726@qq.com>                                     *
*                                                                             *
*******************************************************************************/

#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "util.h"

#define BUF_SIZE 4096
#define URI_SIZE 256
#define VERSION_SIZE 16
#define TOKEN_SIZE 4096
#define MSG_SIZE 4096
#define OUTPUT_SIZE 12288
#define CONTYPE_SIZE 128
#define DATE_SIZE 64
#define HDR_SIZE 11

// state machine status used for parse
typedef enum
{
    STATE_START,           // 0
    STATE_METHOD,          // 1
    STATE_SEP_MET_URI,     // 2
    STATE_URI,             // 3
    STATE_SEP_URI_VER,     // 4
    STATE_VERSION,         // 5
    STATE_FIR_CR,          // 6
    STATE_FIR_LF,          // 7
    STATE_SEC_CR,          // 8
    STATE_SEC_LF,          // 9
    STATE_HDR_STA,         // 10
    STATE_HDR_FIE,         // 11
    STATE_HDR_SEP_FIE_COL, // 12
    STATE_HDR_COL,         // 13
    STATE_HDR_SEP_COL_VAL, // 14
    STATE_HDR_VAL,         // 15
    STATE_MSG,             // 16
    STATE_MET_NOT_IMP,     // 17
    STATE_VER_NOT_SUP,     // 18
    STATE_LEN_REQUIRED,    // 19
    STATE_BAD,             // 20
    STATE_END,             // 21
} sm_state;

// request method
typedef enum
{
    METHOD_NONE,    // 0
    METHOD_HEAD,    // 1
    METHOD_GET,     // 2
    METHOD_POST,    // 3
    METHOD_UNKNOWN, // 4
} method;

// request header
typedef enum
{
    HDR_CONTENT_LENGTH,  //  0
    HDR_CONTENT_TYPE,    //  1
    HDR_ACCEPT,          //  2
    HDR_REFERER,         //  3
    HDR_ACCEPT_ENCODING, //  4
    HDR_ACCEPT_LANGUAGE, //  5
    HDR_ACCEPT_CHARSET,  //  6
    HDR_COOKIE,          //  7
    HDR_USER_AGENT,      //  8
    HDR_CONNECTION,      //  9
    HDR_HOST,            // 10
} header;

// data structure maintained for client
typedef struct
{
    // socket
    sock_info sock;

    // buffer
    char buf[BUF_SIZE]; 
    ssize_t len;
    bool ready;

    // timeout
    time_t timeout;    

    // request
    method meth;
    char uri[URI_SIZE];
    char version[VERSION_SIZE];

    // header
    char *header[HDR_SIZE];

    // message body
    int left;
    char msg[MSG_SIZE];
   
    // parse used
    sm_state state;
    int hdr_len;
    char token[TOKEN_SIZE];
    bool done;

    // cgi used
    int piped_fd;
    int output_len;
    char output[OUTPUT_SIZE];
    bool output_too_long;
    bool cgi_done;
} client_info;

// response message body
typedef struct
{
    // meta data
    int conlen;
    char contype[CONTYPE_SIZE];
    char last_modi[DATE_SIZE];

    // msg body
    int len;
    char *msg;
} msg_info;

#endif
