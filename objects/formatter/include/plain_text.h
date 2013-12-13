/*
 * plain_text.h
 *
 *  Created on: 27/10/2010
 *  (c) Copyright Desmond Schmidt 2010
 */
/* This file is part of formatter.
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
 */

#ifndef PLAIN_TEXT_H_
#define PLAIN_TEXT_H_
typedef struct plain_text_struct plain_text;
plain_text *plain_text_load( const char *text_file );
char *plain_text_get_data( plain_text *pt, int offset );
int plain_text_get_len( plain_text *pt );
#endif /* PLAIN_TEXT_H_ */
