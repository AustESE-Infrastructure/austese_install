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
 *  Created on 21 April 2011, 9:21 AM
 */

#ifndef HTML_H
#define	HTML_H
#ifdef	__cplusplus
extern "C" {
#endif
int html_is_inside( char *tag1, char *tag2 );
int html_is_empty( char *tag );
#ifdef	__cplusplus
}
#endif
#endif	/* HTML_H */

