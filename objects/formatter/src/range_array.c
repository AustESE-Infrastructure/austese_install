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
#include "hashmap.h"
#include "attribute.h"
#include "annotation.h"
#include "range.h"
#include "range_array.h"
#include "error.h"
#include "memwatch.h"
#define RANGE_BLOCK_SIZE 256
struct range_array_struct
{
    range **ranges;
    int num_ranges;
    int allocated;
    int has_removed;
};
/**
 * Create a range array instance
 * @return a completed range array or NULL
 */
range_array *range_array_create()
{
    range_array *ra = calloc( 1, sizeof(range_array));
    if ( ra != NULL )
    {
        ra->ranges = calloc( RANGE_BLOCK_SIZE, sizeof(range*) );
        if ( ra->ranges != NULL )
            ra->allocated = RANGE_BLOCK_SIZE;
        else
        {
            warning("range_array: failed to allocate array\n");
            range_array_dispose( ra, 0 );
            ra = NULL;
        }
    }
    else
    {
        warning("range_array: failed to allocate space\n");
        range_array_dispose( ra, 0 );
    }
    return ra;
}
/**
 * Dispose of an range array
 * @param ra the range array in question
 * @param dispose_contents if 1 then dispose of the contents also
 */
void range_array_dispose( range_array *ra, int dispose_contents )
{
    int i;
    if ( dispose_contents )
        for ( i=0;i<ra->num_ranges;i++ )
            range_dispose( ra->ranges[i] );
    free( ra->ranges );
    ra->ranges = NULL;
    free( ra );
}
/**
 * Does this range array contain removed ranges?
 * @param ra the range array
 * @return 1 if it does else 0
 */
int range_array_has_removed( range_array *ra )
{
    return ra->has_removed;
}
/**
 * Get the size of this range array
 * @param ra
 * @return 
 */
int range_array_size( range_array *ra )
{
    return ra->num_ranges;
}
/**
 * Return the array we are managing
 * @param ra the range array in question
 * @return its range array
 */
range **range_array_ranges( range_array *ra )
{
    return ra->ranges;
}
/**
 * Expand a range array so it can take more ranges
 * @param ra the range array in question
 * @return 1 if it worked, else 0
 */
static int range_array_expand( range_array *ra )
{
    if ( ra->allocated <= ra->num_ranges )
    {
        int new_size = ra->allocated+RANGE_BLOCK_SIZE;
        range **new_ranges = calloc( new_size,sizeof(range*));
        if ( new_ranges != NULL )
        {
            int i;
            for ( i=0;i<ra->num_ranges;i++ )
                new_ranges[i] = ra->ranges[i];
            free( ra->ranges);
            ra->ranges = new_ranges;
            ra->allocated = new_size;
            return 1;
        }
        else
        {
            warning("range_array: failed to reallocate array\n");
            return 0;
        }
    }
    else
        return 1;
}
/**
 * Add a range to the array
 * @param ra a range array instance
 * @param r the range to add
 * @return 1 if it worked, else 0
 */
int range_array_add( range_array *ra, range *r )
{
    if ( range_array_expand(ra) )
    {
        ra->ranges[ra->num_ranges++] = r;
        if ( range_get_removed(r) )
            ra->has_removed = 1;
        return 1;
    }
    else
        return 0;
}
/**
 * Insert a range in an array
 * @param ra a range array instance
 * @param r the range to add
 * @param at the index to add at
 * @return 1 if it worked, else 0
 */
int range_array_insert( range_array *ra, int at, range *r )
{
    if ( range_array_expand(ra) )
    {
        int i;
        // make room
        for ( i=ra->num_ranges;i>at;i-- )
            ra->ranges[i] = ra->ranges[i-1];
        ra->ranges[at] = r;
        if ( range_get_removed(r) )
            ra->has_removed = 1;
        ra->num_ranges++;
        return 1;
    }
    else
        return 0;
}
/**
 * Sort the ranges using shellsort for printing
 * @param d the dom in question
 */
void range_array_sort( range_array *ra )
{
    int i, j, k, h; range *v;
    int incs[16] = { 1391376, 463792, 198768, 86961, 33936,
        13776, 4592, 1968, 861, 336, 
        112, 48, 21, 7, 3, 1 };
    for ( k = 0; k < 16; k++)
    {
        for ( h=incs[k],i=h;i<=ra->num_ranges-1;i++ )
        { 
            v = ra->ranges[i]; 
            j = i;
            while (j >= h && range_compare(ra->ranges[j-h],v)>0 )
            { 
                ra->ranges[j] = ra->ranges[j-h]; 
                j -= h; 
            }
            ra->ranges[j] = v; 
        }
    }
    // reset reloff fields
    for ( k=0,i=0;i<ra->num_ranges;i++ )
    {
        range *r = ra->ranges[i];
        range_set_reloff( r, range_start(r)-k );
        k = range_start(r);
    }
}
/**
 * Get a particular range
 * @param ra the range array in question
 * @param i its index
 * @return the range
 */
range *range_array_get( range_array *ra, int i )
{
    return ra->ranges[i];
}
/**
 * Remove and optionally dispose of the given range
 * @param ra the range array
 * @param i index of the range to remove
 * @param dispose 1 if we are to dispose of the range
 */
void range_array_remove( range_array *ra, int i, int dispose )
{
    if ( i < ra->num_ranges )
    {
        if ( dispose )
            range_dispose( ra->ranges[i] );
        /*else
            printf("not removing range %lx\n",(unsigned long)ra->ranges[i]);*/
        while ( i < ra->num_ranges-1 )
        {
            ra->ranges[i] = ra->ranges[i+1];
            i++;
        }
        ra->num_ranges--;
    }
}
/**
 * Set the has_removed property of a range array
 * @param ra the range array in question
 * @param removed 1 or 0
 */
void range_array_set_removed( range_array *ra, int removed )
{
    ra->has_removed = removed;
}
/**
 * Update the array by eliminating all non-overlapping ranges
 * @param ra the array in question
 * @param r the new range that must overlap with everything in the array
 */
void range_array_update( range_array *ra, range *r )
{
    int i;
    for ( i=ra->num_ranges-1;i>=0;i-- )
    {
        if ( range_start(r)>=range_end(ra->ranges[i]) )
        {
            int j;
            for ( j=i;j<ra->num_ranges-1;j++ )
                ra->ranges[i] = ra->ranges[i+1];
            ra->num_ranges--;
        }
    }
}