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
 */
/* 
 * Store text dynamically
 * Created on August 19, 2011, 9:10 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "text_buf.h"
#include "error.h"
#include "memwatch.h"
/** a dynamically resizeable text buffer to hold output */
struct text_buf_struct
{
    char *buf;
    int len;
    int allocated;
};

/**
 * Create a text buf
 * @param initial_size the buf's initial size
 * @return an allocate text_buf or NULL
 */
text_buf *text_buf_create( int initial_size )
{
    text_buf *tb = calloc( 1, sizeof(text_buf) );
    if ( tb != NULL )
    {
        tb->buf = malloc( initial_size );
        if ( tb->buf == NULL )
        {
            text_buf_dispose( tb );
            tb = NULL;
        }
        else
        {
            tb->allocated = initial_size;
            tb->len = 0;
        }
    }
    else
        warning("text_buf: failed to allocate text buf\n");
    return tb;
}
/**
 * Dispose of a text buf and its contents
 * @param tb the text buf in question
 */
void text_buf_dispose( text_buf *tb )
{
    if ( tb->buf != NULL )
    {
        free( tb->buf );
        tb->buf = NULL;
    }
    free( tb );
}
/**
 * Concatenate some text to the buf
 * @param tb the text buf in question
 * @param text the text
 * @param len the length of the text
 * @return 1 if it worked, else 0
 */
int text_buf_concat( text_buf *tb, char *text, int len )
{
    if ( len+tb->len+1 > tb->allocated )
    {
        int new_size = (tb->len+len+1)*3/2;
        char *temp = malloc( new_size );
        if ( temp == NULL )
        {
            return 0;
        }
        else
        {
            memcpy( temp, tb->buf, tb->len );
            free( tb->buf );
            tb->allocated = new_size;
            tb->buf = temp;
        }
    }
    memcpy( &tb->buf[tb->len], text, len );
    tb->len += len;
    tb->buf[tb->len] = 0;
    return 1;
}
/**
 * Get this text buf's buffer
 * @param tb the text buf in question
 * @return the buffer
 */
char *text_buf_get_buf( text_buf *tb )
{
    return tb->buf;
}
/**
 * Get the text buf's current length
 * @param tb the text buf in question
 * @return the length of it
 */
int text_buf_len( text_buf *tb )
{
    return tb->len;
}
