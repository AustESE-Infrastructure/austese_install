#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <syslog.h>
#include "hashmap.h"

#ifdef DEBUG_MEMORY
#define STRING_BLOCK_SIZE 100000
static hashmap *allocations = NULL;
static char *strings = NULL;
static int highwater = 0;
static int allocated = 0;
/**
 * Change this to report differently when a library or commandline tool
 */
static void debug_report( const char *fmt, ... )
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
static char *string_store( char *str )
{
    int len = strlen( str );
    if ( strings==NULL )
    {
        strings = malloc(STRING_BLOCK_SIZE);
        if ( strings != NULL && len<STRING_BLOCK_SIZE )
        {
            allocated = STRING_BLOCK_SIZE;
        }
        else
        {
            debug_report("out of memory %s %d",__FILE__,__LINE__);
            exit( 0 );
        }
    }
    else if ( len+highwater >= allocated )
    {
        char *copy = malloc( allocated+len+STRING_BLOCK_SIZE );
        if ( copy != NULL )
        {
            memcpy( copy, strings, highwater );
            free( strings );
            strings = copy;
            allocated += len+STRING_BLOCK_SIZE ;
        }
        else
        {
            debug_report("out of memory %s %d",__FILE__,__LINE__);
            exit( 0 );
        }
    }
    if ( strings != NULL )
    {
        char *orig = &strings[highwater];
        strcpy( &strings[highwater], str );
        highwater += len+1;
        return orig;
    }
    else
        return NULL;
}
static void memory_store( void *block, const char *file, int line )
{
    if ( allocations == NULL )
        allocations = hashmap_create();
    if ( allocations != NULL )
    {
        char block_addr[32];
        char block_location[32];
        snprintf( block_addr, 31, "%ld", (long) block );
        snprintf( block_location, 31, "%s: %d", file, line );
        char *value = string_store(block_location);
        hashmap_put( allocations, block_addr, value );
    }
    else
        debug_report("failed to allocate memory allocations table\n");
}
static void memory_free( void *block )
{
    if ( allocations != NULL )
    {
        char block_addr[32];
        snprintf( block_addr, 31, "%ld", (long) block );
        if ( !hashmap_remove(allocations,block_addr) )
            debug_report("double free!\n");
        else
            free( block );
    }
    else
        debug_report("allocations table is empty\n");
}
void vp( void *p )
{
    if ( allocations != NULL )
    {
        char block_addr[32];
        if ( p == NULL )
            debug_report("NULL pointer!\n");
        else
        {
            snprintf( block_addr, 31, "%ld", (long) p );
            if ( !hashmap_contains(allocations,block_addr) )
            {
                debug_report("invalid pointer %s!\n",block_addr);
            }
        }
    }
}
/**
 * Print out details of any unfreed blocks or report that all blocks were freed
 */
void memory_print()
{
    if ( allocations != NULL )
    {
        int unfreed = 0;
        hashmap_iterator *iter = hashmap_iterator_create( allocations );
        while ( hashmap_iterator_has_next(iter) )
        {
            char *key = hashmap_iterator_next( iter );
            char *value = hashmap_get( allocations, key );
            debug_report("unfreed block %s at %s \n", key, value );
            unfreed++;
        }
        if ( unfreed == 0 )
            debug_report("all blocks were freed\n");
        hashmap_dispose( allocations );
        free( strings );
        strings = NULL;
    }
}
void *dbg_malloc(size_t a,const char *f,int l)
{
    void *b = malloc( a );
    memory_store( b, f, l );
    return b;
}
void dbg_free(void *a)
{
    memory_free( a );
}
char *dbg_strdup(char *a,const char *f,int l)
{
    char *b = strdup( a );
    memory_store( b, f, l );
    return b;
}
void *dbg_calloc(size_t a,size_t b,const char *f,int l)
{
    void *c = calloc( a, b );
    memory_store( c, f, l );
    return c;
}
#endif