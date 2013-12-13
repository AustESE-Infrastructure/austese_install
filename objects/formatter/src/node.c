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
#include <string.h>
#include <stdio.h>
#include "hashmap.h"
#include "attribute.h"
#include "annotation.h"
#include "range.h"
#include "node.h"
#include "error.h"
#include "HTML.h"
#include "memwatch.h"
struct node_struct
{
	char *name;
    char *html_name;
	int offset;
	int len;
    int empty;
    int rightmost;
    node *parent;
	node *next;
	node *children;
    attribute *attrs;
};
/**
 * Create a node instance
 * @param name the name of the node
 * @param name of html tag
 * @param offset the offset where the range of the node starts
 * @param len the node's range's length
 * @param rightmost is this the rightmost in a series of split nodes?
 * @param empty 1 if the html element is empty
 * @return the newly formed node
 */
node *node_create( char *name, char *html_name, int offset, int len, 
    int empty, int rightmost )
{
    node *n = calloc( 1, sizeof(node) );
	if ( n != NULL )
    {
        //unsigned long u = 0x10019ecf0;
        //if ( (unsigned long) n == u )
        //    printf("0x10019ecf0\n");
        n->name = strdup( name );
        n->html_name = (html_name==NULL)?NULL:strdup( html_name );
        n->offset = offset;
        n->len = len;
        n->empty = empty;
        n->rightmost = rightmost;
        if ( n->empty > 1 )
            printf("empty>1\n");
        if ( len == 0 )
        {
            warning("Length must not be 0\n");
            node_dispose( n );
            n = NULL;
        }
    }
	return n;
}
/**
 * Just dispose of the node itself
 * @param n the node in question
 */
void node_dispose( node *n )
{
    if ( n->name != NULL )
    {
        free( n->name );
        n->name = NULL;
    }
    if ( n->html_name != NULL )
    {
        free( n->html_name );
        n->html_name = NULL;
    }
    if ( n->attrs != NULL )
        attribute_dispose( n->attrs );
    free( n );
}
/**
 * Is this node already placed in the tree?
 * @param n the node to test
 * @return 1 if it has a parent else 0
 */
 
int node_has_parent( node *n )
{
    return n->parent != NULL;
}
/**
 * Get the named attribute
 * @param n the node to get the attribute from
 * @param name the name of the attribute
 * @return NULL if not found or the attribute
 */
attribute *node_get_attribute( node *n, char *name )
{
    attribute *a = n->attrs;
    while ( a != NULL )
    {
        if ( strcmp(attribute_get_name(a),name)==0 )
            return a;
        else
            a = attribute_get_next( a );
    }
    return NULL;
}
/**
 * Get the first sibling in a list
 * @param n a node in the sibling list
 * @return the first sibling
 */
node *node_first( node *n )
{
    if ( n->parent != NULL )
        return n->parent->children;
    else
    {
        if ( strcmp(node_name(n),"root")!=0 )
            warning("node: attempt to access empty parent\n");
        return n;
    }
}
/**
 * Check that a list of siblings does not overlap and is in order
 * @param first the first sibling
 */
void node_debug_check_siblings( node *first )
{
    //printf("siblings: ");
    while ( first->next != NULL )
    {
        if ( node_end(first)>node_offset(first->next) )
            printf("node: siblings not sorted\n");
        //printf("%s %d:%d ",first->name,first->offset,node_end(first));
        first = first->next;
    }
    //printf("%s %d:%d ",first->name,first->offset,node_end(first));
    //printf("\n");
}
/**
 * Find the correct location of a node among a list of siblings
 * @param n an existing sibling node
 * @param r the new sibling to add in the correct location
 */
void node_add_sibling( node *n, node *r )
{
    node *prev = NULL;
    node *parent = n->parent;
    node *orig = n = node_first( n );
    // find correct location
    while ( n != NULL && node_follows(n,r) )
    {
        prev = n;
        n = n->next;
    }
    // so r does not follow n or n is NULL
    if ( n == NULL )
    {
        // insert after prev
        r->next = NULL;
        prev->next = r;
    }
    else // r precedes or overlaps n
    {
        if ( node_precedes(n,r) )
        {
            // insert r before n
            r->next = n;
            if ( prev == NULL )
                parent->children = r;
            if ( prev != NULL )
                prev->next = r;
        }
        else // overlaps!!
            warning("node: sibling %s %d:%d overlaps %s %d:%d\n",
                n->name,n->offset,node_end(n),
                r->name,r->offset,node_end(r));
    }
    r->parent = parent;
    // debug check
    node_debug_check_siblings( node_first(orig) );
}
/**
 * Check that an individual node is sane
 * @param n the node to check
 */
