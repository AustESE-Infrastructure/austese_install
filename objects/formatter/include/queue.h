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
 * Created on August 9, 2011, 8:33 AM
 */

#ifndef QUEUE_H
#define	QUEUE_H

typedef struct queue_struct queue;
queue *queue_create();
void queue_dispose( queue *q );
int queue_empty( queue *q );
int queue_push( queue *q, range *r );
range *queue_pop( queue *q );
void queue_print( queue *q );

#endif	/* QUEUE_H */

