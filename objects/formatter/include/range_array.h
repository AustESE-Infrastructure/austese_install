/* 
 * File:   range_array.h
 * Author: desmond
 *
 * Created on 17 September 2011, 1:20 PM
 */

#ifndef RANGE_ARRAY_H
#define	RANGE_ARRAY_H
#ifdef	__cplusplus
extern "C" {
#endif

typedef struct range_array_struct range_array;
range_array *range_array_create();
void range_array_dispose( range_array *ra, int dispose_contents );
int range_array_size( range_array *ra );
range **range_array_ranges( range_array *ra );
int range_array_add( range_array *ra, range *r );
int range_array_insert( range_array *ra, int at, range *r );
void range_array_sort( range_array *ra );
range *range_array_get( range_array *ra, int i );
int range_array_has_removed( range_array *ra );
void range_array_remove( range_array *ra, int i, int dispose );
void range_array_set_removed( range_array *ra, int removed );
#ifdef	__cplusplus
}
#endif
#endif	/* RANGE_ARRAY_H */

