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
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "hashmap.h"
#include "attribute.h"
#include "annotation.h"
#include "range.h"
#include "node.h"
#include "error.h"
#include "memwatch.h"
#define MAX_TAGLEN 128
#define MIN_TEXTLEN 512
struct range_struct
{
    /** the name of the range's property */
	char *name;
    /** the mapped html tag name */
    char *html_name;
    /** absolute start offset in the text */
	int start;
    /** relative offset */
    int reloff;
    /** length of the property */
	int len;
    /** is this range rightmost among a sequence of split ranges? */
    int rightmost;
    /** is this a removed range? */
    int removed;
    /** linked list of annotations */
    annotation *annotations;
};
/**
 * Create a range from XML parsed attributes
 * @param name the name of the range
 * @param atts a NULL-terminated array of attribute name/value pairs
 * @return a range object or NULL
 */
range *range_create_atts( const char **atts )
{
    range *r = calloc( 1, sizeof(range) );
    if ( r != NULL )
    {
        int i = 0;
        r->rightmost = 1;
        while ( atts[i] != NULL )
        {
            if ( strcmp(atts[i],"reloff")== 0 )
                r->reloff = atoi(atts[i+1]);
            else if ( strcmp(atts[i],"name")==0 )
            {
                r->name = strdup((char*)atts[i+1]);
                if ( r->name == NULL )
                {
                    warning("range: failed to duplicate name %s\n",
                        atts[i+1]);
                    free( r );
                    r = NULL;
                    break;
                }
            }
            else if ( strcmp(atts[i],"len")==0 )
                r->len = atoi( atts[i+1] );
            else if ( strcmp(atts[i],"removed")==0 )
            {
                warning( "range: attempt to create removed range\n");
                free( r );
                r = NULL;
            }
            else
                warning( "range: invalid attribute %s ignored\n",atts[i]);
            i += 2;
        }
    }
    else
        warning("range: failed to allocate range");
    return r;
}
/**
 * Create an empty range
 * @return  the range struct unfilled
 */
range *range_create_empty()
{
    range *r = calloc( 1, sizeof(range) );
    if ( r==NULL )
        warning("range: failed to create\n");
    else
        r->rightmost = 1;
    return r;
}
/**
 * Copy a range
 * @param r the range to copy
 * @return the duplicated range
 */
range *range_copy( range *r )
{
    range *r2 = range_create( r->name, r->html_name, r->start, r->len );
    if ( r2 != NULL )
    {
        annotation *a = r->annotations;
        r2->reloff = r->reloff;
        r2->rightmost = r->rightmost;
        while ( a != NULL )
        {
            annotation *a_copy = annotation_clone( a );
            range_add_annotation( r2, a_copy );
            a = annotation_get_next( a );
        }
    }
    else
        warning("range: failed to duplicate range\n");
    return r2;
}
/**
 * Create a single range
 * @param name the name of the property
 * @param html_name the range's mapped html name
 * @param start its start offset inside the text
 * @param len its length
 * @return the finished range
 */
range *range_create( char *name, char *html_name, int start, int len )
{
    range *r = calloc( 1, sizeof(range) );
	if ( r != NULL )
    {
        r->name = strdup(name);
        if ( html_name != NULL )
            r->html_name = strdup( html_name );
        r->start = start;
        r->len = len;
        r->rightmost = 1;
    }
    else
        warning("range creation failed\n");
    return r;
}
/**
 * Dispose of a single range
 * @param r the range in question
 */
