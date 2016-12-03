/******************************************************************************
* http.c                                                                      *
*                                                                             *
* Description: This file contains the C source code for processing HTTP/1.1.  *
*              TODO: Decode URI                                               *
*              TODO: POST                                                     *
*                                                                             *
* Author: Shiyu Zhang <1181856726@qq.com>                                     *
*                                                                             *
*******************************************************************************/

#include "http.h"
#include "util.h"
#include "log.h"
#include "socket.h"

#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>

// www folder
char www[WWW_SIZE];

// html files for specified status code
const char *status_htmls[] = {
    "/status_200.html", // 0
    "/status_400.html", // 1
    "/status_404.html", // 2
    "/status_411.html", // 3
    "/status_500.html", // 4
    "/status_501.html", // 5
    "/status_503.html", // 6
    "/status_505.html", // 7
};

// status code descriptions
const char *status_reason_strings[] = {
	"200 OK",                        // 0
	"400 Bad Request",               // 1
	"404 Not Found",                 // 2
	"411 Length Required",           // 3
	"500 Internal Server Error",     // 4
    "501 Not Implemented",           // 5
    "503 Service Unavailable",       // 6
    "505 HTTP Version Not Supported", //7
};

// Content-Type supported descriptions
const char *contype_strings[] = {
    "text/plain",
    "application/x-www-form-urlencoded",
    NULL,
};

// set www folder
void set_www( const char *folder )
{
    snprintf( www, WWW_SIZE, "%s", folder );
}

// check Content-Type
bool check_contype( const char *str, size_t len )
{
    int i;

    for( i = 0; contype_strings[i] != NULL; ++i )
        if( strncmp( str, contype_strings[i], MAX( len, strlen( contype_strings[i] ) ) ) == 0 )
            return TRUE;

    return FALSE;
}

