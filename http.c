/******************************************************************************
* http.c                                                                      *
*                                                                             *
* Description: This file contains the C source code for processing HTTP/1.1.  *
*                                                                             *
* Author: Shiyu Zhang <1181856726@qq.com>                                     *
*                                                                             *
*******************************************************************************/

#include "http.h"
#include "log.h"
#include "cgi.h"

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

// set www folder
int set_www( const char *folder )
{
    if( strlen( folder ) > ( WWW_SIZE - 1 ) )
    {
        LOG_ERROR( "WWW folder too long\n" );
        return -1;
    }

    snprintf( www, WWW_SIZE, "%s", folder );

    return 0;
}

// fill header
void fill_header( client_info *client, const char *field, const char *value )
{
    int fie_size, val_size;

    fie_size = strlen( field );
    val_size = strlen( value ) + 1;

    if( strncmp( field, "connection", MAX( fie_size, 10 ) ) == 0 )
    {
        client->header[HDR_CONNECTION] = ( char * )malloc( val_size );
        snprintf( client->header[HDR_CONNECTION], val_size, "%s", value );
    }
    else if( strncmp( field, "content-length", MAX( fie_size, 14 ) ) == 0 )
    {
        client->header[HDR_CONTENT_LENGTH] = ( char * )malloc( val_size );
        snprintf( client->header[HDR_CONTENT_LENGTH], val_size, "%s", value );
    }
    else if( strncmp( field, "content-type", MAX( fie_size, 12 ) ) == 0 )
    {
        client->header[HDR_CONTENT_TYPE] = ( char * )malloc( val_size );
        snprintf( client->header[HDR_CONTENT_TYPE], val_size, "%s", value );
    }
    else if( strncmp( field, "accept", MAX( fie_size, 6 ) ) == 0 )
    {
        client->header[HDR_ACCEPT] = ( char * )malloc( val_size );
        snprintf( client->header[HDR_ACCEPT], val_size, "%s", value );
    }
    else if( strncmp( field, "referer", MAX( fie_size, 7 ) ) == 0 )
    {
        client->header[HDR_REFERER] = ( char * )malloc( val_size );
        snprintf( client->header[HDR_REFERER], val_size, "%s", value );
    }
    else if( strncmp( field, "accept-encoding", MAX( fie_size, 15 ) ) == 0 )
    {
        client->header[HDR_ACCEPT_ENCODING] = ( char * )malloc( val_size );
        snprintf( client->header[HDR_ACCEPT_ENCODING], val_size, "%s", value );
    }
    else if( strncmp( field, "accept-language", MAX( fie_size, 15 ) ) == 0 )
    {
        client->header[HDR_ACCEPT_LANGUAGE] = ( char * )malloc( val_size );
        snprintf( client->header[HDR_ACCEPT_LANGUAGE], val_size, "%s", value );
    }
    else if( strncmp( field, "accept-charset", MAX( fie_size, 14 ) ) == 0 )
    {
        client->header[HDR_ACCEPT_CHARSET] = ( char * )malloc( val_size );
        snprintf( client->header[HDR_ACCEPT_CHARSET], val_size, "%s", value );
    }
    else if( strncmp( field, "cookie", MAX( fie_size, 6 ) ) == 0 )
    {
        client->header[HDR_COOKIE] = ( char * )malloc( val_size );
        snprintf( client->header[HDR_COOKIE], val_size, "%s", value );
    }
    else if( strncmp( field, "user-agent", MAX( fie_size, 10 ) ) == 0 )
    {
        client->header[HDR_USER_AGENT] = ( char * )malloc( val_size );
        snprintf( client->header[HDR_USER_AGENT], val_size, "%s", value );
    }
    else if( strncmp( field, "host", MAX( fie_size, 4 ) ) == 0 )
    {
        client->header[HDR_HOST] = ( char * )malloc( val_size );
        snprintf( client->header[HDR_HOST], val_size, "%s", value );
    }
}

