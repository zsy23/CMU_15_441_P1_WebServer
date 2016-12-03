/******************************************************************************
* log.h                                                                       *
*                                                                             *
* Description: This file contains the C declaration code for the logging      *
*              module.                                                        *
*                                                                             *
* Author: Shiyu Zhang <1181856726@qq.com>                                     *
*                                                                             *
*******************************************************************************/

#ifndef _LOG_H_
#define _LOG_H_

#include <stdarg.h>
#include <stdio.h>

// logging level
typedef enum {
	LOG_LEVEL_NONE,    // 0
	LOG_LEVEL_ERROR,   // 1
	LOG_LEVEL_WARNING, // 2
	LOG_LEVEL_INFO,    // 3
	LOG_LEVEL_DEBUG,   // 4
	LOG_LEVEL_NEVER    // 5
} log_level;

extern FILE *log_fp;
extern log_level log_run_level;
extern const char *log_level_strings[];

// The LOG_BUILD_LEVEL defines what will be compiled in the executable
// In production, it should be set to LOG_LEVEL_INFO
#ifndef LOG_BUILD_LEVEL
#ifdef NO_DEBUG
#define LOG_BUILD_LEVEL LOG_LEVEL_INFO
#else
#define LOG_BUILD_LEVEL LOG_LEVEL_DEBUG
#endif
#endif

// logging permission
#define LOG_IS_ALLOWED( level ) ( level <= LOG_BUILD_LEVEL && level <= log_run_level )

// generic logging macro
#define LOG( level, fmt, ... ) do {	\
	if ( LOG_IS_ALLOWED( level ) ) { \
        char _buf[64]; \
        log_get_timestamp( _buf, 64 ); \
		fprintf( log_fp, "[%s %s %s:%d] " fmt, _buf, log_level_strings[level], __FUNCTION__, __LINE__, ##__VA_ARGS__ ); \
		fflush( log_fp ); \
	} \
} while( 0 )
 
// specific logging macro
#define LOG_DEBUG( fmt, ... ) LOG( LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__ )
#define LOG_INFO( fmt, ... ) LOG( LOG_LEVEL_INFO, fmt, ##__VA_ARGS__ )
#define LOG_WARNING( fmt, ... ) LOG( LOG_LEVEL_WARNING, fmt, ##__VA_ARGS__ )
#define LOG_ERROR( fmt, ... ) LOG( LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__ )

// init logging level and file
void log_init( log_level level, const char *filename );

// set logging level
void log_set_level( log_level level );

// get logging timestamp
void log_get_timestamp( char *buf, size_t size );

// close logging file
void log_close();

#endif