// parse HTTP/1.1 request
void parse( client_info *client )
{
    int i;
    char c, field[TOKEN_SIZE], value[TOKEN_SIZE];
    CLR_BUF( field        , TOKEN_SIZE );
    CLR_BUF( value        , TOKEN_SIZE );        

    i = 0;
    // state machine
    while( i < client->len )
    {
        c = client->buf[i];

        switch( client->state )
        {
            case STATE_START:
                switch( c )
                {
                    case SP:
                    case CR:
                    case LF:
                        break;
                    default:
                        strncat( client->token, &c, 1 );
                        client->state = STATE_METHOD;
                        break;
                }
                break;
            case STATE_METHOD:
                switch( c )
                {
                    case SP:
                        to_upper( client->token, strlen( client->token ) );
                        client->state = STATE_SEP_MET_URI;
                        if( strncmp( client->token, "HEAD", MAX( strlen( client->token ), 4 ) ) == 0 )
                            client->meth = METHOD_HEAD;
                        else if( strncmp( client->token, "GET", MAX( strlen( client->token ), 3 ) ) == 0 )
                            client->meth = METHOD_GET;
                        else if( strncmp( client->token, "POST", MAX( strlen( client->token ), 4 ) ) == 0 )
                            client->meth = METHOD_POST;
                        else
                        {   
                            client->meth = METHOD_UNKNOWN;
                            client->state = STATE_MET_NOT_IMP;
                        }
                        CLR_BUF( client->token, TOKEN_SIZE );
                        break;
                    case CR:
                    case LF:
                        CLR_BUF( client->token, TOKEN_SIZE );
                        client->state = STATE_BAD; 
                        break;
                    default:
                        if( strlen( client->token ) >= ( TOKEN_SIZE - 1 ) )
                        {
                            CLR_BUF( client->token, TOKEN_SIZE );
                            client->state = STATE_BAD;
                        }
                        else
                            strncat( client->token, &c, 1 );
                        break;
                }
                break;
            case STATE_SEP_MET_URI:
                switch( c )
                {
                    case SP:
                        break;
                    case CR:
                    case LF:
                        client->state = STATE_BAD;
                        break;
                    default:
                        strncat( client->token, &c, 1 );
                        client->state = STATE_URI;
                        break;
                }
                break;
            case STATE_URI:
                switch( c )
                {
                    case SP:
                        snprintf( client->uri, URI_SIZE, "%s", client->token );
                        CLR_BUF( client->token, TOKEN_SIZE );
                        client->state = STATE_SEP_URI_VER;
                        break;
                    case CR:
                    case LF:
                        CLR_BUF( client->token, TOKEN_SIZE );
                        client->state = STATE_BAD;
                        break;
                    default:
                        if( strlen( client->token ) >= ( TOKEN_SIZE - 1 ) )
                        {
                            CLR_BUF( client->token, TOKEN_SIZE );
                            client->state = STATE_BAD;
                        }
                        else
                            strncat( client->token, &c, 1 );
                        break;
                }
                break;
            case STATE_SEP_URI_VER:
                switch( c )
                {
                    case SP:
                        break;
                    case CR:
                    case LF:
                        client->state = STATE_BAD;
                        break;
                    default:
                        strncat( client->token, &c, 1 );
                        client->state = STATE_VERSION;
                        break;                     
                }
                break;
            case STATE_VERSION:
                switch( c )
                {
                    case CR:
                        strip( client->token, strlen( client->token ) );
                        to_upper( client->token, strlen( client->token ) );
                        if( strncmp( client->token, "HTTP/1.1", MAX( strlen( client->token ), 8 ) ) == 0 )
                        {
                            snprintf( client->version, VERSION_SIZE, "HTTP/1.1" );
                            client->state = STATE_FIR_CR;
                        }
                        else
                            client->state = STATE_VER_NOT_SUP;
                        CLR_BUF( client->token, TOKEN_SIZE );
                        break;
                    case LF:
                        CLR_BUF( client->token, TOKEN_SIZE );
                        client->state = STATE_BAD;
                        break;
                    default:
                        if( strlen( client->token ) >= ( TOKEN_SIZE - 1 ) )
                        {
                            CLR_BUF( client->token, TOKEN_SIZE );
                            client->state = STATE_BAD;
                        }
                        else
                            strncat( client->token, &c, 1 );
                        break;
                }
                break;
            case STATE_FIR_CR:
                // +1 because first LF
                if( ++client->hdr_len > ( HDR_MAX_SIZE + 1 ) )
                    client->state = STATE_BAD;

                switch( c )
                {
                    case LF:
                        client->state = STATE_FIR_LF;
                        break;
                    default:
                        client->state = STATE_BAD;
                        break;
                }
                break;
            case STATE_FIR_LF:
                // check header length
                if( ++client->hdr_len > ( HDR_MAX_SIZE + 1 ) )
                    client->state = STATE_BAD;

                switch( c )
                {
                    case SP:
                        client->state = STATE_HDR_STA;
                        break;
                    case CR:
                        if( client->meth == METHOD_POST && client->conlen < 0 ) 
                            client->state = STATE_LEN_REQUIRED;
                        else
                            client->state = STATE_SEC_CR;
                        break;
                    case LF:
                    case ':':
                        client->state = STATE_BAD;
                        break;
                    default:
                        strncat( client->token, &c, 1 );
                        client->state = STATE_HDR_FIE;
                        break; 
                }
                break;
            case STATE_SEC_CR:
                switch( c )
                {
                    case LF:
                        client->state = STATE_SEC_LF;
                        if( client->conlen <= 0 )
                            client->state = STATE_END;
                        else if( client->conlen > MSG_MAX_SIZE )
                            client->state = STATE_BAD;
                        else
                        {
                            client->left = client->conlen;
                            client->state = STATE_MSG;
                        }
                        break;
                    default:
                        client->state = STATE_BAD;
                        break;
                }
                break;
            case STATE_HDR_STA:
                if( ++client->hdr_len > ( HDR_MAX_SIZE + 1 ) )
                    client->state = STATE_BAD;

                switch( c )
                {
                    case SP:
                        break;
                    case ':':
                    case CR:
                    case LF:
                        client->state = STATE_BAD;
                        break;
                    default:
                        strncat( client->token, &c, 1 );
                        client->state = STATE_HDR_FIE;
                        break;
                }
                break;
            case STATE_HDR_FIE:
                if( ++client->hdr_len > ( HDR_MAX_SIZE + 1 ) )
                    client->state = STATE_BAD;

                switch( c )
                {
                    case SP:
                        to_lower( client->token, strlen( client->token ) );
                        snprintf( field, TOKEN_SIZE, "%s", client->token );
                        CLR_BUF( client->token, TOKEN_SIZE );
                        client->state = STATE_HDR_SEP_FIE_COL;
                        break;
                    case CR:
                    case LF:
                        CLR_BUF( client->token, TOKEN_SIZE );
                        client->state = STATE_BAD;
                        break;
                    case ':':
                        to_lower( client->token, strlen( client->token ) );
                        snprintf( field, TOKEN_SIZE, "%s", client->token );
                        CLR_BUF( client->token, TOKEN_SIZE );
                        client->state = STATE_HDR_COL;
                        break;
                    default:
                        if( strlen( client->token ) >= ( TOKEN_SIZE - 1 ) )
                        {
                            CLR_BUF( client->token, TOKEN_SIZE );
                            client->state = STATE_BAD;
                        }
                        else
                            strncat( client->token, &c, 1 );
                        break;
                }
                break;
            case STATE_HDR_SEP_FIE_COL:
                if( ++client->hdr_len > ( HDR_MAX_SIZE + 1 ) )
                    client->state = STATE_BAD;

                switch( c )
                {
                    case SP:
                        break;
                    case ':':
                        client->state = STATE_HDR_COL;
                        break;
                    default:
                        client->state = STATE_BAD;
                        break;
                }
                break;
            case STATE_HDR_COL:
                if( ++client->hdr_len > ( HDR_MAX_SIZE + 1 ) )
                    client->state = STATE_BAD;

                switch( c )
                {
                    case SP:
                        client->state = STATE_HDR_SEP_COL_VAL;
                        break;
                    case CR:
                    case LF:
                        client->state = STATE_BAD;
                        break;
                    default:
                        strncat( client->token, &c, 1 );
                        client->state = STATE_HDR_VAL;
                        break;
                }
                break;
            case STATE_HDR_SEP_COL_VAL:
                if( ++client->hdr_len > ( HDR_MAX_SIZE + 1 ) )
                    client->state = STATE_BAD;

                switch( c )
                {
                    case SP:
                        break;
                    case CR:
                    case LF:
                        client->state = STATE_BAD;
                        break;
                    default:
                        strncat( client->token, &c, 1 );
                        client->state = STATE_HDR_VAL;
                        break;
                }
                break;
            case STATE_HDR_VAL:
                if( ++client->hdr_len > ( HDR_MAX_SIZE + 1 ) )
                    client->state = STATE_BAD;

                switch( c )
                {
                    case CR:
                        client->state = STATE_FIR_CR;

                        strip( client->token, strlen( client->token ) );
                        to_lower( client->token, strlen( client->token ) );
                        snprintf( value, TOKEN_SIZE, "%s", client->token );

                        if( strncmp( field, "connection", MAX( strlen( field ), 10 ) ) == 0 )
                        {
                            if( strncmp( value, "keep-alive", MAX( strlen( value ), 10 ) ) == 0 )
                                client->conn = CONN_KEEP_ALIVE;
                            else if( strncmp( value, "close", MAX( strlen( value ), 5 ) ) == 0 )
                                client->conn = CONN_CLOSE;
                            else
                                client->state = STATE_BAD;
                        }
                        else if( strncmp( field, "content-length", MAX( strlen( field ), 14 ) ) == 0 )
                        {
                            if( is_uint( value, strlen( value ) ) == TRUE )
                                client->conlen = atoi( value );
                            else
                                client->state = STATE_BAD;
                        }
                        else if( strncmp( field, "content-type", MAX( strlen( field ), 12 ) ) == 0 )
                        {
                            if( check_contype( value, strlen( value ) ) == TRUE ) 
                                snprintf( client->contype, CONTYPE_SIZE, "%s", value );
                            else
                                client->state = STATE_BAD;
                        }
                        memset( field, 0, TOKEN_SIZE );
                        memset( value, 0, TOKEN_SIZE );

                        CLR_BUF( client->token, TOKEN_SIZE );
                        break;
                    case LF:
                        CLR_BUF( client->token, TOKEN_SIZE );
                        client->state = STATE_BAD;
                        break;
                    default:
                        if( strlen( client->token ) >= ( TOKEN_SIZE - 1 ) )
                        {
                            CLR_BUF( client->token, TOKEN_SIZE );
                            client->state = STATE_BAD;
                        }
                        else
                            strncat( client->token, &c, 1 );
                        break;
                }
                break;
            case STATE_MSG:
                switch( c )
                {
                    case CR:
                        client->left = 0;
                        client->state = STATE_END;
                        break;
                    default:
                        strncat( client->msg, &c, 1 );
                        if( --client->left == 0 )
                            client->state = STATE_END;
                        break;
                }
                break;
           default:
                LOG_ERROR( "Unknown state\n" );
                break;
        }

        // if state below, parse end
        if( client->state == STATE_MET_NOT_IMP  || 
            client->state == STATE_VER_NOT_SUP  || 
            client->state == STATE_LEN_REQUIRED || 
            client->state == STATE_BAD          || 
            client->state == STATE_END             )
        {
            client->done = TRUE;
            break;
        }

        ++i;
    }
}

