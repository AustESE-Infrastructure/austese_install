/*
 * hashmap.h
 *
 *  Created on: 01/11/2010
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
#ifndef HASHMAP_H_
#define HASHMAP_H_
#ifdef	__cplusplus
extern "C" {
#endif
typedef struct hashmap_struct hashmap;
typedef void (*print_value)( void *value );
hashmap *hashmap_create();
void hashmap_dispose( hashmap *map );
int hashmap_put( hashmap *map, char *key, void *value );
int hashmap_contains( hashmap *map, char *key );
void *hashmap_get( hashmap *map, char *key );
int hashmap_size( hashmap *map );
void hashmap_print( hashmap *map, print_value pv );
int hashmap_remove( hashmap *map, char *key );
// iterator methods
typedef struct hashmap_iterator_struct hashmap_iterator;
hashmap_iterator *hashmap_iterator_create( hashmap *map );
void hashmap_iterator_dispose( hashmap_iterator *iter );
char *hashmap_iterator_next( hashmap_iterator *iter );
int hashmap_iterator_has_next( hashmap_iterator *iter );
#ifdef	__cplusplus
}
#endif
#endif /* HASHMAP_H_ */
