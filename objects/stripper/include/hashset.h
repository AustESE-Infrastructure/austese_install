/* 
 * File:   hashset.h
 * Author: desmond
 *
 * Created on 25 March 2011, 8:29 AM
 */

#ifndef HASHSET_H
#define	HASHSET_H
typedef struct hashset_struct hashset;
hashset *hashset_create();
void hashset_set_id( hashset *set, char *key, int id );
int hashset_get_id( hashset *set, char *key );
int hashset_contains( hashset *set, char *key );
int hashset_put( hashset *set, char *key, int id );
int hashset_size( hashset *set );
void hashset_to_array( hashset *set, char **array );
#endif	/* HASHSET_H */

