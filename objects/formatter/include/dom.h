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
 *  Created on August 18, 2011, 8:53 AM
 */

#ifndef DOM_H
#define	DOM_H

typedef struct dom_struct dom;
dom *dom_create( const char *text, int len, range_array *ranges,  
    hashmap *rules, hashset *properties );
void dom_dispose( dom *d );
int dom_build( dom *d );
void dom_print( dom *d );
void dom_check_output( dom *d );
text_buf *dom_get_text_buf( dom *d );
int dom_check_tree( dom *d );
void dom_print_ranges( dom *d );
#endif	/* DOM_H */

