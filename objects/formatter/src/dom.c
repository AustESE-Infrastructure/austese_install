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
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include "hashmap.h"
#include "attribute.h"
#include "annotation.h"
#include "range.h"
#include "node.h"
#include "text_buf.h"
#include "range_array.h"
#include "hashset.h"
#include "matrix.h"
#include "dom.h"
#include "queue.h"
#include "css_property.h"
#include "css_selector.h"
#include "css_rule.h"
#include "error.h"
#include "HTML.h"
#include "memwatch.h"

#define BUFLEN 1024
#define TEXT_BUF_SIZE 10000

/**
 * Represent a document object model to test the dom-building algorithm 
 */

struct dom_struct
{
    queue *q;
    text_buf *buf;
    matrix *pm;
    int text_len;
    const char *text;
    /** copy of css ranges used to build matrix */
    range_array *ranges;
    /** css rules indexed by class name */
    hashmap *css_rules;
    node *root;
};
static void dom_add_node( dom *d, node *n, node *r );
static void dom_range_inside_node( dom *d, node *n, node *r );

/**
 * Convert a raw array of properties to a real array of interleaved property 
 * names and their HTML equivalents. Only include those that are mentioned 
 * in the CSS file.
 * @param p_size VAR param the size of the property list (adjust)
 * @param array the raw array of property names
 * @param rules the css rules defined
 * @param props the destination array of properties, NULL-terminated. Caller 
 * must dispose!
 * @return 1 if it worked, else 0
 */
static int array_to_props( int *p_size, char **array, hashmap *rules, 
    char ***props )
{
    int i,j;
    int required = 0;
    // measure
    for ( i=0;i<*p_size;i++ )
    {
        if ( hashmap_contains(rules,array[i]) )
            required += 2;
    }
    *props = calloc( required+1, sizeof(char*) );
    if ( *props != NULL )
    {
        for ( j=0,i=0;i<*p_size;i++ )
        {
            if ( array[i] != NULL )
            {
                if ( hashmap_contains(rules,array[i]) )
                {
                    css_rule *r = hashmap_get(rules,array[i]);
                    (*props)[j++] = array[i];
                    (*props)[j++] = css_rule_get_element(r);
                }
            }
        }
        *p_size = j;
        return 1;
    }
    else
    {
        fprintf(stderr,"failed to allocate props p_size=%d\n",*p_size);
        return 0;
    }
}
/**
 * Store a range on the queue and a copy in the range array
 * @param d the dom object
 * @param r the range to add
 * @return 1 if successful
 */
static int dom_store_range( dom *d, range *r )
{
    int res = queue_push( d->q, r );
    if ( res )
        res = range_array_add( d->ranges,r );
    return res;
}
static int dom_filter_ranges( dom *d, range_array *ranges )
{
    // first range must be root
    range *root = range_array_get(ranges,0);
    if ( root != NULL )
    {
        if ( !queue_push(d->q,root) )
            return 0;
        else
        {
            d->ranges = range_array_create();
            if ( d->ranges != NULL )
            {
                int i,num_ranges = range_array_size( ranges );
                for ( i=1;i<num_ranges;i++ )
                {
                    range *r = range_array_get( ranges, i );
                    char *r_name = range_name( r );
                    hashset *pruned = matrix_get_lookup(d->pm);
                    if ( hashset_contains(pruned,r_name) )
                    {
                        css_rule *rule = hashmap_get( d->css_rules,r_name );
                        char *html_name = css_rule_get_element(rule);
                        if ( range_html_name(r) != NULL||
                            range_set_html_name(r,html_name) )
                        {
                            // this should duplicate the range
                            range *r_dup = range_copy( r );
                            if ( r_dup == NULL || !dom_store_range(d,r_dup) )
                            {
                                warning("dom: failed to push onto queue\n");
                                return 0;
                            }      
                        }
                        else
                        {
                            warning("dom: couldn't set html name for %s\n", 
                                r_name);
                            return 0;
                        }
                    }
                }
            }
            return 1;
        }
    }
    else
        return 0;
}
/**
 * Convert a range to a node
 * @param d the dom we are associated with
 * @param r the range to convert
 * @return the finished node
 */
