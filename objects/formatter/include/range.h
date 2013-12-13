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
 * Created on 10 August 2011, 5:58 AM
 */

#ifndef RANGE_H
#define	RANGE_H
#ifdef	__cplusplus
extern "C" {
#endif
typedef struct range_struct range;
int range_compare( void *key1, void *key2 );
range *range_create_atts( const char **atts );
range *range_copy( range *r );
range *range_create_empty();
range *range_create( char *name, char *html_name, int start, int len );
void range_dispose( range *r );
range **range_randomise( int n, int t_len, const char *text, int n_tags, 
    char **props, unsigned int seed );
int range_end( range *r );
int range_start( range *r );
char *range_name( range *r );
char *range_html_name( range *r );
int range_len( range *r );
void range_set_reloff( range *r, int reloff );
void range_set_len( range *r, int len );
int range_inside( range *r1, range *r2 );
int range_set_name( range *r, char *name );
int range_set_html_name( range *r, char *html_name );
int range_equals( range *r1, range *r2 );
void range_set_absolute( range *r, int absolute );
void range_set_rightmost( range *r, int rightmost );
int range_get_rightmost( range *r );
int range_get_reloff( range *r );
void range_add_annotation( range *r, annotation *a );
annotation *range_get_annotations( range *r );
int range_get_removed( range *r );
void range_set_removed( range *r, int removed );
int range_overlaps_left( range *r, range *q );
int range_overlaps_right( range *r, range *q );
range *range_split_delete( range *r, range *q );
#ifdef	__cplusplus
}
#endif
#endif	/* RANGE_H */

