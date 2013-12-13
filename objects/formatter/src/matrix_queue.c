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
#include "hashset.h"
#include "matrix.h"
#include "matrix_queue.h"
#include "HTML.h"
#include "error.h"
#include "memwatch.h"
struct matrix_queue_struct
{
    struct queue_element *head;
    struct queue_element *tail;
};
struct queue_element
{
    range *r;
    struct queue_element *next;
    struct queue_element *prev;
};
/**
 * Create an empty matrix queue
 * @return 
 */
matrix_queue *matrix_queue_create()
{
    matrix_queue *mq = calloc( 1, sizeof(matrix_queue) );
    if ( mq == NULL )
        warning("matrix_queue: failed to allocate mq object\n");
    return mq;
}
/**
 * Dispose of a matrix queue
 * @param mq the queue to dispose
 */
void matrix_queue_dispose( matrix_queue *mq )
{
    struct queue_element *qe = mq->tail;
    while ( qe != NULL )
    {
        struct queue_element *prev = qe->prev;
        free( qe );
        qe = prev;
    }
    free( mq );
}
/**
 * Add a range to the queue and throw out any obsolete ones.
 * Because the ranges are sorted on increasing start-offset
 * and decreasing length the only ranges a new range can overlap
 * with are those on the queue and any following and equal ranges.
 * @param mq the matrix queue in question
 * @param r the range to add
 * @return 1 if it worked, else 0
 */
int matrix_queue_add( matrix_queue *mq, matrix *m, range *r )
{
    // remove any element from the queue 
    // that ends before r starts
    struct queue_element *temp = mq->tail;
    while ( temp != NULL )
    {
        struct queue_element *prev = temp->prev;
        if ( range_end(temp->r)<=range_start(r) )
        {
            // purge queue of unwanted ranges
            if ( temp->prev != NULL )
                temp->prev->next = temp->next;
            if ( temp->next != NULL )
                temp->next->prev = temp->prev;
            if ( temp == mq->head )
                mq->head = temp->next;
            if ( temp == mq->tail )
                mq->tail = temp->prev;
            free( temp );
            temp = NULL;
        }
        else if ( range_inside(r,temp->r) )
        {
            matrix_record( m, range_name(r), range_name(temp->r) );
            // an equal range may have preceded us
            if ( range_equals(r,temp->r) )
                matrix_record( m, range_name(temp->r),range_name(r) );
        }
        temp = prev;
    }
    // now add r to the end of the queue
    temp = calloc( 1, sizeof(struct queue_element) );
    if ( temp != NULL )
    {
        temp->r = r;
        if ( mq->head == NULL )
        {
            mq->head = mq->tail = temp;
        }
        else
        {
            temp->prev = mq->tail;
            mq->tail->next = temp;
            mq->tail = temp;
        }
        return 1;
    }
    return 0;
}
