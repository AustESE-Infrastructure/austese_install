/*
 * This file is part of formatter.
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
 *  (c) copyright Desmond Schmidt 2011
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hashmap.h"
#include "error.h"
#include "memwatch.h"

#define INITIAL_BUCKETS 12
#define MOD_ADLER 65521
#define MAX_RATIO 0.8f

struct hashmap_struct
{
	struct bucket **buckets;
	int num_buckets;
	int num_keys;
};
struct bucket
{
	char *key;
	void *value;
	struct bucket *next;
};
struct hashmap_iterator_struct
{
    int position;
    int num_keys;
    char **values;
};
/**
 * Create a new hashmap
 * @return an allocated hashmap structure
 */
hashmap *hashmap_create()
{
	hashmap *map = malloc( sizeof(hashmap) );
	if ( map != NULL )
	{
		map->buckets = calloc( INITIAL_BUCKETS, sizeof(struct bucket*) );
		if ( map->buckets != NULL )
		{
			map->num_buckets = INITIAL_BUCKETS;
			map->num_keys = 0;
		}
		else
		{
			free( map );
			map = NULL;
			warning("failed to allocate hashmap\n");
		}
	}
	else
		warning("couldn't allocate hashmap\n");
	return map;
}
/**
 * Dispose of a bucket and all its children. Just dispose of key not value
 * @param b the bucket to kill off
 */
static void bucket_dispose( struct bucket *b )
{
    if ( b->key != NULL )
    {
        free( b->key );
        b->key = NULL;
    }
    if ( b->next != NULL )
        bucket_dispose( b->next );
    // don't free value - not our resp
    free( b );
}
/**
 * Dispose of the memory associated with the map
 * @param map the map in question
 */
void hashmap_dispose( hashmap *map )
{
    int i;
    for ( i=0;i<map->num_buckets;i++ )
    {
        if ( map->buckets[i] != NULL )
            bucket_dispose( map->buckets[i] );
    }
    free( map->buckets );
    map->buckets = NULL;
    free( map );
}
/**
 * Hash a string, adler32 method
 * @param data the data to hash
 * @param len its length
 */
static unsigned hash( unsigned char *data, int len )
{
    unsigned a = 1, b = 0;
    int index;

    /* Process each byte of the data in order */
    for (index = 0; index < len; ++index)
    {
        a = (a + data[index]) % MOD_ADLER;
        b = (b + a) % MOD_ADLER;
    }
    return (b << 16) | a;
}
/**
 * Set the key in a bucket
 * @param key the key for it
 * @param value for the bucket (not our responsibility)
 * @param the bucket or NULL
 */
static struct bucket *bucket_create( char *key, void *value )
{
	struct bucket *b = calloc( 1, sizeof(struct bucket) );
    if ( b != NULL )
    {
        b->key = strdup(key);
        if ( b->key == NULL )
        {
            warning("failed to allocate store for hashmap key\n");
            bucket_dispose( b );
            return NULL;
        }
        b->value = value;
    }
    return b;
}
/**
 * Reallocate all the keys in a new bucket map that must be
 * 1.5 times bigger than before
 * @param map the map to rehash
 */
static int hashmap_rehash( hashmap *map )
{
	int i,new_size = map->num_buckets + map->num_buckets/2;
	struct bucket **new_buckets = calloc( new_size, sizeof(struct bucket*) );
	if ( new_buckets == NULL )
	{
		warning("hashmap: failed to resize hash table\n");
        return 0;
	}
	// copy the old keys over
	for ( i=0;i<map->num_buckets;i++ )
	{
		if ( map->buckets[i] )
        {
            struct bucket *b = map->buckets[i];		
            while ( b != NULL )
            {
                unsigned slot = hash((unsigned char*)b->key,
                    strlen(b->key))%new_size;
                struct bucket *d = bucket_create(b->key,b->value);
                if ( d==NULL )
                    return 0;
                else if ( new_buckets[slot] == NULL )
                    new_buckets[slot] = d;
                else
                {
                    struct bucket *c = new_buckets[slot];
                    while ( c->next != NULL )
                        c = c->next;
                    c->next = d;
                }
                b = b->next;
            }
            // gets rid of all connected buckets b
            bucket_dispose( map->buckets[i] );
        }
	}
	free( map->buckets );
    map->num_buckets = new_size;
    map->buckets = new_buckets;
	return 1;
}
/**
 * Get the value for a key
 * @param map the hashmap to query
 * @param key the key to test for
 * @return the value or NULL if not found
 */
void *hashmap_get( hashmap *map, char *key )
{
	unsigned hashval = hash( (unsigned char*)key, strlen(key) );
	int bucket = hashval % map->num_buckets;
	struct bucket *b = map->buckets[bucket];
	while ( b != NULL )
	{
		// if key already present, just return
		if ( strcmp(key,b->key)==0 )
			return b->value;
		else
			b = b->next;
	}
	// not present
	return NULL;
}
/**
 * Does the hashmap contain a key?
 * @param map the hashmap to query
 * @param key the key to test for
 * @return 1 if present, 0 otherwise
 */
