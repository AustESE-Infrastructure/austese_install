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
#include <math.h>
#include <string.h>
#include "hashmap.h"
#include "attribute.h"
#include "annotation.h"
#include "range.h"
#include "hashset.h"
#include "range_array.h"
#include "formatter.h"
#include "css_selector.h"
#include "css_property.h"
#include "css_rule.h"
#include "css_parse.h"
#include "matrix.h"
#include "queue.h"
#include "text_buf.h"
#include "dom.h"
#include "error.h"
#include "memwatch.h"


#define RANGES_BLOCK_SIZE 256

struct formatter_struct
{
    range_array *ranges;
    hashmap *css_rules;
    hashset *properties;
    dom *tree;
};
/**
 * Create a formatter
 * @return a formatter object
 */
formatter *formatter_create( int len )
{
    formatter *f = calloc( 1, sizeof(formatter) );
    if ( f != NULL )
    {
        f->ranges = range_array_create();
        if ( f->ranges == NULL )
        {
            formatter_dispose( f );
            return NULL;
        }
        f->css_rules = hashmap_create();
        if ( f->css_rules == NULL )
        {
            formatter_dispose( f );
            return NULL;
        }
        else
        {
            css_rule *root = css_rule_create();
            css_selector *sel = css_selector_create( NULL, "root");
            if ( root==NULL || sel==NULL || !css_rule_add_selector(root,sel) )
            {
                warning("could not add root selector to css rules\n");
                formatter_dispose( f );
                return NULL;
            }
            else
                hashmap_put( f->css_rules, "root", root );
        }
        f->properties = hashset_create();
        if ( f->properties == NULL )
        {
            formatter_dispose( f );
            return NULL;
        }
        else
            hashset_put( f->properties, "root" );
    }
    else
        warning("formatter: failed to allocate formatter\n");
    return f;
}
/**
 * Dispose of a formatter
 * @param f the formatter to free
 */
void formatter_dispose( formatter *f )
{
    if ( f->ranges != NULL )
        range_array_dispose( f->ranges, 1 );
    if ( f->css_rules != NULL )
    {
        hashmap_iterator *iter = hashmap_iterator_create( f->css_rules );
        if ( iter != NULL )
        {
            while ( hashmap_iterator_has_next(iter) )
            {
                char *key = hashmap_iterator_next( iter );
                css_rule *rule = hashmap_get( f->css_rules, key );
                css_rule_dispose( rule );
            }
            hashmap_dispose( f->css_rules );
            hashmap_iterator_dispose(iter);
        }
    }
    if ( f->properties != NULL )
        hashset_dispose( f->properties );
    if ( f->tree != NULL )
        dom_dispose( f->tree );
    free( f );
}
/**
 * Add a css file contents to the formatter
 * @param f the formatter to apply it to
 * @param data the css data
 * @param len its length
 * @return 1 if it succeeded, else 0
 */
int formatter_css_parse( formatter *f, const char *data, int len )
{
    return css_parse( data, len, f->properties, f->css_rules );
}
/**
 * Load the markup of a single file contents
 * @param f the formatter in question
 * @param mfunc the markup function for loading
 * @param data the data to use
 * @param len its length
 * @return 1 if it succeeded, else 0
 */
int formatter_load_markup( formatter *f, load_markup_func mfunc, 
    const char *data, int len )
{
    int res = (mfunc)( data, len, f->ranges, f->properties );
    if ( res )
        range_array_sort( f->ranges );
    return res;
}
/**
 * Make HTML using the already loaded markup and css data
 * @param f the formatter in question
 * @param text the text for format
 * @param len its length
 * @return 1 if it worked, else 0
 */
int formatter_make_html( formatter *f, const char *text, int len )
{
    int res = 0;
    f->tree = dom_create( text, len, f->ranges, f->css_rules, f->properties );
    if ( f->tree != NULL )
    {
        res = dom_build( f->tree );
    }
    return res;
}
/**
 * Save the formatted HTML to disk
 * @param f the formatter containing the formatted HTML
 * @param file the file to save it to
 * @return 1 if it succeeded, else 0
 */
int formatter_save_html( formatter *f, char *file )
{
    int t_len,res = 0;
    text_buf *tb;
    FILE *output = fopen( file, "w" );
    dom_print( f->tree );
    tb = dom_get_text_buf( f->tree );
    t_len = text_buf_len(tb);
    res = fwrite( text_buf_get_buf(tb), 1, t_len, output );
    return res == t_len;
}
/**
 * Get the HTML data as a string
 * @param f the formatter in question
 * @param VAR param for HTML length
 * @return the HTML or NULL
 */
char *formatter_get_html( formatter *f, int *len )
{
    dom_print( f->tree );
    text_buf *tb = dom_get_text_buf( f->tree );
    if ( tb != NULL )
    {
        *len = text_buf_len( tb );
        return text_buf_get_buf( tb );
    }
    else
        return NULL;
}
static int MIN( int a, int b )
{
    return (a<b)?a:b;
}
static int MAX( int a, int b )
{
    return (a>b)?a:b;
}
/**
 * How many characters overlap between ranges a and b?
 * @param a the first range
 * @param b the second range
 * @return the number of overlapping chars
 */