void range_dispose( range *r )
{
    if ( r->name != NULL )
    {
        free( r->name );
        r->name = NULL;
    }
    if ( r->html_name != NULL )
    {
        free( r->html_name );
        r->html_name = NULL;
    }
    annotation *a = r->annotations;
    while ( a != NULL )
    {
        annotation *next = annotation_get_next( a );
        annotation_dispose( a );
        a = next;
    }
    free( r );
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
 * Get the end offset of this range
 * @param r the range in question
 * @return the offset of the first char beyond the end
 */
int range_end( range *r )
{
    return r->start+r->len;
}
/**
 * Get the start of this range
 * @param r the range in question
 * @return its start offset
 */
int range_start( range *r )
{
    return r->start;
}
/**
 * Get the length of this range
 * @param r the range in question
 * @return its length
 */
int range_len( range *r )
{
    return r->len;
}
/**
 * Set the length of the range
 * @param r the range in question
 * @param len its length
 */
void range_set_len( range *r, int len )
{
    r->len = len;
}
/**
 * Get this range's name
 * @param r the range in question
 * @return the name as a string
 */
char *range_name( range *r )
{
    return r->name;
}
/**
 * Get this range's html name
 * @param r the range in question
 * @return the name as a string
 */
char *range_html_name( range *r )
{
    return r->html_name;
}
/**
 * Set the html name of the range post factum (normal case)
 * @param r the range
 * @param html_name name of the range in html
 * @return 1 if it worked, else 0
 */
int range_set_html_name( range *r, char *html_name )
{
    if ( r->html_name != NULL )
    {
        free( r->html_name );
        r->html_name = NULL;
    }
    if ( html_name != NULL )
        r->html_name = strdup( html_name );
    return r->html_name != NULL;
}
/**
 * Is a range inside another. Inside can be equal
 * @param r1 the first range (is it inside r2?)
 * @param r2 the second range being the outside one
 * @return 1 if r1 is inside r2, else 0
 */
int range_inside( range *r1, range *r2 )
{
    return range_start(r1)>=range_start(r2)&&range_end(r1)<=range_end(r2);
}
/**
 * Is one range equal to another?
 * @param r1 the first range 
 * @param r2 the second range
 * @return 1 if r1 equals the range of r2
 */
int range_equals( range *r1, range *r2 )
{
    return r1->start == r2->start && r1->len == r2->len;
}
/**
 * Set the range's name to a new value
 * @param r the range in question
 * @param name the new name (maybe like the old one)
 * @return 1 if it worked, else 0
 */
int range_set_name( range *r, char *name )
{
    if ( r->name == NULL )
        r->name = strdup( name );
    else if ( strcmp(r->name,name)!= 0 )
    {
        free( r->name );
        r->name = strdup( name );
    }
    return r->name != NULL;
}
/**
 * Set the absolute offset of this range in the text
 * @param r the range in question
 * @param absolute the absolute offset
 */
void range_set_absolute( range *r, int absolute )
{
    r->start = absolute;
}
/**
 * Set the relative offset of this range from the previous one
 * @param r the range in question
 * @param reloff the relative offset
 */
void range_set_reloff( range *r, int reloff )
{
    r->reloff = reloff;
}
/**
 * Get the relative offset
 * @param r the range in question
 * @return its relative offset
 */
int range_get_reloff( range *r )
{
    return r->reloff;
}
/**
 * Add an annotation to the range
 * @param r the range in question
 * @param a the annotation to add
 */
void range_add_annotation( range *r, annotation *a )
{
    if ( r->annotations == NULL )
        r->annotations = a;
    else
        annotation_append( r->annotations, a );
}
/**
 * Get the annotations of this range
 * @param r the range in question
 * @return its annotations as a list
 */
annotation *range_get_annotations( range *r )
{
    return r->annotations;
}
/**
 * Set the rightmost property of this range
 * @param r the range in question
 * @param rightmost the desired rightmost value
 */
void range_set_rightmost( range *r, int rightmost )
{
    r->rightmost = rightmost;
}
/**
 * Get the range's rightmost value
 * @param r the range in question
 * @return the rightmost value
 */
int range_get_rightmost( range *r )
{
    return r->rightmost;
}
/**
 * Is this a removed range?
 * @param r the range in question
 * @return 1 if it is else 0
 */
int range_get_removed( range *r )
{
    return r->removed;
}
/**
 * Set the removed property 
 * @param r the range in question
 * @param removed the new value of the removed property, can be 0 or 1
 */
void range_set_removed( range *r, int removed )
{
    r->removed = removed;
}
/**
 * Does a range overlap this range on the left?
 * @param q the range that might overlap on the left
 * @param r the range to compare it to
 * @return 1 if it is true
 */
int range_overlaps_left( range *q, range *r )
{
    return range_end(q)>r->start && range_end(q)<range_end(r);
}
/**
 * Does a range overlap this range on the right?
 * @param q the range that might overlap on the right
 * @param r the range to compare it to
 * @return 1 if it is true
 */
int range_overlaps_right( range *q, range *r )
{
    return range_end(r)>q->start && range_end(r)<range_end(q);
}
/**
 * Split a range into two halves by deleting a middle portion
 * @param r the range to split-delete
 * @param q the range to delete from r
 * @return NULL if it aligned at either end else the second half
 */
range *range_split_delete( range *r, range *q )
{
    if ( q->start == r->start )
    {
        r->len -= q->len;
        r->start += q->len;
        return NULL;
    }
    else if ( range_end(q) == range_end(r) )
    {
        r->len -= q->len;
        return NULL;
    }
    else
    {
        range *q2 = range_create( r->name, r->html_name, r->start, r->len );
        if ( q2 != NULL )
        {
            q2->start = range_end(q);
            q2->len = range_end(r) - q->start;
            r->len = q->start-r->start;
        }
        return q2;
    }
}