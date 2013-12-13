#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
/**
 * Get the file length of the src file
 * @return its length as an int
 */
int file_size( const char *file_name )
{
    FILE *fp = fopen( file_name, "r" );
    long sz = -1;
    if ( fp != NULL )
    {
        fseek(fp, 0L, SEEK_END);
        sz = ftell(fp);
        fclose( fp );
    }
    return (int) sz;
}
/**
 * Read a file contents 
 * @param file_name the name of the file to read
 * @param len its length once entirely read
 * @return an allocated buffer. caller MUST dispose
 */
const char *read_file( const char *file_name, int *len )
{
    char *buf = NULL;
    *len = file_size(file_name);
    if ( *len > 0 )
    {
        FILE *src_file = fopen( file_name, "r" );
        int flen;
        if ( src_file != NULL )
        {
            buf = calloc( 1, (*len)+1 );
            if ( buf != NULL )
            {
                flen = (int)fread( buf, 1, *len, src_file );
                if ( flen != *len )
                    fprintf(stderr,"couldn't read %s\n",file_name);
            }
            else
                fprintf(stderr,"couldn't allocate buf\n");
            fclose( src_file );
            
        }
        else
            fprintf(stderr,"failed to open %s\n",file_name );
    }
    return buf;
}
