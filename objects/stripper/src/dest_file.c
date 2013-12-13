#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "milestone.h"
#include "layer.h"
#ifdef JNI
#include "ramfile.h"
#endif
#include "format.h"
#include "range.h"
#include "dest_file.h"
#include "log.h"
#include "hashmap.h"
/**
 * Manage the contents of an output file in memory or for writing to disk.
 */
struct dest_file_struct
{
    dest_kind kind;
    char *midname;
    char *name;
    char *buffer;
    // only needed by markup dest files
    int first;
    // previous offset used by markup files
    int prevoff;
    layer *l;
    int len;
    DST_FILE *dst;
    range *queue;
    range *queue_end;
    format *f;
    dest_file *next;
};
/** the output queue to straighten out the range ordering */
/*
range *queue;
range *queue_end;
*/
void dest_file_enqueue( dest_file *df, range *r )
{
    if ( df->queue_end == NULL )
        df->queue = df->queue_end = r;
    else
    {
        range_set_next( df->queue_end, r );
        df->queue_end = r;
    }
}
/**
 * Create a dest file object
 * @param kind the kind of dest file: markup or text
 * @param l the layer this dest_file is for or NULL for the main markup
 * @param midname its middle name e.g. -aese, up to 5
 * @param name the name of the file, any length
 * @param f the format for this dest_file to follow
 * @return an initialised det file object
 */
dest_file *dest_file_create( dest_kind kind, layer *l, char *midname, 
    char *name, format *f )
{
    if ( f != NULL )
    {
        dest_file *df = calloc( 1, sizeof(dest_file) );
        if ( df != NULL )
        {
            df->first = 1;
            df->kind = kind;
            df->f = f;
            df->l = l;
            df->midname = malloc(strlen(midname)+1);
            if ( df->midname != NULL )
            {
                strcpy( df->midname, midname );
                df->name = strdup( name );
            }
            if ( df->midname==NULL||df->name==NULL )
            {
                df = dest_file_dispose( df );
                fprintf(stderr,"dest_file: failed to initialise object\n");
            }
        }
        else
            fprintf(stderr,"dest_file: failed to allocate dest file\n");
        return df;
    }
    else
        fprintf(stderr,"dest_file: formatter may not be NULL\n");
    return NULL;
}
/**
 * Get the relative offset given the absolute offset of a range. Update 
 * the previous offset to the new value
 * @param df the dest file in question
 * @param absolute_off the absolute offset of the new range
 * @return the relative offset of the new range from the last one written
 */
int dest_file_reloff( dest_file *df, int absolute_off )
{
    int reloff = absolute_off-df->prevoff;
    df->prevoff = absolute_off;
    return reloff;
}
/**
 * Compute the range's length if it was a milestone
 * @param df the dest file whose queue is to be scanned
 * @param tlen the length of the underlying text
 * @return true if it worked else 0
 */
static int compute_range_lengths( dest_file *df, int tlen )
{
    int res = 1;
    hashmap *map = hashmap_create();
    if ( map != NULL )
    {
        range *r = df->queue;
        while ( r != NULL )
        {
            range *s = hashmap_get( map, range_get_name(r) );
            if ( s != NULL )
                range_set_len( s, range_get_start(r)-range_get_start(s) );
            hashmap_put( map, range_get_name(r), r );
            r = range_get_next( r );
        }
        hashmap_iterator *iter = hashmap_iterator_create( map );
        if ( iter != NULL )
        {
            while ( hashmap_iterator_has_next(iter) )
            {
                char *key = hashmap_iterator_next(iter);
                range *t = hashmap_get( map, key );
                range_set_len( t, tlen-range_get_start(t) );
            }
            hashmap_iterator_dispose( iter );
        }
        else
            res = 0;
        hashmap_dispose( map );
    }
    else
        res = 0;
    return res;
}
/**
 * Sort the ranges using shellsort for printing
 * @param d the dom in question
 */
void range_array_sort( range **ra, int len )
{
    int i, j, k, h; range *v;
    int incs[16] = { 1391376, 463792, 198768, 86961, 33936,
        13776, 4592, 1968, 861, 336, 
        112, 48, 21, 7, 3, 1 };
    for ( k = 0; k < 16; k++)
    {
        for ( h=incs[k],i=h;i<=len-1;i++ )
        { 
            v = ra[i]; 
            j = i;
            while (j >= h && range_compare(ra[j-h],v)>0 )
            { 
                ra[j] = ra[j-h]; 
                j -= h; 
            }
            ra[j] = v; 
        }
    }
}
/**
 * Convert a list of ranges to a sorted array
 * @param r the first range in the list
 * @param len VAR param: set to length of array
 * @return the array of ranges or NULL
 */
