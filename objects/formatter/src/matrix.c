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
#include "attribute.h"
#include "hashmap.h"
#include "annotation.h"
#include "range.h"
#include "range_array.h"
#include "hashset.h"
#include "matrix.h"
#include "matrix_queue.h"
#include "HTML.h"
#include "error.h"
#include "memwatch.h"

/** Record which properties may nest inside which other properties */
struct matrix_struct
{
    int n_props;
    int inited;
    hashset *lookup;
    int *cells;
    char **html_tags;
    char **names;
};
/**
 * 
 * @param n_props number of properties (html_tags+property-names)
 * @param properties array of props and html tag names alternately
 * @return an initialised matrix
 */
matrix *matrix_create( int n_props, char **properties )
{
    matrix *m = calloc(1,sizeof(matrix) );
    if ( m != NULL )
    {
        m->n_props = n_props/2;
        m->names = calloc( m->n_props, sizeof(char*) );
        m->html_tags = calloc( m->n_props, sizeof(char*) );
        if ( m->names != NULL && m->html_tags != NULL )
        {
            int i,j;
            for ( j=0,i=0;i<n_props-1;j++,i+=2 )
            {
                m->names[j] = strdup(properties[i]);
                //printf("%s\n",properties[i]);
                m->html_tags[j] = (properties[i+1]==NULL)
                    ?NULL:strdup(properties[i+1]);
            }
            m->cells = (int*)calloc( m->n_props*m->n_props, sizeof(int) );
            if ( m->cells == NULL )
            {
                matrix_dispose( m );
                m = NULL;
                warning("failed to allocate %dx%d matrix\n",n_props,n_props);
            }
            else
            {
                m->lookup = hashset_create();
                if ( m->lookup == NULL )
                {
                    matrix_dispose( m );
                    m = NULL;
                }
                else
                {
                    int k;
                    for ( k=0;k<m->n_props;k++ )
                    {
                        hashset_put( m->lookup, m->names[k] );
                    }
                }
            }
        }
        else
        {
            warning("matrix: failed to allocate names and tags\n");
            matrix_dispose( m );
            m = NULL;
        }
    }
    return m;
}
/**
 * Destroy the matrix's memory
 * @param m the matrix in question
 */
void matrix_dispose( matrix *m )
{
    //matrix_dump( m );
    if ( m->lookup != NULL )
        hashset_dispose( m->lookup );
    if ( m->cells != NULL )
    {
        free( m->cells );
        m->cells = NULL;
    }
    if ( m->names != NULL )
    {
        int i;
        for ( i=0;i<m->n_props;i++ )
            if ( m->names[i] != NULL )
            {
                free( m->names[i] );
                m->names[i] = NULL;
            }
        free( m->names );
        m->names = NULL;
    }
    if ( m->html_tags != NULL )
    {
        int i;
        for ( i=0;i<m->n_props;i++ )
            if ( m->html_tags[i] != NULL )
            {
                free( m->html_tags[i] );
                m->html_tags[i] = NULL;
            }
        free( m->html_tags );
        m->html_tags = NULL;
    }
    free( m );
}
/**
 * Get our quick lookup table
 * @param m the matrix in question
 * @return a hashset containing only property names of interest
 */
hashset *matrix_get_lookup( matrix *m )
{
    return m->lookup;
}
/**
 * Record that a property is inside another
 * @param m the matrix in question
 * @param prop1 the property that is inside
 * @param prop2 the outer property
 */
void matrix_record( matrix *m, char *prop1, char *prop2 )
{
    int index1 = hashset_get( m->lookup, prop1 )-1;
    int index2 = hashset_get( m->lookup, prop2 )-1;
    if ( index1 != -1 && index2 != -1 )
    {
        m->cells[m->n_props*index1+index2]++;
    }
}
/**
 * Forbid property 1 to be inside property 2. Do this after matrix init
 * @param m the matrix in question
 * @param prop1 this can't be inside prop2
 * @param prop2 the outer property that may NOT contain prop1
 */