static node *dom_range_to_node( dom *d, range *r )
{
    char *html_name = range_html_name(r);
    node *n = node_create( range_name(r),range_html_name(r),range_start(r),
        range_len(r), (html_name==NULL)?0:html_is_empty(html_name), 
        range_get_rightmost(r) );
    if ( n != NULL )
    {
        annotation *ann = range_get_annotations( r );
        while ( ann != NULL )
        {
            attribute *attr = annotation_to_attribute( ann, range_name(r), 
                d->css_rules );
            if ( attr != NULL )
                node_add_attribute( n, attr );
            ann = annotation_get_next( ann );
        }
    }
    return n;
}
/**
 * Create a dom instance representing a document tree
 * @param text the text to represent
 * @param len the length of the text
 * @param properties the set of all used property names
 * @param ranges an array of ranges read from the markup file(s)
 * @param rules a read-only hashmap of css_rules indexed by property name
 * @param properties the set of property names we are interested in
 * @return the constructed dom
 */
dom *dom_create( const char *text, int len, range_array *ranges,  
    hashmap *rules, hashset *properties )
{
    dom *d = calloc( 1, sizeof(dom));
    if ( d != NULL )
    {
        int p_size;
        char **array;
        d->text = text;
        d->text_len = len;
        d->css_rules = rules;
        p_size = hashset_size(properties);
        if ( p_size > 0 )
        {
            char **props;
            array = calloc( p_size, sizeof(char*) );
            if ( array == NULL )
            {
                warning("dom: failed to allocate properties array\n");
                dom_dispose( d );
                return NULL;
            }
            hashset_to_array( properties, array );
            if ( array_to_props(&p_size,array,rules,&props) )
            {
                free( array );
                array = NULL;
                d->pm = matrix_create( p_size, props );
                free( props );
                props = NULL;
                if ( d->pm == NULL )
                {
                    warning("dom: failed to create property matrix\n");
                    dom_dispose( d );
                    return NULL;
                }
                if ( range_array_size(ranges) > 0 )
                {
                    d->q = queue_create();
                    if ( d->q == NULL || !dom_filter_ranges(d,ranges) )
                    {
                        dom_dispose( d );
                        return NULL;
                    }
                    else
                    {
                        range *r = queue_pop( d->q );
                        d->root = dom_range_to_node( d, r );
                    }
                    d->buf = text_buf_create( (len*150)/100 );
                    if ( d->buf == NULL )
                    {
                        dom_dispose( d );
                        d = NULL;
                    }
                    else
                    {
                        range_array_sort( d->ranges );
                        matrix_init( d->pm, d->ranges );
                        matrix_update_html( d->pm );
                    }
                }
                else
                {
                    warning("dom: property list empty\n");
                    dom_dispose( d );
                    d = NULL;
                }
            }
            else
            {
                warning("dom: failed to build property array\n");
                dom_dispose( d );
                d = NULL;
            }
        }
        else
        {
            warning("dom: no properties specified. aborting.\n");
            dom_dispose( d );
            d = NULL;
        }
    }
    return d;
}
/**
 * Dispose of the whole tree recursively. node_dispose just kills one node
 * @param n the node to start from
 */
static void dom_dispose_node( node *n )
{
    node *next = node_next_sibling( n );
    node *child = node_first_child( n );
    node_dispose( n );
    if ( next != NULL )
        dom_dispose_node( next );
    if ( child != NULL )
        dom_dispose_node( child );
}
/**
 * Dispose of the dom
 * @param d the dom in question
 */
void dom_dispose( dom *d )
{
    if ( d->ranges != NULL )
        range_array_dispose( d->ranges, 1 );
    if ( d->root != NULL )
        dom_dispose_node( d->root );
    if ( d->pm != NULL )
        matrix_dispose( d->pm );
    if ( d->buf != NULL  )
        text_buf_dispose( d->buf );
    if ( d->q != NULL )
        queue_dispose( d->q );
    // rules are read-only from formatter
    free( d );
}
/**
 * Build the dom
 * @param d the dom object to build
 */
