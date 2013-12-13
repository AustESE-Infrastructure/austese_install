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
 * Created on August 19, 2011, 9:11 AM
 */

#ifndef TEXT_BUF_H
#define	TEXT_BUF_H

typedef struct text_buf_struct text_buf;
text_buf *text_buf_create( int initial_size );
void text_buf_dispose( text_buf *tb );
int text_buf_concat( text_buf *tb, char *text, int len );
char *text_buf_get_buf( text_buf *tb );
int text_buf_len( text_buf *tb );

#endif	/* TEXT_BUF_H */

