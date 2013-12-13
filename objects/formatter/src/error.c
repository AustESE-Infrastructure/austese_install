/*
 * This file is part of formatter.
 *
 *  formatter is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  formatter is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with formatter.  If not, see <http://www.gnu.org/licenses/>.
 *  (c) copyright Desmond Schmidt 2011
 */
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "error.h"
#include "memwatch.h"
static char message[256];
/**
 * Report an error. 
 * @param fmt the format of the error message
 * @param l the variable list of arguments to include in the format
 */
static void display_error( const char *fmt, va_list l )
{
    vsnprintf( message, 255, fmt, l );
    fprintf( stderr, "%s", message );
}
/**
 * Report a fatal error. Stop the program.
 * @param fmt the format of the error message
 * @param ... the arguments to include in the format
 */
void error( const char *fmt, ... )
{
    va_list l;
    va_start(l,fmt);
    display_error( fmt, l );
    va_end(l);
    exit( 0 );
}
/**
 * Report a non-fatal error.
 * @param fmt the format of the error message
 * @param ... the arguments to include in the format
 */
void warning( const char *fmt, ... )
{
    va_list l;
    va_start(l,fmt);
    display_error( fmt, l );
    va_end(l);
}
/**
 * Change this to report differently when a library or commandline tool
 */
void report( const char *fmt, ... )
{
    va_list ap;
    char message[128];
    va_start( ap, fmt );
    vsnprintf( message, 128, fmt, ap );
#ifdef DEBUG_SYSLOG 
    syslog( LOG_ALERT, "%s", message );
#else
    FILE *db = fopen("/tmp/formatter-debug.txt","a+");
    if ( db != NULL )
    {
        fprintf( db, "%s", message );
        fclose( db );
    }
#endif
    va_end( ap );
}