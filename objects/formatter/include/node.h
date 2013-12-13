/*
 * This file is part of dom.
 *
 *  dom is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  dom is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with dom.  If not, see <http://www.gnu.org/licenses/>.
 *  (c) copyright Desmond Schmidt 2011
 * Created on 13 August 2011, 7:17 PM
 */

#ifndef NODE_H
#define	NODE_H

typedef struct node_struct node;
node *node_create( char *name, char *html_name, int offset, int len, 
     int empty, int rightmost );
void node_dispose( node *n );
void node_add_child( node *n, node *c );
void node_add_sibling( node *n, node *sibling );
void node_fit_sibling( node *n, node *r );
void node_insert_sibling( node *n, node *sibling );
node *node_align_sibling( node *n, node *r );
int node_has_children( node *n );
node *node_first_child( node *n );
int node_offset( node *n );
int node_len( node *n );
void node_split( node *n, int pos );
int node_end( node *n );
node *node_parent( node *n );
int node_has_next_sibling( node *n );
attribute *node_get_attribute( node *n, char *name );
node *node_next_sibling( node *n );
node *node_prec_sibling( node *n );
node *node_split_off_left( node *n, int rhs_start );
char *node_name( node *n );
char *node_html_name( node *n );
int node_precedes( node *n, node *m );
int node_follows( node *n, node *r );
int node_overlaps_on_left( node *r, node *n );
int node_overlaps_on_right( node *r, node *n );
int node_has_parent( node *n );
range *node_to_range( node *n );
void node_detach_sibling( node *n, node *prev );
void node_debug_check_siblings( node *first );
node *node_first( node *n );
void node_add_attribute( node *n, attribute *a );
void node_get_attributes( node *n, char *atts, int limit );
int node_empty( node *n );
int node_rightmost( node *n );
int node_is_root( node *n );
#endif	/* NODE_H */

