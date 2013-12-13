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
 * Created on 10 August 2011, 7:13 AM
 */

#ifndef MATRIX_H
#define	MATRIX_H

typedef struct matrix_struct matrix;
matrix *matrix_create();
void matrix_dispose( matrix *m );
int matrix_inside( matrix *m, char *name1, char *name2 );
void matrix_init( matrix *m, range_array *ranges );
void matrix_update_html( matrix *m );
hashset *matrix_get_lookup( matrix *m );
void matrix_dump( matrix *m );
void matrix_record( matrix *m, char *prop1, char *prop2 );

#endif	/* MATRIX_H */