// process request and respond
void process( client_info *client )
{
    int status;
    time_t now;
    char date[DATE_SIZE];
    msg_info mi;
    char version[] = "HTTP/1.1";
    char server[] = "Liso/1.0";

    status = -1;
    CLR_BUF( date, DATE_SIZE );
    mi.conlen = -1;
    CLR_BUF( mi.contype, CONTYPE_SIZE );
    CLR_BUF( mi.last_modi, DATE_SIZE );
    mi.len = -1;

    // Date field
    time( &now );
    date_format( date, DATE_SIZE, &now );

    // according to state
    switch( client->state )
    {
        // Not implemented
        case STATE_MET_NOT_IMP:
            status = STATUS_501;
            do_get( status_htmls[STATUS_501], &mi );
            respond( client->addr, client->sockfd, version, status, CONN_KEEP_ALIVE, date, server, &mi );

            break;
        // HTTP Version Not Supported
        case STATE_VER_NOT_SUP:
            status = STATUS_505;
            do_get( status_htmls[STATUS_505], &mi );
            respond( client->addr, client->sockfd, version, status, CONN_KEEP_ALIVE, date, server, &mi );

            break;
        // Length Required
        case STATE_LEN_REQUIRED:
            status = STATUS_411;
            do_get( status_htmls[STATUS_411], &mi );
            respond( client->addr, client->sockfd, version, status, CONN_KEEP_ALIVE, date, server, &mi );

            break;
        // Bad Request
        case STATE_BAD:
            status = STATUS_400;
            do_get( status_htmls[STATUS_400], &mi );
            respond( client->addr, client->sockfd, version, status, CONN_KEEP_ALIVE, date, server, &mi );

            break;
        // Normal Request
        case STATE_END:
            switch( client->meth )
            {
                // HEAD Method
                case METHOD_HEAD:
                    status = do_head( client->uri, &mi );
                    if( status != STATUS_200 )
                        do_get( status_htmls[status], &mi );
                    respond( client->addr, client->sockfd, client->version, status, client->conn, date, server, &mi );

                    break;
                // GET Method
                case METHOD_GET:
                    status = do_get( client->uri, &mi );
                    if( status != STATUS_200 )
                        do_get( status_htmls[status], &mi );
                    respond( client->addr, client->sockfd, client->version, status, client->conn, date, server, &mi );

                    break;
                // POST Method
                case METHOD_POST:
                    status = do_post( client->uri, client->msg, &mi );
                    if( status != STATUS_200 )
                        do_get( status_htmls[status], &mi );
                    respond( client->addr, client->sockfd, client->version, status, client->conn, date, server, NULL );

                    break;
                default:
                    break;
            }

            break;
        default:
            LOG_ERROR( "Unknow parse result\n" );
            break;
    }

    if( mi.len > 0 )
        free( mi.msg );
}