int hashmap_contains( hashmap *map, char *key )
{
	unsigned hashval = hash( (unsigned char*)key, strlen(key) );
	int bucket = hashval % map->num_buckets;
	struct bucket *b = map->buckets[bucket];
	while ( b != NULL )
	{
		// if key already present, just return
        if ( strcmp(key,b->key)==0 )
			return 1;
		else
			b = b->next;
	}
	// not present
	return 0;
}
/**
 * Put a key into the hashmap or test for membership. The
 * destination bucket is decided by modding the hash by
 * the number of bucket slots available. This gives us up
 * to 4 billion buckets maximum.
 * @param map the hashmap to add to
 * @param key the key to put in there
 * @param value at the key
 * @param replace if 1 then replace any value already in the bucket
 * @return 1 if it was added, 0 otherwise (already there)
 */
int hashmap_put( hashmap *map, char *key, void *value )
{
	unsigned hashval,slot;
    struct bucket *d;
    if ( (float)map->num_keys/(float)map->num_buckets > MAX_RATIO )
	{
		if ( !hashmap_rehash(map) )
            return 0;
	}
    hashval = hash( (unsigned char*)key, strlen(key) );
	slot = hashval % map->num_buckets;
    d = bucket_create( key, value );
    if ( d != NULL )
    {
        if ( map->buckets[slot] == NULL )
        {
            map->num_keys++;
            map->buckets[slot] = d;
        }
        else 
        {
            struct bucket *prev = NULL;
            struct bucket *c = map->buckets[slot];
            do
            {
                if ( strcmp(c->key,key)==0 )
                {
                    // key already present: replace
                    if ( prev != NULL )
                        prev->next = d;
                    else
                        map->buckets[slot] = d;
                    d->next = c->next;
                    free( c->key );
                    free( c );
                    break;
                }
                else if ( c->next == NULL )
                {
                    c->next = d;
                    map->num_keys++;
                    break;
                }
                prev = c;
                c = c->next;
            } while ( c != NULL );
        }
        return 1;
    }
    else
        return 0;
}
/**
 * Get the size of this hashmap
 * @param map the hashmap to get the size of
 * @return the number of its current entries
 */
int hashmap_size( hashmap *map )
{
	return map->num_keys;
}
/**
 * Create an iterator
 * @param map the map to make an iterator of
 * @return the iterator or NULL
 */
hashmap_iterator *hashmap_iterator_create( hashmap *map )
{
    hashmap_iterator *iter = calloc( 1, sizeof(hashmap_iterator) );
    if ( iter != NULL )
    {
        iter->position = 0;
        iter->num_keys = hashmap_size(map);
        iter->values = (char**)calloc( iter->num_keys, sizeof(char*) );
        if ( iter->values != NULL )
        {
            int i,k;
            for ( k=0,i=0;i<map->num_buckets;i++ )
            {
                if ( map->buckets[i] != NULL )
                {
                    struct bucket *b = map->buckets[i];
                    while ( b != NULL )
                    {
                        iter->values[k++] = b->key;
                        b = b->next;
                    }
                }
            }
        }
        else
        {
            warning("hashmap_iterator: couldn't allocate key array\n");
        }
    }
    else
        warning("hashmap_iterator: failed to allocate object\n");
    return iter;
}
/**
 * Dispose of an iterator
 * @param iter the iterator in question
 */
void hashmap_iterator_dispose( hashmap_iterator *iter )
{
    if ( iter->values != NULL )
    {
        free( iter->values );
        iter->values = NULL;
    }
    free( iter );
}
/**
 * Get the next key in the iterator
 * @return a key or NULL
 */
char *hashmap_iterator_next( hashmap_iterator *iter )
{
    if ( iter->position >= iter->num_keys )
        return NULL;
    else
        return iter->values[iter->position++];
}
/**
 * Does this iterator have any more keys?
 * @return 1 if it does else 0
 */
int hashmap_iterator_has_next( hashmap_iterator *iter )
{
    return iter->position < iter->num_keys;
}
/**
 * Print a hashmap to the console for debugging
 * @param map the map to print
 * @param pv a function for printing values
 */
void hashmap_print( hashmap *map, print_value pv )
{
    int i;
    for ( i=0;i<map->num_buckets;i++ )
    {
        struct bucket *b = map->buckets[i];
        while ( b != NULL )
        {
            printf("key: %s value: ",b->key);
            (pv)(b->value);
            b = b->next;
        }
    }
}
/**
 * Remove a key from the table
 * @param map the hashmap
 * @param key the key to remove
 * @return 1 if it was removed
 */
int hashmap_remove( hashmap *map, char *key )
{
    int hashval = hash( (unsigned char*)key, strlen(key) );
	int slot = hashval % map->num_buckets;
    struct bucket *b = map->buckets[slot];
    struct bucket *prev = NULL;
    while ( b != NULL )
    {
        if ( strcmp(b->key,key)==0 )
        {
            free( b->key );
            b->key = NULL;
            if ( prev != NULL )
                prev->next = b->next;
            else if ( b->next != NULL )
                map->buckets[slot] = b->next;
            else //b->next is NULL and prev is NULL
                map->buckets[slot] = NULL;
            free( b );
            b = NULL;
            map->num_keys--;
            return 1;
        }
        prev = b;
        b = b->next;
    }
    return 0;
}