static range **ranges_to_array( range *r, int *len )
{
    int i = 0;
    range *s = r;
    while ( s != NULL )
    {
        i++;
        s = range_get_next( s );
    }
    // so i is the length of the list
    range **array = calloc( i, sizeof(range*) );
    if ( array != NULL )
    {
        *len = i;
        s = r;
        i=0;
        while ( s != NULL )
        {
            array[i++] = s;
            s = range_get_next( s );
        }
        // now sort the array
        range_array_sort( array, *len );
    }
    return array;
}
/**
 * Write the enqueued ranges to disk, free them
 * @param df the dest file object
 * @return 1 if it worked, else 0
 */
static int dest_file_dequeue( dest_file *df )
{
    int i,res = 1;
    int len = 0;
    range **array = ranges_to_array( df->queue, &len );
    if ( array != NULL )
    {
        range *r = df->queue;
        dest_file_set_first( df, 1 );
        for ( i=0;i<len;i++ )
        {
            r = array[i];
            res = df->f->rfunc( 
                range_get_name(r),
                range_get_atts(r),
                range_removed(r),
                dest_file_reloff(df,range_get_start(r)),
                range_get_len(r),
                range_get_content(r),
                range_get_content_len(r),
                dest_file_first(df), 
                dest_file_dst(df) );
            dest_file_set_first( df, 0 );
            range_delete( r );
            if ( !res )
            {
                fprintf(stderr, "stripper: failed to write range" );
                res = 0;
                break;
            }
            if ( !res )
                break;
        }
        free( array );
    }
    if ( res )
        df->queue = NULL;
    return res;
}
/**
 * Close a dest file
 * @param df the dest file to close
 * @param tlen the length of the underling text 
 * @return 1 if successful else 0
 */
int dest_file_close( dest_file *df, int tlen )
{
    int res = 1;
    if ( df->kind == markup_kind )  
    {
        if ( df->l != NULL )
            res = compute_range_lengths( df, tlen );
#ifdef JNI        
        tmplog("compute_range_lengths returned %d\n",res);
#endif
        if ( res )
            res = dest_file_dequeue(df);
#ifdef JNI        
        tmplog("dest_file_dequeue returned %d\n",res);
#endif
        // write tail
        if ( res )
            res = df->f->tfunc(NULL, dest_file_dst(df) );
#ifdef JNI        
        tmplog("df->f->tfunc returned %d\n",res);
#endif
    }
#ifndef JNI
    if ( df->dst != NULL )
        fclose( df->dst );
#endif
    return res;
}
/**
 * Dispose of a destination file
 * @param df the dest file to dispose
 * @return NULL
 */
dest_file *dest_file_dispose( dest_file *df )
{
    if ( df->name != NULL )
        free( df->name );
    if ( df->next != NULL )
        dest_file_dispose( df->next );
    if ( df->queue != NULL )
    {
        range *r = df->queue;
        while ( r != NULL )
        {
            range *next = range_get_next(r);
            range_delete( r );
            r = next;
        }
    }
#ifdef JNI
    if ( df->dst != NULL )
        ramfile_dispose( df->dst );
#endif
    free( df );
    return NULL;
}
/**
 * Is this the first dst file?
 * @param df the dest file
 * @return 1 if this is the first access
 */
int dest_file_first( dest_file *df )
{
    return df->first;
}
/**
 * Is this the first dst file?
 * @param df the dest file
 * @param value the value to set it to: 1 or 0
 */
void dest_file_set_first( dest_file *df, int value )
{
    df->first = value;
}
/**
 * Get the actual destination file
 * @param df the dest file object
 * @return a DST_FILE object (FILE* or ramfile*)
 */
DST_FILE *dest_file_dst( dest_file *df )
{
    return df->dst;
}
/**
 * Get the layer to which we belong
 * @param df the dest file object
 * @return the layer or NULL if the default layer
 */
layer *dest_file_layer( dest_file *df )
{
    return df->l;
}
/**
 * Write to the file
 * @param df the dest file object
 * @param data the data to write
 * @param len the length of the data
 * @return the number of chars written
 */
int dest_file_write( dest_file *df, char *data, int len )
{
    df->len += len;
    return DST_WRITE(data,len,df->dst);
}
/**
 * Open the destination file
 */
int dest_file_open( dest_file *df )
{
#ifdef JNI
    df->dst = ramfile_create();
    return ( df->dst != NULL );
#else
    int res = 0;
    const char *suffix = (df->kind==text_kind)?df->f->text_suffix
        :df->f->markup_suffix;
    int len = strlen(df->name)+strlen(suffix)+strlen(df->midname)+1;
    char *markup = malloc( len );
	if ( markup != NULL )
    {
        strcpy( markup, df->name );
        strcat( markup, df->midname );
		strcat( markup, suffix );
        df->dst = fopen( markup, "w" );
        if ( df->dst == NULL )
            fprintf( stderr,"stripper: couldn't open %s", markup );
        else
            res = 1;
    }
    return res;
#endif
}
/**
 * Get the length of this file (only valid once closed)
 * @param df the dest file in quetion
 * @return its length
 */
int dest_file_len( dest_file *df )
{
    return df->len;
}