// parse HTTP/1.1 request
void parse( client_info *client )
{
    int i;
    char c;
    char field[TOKEN_SIZE] = { 0 };
    char value[TOKEN_SIZE] = { 0 };

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
                        LOG_DEBUG( "Bad Request: method lf\n" );
                        break;
                    default:
                        if( strlen( client->token ) >= ( TOKEN_SIZE - 1 ) )
                        {
                            CLR_BUF( client->token, TOKEN_SIZE );
                            client->state = STATE_BAD;
                            LOG_DEBUG( "Bad Request: method token too long\n" );
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
                        LOG_DEBUG( "Bad Request: sep met uri lf\n" );
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
                        LOG_DEBUG( "Bad Request: uri lf\n" );
                        break;
                    default:
                        if( strlen( client->token ) >= ( TOKEN_SIZE - 1 ) )
                        {
                            CLR_BUF( client->token, TOKEN_SIZE );
                            client->state = STATE_BAD;
                            LOG_DEBUG( "Bad Request: uri token too long\n" );
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
                        LOG_DEBUG( "Bad Request: sep uri ver lf\n" );
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
                        LOG_DEBUG( "Bad Request: version lf\n" );
                        break;
                    default:
                        if( strlen( client->token ) >= ( TOKEN_SIZE - 1 ) )
                        {
                            CLR_BUF( client->token, TOKEN_SIZE );
                            client->state = STATE_BAD;
                            LOG_DEBUG( "Bad Request: version too long\n" );
                        }
                        else
                            strncat( client->token, &c, 1 );
                        break;
                }
                break;
            case STATE_FIR_CR:
                // +1 because first LF
                if( ++client->hdr_len > ( HDR_MAX_SIZE + 1 ) )
                {
                    client->state = STATE_BAD;
                    LOG_DEBUG( "Bad Request: fir cr header too long\n" );
                }

                switch( c )
                {
                    case LF:
                        client->state = STATE_FIR_LF;
                        break;
                    default:
                        client->state = STATE_BAD;
                        LOG_DEBUG( "Bad Request: fir cr other\n" );
                        break;
                }
                break;
            case STATE_FIR_LF:
                // check header length
                if( ++client->hdr_len > ( HDR_MAX_SIZE + 1 ) )
                {
                    client->state = STATE_BAD;
                    LOG_DEBUG( "Bad Request: fir lf header too long\n" );
                }

                switch( c )
                {
                    case SP:
                        client->state = STATE_HDR_STA;
                        break;
                    case CR:
                        if( client->meth == METHOD_POST && ( client->header[HDR_CONTENT_LENGTH] == NULL || is_uint( client->header[HDR_CONTENT_LENGTH], strlen( client->header[HDR_CONTENT_LENGTH] ) ) == FALSE ) ) 
                            client->state = STATE_LEN_REQUIRED;
                        else
                            client->state = STATE_SEC_CR;
                        break;
                    case LF:
                    case ':':
                        client->state = STATE_BAD;
                        LOG_DEBUG( "Bad Request: fir lf :\n" );
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
                        if( client->header[HDR_CONTENT_LENGTH] == NULL || is_uint( client->header[HDR_CONTENT_LENGTH], strlen( client->header[HDR_CONTENT_LENGTH] ) ) == FALSE )
                            client->state = STATE_END;
                        else if( atoi( client->header[HDR_CONTENT_LENGTH] ) > MSG_SIZE )
                        {
                            client->state = STATE_BAD;
                            LOG_DEBUG( "Bad Request: sec cr msg too long\n" );
                        }
                        else
                        {
                            client->left = atoi( client->header[HDR_CONTENT_LENGTH] );
                            client->state = STATE_MSG;
                        }
                        break;
                    default:
                        client->state = STATE_BAD;
                        LOG_DEBUG( "Bad Request: sec cr other\n" );
                        break;
                }
                break;
            case STATE_HDR_STA:
                if( ++client->hdr_len > ( HDR_MAX_SIZE + 1 ) )
                {
                    client->state = STATE_BAD;
                    LOG_DEBUG( "Bad Request: hdr sta header too long\n" );
                }

                switch( c )
                {
                    case SP:
                        break;
                    case ':':
                    case CR:
                    case LF:
                        client->state = STATE_BAD;
                        LOG_DEBUG( "Bad Request: hdr sta lf\n" );
                        break;
                    default:
                        strncat( client->token, &c, 1 );
                        client->state = STATE_HDR_FIE;
                        break;
                }
                break;
            case STATE_HDR_FIE:
                if( ++client->hdr_len > ( HDR_MAX_SIZE + 1 ) )
                {
                    client->state = STATE_BAD;
                    LOG_DEBUG( "Bad Request: hdr fie header too long\n" );
                }

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
                        LOG_DEBUG( "Bad Request: hdr fie lf\n" );
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
                            LOG_DEBUG( "Bad Request: hdr fie token too long\n" );
                        }
                        else
                            strncat( client->token, &c, 1 );
                        break;
                }
                break;
            case STATE_HDR_SEP_FIE_COL:
                if( ++client->hdr_len > ( HDR_MAX_SIZE + 1 ) )
                {
                    client->state = STATE_BAD;
                    LOG_DEBUG( "Bad Request: hdr sep fie col header too long\n" );
                }

                switch( c )
                {
                    case SP:
                        break;
                    case ':':
                        client->state = STATE_HDR_COL;
                        break;
                    default:
                        client->state = STATE_BAD;
                        LOG_DEBUG( "Bad Request: hdr sep fie col other\n" );
                        break;
                }
                break;
            case STATE_HDR_COL:
                if( ++client->hdr_len > ( HDR_MAX_SIZE + 1 ) )
                {
                    client->state = STATE_BAD;
                    LOG_DEBUG( "Bad Request: hdr col header too long\n" );
                }

                switch( c )
                {
                    case SP:
                        client->state = STATE_HDR_SEP_COL_VAL;
                        break;
                    case CR:
                    case LF:
                        client->state = STATE_BAD;
                        LOG_DEBUG( "Bad Request: hdr col lf\n" );
                        break;
                    default:
                        strncat( client->token, &c, 1 );
                        client->state = STATE_HDR_VAL;
                        break;
                }
                break;
            case STATE_HDR_SEP_COL_VAL:
                if( ++client->hdr_len > ( HDR_MAX_SIZE + 1 ) )
                {
                    client->state = STATE_BAD;
                    LOG_DEBUG( "Bad Request: hdr sep col val header too long\n" );
                }

                switch( c )
                {
                    case SP:
                        break;
                    case CR:
                    case LF:
                        client->state = STATE_BAD;
                        LOG_DEBUG( "Bad Request: hdr sep col val lf\n" );
                        break;
                    default:
                        strncat( client->token, &c, 1 );
                        client->state = STATE_HDR_VAL;
                        break;
                }
                break;
            case STATE_HDR_VAL:
                if( ++client->hdr_len > ( HDR_MAX_SIZE + 1 ) )
                {
                    client->state = STATE_BAD;
                    LOG_DEBUG( "Bad Request: hdr val header too long\n" );
                }

                switch( c )
                {
                    case CR:
                        client->state = STATE_FIR_CR;

                        strip( client->token, strlen( client->token ) );
                        snprintf( value, TOKEN_SIZE, "%s", client->token );

                        fill_header( client, field, value );

                        memset( field, 0, TOKEN_SIZE );
                        memset( value, 0, TOKEN_SIZE );

                        CLR_BUF( client->token, TOKEN_SIZE );
                        break;
                    case LF:
                        CLR_BUF( client->token, TOKEN_SIZE );
                        client->state = STATE_BAD;
                        LOG_DEBUG( "Bad Request: hdr val lf\n" );
                        break;
                    default:
                        if( strlen( client->token ) >= ( TOKEN_SIZE - 1 ) )
                        {
                            CLR_BUF( client->token, TOKEN_SIZE );
                            client->state = STATE_BAD;
                            LOG_DEBUG( "Bad Request: hdr val token too long\n" );
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
    int status, size;
    time_t now;
    msg_info mi;
    char date[DATE_SIZE] = { 0 };
    char version[] = "HTTP/1.1";
    char server[] = "Liso/1.0";
    char conn[CONN_MAX_SIZE] = { 0 };

    status = -1;
    mi.conlen = -1;
    CLR_BUF( mi.contype, CONTYPE_SIZE );
    CLR_BUF( mi.last_modi, DATE_SIZE );
    mi.len = -1;

    // Date field
    time( &now );
    date_format( date, DATE_SIZE, &now );

    // Connection field
    if( client->header[HDR_CONNECTION] != NULL )
    {
        size = strlen( client->header[HDR_CONNECTION] );
        if( strncmp( client->header[HDR_CONNECTION], "close", MAX( size, 5 ) ) == 0 )
            snprintf( conn, CONN_MAX_SIZE, "close" );
        else if( strncmp( client->header[HDR_CONNECTION], "keep-alive", MAX( size, 10 ) ) == 0 )
            snprintf( conn, CONN_MAX_SIZE, "keep-alive" );
        else
            snprintf( conn, CONN_MAX_SIZE, "keep-alive" );
    }
    else
        snprintf( conn, CONN_MAX_SIZE, "keep-alive" );

    if( strlen( client->uri ) >= 5 && strncmp( client->uri, "/cgi/", 5 ) == 0 )
    {
        int r;
        
        r = cgi( client );
        if( r == SERVER_ERROR )
            status = STATUS_500;
        else if( r == REQUEST_ERROR )
            status = STATUS_400;
        else
            return;

        respond( &client->sock, version, status, conn, date, server, NULL );
    }
    else
    {
        // according to state
        switch( client->state )
        {
            // Not implemented
            case STATE_MET_NOT_IMP:
                status = STATUS_501;
                do_get( status_htmls[STATUS_501], &mi );
                respond( &client->sock, version, status, conn, date, server, &mi );

                break;
            // HTTP Version Not Supported
            case STATE_VER_NOT_SUP:
                status = STATUS_505;
                do_get( status_htmls[STATUS_505], &mi );
                respond( &client->sock, version, status, conn, date, server, &mi );

                break;
            // Length Required
            case STATE_LEN_REQUIRED:
                status = STATUS_411;
                do_get( status_htmls[STATUS_411], &mi );
                respond( &client->sock, version, status, conn, date, server, &mi );

                break;
            // Bad Request
            case STATE_BAD:
                status = STATUS_400;
                do_get( status_htmls[STATUS_400], &mi );
                respond( &client->sock, version, status, conn, date, server, &mi );

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
                        respond( &client->sock, client->version, status, conn, date, server, &mi );

                        break;
                    // GET Method
                    case METHOD_GET:
                        status = do_get( client->uri, &mi );
                        if( status != STATUS_200 )
                            do_get( status_htmls[status], &mi );
                        respond( &client->sock, client->version, status, conn, date, server, &mi );

                        break;
                    // POST Method
                    case METHOD_POST:
                        status = do_post( client->uri, client->msg, &mi );
                        if( status != STATUS_200 )
                            do_get( status_htmls[status], &mi );
                        respond( &client->sock, client->version, status, conn, date, server, NULL );

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
}

// respond directly
void respond_directly( const sock_info *sock, const char *response, size_t len )
{
    if( generic_send( sock, response, len, 0 ) < len )
        LOG_ERROR( "Response not complete\n" );
}   

// respond according to header and message
void respond( const sock_info *sock, const char *version, const int status, const char *conn, const char *date, const char *server, const msg_info *mi )
{
    int size;
    char *response;
    char status_reason[STA_REA_MAX_SIZE] = { 0 };
    char conlen_s[CONLEN_MAX_SIZE] = { 0 };

    size = -1;
    response = NULL;

    // status code and reason phrase
    snprintf( status_reason, STA_REA_MAX_SIZE, "%s", status_reason_strings[status] );

    size = strlen( version ) + strlen( status_reason ) + strlen( conn ) +strlen( date ) + strlen( server );

    // if mi is NULL
    if( mi == NULL )
    {
        // +64 because other text
        response = ( char * )malloc( size + 64 );

        snprintf( response, size + 64, "%s %s\r\nDate: %s\r\nConnection: %s\r\nServer: %s\r\n\r\n",   
                  version, status_reason, date, conn, server );
    
        size = strlen( response );
        if( generic_send( sock, response, size, 0 ) < size )
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
                  version, status_reason, date, conn, server );
    }
    else if( mi->conlen >= 0 )
    {
        snprintf( response, size + 128, "%s %s\r\nDate: %s\r\nConnection: %s\r\nServer: %s\r\nContent-Type: %s\r\nContent-Length: %s\r\nLast-Modified: %s\r\n\r\n",
                  version, status_reason, date, conn, server, mi->contype, conlen_s, mi->last_modi );
    }

    size = strlen( response );
    LOG_INFO( "Response Header To %s:%u( %d ):\n%s\n", inet_ntoa( sock->addr.sin_addr ), ntohs( sock->addr.sin_port ), size, response );

    if( mi->len > 0 )
    {
        // build response's message body
        memcpy( response + size, mi->msg, mi->len );
        size += mi->len;
    
        LOG_INFO( "Message Body To %s:%u( %d ):\n%s\n", inet_ntoa( sock->addr.sin_addr ), ntohs( sock->addr.sin_port ), mi->len, mi->msg );
    }

    // send response to client
    if( generic_send( sock, response, size, 0 ) < size )
        LOG_ERROR( "Response not complete\n" );

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
    // fprintf( stdout, "uri: %s\nmsg: %s\n", uri, msg );

    return STATUS_200;
}

// get resource according to URI
int get_resource( const char *uri, msg_info *mi, bool msg_needed )
{
    int pos;
    struct stat buf;
    file_type ft;
    char resource_path[RESOURCE_SIZE] = { 0 };
    char suffix[SUFFIX_MAX_SIZE] = { 0 };

    pos = -1;
    ft = FT_UNKNOWN;

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