static void matrix_forbid( matrix *m, char *prop1, char *prop2 )
{
    if ( m->inited )
    {
        int index1 = hashset_get(m->lookup, prop1)-1;
        int index2 = hashset_get(m->lookup, prop2)-1;
        int cell_index = m->n_props*index1+index2;
        m->cells[cell_index] = 0;
    }
    else
        warning("initialise matrix first\n");
}
/**
 * Allow property 1 to be inside property 2. Do this after matrix init
 * @param m the matrix in question
 * @param prop1 this can be inside prop2
 * @param prop2 the outer property that may contain prop1
 */
static void matrix_allow( matrix *m, char *prop1, char *prop2 )
{
    if ( m->inited )
    {
        int index1 = hashset_get(m->lookup, prop1)-1;
        int index2 = hashset_get(m->lookup, prop2)-1;
        int cell_index = m->n_props*index1+index2;
        if ( m->cells[cell_index]==0 )
            m->cells[cell_index] = 1;
    }
    else
        warning("initialise matrix first\n");
}
/**
 * Does name2 contain name1?
 * @param m the matrix in question
 * @param name1 the name that may be inside
 * @param name2 the name that may be outside
 * @return the number of times name1 is inside
 */
int matrix_inside( matrix *m, char *name1, char *name2 )
{
    int index1 = hashset_get(m->lookup, name1)-1;
    int index2 = hashset_get(m->lookup, name2)-1;
    if ( index1 == -1 || index2 == -1 )
        return 0;
    else
        return m->cells[m->n_props*index1+index2];
}
/**
 * Initialise a matrix with a set of ranges that may be within one another
 * @param m the matrix in question
 * @param ranges the array of range object pointers
 */
void matrix_init( matrix *m, range_array *ranges )
{
    int i;
    int n_ranges = range_array_size( ranges );
    matrix_queue *mq = matrix_queue_create();
    for ( i=0;i<n_ranges;i++ )
    {
        if ( !matrix_queue_add(mq,m,range_array_get(ranges,i)) )
            break;
    }
    matrix_queue_dispose( mq );
    m->inited = 1;
}
/**
 * Revise the matrix, setting to 0 any cells representing HTML tags that 
 * may not nest
 * @param m the matrix to update
 */
void matrix_update_html( matrix *m )
{
    int i,j;
    for ( i=0;i<m->n_props;i++ )
    {
        for ( j=0;j<m->n_props;j++ )
        {
            int res = html_is_inside(m->html_tags[i], m->html_tags[j]);
            switch ( res )
            {
                case 0: // either
                    matrix_allow( m, m->names[j], m->names[i] );
                    matrix_allow( m, m->names[i], m->names[j] );
                    break;
                case 1: // tag1 inside tag2
                    matrix_allow( m, m->names[i], m->names[j] );
                    matrix_forbid( m, m->names[j], m->names[i] );
                    break;
                case -1:    // tag2 inside tag1
                    matrix_allow( m, m->names[j], m->names[i] );
                    matrix_forbid( m, m->names[i], m->names[j] );
                    break;
                case -2:    // neither
                    matrix_forbid( m, m->names[i], m->names[j] );
                    matrix_forbid( m, m->names[j], m->names[i] );
                    break;
            }
        }     
    }
}
/**
 * Write out a single row
 * @param m the matrix to dump the row off
 * @param index1 the index of the row zero-based
 */
static void matrix_dump_row( matrix *m, int index1 )
{
    // get longest label
    int llen,i;
    for ( llen=0,i=0;i<m->n_props;i++ )
    {
        int j = strlen( m->names[i] );
        if ( j > llen )
            llen = j;
    }
    fprintf( stderr,"%s",m->names[index1] );
    // left-justify
    for ( i=strlen(m->names[index1]);i<llen+1;i++ )
        fprintf( stderr, " " );
    for ( i=0;i<m->n_props;i++ )
    {
        fprintf( stderr,"%6d",m->cells[m->n_props*index1+i]);
    }
    fprintf( stderr,"\n");
}
/**
 * Debug routine. Dump a matrix to the console.
 * @param m the matrix to dump.
 */
void matrix_dump( matrix *m )
{
    int i;
    for ( i=0;i<m->n_props;i++ )
        matrix_dump_row( m, i );
}
