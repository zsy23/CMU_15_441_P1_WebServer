/******************************************************************************
* log.h                                                                       *
*                                                                             *
* Description: This file contains the C source code for the logging module.   *
*              Logging contains 5 levels: NONE, ERROR, WARNING, LOG, DEBUG.   *
*              Fix LOG_RUN_LEVEL to variable log_run_level. Add log file      * 
*              creation and close. Add timestamp to log.                      *
*                                                                             *
* Author: Shiyu Zhang <1181856726@qq.com>                                     *
*                                                                             *
*******************************************************************************/

#ifdef	__cplusplus
extern "C" {
#endif
 
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <time.h>
 
typedef enum {
	LOG_LEVEL_NONE, // 0
	LOG_LEVEL_ERROR, // 1
	LOG_LEVEL_WARNING, // 2
	LOG_LEVEL_LOG, // 3
	LOG_LEVEL_DEBUG, // 4
	LOG_LEVEL_NEVER // 5
} log_level;
 
// The LOG_BUILD_LEVEL defines what will be compiled in the executable
// In production, it should be set to LOG_LEVEL_LOG

#ifndef LOG_BUILD_LEVEL
#ifdef NO_DEBUG
#define LOG_BUILD_LEVEL LOG_LEVEL_LOG
#else
#define LOG_BUILD_LEVEL LOG_LEVEL_DEBUG
#endif
#endif
 
static FILE *log_fp = NULL;
static log_level log_run_level = LOG_LEVEL_DEBUG;
static const char *log_level_strings [] = {
	"   NONE", // 0
	"  ERROR", // 1
	"WARNING", // 2
	"    LOG", // 3
	"  DEBUG"  // 4
};
 
static void log_get_timestamp( char *buf );

void log_init( log_level level, const char *filename );
void log_set_level( log_level level ) { log_run_level = level; }
void log_close() { if ( log_fp != stderr ) fclose( log_fp ); log_fp = NULL; }

#define LOG_IS_ALLOWED( level ) ( level <= LOG_BUILD_LEVEL && level <= log_run_level )

#define LOG( level, fmt, ... ) do {	\
	if ( LOG_IS_ALLOWED( level ) ) { \
        char buf[64]; \
        log_get_timestamp( buf ); \
		fprintf( log_fp, "[%s%s %s:%d] " fmt, buf, log_level_strings[level], __FUNCTION__, __LINE__, ##__VA_ARGS__ ); \
		fflush( log_fp ); \
	} \
} while( 0 )
 
#define LOG_DEBUG( fmt, ... ) LOG( LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__ )
#define LOG_LOG( fmt, ... ) LOG( LOG_LEVEL_LOG, fmt, ##__VA_ARGS__ )
#define LOG_WARNING( fmt, ... ) LOG( LOG_LEVEL_WARNING, fmt, ##__VA_ARGS__ )
#define LOG_ERROR( fmt, ... ) LOG( LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__ )
 
static void log_get_timestamp( char *buf )
{
    time_t now;
    time( &now );
    strftime( buf, 64, "%Y/%m/%d %T", localtime( &now ) );
}

void log_init( log_level level, const char *filename )
{
    log_run_level = level; 

    if ( filename )    
        log_fp = fopen( filename, "a" );
    if ( filename == NULL || log_fp == NULL )
        log_fp = stderr;
    
    assert( log_fp != NULL );
}

#ifdef	__cplusplus
}
#endif
