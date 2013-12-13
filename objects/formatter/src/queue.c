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
#include "queue.h"
#include "error.h"
#include "memwatch.h"
#define QUEUE_BLK_SIZE 20
/**
 * Implement a queue of ranges for dom algorithm
 */
struct queue_element
{
    struct queue_element *next;
    struct queue_element *prev;
    range *r;
};
struct queue_struct
{
    struct queue_element *head;
    struct queue_element *tail;
};
queue *queue_create()
{
    queue *q = calloc( 1, sizeof(queue) );
    if ( q == NULL )
        warning("failed to allocate queue\n");
    return q;
}
/**
 * Dispose of the memory in the queue. Ranges are probably freed elsewhere.
 * @param q the queue to dispose
 */
void queue_dispose( queue *q )
{
    struct queue_element *qe = q->head;
    int count = 0;
    while ( qe != NULL )
    {
        struct queue_element *next = qe->next;
        count++;
        free( qe );
        qe = next;
    }
    free( q );
}
/**
 * Push a range onto the front of the queue
 * @param q the queue to push it on
 * @param r the range to push
 * @return 1 if it worked, else 0
 */
int queue_push( queue *q, range *r )
{
    struct queue_element *qe = calloc( 1, sizeof(struct queue_element));
    if ( qe != NULL )
    {
        qe->r = r;
        if ( q->head == NULL )
        {
            q->head= qe;
            q->tail = qe;
        }
        else
        {
            q->head->prev = qe;
            qe->next = q->head;
            q->head = qe;
        }
        return 1;
    }
    else
        return 0;
}
/**
 * Pop off a queue
 * @param q the queue to pop from
 * @return NULL if queue is empty else the last element
 */
range *queue_pop( queue *q )
{
    struct queue_element *qe = q->tail;
    range *r = NULL;
    if ( qe != NULL )
    {
        q->tail = qe->prev;
        r = qe->r;
        if ( qe->prev == NULL )
            q->head = q->tail = NULL;
        if ( qe->prev != NULL )
            qe->prev->next = NULL;
        free( qe );
        qe = NULL;
    }
    return r;
}
/**
 * Is this queue empty?
 * @param q the queue in question
 * @return 1 if it is empty, else 0
 */
int queue_empty( queue *q )
{
    return q->head == NULL;
}
/**
 * Debug: print current queue to stdout
 * @param q the queue to print
 */
void queue_print( queue *q )
{
    struct queue_element *qe = q->head;
    while ( qe != NULL )
    {
        fprintf( stderr,"name=%s start=%d len=%d\n",range_name(qe->r),
            range_start(qe->r),range_len(qe->r));
        qe = qe->next;
    }
}