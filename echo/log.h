/******************************************************************************
* log.h                                                                       *
*                                                                             *
* Description: This file contains the C source code for the logging module.   *
*              Logging contains 5 levels: NONE, ERROR, WARNING, LOG, DEBUG.   *
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
 
enum {
	LOG_LEVEL_NONE, // 0
	LOG_LEVEL_ERROR, // 1
	LOG_LEVEL_WARNING, // 2
	LOG_LEVEL_LOG, // 3
	LOG_LEVEL_DEBUG, // 4
	LOG_LEVEL_NEVER // 5
};
 
#ifndef LOG_BUILD_LEVEL
#ifdef NO_DEBUG
#define LOG_BUILD_LEVEL LOG_LEVEL_LOG
#else
#define LOG_BUILD_LEVEL LOG_LEVEL_DEBUG
#endif
#endif
 
#ifndef LOG_RUN_LEVEL
#ifdef NO_DEBUG
#define LOG_RUN_LEVEL LOG_LEVEL_LOG
#else
#define LOG_RUN_LEVEL LOG_LEVEL_DEBUG
#endif
#endif

const char * log_level_strings [] = {
	"   NONE", // 0
	"  ERROR", // 1
	"WARNING", // 2
	"    LOG", // 3
	"  DEBUG"  // 4
};
 
// The LOG_BUILD_LEVEL defines what will be compiled in the executable
// In production, it should be set to LOG_LEVEL_LOG
 
// #ifndef LOG_FD
// #ifdef stdout
// #define LOG_FD stdout
// #endif
// #endif
 
#define LOG_IS_ALLOWED( level ) ( level <= LOG_BUILD_LEVEL && level <= LOG_RUN_LEVEL )

// #ifdef WIN32
// #define arg_def ...
// #define arg_use ##__VA_ARGS__
// #else
// #define arg_def arg...
// #define arg_use ##arg
// #endif

#define LOG(level, fd, fmt, ...) do {	\
	if ( LOG_IS_ALLOWED( level ) ) { \
		fprintf(fd, "[%s] %s:%d: " fmt, log_level_strings[level], __FUNCTION__,__LINE__, ##__VA_ARGS__); \
		fflush( fd ); \
	} \
} while(0)
 
#define LOG_DEBUG( fd, fmt, ... ) LOG( LOG_LEVEL_DEBUG, fd, fmt, ##__VA_ARGS__ )
#define LOG_LOG( fd, fmt, ... ) LOG( LOG_LEVEL_LOG, fd, fmt, ##__VA_ARGS__ )
#define LOG_WARNING( fd, fmt, ... ) LOG( LOG_LEVEL_WARNING, fd, fmt, ##__VA_ARGS__ )
#define LOG_ERROR( fd, fmt, ... ) LOG( LOG_LEVEL_ERROR, fd, fmt, ##__VA_ARGS__ )
 
#ifdef	__cplusplus
}
#endif