// respond according to header and message
void respond( const struct sockaddr_in addr, const int sockfd, const char *version, const int status, const connection conn, const char *date, const char *server, const msg_info *mi )
{
    int size;
    char status_reason[STA_REA_MAX_SIZE], conn_s[CONN_MAX_SIZE], conlen_s[CONLEN_MAX_SIZE];
    char *response;

    size = -1;
    response = NULL;
    CLR_BUF( status_reason, STA_REA_MAX_SIZE );
    CLR_BUF( conn_s, CONN_MAX_SIZE );
    CLR_BUF( conlen_s, CONLEN_MAX_SIZE );

    // status code and reason phrase
    snprintf( status_reason, STA_REA_MAX_SIZE, "%s", status_reason_strings[status] );

    // Connection field
    if( conn == CONN_KEEP_ALIVE )
        snprintf( conn_s, CONN_MAX_SIZE, "keep-alive" );
    else if( conn == CONN_CLOSE )
        snprintf( conn_s, CONN_MAX_SIZE, "close" );
    else
        snprintf( conn_s, CONN_MAX_SIZE, "keep-alive" );

    size = strlen( version ) + strlen( status_reason ) + strlen( conn_s ) +strlen( date ) + strlen( server );

    // if mi is NULL
    if( mi == NULL )
    {
        // +64 because other text
        response = ( char * )malloc( size + 64 );

        snprintf( response, size + 64, "%s %s\r\nDate: %s\r\nConnection: %s\r\nServer: %s\r\n\r\n",   
                  version, status_reason, date, conn_s, server );
    
        size = strlen( response );
        if( _send( sockfd, response, size, 0 ) < size )
            LOG_ERROR( "Response not complete\n" );

        free( response );

        return;
    }

    // Content-Length field
    if( mi->conlen >= 0 )
    {
        snprintf( conlen_s, CONLEN_MAX_SIZE, "%d", mi->conlen );
        size += strlen( conlen_s ) + strlen( mi->contype ) + strlen( mi->last_modi );
    }

    if( mi->len > 0 ) size += mi->len;

    // build response's header
    response = ( char * )malloc( size + 128 );
    if( mi->conlen < 0 )
    {
        snprintf( response, size + 128, "%s %s\r\nDate: %s\r\nConnection: %s\r\nServer: %s\r\n\r\n",
                  version, status_reason, date, conn_s, server );
    }
    else if( mi->conlen >= 0 )
    {
        snprintf( response, size + 128, "%s %s\r\nDate: %s\r\nConnection: %s\r\nServer: %s\r\nContent-Type: %s\r\nContent-Length: %s\r\nLast-Modified: %s\r\n\r\n",
                  version, status_reason, date, conn_s, server, mi->contype, conlen_s, mi->last_modi );
    }

    size = strlen( response );
    LOG_INFO( "Response Header To %s:%u( %d ): %s\n", inet_ntoa( addr.sin_addr ), ntohs( addr.sin_port ), size, response );

    if( mi->len > 0 )
    {
        // build response's message body
        memcpy( response + size, mi->msg, mi->len );
        size += mi->len;
    
        LOG_INFO( "Message Body To %s:%u( %d ): %s\n", inet_ntoa( addr.sin_addr ), ntohs( addr.sin_port ), mi->len, mi->msg );
    }

    // send response to client
    if( _send( sockfd, response, size, 0 ) < size )
        LOG_ERROR( "Response damaged\n" );

    free( response );
}