int dom_build( dom *d )
{
    int res = 1;
    while ( !queue_empty(d->q) )
    {
        range *rx = queue_pop( d->q );
        node *r = dom_range_to_node( d, rx );
        if ( r != NULL )
        {
            if ( node_end(r) <= d->text_len )
                dom_add_node( d, d->root, r );
            else
            {
                fprintf(stderr,"node range %d:%d > text length (%d)\n",
                    node_offset(r),node_end(r), d->text_len );
                node_dispose( r );
                res = 0;
                break;
            }
        }
    }
    //matrix_dump( d->pm );
    return res;
}
/**
 * Concatenate a formatted string onto the buffer for printing
 * @param d the dom in question
 * @param format the format of the data
 * @param len the number of bytes to copy
 */
 
static void dom_concat( dom *d, const char *format, int len, ... )
{
    char *temp = malloc( len+1 );
    if ( temp != NULL )
    {
        va_list args;
        va_start( args, len );
        int res = vsnprintf( temp, len+1, format, args );
        if ( res == len )
            text_buf_concat( d->buf, temp, len );
        else
            warning("dom: failed to print %d bytes to output\n",len);
        free( temp );
        va_end( args );
    }
}
/**
 * Add some text to the buffer
 * @param d the dom whose buffer will be extended
 * @param offset the offset to start printing at
 * @param len the length of the text to print
 */
static void dom_print_text( dom *d, int offset, int len )
{
    char *copy = malloc(len+1);
    if ( copy != NULL )
    {
        memcpy( copy, &d->text[offset], len );
        copy[len] = 0;
        dom_concat( d, "%s", len, copy );
        free( copy );
    }
    else
        warning("dom: failed to allocate string for printing\n");
}
/**
 * Print a single node and its children, siblings
 * @param d the dom in question
 * @param n the node to print
 */
static void dom_print_node( dom *d, node *n )
{
	node *c;
    int start,end;
    char *html_name = node_html_name(n);
    char *class_name = node_name(n);
    char attrs[128];
    node_get_attributes( n, attrs, 128 );
    if ( !node_empty(n) )
    {
        if ( !node_is_root(n) )
            dom_concat( d, "<%s%s class=\"%s\">", strlen(html_name)
                +strlen(class_name)+strlen(attrs)+11, html_name, 
                attrs, class_name );
    }
    c = node_first_child(n);
    start = node_offset(n);
    end = node_end(n);
    while ( c != NULL )
    {
        int pos = node_offset( c );
        if ( pos > start )
            dom_print_text( d, start, pos-start );
        dom_print_node( d, c );
        start = node_end( c );
        c = node_next_sibling( c );
    }
    if ( end > start )
        dom_print_text( d, start, end-start );
    if ( !node_is_root(n) )
    {
        if ( !node_empty(n) )
            dom_concat(d, "</%s>",strlen(html_name)+3, html_name);
        else if ( node_rightmost(n) )
            dom_concat(d,"<%s>",strlen(html_name)+2,html_name);
    }
}
/**
 * Print the entire tree
 * @param d the tree to print
 */
void dom_print( dom *d )
{
    dom_print_node( d, d->root );
}
/**
 * Does the range (now a node) properly contain the node?
 * @param n the node already in the tree
 * @param r a loose node looking for a home
 * @return 1 if r encloses n by a greater range
 */
static int range_encloses_node( node *n, node *r )
{
    int r_end = node_end(r);
    int n_end = node_end(n);
    if ( node_offset(n) == node_offset(r) && r_end==n_end )
        return 0;
    else
        return node_offset(r)<=node_offset(n) && r_end>=n_end;
}
/**
 * Does the node properly contain the range (now a node)?
 * @param n the node already in the tree
 * @param r a loose node looking for a home
 * @return 1 if n encloses r by a greater range
 */
static int node_encloses_range( node *n, node *r )
{
    int r_end = node_end(r);
    int n_end = node_end(n);
    if ( node_offset(n) == node_offset(r) && r_end==n_end )
        return 0;
    else
        return node_offset(n)<=node_offset(r) && n_end>=r_end;
}
/**
 * Is one property more often inside another than vice versa?
 * @param d the dom in question
 * @param n_name the inside property name
 * @param r_name the outside property name
 * @return 1 if it's true else 0
 */
static int dom_mostly_nests( dom *d, char *n_name, char *r_name )
{
    int nInR = matrix_inside( d->pm, n_name, r_name );
    int rInN = matrix_inside( d->pm, r_name, n_name );
    return nInR >= rInN;
}
/**
 * May one property nest inside another at all?
 * @param d the dom in question
 * @param name1 the first property name
 * @param name2 the second property name
 * @return 1 if nesting of name1 inside name2 is possible, else 0
 */
