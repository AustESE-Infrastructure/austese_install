/*
 * parse_css.h
 *
 *  Created on: 26/10/2010
 *  (c) Desmond Schmidt 2010
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

#ifndef CSS_PARSE_H_
#define CSS_PARSE_H_
int css_parse( const char *data, int len, hashset *props, hashmap *css );
#endif /* CSS_PARSE_H_ */
