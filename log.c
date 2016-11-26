/******************************************************************************
* log.c                                                                       *
*                                                                             *
* Description: This file contains the C source code for the logging module.   *
*              Logging contains 5 levels: NONE, ERROR, WARNING, INFO, DEBUG.   *
*              Fix LOG_RUN_LEVEL to variable log_run_level. Add log file      * 
*              creation and close. Add timestamp to log.                      *
*                                                                             *
* Author: Shiyu Zhang <1181856726@qq.com>                                     *
*                                                                             *
*******************************************************************************/

#include "log.h"

#include <time.h>
#include <assert.h>

FILE *log_fp = NULL;
log_level log_run_level = LOG_LEVEL_DEBUG;
const char *log_level_strings [] = {
	"   NONE", // 0
	"  ERROR", // 1
	"WARNING", // 2
	"   INFO", // 3
	"  DEBUG"  // 4
};
 
void log_init( log_level level, const char *filename )
{
    log_run_level = level; 

    if ( filename )    
        log_fp = fopen( filename, "a" );
    if ( filename == NULL || log_fp == NULL )
        log_fp = stderr;
    
    assert( log_fp != NULL );
}

void log_set_level( log_level level )
{ 
    log_run_level = level; 
}

void log_get_timestamp( char *buf )
{
    time_t now;
    time( &now );
    strftime( buf, 64, "%Y/%m/%d %T", localtime( &now ) );
}

void log_close() 
{ 
    if ( log_fp != stderr ) fclose( log_fp ); 
    log_fp = NULL; 
}
