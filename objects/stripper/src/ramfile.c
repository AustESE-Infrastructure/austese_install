#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "ramfile.h"
#include "error.h"
#include "memwatch.h"
#define BLOCK_SIZE 8096
#define PRINT_LIMIT 1024
static char buf[PRINT_LIMIT];
struct ramfile_struct
{
    int allocated;
    char *buf;
    int used;
};
/**
 * Create a ramfile - a file-like holder of a string buffer in memory
 * @return an initialised ramfile object
 */
ramfile *ramfile_create()
{
    ramfile *rf = calloc( 1, sizeof(ramfile) );
    if ( rf != NULL )
    {
        rf->buf = malloc( BLOCK_SIZE );
        if ( rf->buf == NULL )
        {
            free( rf );
            rf = NULL;
        }
        else
        {
            rf->allocated = BLOCK_SIZE;
            rf->used = 0;
        }
    }
	return rf;
}
/**
 * Dispose of a ramfile
 */
void ramfile_dispose( ramfile *rf )
{
    if ( rf->buf != NULL )
        free( rf->buf );
    free( rf );
}
/**
 * Write some data to a ramfile
 * @param rf the ramfile in question
 * @param data the data to write
 * @param len the length of the data 
 * @return nchars if it worked, else 0
 */
int ramfile_write( ramfile *rf, const char *data, int len )
{
    if ( len+rf->used >= rf->allocated )
    {
        int new_size = len+rf->used+BLOCK_SIZE ;
        char *tmp = malloc( new_size );
        if ( tmp != NULL )
        {
            memcpy( tmp, rf->buf, rf->used );
            rf->allocated = new_size;
            tmp[rf->used] = 0;
            free( rf->buf );
            rf->buf = tmp;
        }
        else
        {
            error("ramfile: failed to reallocate ramfile buffer\n");
            return 0;
        }  
    }
    memcpy( &rf->buf[rf->used], data, len );
    rf->used += len;
    rf->buf[rf->used] = 0;
    return len;
}
/**
 * Equivalent of printf. Works up to PRINT_LIMIT bytes
 * @param rf the ramfile in question
 * @param fmt the printf format string
 * ... the other args
 * @return 1 if it worked, else 0
 */
int ramfile_print( ramfile *rf, const char *fmt, ... )
{
    int slen,res = 1;
    va_list ap;
    va_start( ap, fmt );
    vsnprintf( buf, PRINT_LIMIT, fmt, ap );
    slen = strlen(buf);
    res = ramfile_write( rf, buf, slen );
    va_end( ap );
    return res;
}
/**
 * Get the NULL-terminated string buffer
 * @param rf the ramfile in question
 * @return a C-string
 */
char *ramfile_get_buf( ramfile *rf )
{
    return rf->buf;
}
/**
 * Get the length of the string stored here.
 * @param rf the ramfile in question
 * @return the length of the used part of the buffer
 */
int ramfile_get_len( ramfile *rf )
{
    return rf->used;
}