static int dom_nests( dom *d, char *name1, char *name2 )
{
    return matrix_inside( d->pm, name1, name2 ) > 0;
}
/**
 * Try to add r as a child to n, already in the tree
 * @param n the tree-node
 * @param r the new r-node
 */
static void dom_make_child( dom *d, node *n, node *r )
{
    if ( node_has_children(n) )
        dom_add_node(d,node_first_child(n),r );
    else
        node_add_child( n, r );
}
/**
 * Is an unattached node equal to a dom-attached node
 * @param n the node in the tree
 * @param r the unattached node aka range
 * @return 1 if they have the same range
 */
static int range_equals_node( node *n, node *r )
{
    return node_offset(n)==node_offset(r) && node_len(n)==node_len(r);
}
/**
 * Try to make the new node into a parent of the tree-node n. The problem 
 * here is that we must include any siblings of n in r if they fit.
 * @param n the node above which to add the parent
 * @param r the new unattached node 
 */
static void dom_make_parent( dom *d, node *n, node *r )
{
    node *parent = node_parent(n);
    node *prev = node_prec_sibling( n );
    if ( parent==NULL )
        printf("parent is NULL\n");
    //fprintf( stderr,"n: %s %d:%d; r %s %d:%d\n",node_name(n),node_offset(n),
    //    node_end(n),node_name(r),node_offset(r),node_end(r));
    //node_debug_check_siblings( node_first_child(parent) );
    while ( n != NULL && !node_follows(r,n) )
    {
        node *next = node_next_sibling(n);
        if ( dom_nests(d,node_name(n),node_name(r)) )
        {
            if ( range_encloses_node(n,r) || range_equals_node(n,r) )
            {
                node_detach_sibling( n, prev );
                node_add_child( r, n );
                if ( node_overlaps_on_right(parent,r) )
                {
                    node_split( r, node_end(parent) );
                    node *r2 = node_next_sibling( r );
                    node_detach_sibling( r, NULL );
                    dom_store_range( d, node_to_range(r2) );
                    node_dispose( r2 );
                }
            }
            else if ( node_overlaps_on_left(n,r) )
            {
                node_split( n, node_end(r) );
                node_detach_sibling( n, prev );
                node_add_child( r, n );
                break;
            }
            else
                break;
        }
        else 
        {
            // split off the rest of r and and push it back
            // Q: what happens to r??
            node *r2;
            node_split( r, node_offset(n) );
            r2 = node_next_sibling( r );
            node_detach_sibling( r, NULL );
            dom_store_range( d, node_to_range(r2) );
            //queue_push( d->q, node_to_range(r2) );
            node_dispose( r2 );
            break;
        }
        n = next;
        if ( n != NULL )
            prev = node_prec_sibling( n );
    }
    // make n's original parent the parent of r
    node_add_child( parent, r );
   // node_debug_check_siblings( node_first_child(parent) );
}
/**
 * Write to the console details of the dropped node
 * @param d the dom in question
 * @param r the node we are dropping
 * @param n the parent node
 */
static void dom_drop_notify( dom *d, node *r, node *n )
{
    warning("dom: dropping %s at %d:%d - %s and %s incompatible\n",
        node_name(r),node_offset(r),
        node_end(r),node_html_name(r),node_html_name(n));
    attribute *id = node_get_attribute( r, "id" );
    if ( id != NULL )
    {
        char *value = attribute_get_value( id );
        if ( value[strlen(value)-1]=='b' )
            printf( "aha! dropping id %s\n",value );
    }
    node_dispose( r );
}
/**
 * Handle the case where node and range are equal
 * @param d the dom in question
 * @param n the tree-node already there
 * @param r the interloper barging in
 */
static void dom_node_equals( dom *d, node *n, node *r )
{
    if ( dom_mostly_nests(d,node_name(r),node_name(n)) )
        dom_make_child( d, n, r );
    else if ( dom_nests(d,node_name(n),node_name(r)) )
        dom_make_parent( d, n, r );
    else 
        dom_drop_notify( d, r, n );
}
/**
 * Split a node into 3 parts to accommodate a range
 * @param d the dom in question
 * @param n the node in the tree to split up
 * @param r the out of tree node r that overlaps with n
 */