// process HEAD method
int do_head( const char *uri, msg_info *mi )
{
    // get resource without message body
    return get_resource( uri, mi, FALSE );
}

// process GET method
int do_get( const char *uri, msg_info *mi )
{
    // get resource with message body
    return get_resource( uri, mi, TRUE );
}

// process POST method
int do_post( const char *uri, const char *msg, msg_info *mi )
{
    // TODO
    fprintf( stdout, "uri: %s\nmsg: %s\n", uri, msg );

    return STATUS_200;
}

// get resource according to URI
int get_resource( const char *uri, msg_info *mi, bool msg_needed )
{
    int pos;
    char resource_path[RESOURCE_SIZE], suffix[SUFFIX_MAX_SIZE];
    struct stat buf;
    file_type ft;

    pos = -1;
    ft = FT_UNKNOWN;
    CLR_BUF( resource_path, RESOURCE_SIZE );
    CLR_BUF( suffix, SUFFIX_MAX_SIZE );

    // build resource path and check
    if( strlen( www ) + strlen( uri ) > RESOURCE_SIZE - 1 )
        return STATUS_404;
    else
        snprintf( resource_path, RESOURCE_SIZE, "%s%s", www, uri );
    if( resource_path[strlen( resource_path ) - 1] == SEPARATE )
    {
        if( strlen( resource_path ) + strlen( INDEX_HTML ) > RESOURCE_SIZE - 1 )
            return STATUS_404;
        else
            strncat( resource_path, INDEX_HTML, strlen( INDEX_HTML ) );
    }

    // get resource suffix
    for( pos = strlen( resource_path ) - 1; pos >= 0 && resource_path[pos] != '.'; --pos );
    snprintf( suffix, SUFFIX_MAX_SIZE, "%s", resource_path + pos + 1 );
    to_lower( suffix, strlen( suffix ) );

    // get resource's content type
    if( strncmp( suffix, "html", MAX( strlen( suffix ), 4 )  ) == 0 )
    {
        ft = FT_ASCII;
        snprintf( mi->contype, CONTYPE_SIZE, "text/html" );
    }
    else if( strncmp( suffix, "css", MAX( strlen( suffix ), 3 ) ) == 0 )
    {
        ft = FT_ASCII;
        snprintf( mi->contype, CONTYPE_SIZE, "text/css" );
    }
    else if( strncmp( suffix, "png", MAX( strlen( suffix ), 3 ) ) == 0 )
    {
        ft = FT_BINARY;
        snprintf( mi->contype, CONTYPE_SIZE, "image/png" );
    }
    else if( strncmp( suffix, "jpeg", MAX( strlen( suffix ), 4 ) ) == 0 )
    {
        ft = FT_BINARY;
        snprintf( mi->contype, CONTYPE_SIZE, "image/jpeg" );
    }
    else if( strncmp( suffix, "gif", MAX( strlen( suffix ), 3 ) ) == 0 )
    {
        ft = FT_BINARY;
        snprintf( mi->contype, CONTYPE_SIZE, "image/gif" );
    }
    else
        return STATUS_404;

    // get resource stat info
    if( stat( resource_path, &buf ) < 0 )
    {
        if( errno == ENOENT || errno == ENAMETOOLONG || errno == EACCES )
            return STATUS_404;
        else
            return STATUS_500;
    }

    // get resource's last modified date
    date_format( mi->last_modi, DATE_SIZE, &buf.st_mtime );

    // get resource file pointer
    FILE *fp;
    fp = NULL;
    if( ft == FT_ASCII )
        fp = fopen( resource_path, "r" );
    else if( ft == FT_BINARY )
        fp = fopen( resource_path, "rb" );

    // get resource's byte length
    fseek( fp, 0, SEEK_END );
    mi->conlen = ftell( fp );

    if( msg_needed == TRUE )
    {
        // read resource content
        mi->len = mi->conlen;
        mi->msg = ( char * )malloc( mi->len );

        fseek( fp, 0, SEEK_SET );
        fread( mi->msg, sizeof( char ), mi->len, fp );
        mi->msg[mi->len] = 0;
    }

    fclose( fp );

    return STATUS_200;
}
