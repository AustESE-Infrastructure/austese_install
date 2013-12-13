#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "range.h"
#include "error.h"
#include "memwatch.h"
/** we push half-finished ranges onto the stack then
 * complete them when we get to the corresponding end-tags */
struct range_struct
{
	char *name;
	char **atts;
	int start;
	int len;
    int removed;
    char *content;
    int content_len;
	struct range_struct *next;
};
/**
 * Create a new range object
 * @param removed if 1 then the range has been removed
 * @param name the property name of the range
 * @param atts copy of the simplified attributes
 * @param offset the range's absolute offset
 * @return
 */
range *range_new( int removed, char *name, char **atts, int offset )
{
    range *r = malloc( sizeof(range) );
    if ( r == NULL )
        error( "range: failed to allocate range structure\n" );
    r->removed = removed;
    r->start = offset;
    r->name = strdup( name );
    r->content = NULL;
    r->content_len = 0;
    if ( r->name == NULL )
        error( "range: failed to duplicate range name\n" );
    r->next = NULL;
    r->atts = atts;
    return r;
}
/**
 * Free all the allocated memory associated with a range
 * @param r the range to free
 */
void range_delete( range *r )
{
	int i=0;
	if ( r->atts != NULL )
	{
		while ( r->atts[i] != NULL )
		{
			if ( r->atts[i] != NULL )
				free( r->atts[i] );
			if ( r->atts[i+1] != NULL )
				free( r->atts[i+1] );
			i += 2;
		}
        free( r->atts );
	}
	if ( r->name != NULL )
		free( r->name );
    if ( r->content != NULL )
        free( r->content );
	free( r );
}
/**
 * Is this range removed?
 * @param r the range in question
 * @return 1 if it was, 0 otherwise
 */
int range_removed( range *r )
{
    return r->removed;
}
/**
 * Compare two ranges. Sort on increasing offset then on decreasing length
 * @param r1 the first range
 * @param r2 the second range
 * @return 1 if r1 > r2, if equal 0 else -1
 */
int range_compare( void *key1, void *key2 )
{
    range *r1 = key1;
    range *r2 = key2;
    if ( r1->start > r2->start )
        return 1;
    else if ( r2->start > r1->start )
        return -1;
    else if ( r2->len > r1->len )
        return 1;
    else if ( r1->len > r2->len )
        return -1;
    else
        return 0;
}
/**
 * Add some content to a removed range
 * @param r the range in question
 * @param s the content
 * @param len its length
 */
void range_add_content( range *r, const char *s, int len )
{
    if ( r->content_len > 0 )
    {
        char *new_content = malloc( r->content_len+len+1 );
        if ( new_content == NULL )
            error( "range: failed to reallocate content\n");
        else
        {
            memcpy( new_content, r->content, r->content_len );
            memcpy( &new_content[r->content_len], s, len );
            new_content[r->content_len+len] = 0;
            free( r->content );
            r->content = new_content;
            r->content_len += len;
            //printf("content now %s\n",r->content );
        }
    }
    else
    {
        r->content = malloc( len+1 );
        if ( r->content == NULL )
            error( "range: failed to allocate new content\n");
        memcpy( r->content, s, len );
        r->content[len] = 0;
        r->content_len = len;
       // printf("created content length %d\n",len);
    }
}
/**
 * Get the content of a range
 * @param r the range in question
 * @return its content (usually NULL)
 */
char *range_get_content( range *r )
{
    return r->content;
}
/**
 * Get the length of a content range
 * @param r the range in question
 * @return its length
 */
int range_get_content_len( range *r )
{
    return r->content_len;
}
/**
 * Get a range's name
 * @param r the range in question
 * @return its name
 */
char *range_get_name( range *r )
{
    return r->name;
}
/**
 * Get the nage's remaining attributes
 * @param r the range in question
 * @return its atts in expat format
 */
char **range_get_atts( range *r )
{
    return r->atts;
}
/**
 * Get this range's length
 * @param r the range in question
 * @return its length
 */
int range_get_len( range *r )
{
    return r->len;
}
/**
 * Set a range's length
 * @param r the range in question
 * @param len its length
 */
void range_set_len( range *r, int len )
{
    r->len = len;
}
/**
 * Get the next range in the list
 * @param r the range in question
 * @return a range
 */
range *range_get_next( range *r )
{
    return r->next;
}
/**
 * Set the next range in the list
 * @param r the range in question
 * @param next another range
 */
void range_set_next( range *r, range *next )
{
    r->next = next;
}
/**
 * Get the absolute offset of the range
 * @param r the range in question
 * @return its offset
 */
int range_get_start( range *r )
{
    return r->start;
}