static void dom_breakup_node( dom *d, node *n, node *r )
{
    node *n2;
    if ( node_offset(r) > node_offset(n) )
    {
        node_split( n, node_offset(r) );
        n2 = node_next_sibling( n );
    }
    else
        n2 = n;
    if ( node_end(r) < node_end(n2) )
        node_split( n2, node_end(r) );
    dom_node_equals( d, n2, r );
}
/**
 * Split a range into 3 parts to accommodate a tree-node
 * @param d the dom in question
 * @param n the node in the tree
 * @param r the out of tree range r to split up
 */
static void dom_breakup_range( dom *d, node *n, node *r )
{
    node *r2;
    if ( node_offset(n) > node_offset(r) )
    {
        node_split( r, node_offset(n) );
        r2 = node_next_sibling( r );
        node_detach_sibling( r, NULL );
        dom_store_range( d, node_to_range(r) );
        //queue_push( d->q, node_to_range(r) );
        node_dispose( r );
    }
    else
        r2 = r;
    if ( node_end(r2)>node_end(n) )
    {
        node *r3;
        node_split( r2, node_end(n) );
        r3 = node_next_sibling(r2);
        node_detach_sibling(r3,r2);
        dom_store_range( d, node_to_range(r3) );
        //queue_push( d->q, node_to_range(r3) );
        node_dispose(r3);
    }
    dom_node_equals( d, n, r2 );
}
/**
 * If a range is inside a node's child call dom_range_inside_node again
 * @param dom the dom object
 * @param n the node already in the tree and possibly with child
 * @param r the range inside n but also maybe inside n's child
 * @return 1 if this was the case
 */
int dom_range_inside_node_child( dom *d, node *n, node *r )
{
    node *child = node_first_child(n);
    while ( child != NULL )
    {
        if ( node_encloses_range(child,r) )
        {
            dom_range_inside_node(d,child,r);
            return 1;
        }
        else if ( range_equals_node(child,r)
            && !dom_nests(d,node_name(r),node_name(n))
            && dom_nests(d,node_name(r),node_name(child)) )
        {
            dom_range_inside_node(d,child,r);
            return 1;
        }
        child = node_next_sibling(child);
    }
    return 0;
}
/**
 * Handle the case when the range is enclosed by the node
 * @param d the dom in question
 * @param n the node in question
 * @param r the range as a node to compare with n
 */
static void dom_range_inside_node( dom *d, node *n, node *r )
{
    if ( !dom_range_inside_node_child(d,n,r) )
    {
        if ( dom_nests(d, node_name(r),node_name(n)) )
            dom_make_child( d, n, r );
        else if ( dom_nests(d,node_name(n),node_name(r)) )
            dom_breakup_node( d, n, r );
        else    // neither fits inside the other
            dom_drop_notify( d, r, n);
    }
}
/**
 * Handle the case where the node is properly inside the range
 * @param d the dom in question
 * @param n the node we are comparing the range to
 * @param r the (ex-)range in question
 */
static void dom_node_inside_range( dom *d, node *n, node *r )
{
    if ( dom_nests(d,node_name(n),node_name(r)) )
        dom_make_parent( d, n, r );
    else if ( dom_nests(d,node_name(r),node_name(n)) )
        dom_breakup_range( d, n, r );
    else    // neither fits inside the other
        dom_drop_notify(d, r, n );
}
/**
 * Handle overlap on the left of a tree-node
 * @param d the dom in question
 * @param n the node to test against
 * @param r the rogue who overlaps on the left
 */
static void dom_range_overlaps_left( dom *d, node *n, node *r )
{
    if ( dom_mostly_nests(d,node_name(n),node_name(r)) )
    {
        node_split( n, node_end(r) );
        dom_add_node( d, n, r );
    }
    else if ( dom_mostly_nests(d,node_name(r),node_name(n)) )
    {
        node *r2;
        node_split( r, node_offset(n) );
        r2 = node_next_sibling(r);
        node_detach_sibling( r2, r );
        dom_store_range( d, node_to_range(r) );
        //queue_push( d->q, node_to_range(r) );
        node_dispose( r );
        dom_add_node( d, n, r2 );
    }
    else
        dom_drop_notify( d, r, n );
}
/**
 * Handle overlap on the right of a tree-node
 * @param d the dom in question
 * @param n the node to test against
 * @param r the rogue who overlaps on the right
 */
