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
 * Created on August 10, 2011, 9:49 AM
 */

#ifndef MATRIX_QUEUE_H
#define	MATRIX_QUEUE_H

typedef struct matrix_queue_struct matrix_queue;
matrix_queue *matrix_queue_create();
void matrix_queue_dispose( matrix_queue *mq );
int matrix_queue_add( matrix_queue *mq, matrix *m, range *r );

#endif	/* MATRIX_QUEUE_H */