void node_check( node *n )
{
    int left = n->offset;
    int right = n->offset+n->len;
    node *c = n->children;
    if ( c != NULL )
    {
        if ( c->offset < left )
            warning("node: child precedes parent\n");
        while ( c->next != NULL )
            c = c->next;
        if ( c->offset+c->len > right )
            warning("node: child extends beyond right of parent\n");
    }
}
/**
 * Append a child to the children of n in order
 * @param n the node to add the child to
 * @param c the child node all ready to go
 */
void node_add_child( node *n, node *c )
{
    if ( n->children == NULL )
		n->children = c;
	else
		node_add_sibling( n->children, c );
    c->parent = n;
    //node_check( n );
}
/**
 * Separate this node from its next sibling
 * @param n the node to sever from its siblings and parent
 * @param prev the previous node in the list or NULL
 */
void node_detach_sibling( node *n, node *prev )
{
    node *next = n->next;
    n->next = NULL;
    if ( prev != NULL )
    {
        if ( prev->next != n )
            warning("node: invalid detachment!\n");
        prev->next = next;
    }
    // check that if this is the first child of the parent 
    // so that the children pointer gets updated
    if ( n->parent != NULL && n->parent->children == n )
        n->parent->children = next;
    n->parent = NULL;
    /*if ( prev != NULL && prev->parent != NULL )
        node_check(prev->parent);
    if ( next != NULL && next->parent != NULL )
        node_check(next->parent);*/
}
/**
 * Align r against the correct sibling and insert it in the list if it 
 * doesn't overlap anything
 * @param n the node to start with
 * @param r the new node
 * @return NULL if you added it, else the node it aligns with
 */
node *node_align_sibling( node *n, node *r )
{
    node *prev;
    n = node_first( n );
    while ( n != NULL && node_follows(n,r) )
    {
        prev = n;
        n = n->next;
    }
    // found nothing that overlaps with us
    if ( n == NULL )
    {
        prev->next = r;
        r->parent = prev->parent;
        return NULL;
    }
    else if ( node_precedes(n,r) )
    {
        node_add_sibling(n,r);
        //if ( n->parent != NULL )
        //    node_check(n->parent);
        return NULL;
    }
    else    // overlaps
    {
        return n;
    }
}
/**
 * Does this node have any children?
 * @param n the node in question
 * @return 1 if it has else 0
 */
int node_has_children( node *n )
{
    return n->children != NULL;
}
/**
 * Get the first child of the node
 * @param n the node in question
 * @return the first child (may be NULL)
 */
node *node_first_child( node *n )
{
    return n->children;
}
/**
 * Get the offset of the node's range
 * @param n the node in question
 * @return the offset
 */
int node_offset( node *n )
{
    return n->offset;
}
/**
 * Get the length of the node's range
 * @param n the node in question
 * @return the offset
 */
int node_len( node *n )
{
    return n->len;
}
/**
 * Get the end offset of the first character after n
 * @param n the node in question
 * @return the end
 */
int node_end( node *n )
{
    return n->offset+n->len;
}
/**
 * Split a node into two sibling nodes
 * @param n the node to split
 * @param pos the location where to split
 */
void node_split( node *n, int pos )
{
    node *next = node_create( n->name, n->html_name, pos, node_end(n)-pos,
        html_is_empty(n->html_name), n->rightmost );
    attribute *attr = n->attrs;
    while ( attr != NULL )
    {
        attribute *clone = attribute_clone(attr);
        if ( clone != NULL )
        {
            node_add_attribute( next, clone );
            attr = attribute_get_next( attr );
        }
        else
            fprintf(stderr,"node: failed to clone attribute\n");
    }
    // insert next into the sibling list
    n->len = pos-n->offset;
    next->next = n->next;
    n->next = next;
    // we can't be rightmost any more
    n->rightmost = 0;
    next->parent = n->parent;
    // now go through the children of n moving them into next
    node *c = n->children;
    node *prev = NULL;
    while ( c != NULL )
    {
        if ( node_overlaps_on_right(n,c) )
        {
            node *c2;
            node_split( c, node_end(n) );
            c2 = c->next;
            node_detach_sibling( c2, c );
            node_add_child( next, c2 );
            /*node_check(n);
            node_check(next);*/
            prev = c;
            c = c->next;
        }
        else if ( node_follows(n,c) )
        {
            node *following = c->next;
            node_detach_sibling( c, prev );
            node_add_child( next, c );
            c = following;
            /*node_check(n);
            node_check(next);*/
        }
        else 
        {
            prev = c;
            c = c->next;
        }
    }
    /*node_check( n );
    node_check( next );*/
}
/**
 * Get the parent of this node
 * @param n the node in question
 * @return the parent or NULL
 */