static void dom_range_overlaps_right( dom *d, node *n, node *r )
{
    if ( dom_mostly_nests(d,node_name(n),node_name(r)) )
    {
        node_split( n, node_offset(r) );
        dom_add_node( d, node_next_sibling(n), r );
    }
    else if ( dom_mostly_nests(d,node_name(r),node_name(n)) )
    {
        node *r2;
        node_split( r, node_end(n) );
        r2 = node_next_sibling(r);
        node_detach_sibling( r, NULL );
        dom_store_range( d, node_to_range(r2) );
        //queue_push( d->q, node_to_range(r2) );
        node_dispose( r2 );
        dom_add_node( d, n, r );
    }
    else
        dom_drop_notify( d, r, n );
}
/**
 * Add a range before a given node
 * @param d the dom in question
 * @param n the node before which we are being tested
 * @param r the former range, as a new node to compare with n
 */
static void dom_add_node( dom *d, node *n, node *r )
{
    n = node_align_sibling( n, r );
    if ( n == NULL )
        return;
    else if ( node_encloses_range(n,r) )
    {
        dom_range_inside_node(d,n,r);
    }
    else if ( range_encloses_node(n,r) )
    {
        dom_node_inside_range( d, n, r );
    }
    else if ( node_overlaps_on_right(n,r) )
    {
        dom_range_overlaps_right( d, n, r );
    }
    else if ( node_overlaps_on_left(n,r) )
    {
        dom_range_overlaps_left( d, n, r );
    }
    else
    {
        dom_node_equals( d, n, r );
    }
    //dom_check_tree( d );
}
/**
 * Debug routine: check output to see if well-formed
 * @param dom the dom in question
 */
void dom_check_output( dom *d )
{
    int len = text_buf_len( d->buf );
    char *output = text_buf_get_buf( d->buf );
    int i,state = 0;
    int pos = 0;
    for ( i=0;i<len;i++ )
    {
        switch ( state )
        {
            case 0: // reading text
                if ( output[i] == '<' )
                    state = 1;
                else if ( pos >= d->text_len || output[i] != d->text[pos] )
                {
                    state = -1;
                    warning("mismatch at character %d\n",pos);
                }
                else
                    pos++;
                break;
            case 1: // reading tag
                if ( output[i] == '>' )
                    state = 0;
                break;
        }
        if ( state == -1 )
            break;
    }
}
/**
 * Get a dom's textbuf object
 * @param d the dom in question
 * @return a text buf object
 */
text_buf *dom_get_text_buf( dom *d )
{
    return d->buf;
}
/**
 * Check a single tree-node, recursively
 */
static int dom_check_node( node *n )
{
    int res = 1;
    int start = node_offset(n);
    int end = node_end(n);
    node *c = node_first_child(n);
    node *prev = NULL;
    while ( c != NULL )
    {
        node *next = node_next_sibling( c );
        if ( node_offset(c)<start )
        {
            warning("dom: invalid offset %d < parent start %d\n",node_offset(c),
                start);
            return 0;
        }
        else if ( node_end(c)>end )
        {
            warning("dom: invalid end %d (%s) > parent end %d (%s)\n",
                node_end(c), node_name(c), end, node_name(n) );
            return 0;
        }
        else if ( prev != NULL && node_end(prev)>node_offset(c) )
        {
            warning("dom: prev node ending %d encroaches on child node at %d\n",
                node_end(prev), node_offset(c));
            return 0;
        }
        else if ( next != NULL && node_end(c)>node_offset(next) )
        {
            warning("dom: next node starting %d encroaches on child node ending at %d\n",
                node_offset(next), node_end(c));
            return 0;
        }
        else
            res = dom_check_node( c );
        prev = c;
        c = node_next_sibling( c );
    }
    return res;
}
/**
 * Check that the tree as constructed meets nesting criteria
 * @param d the document object model
 * @return 1 if it was OK, 0 otherwise
 */
int dom_check_tree( dom *d )
{
    return dom_check_node( d->root );
}
