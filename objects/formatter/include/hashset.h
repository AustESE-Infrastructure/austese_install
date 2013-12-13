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
 * Created on August 11, 2011, 9:58 AM
 */

#ifndef HASHSET_H
#define	HASHSET_H
#ifdef	__cplusplus
extern "C" {
#endif
typedef struct hashset_struct hashset;
hashset *hashset_create();
void hashset_dispose( hashset *hs );
int hashset_get( hashset *hs, char *prop );
int hashset_put( hashset *hs, char *prop );
int hashset_size( hashset *hs );
void hashset_to_array( hashset *hs, char **items );
int hashset_contains( hashset *hs, char *key );
void hashset_print( hashset *hs );
#ifdef	__cplusplus
}
#endif
#endif	/* HASHSET_H */