static int overlap( range *a, range *b )
{
    int overlap = MIN(range_end(a),range_end(b))
        -MAX(range_start(a),range_start(b));
    return (overlap>0)?overlap:0;
}
/**
 * Remove a set of ranges from the text. The ranges might overlap.
 * @param removals the array of removals
 * @param text the text to adjust
 * @param len its length on entry; its new length on exit
 * @return the modified text (not a copy)
 */
static char *remove_text( range_array *removals, char *text, int *len )
{
    int from = 0;
    int to = 0;
    int i;
    for ( i=0;i<range_array_size(removals);i++ )
    {
        range *r = range_array_get( removals, i );
        if ( from < range_start(r) )
        {
            int prefix_len = range_start(r)-from;
            if ( prefix_len > 0 )
            {
                if ( from > to )
                    memcpy( &text[to], &text[from], prefix_len );
                to += prefix_len;
            }
            from = range_end( r );
        }
    }
    // copy bit left over at end
    if ( from > to )
    {
        memcpy( &text[to], &text[from], *len-from );
        to += *len-from;
        *len = to;
        text[*len] = 0;
    }
    return text;
}
/*static void range_array_print( range_array *ra, int index )
{
    int i;
    int len = range_array_size(ra );
    printf("%d: ",len);
    for ( i=0;i<len;i++ )
    {
        range *r = range_array_get(ra,i);
        if ( i!=index )
            printf("[%s:%d-%d]",range_name(r),range_start(r),range_end(r) );
        else
            printf("{%s:%d-%d}",range_name(r),range_start(r),range_end(r) );
        if ( i != len-1 )
            printf(",");
    }
    printf("\n");
        
}*/
/**
 * Add the root range at the start of the range array
 * @param f the formatter to add it to
 * @param tlen the length of the text
 * @return 1 if it worked else 0
 */
static int formatter_add_root_range( formatter *f, int tlen )
{
    int res = 1;
    range *root = range_create( "root", NULL, 0, tlen );
    if ( root == NULL )
    {
        fprintf(stderr,"formatter: failed to create document root\n");
        formatter_dispose( f );
        res = 0;
    }
    else
    {
        range_array_insert( f->ranges, 0, root );
    }
    return res;
}
/**
 * Actually remove ranges and any overlapping parts of non-removed ranges.
 * @param f the formatter instance
 * @param text the text to update
 * @param len its length
 * @return 1 if it succeeded else 0
 */
static int formatter_remove_ranges( formatter *f, char *text, int *len )
{
    range_array *removals = range_array_create();
    if ( removals != NULL )
    {
        int i,j,N;
        for ( i=0;i<range_array_size(f->ranges);i++ )
        {
            range *r = range_array_get( f->ranges, i );
            if ( range_get_removed(r) )
                range_array_add( removals, range_copy(r) );
        }
        // now we have foreknowledge of all the removals
        // merge them wherever possible
        range_array_sort( removals );
        i = 0;
        // all ranges in removals must be separated by gaps
        while ( i < range_array_size(removals)-1 )
        {
            range *r = range_array_get( removals, i );
            range *s = range_array_get( removals, i+1 );
            if ( range_end(r)>=range_start(s) )
            {
                range_set_len( r,MAX(range_end(r),range_end(s))-range_start(r));
                range_array_remove( removals, i+1, 1 );
            }
            else
                i++;
        }
        N=range_array_size( removals );
        i = 0;
        //range_array_print( removals, -1 );
        while ( i < range_array_size(f->ranges) )
        {
            range *r = range_array_get(f->ranges,i);
            if ( !range_get_removed(r) )
            {
                // how much needs to be removed from r
                int removal = 0;
                // number of chars to move r left
                int move_left = 0;
                for ( j=0;j<N;j++ )
                {
                    range *q = range_array_get( removals, j );
                    if ( range_end(q) <= range_start(r) )
                        move_left += range_len(q);
                    else if ( range_start(q)<range_start(r) )
                        move_left += range_start(r)-range_start(q);
                    removal += overlap( q, r );
                }
                if ( removal == range_len(r) )
                    range_array_remove( f->ranges, i--, 1 );
                else if ( removal > 0 )
                    range_set_len( r, range_len(r)-removal );
                if ( move_left > 0 )
                    range_set_absolute( r, range_start(r)-move_left );
            }
            i++;
        }
        //range_array_print( f->ranges, i );
        text = remove_text( removals, text, len );
        range_array_dispose( removals, 1 );
        range_array_sort( f->ranges );
        // add root range
        return formatter_add_root_range( f, *len );
    }
    else
        return 0;
}
/**
 * Remove all ranges and portions of other ranges that overlap with them
 * @param f the formatter
 * @param text the text to erase
 * @param len the text's length, updated on exit
 * @return 1 for success else 0
 */
int formatter_cull_ranges( formatter *f, char *text, int *len )
{
    if ( !range_array_has_removed(f->ranges) )
        return formatter_add_root_range( f, *len );
    else
        return formatter_remove_ranges(f,text,len);
}