node *node_parent( node *n )
{
    return n->parent;
}
/**
 * Does this node have a next sibling?
 * @param n the node in question
 * @return 1 if it has a next sibling
 */
int node_has_next_sibling( node *n )
{
    return n->next != NULL;
}
/**
 * Get the next sibling of n
 * @param n the node in question
 * @return the next node or NULL
 */
node *node_next_sibling( node *n )
{
    return n->next;
}
/**
 * Because we don't keep pointers to preceding nodes we have to search for it
 * @param n the node to get the preceding node of
 * @return the preceding node or NULL
 */
node *node_prec_sibling( node *n )
{
    node *parent = n->parent;
    if ( parent != NULL )
    {
        node *c = parent->children;
        while ( c != NULL )
        {
            if ( c->next == n )
                return c;
            else
                c = c->next;
        }
        return NULL;
    }
    else
        return NULL;
}
/**
 * Get a node's name
 * @param n the node in question
 * @return the name of the node
 */
char *node_name( node *n )
{
    return n->name;
}
/**
 * Get a node's html tag name
 * @param n the node in question
 * @return the name of the node
 */
char *node_html_name( node *n )
{
    return n->html_name;
}
/**
 * Does the new node precede the in-tree node?
 * @param n the in-tree node
 * @param r the new node
 * @return 1 if it precedes, else 0
 */
int node_precedes( node *n, node *r )
{
    return node_end(r)<=node_offset(n);
}
/**
 * Does the new node follow the current tree-node?
 * @param n the tree-node
 * @param r the new node
 * @return 1 if it is entirely to the right else 0
 */
int node_follows( node *n, node *r )
{
    return node_end(n)<=node_offset(r);
}
/**
 * Does the new node overlap on the left of the in-tree node
 * @param r the new node
 * @param n the in-tree node
 * @return 1 if it does overlap on the left, else 0
 */
int node_overlaps_on_left( node *n, node *r )
{
    int r_end = node_end(r);
    int n_end = node_end(n);
    return (r_end<=n_end&&r_end>n->offset&&r->offset<n->offset);
}
/**
 * Does the new node overlap on the right of the in-tree node
 * @param r the new node
 * @param n the in-tree node
 * @return 1 if it does overlap on the right, else 0
 */
int node_overlaps_on_right( node *n, node *r )
{
    int r_end = node_end(r);
    int n_end = node_end(n);
    return (n->offset<=r->offset&&n_end>r->offset&&r_end>n_end);
}
/**
 * Convert a node back to a range
 * @param n the node in question
 * @return a range covering the same span
 */
range *node_to_range( node *n )
{
    range *r = range_create( node_name(n), node_html_name(n), node_offset(n), 
        node_len(n) ); 
    range_set_rightmost( r, n->rightmost );
    attribute *attr = n->attrs;
    while ( attr != NULL )
    {
        annotation *a = annotation_create_simple( 
            attribute_prop_name(attr),
            attribute_get_value(attr) );
        if ( a != NULL )
            range_add_annotation( r, a );
        attr = attribute_get_next( attr );
    }
    return r;
}
/**
 * Add an attribute to a node
 * @param n the node in question
 * @param a the attribute to add
 */
void node_add_attribute( node *n, attribute *a )
{
    if ( n->attrs == NULL )
        n->attrs = a;
    else
        attribute_append( n->attrs, a );
}
/**
 * Get the attributes of a node for writing out
 * @param n the node in question
 * @param atts its attributes
 * @param limit the limit on the length of atts
 */
void node_get_attributes( node *n, char *atts, int limit )
{
    attribute *temp = n->attrs;
    int pos = 0;
    atts[0] = 0;
    while ( temp != NULL )
    {
        char *name = attribute_get_name( temp );
        char *value = attribute_get_value( temp );
        if ( strlen(name)+strlen(value)+6+pos < limit )
        {
            pos += snprintf( &atts[pos],limit-pos," %s=\"%s\"",name,value );
            temp = attribute_get_next( temp );
        }
        else
            break;
    }
}
/**
 * Is this an empty node?
 * @param n the node in question
 * @return 
 */
int node_empty( node *n )
{
    return n->empty;
}
/**
 * Is this the fabled root node?
 * @param n the node in question
 * @return 1 if it is else 0
 */
int node_is_root( node *n )
{
    return n->parent == NULL;
}
/**
 * Is this node rightmost?
 * @param n the node in question
 * @return 1 if it is rightmost in a series of split nodes
 */
int node_rightmost( node *n )
{
    return n->rightmost;
}