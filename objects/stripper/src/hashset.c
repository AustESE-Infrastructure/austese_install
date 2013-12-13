/*
 * C implementation of a hashset for strings
 *
 * Created on: 17/10/2010
 * (c) Desmond Schmidt 2010
 */
/* This file is part of stripper.
 *
 *  stripper is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  stripper is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with stripper.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hashset.h"
#include "memwatch.h"
#define INITIAL_BUCKETS 12
#define MOD_ADLER 65521
#define MAX_RATIO 1.2f

struct hashset_struct
{
	struct bucket **buckets;
	int num_buckets;
	int num_keys;
};
struct bucket
{
	char *key;
	int id;
	struct bucket *next;
};
/**
 * Create a new hashset
 * @return an allocated hashset structure
 */
hashset *hashset_create()
{
	hashset *set = malloc( sizeof(hashset) );
	if ( set != NULL )
	{
		set->buckets = calloc( INITIAL_BUCKETS, sizeof(struct bucket*) );
		if ( set->buckets != NULL )
		{
			set->num_buckets = INITIAL_BUCKETS;
			set->num_keys = 0;
		}
		else
		{
			free( set );
			set = NULL;
			printf("failed to allocate hashset\n");
		}
	}
	else
		printf("couldn't allocate hashset\n");
	return set;
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
 * @param b the bucket
 * @param key the key for it
 * @param id the id for the bucket
 */
static void bucket_set_key( struct bucket *b, char *key, int id )
{
	b->key = malloc( strlen(key)+1 );
	if ( b->key == NULL )
	{
		printf("failed to allocate store for hashset key\n");
		exit( 0 );
	}
	strcpy( b->key, key );
	b->id = id;
}
/**
 * Reallocate all the keys in a new bucket set that must be
 * 1.5 times bigger than before
 * @param set the set to rehash
 */
static void hashset_rehash( hashset *set )
{
	int i,old_size = set->num_buckets;
	set->num_buckets = old_size + (old_size/2);
	struct bucket **old_buckets = set->buckets;
	set->buckets = calloc( set->num_buckets, sizeof(struct bucket) );
	if ( set->buckets == NULL )
	{
		printf("failed to resize hash table\n");
		exit( 0 );
	}
	// copy the old keys over
	for ( i=0;i<old_size;i++ )
	{
		struct bucket *b = old_buckets[i];
		while ( b != NULL )
		{
			struct bucket *old = b;
			hashset_put( set, b->key, b->id );
			b = b->next;
			free( old->key );
			free( old );
		}
	}
	free( old_buckets );
}
/**
 * Set the id for a key. Ignored if not found
 * @param set the hashset to query
 * @param key the key to test for
 * @param the key's id
 */
void hashset_set_id( hashset *set, char *key, int id )
{
	unsigned hashval = hash( (unsigned char*)key, strlen(key) );
	int bucket = hashval % set->num_buckets;
	struct bucket *b = set->buckets[bucket];
	while ( b != NULL )
	{
		// if key already present, just return
		if ( strcmp(key,b->key)==0 )
		{
			b->id = id;
			break;
		}
		else
			b = b->next;
	}
}
/**
 * Get the id for a key
 * @param set the hashset to query
 * @param key the key to test for
 * @return the key's id or -1 if not found
 */
int hashset_get_id( hashset *set, char *key )
{
	unsigned hashval = hash( (unsigned char*)key, strlen(key) );
	int bucket = hashval % set->num_buckets;
	struct bucket *b = set->buckets[bucket];
	while ( b != NULL )
	{
		// if key already present, just return
		if ( strcmp(key,b->key)==0 )
			return b->id;
		else
			b = b->next;
	}
	// not present
	return -1;
}
/**
 * Does the hashset contain a key?
 * @param set the hashset to query
 * @param key the key to test for
 * @return 1 if present, 0 otherwise
 */
int hashset_contains( hashset *set, char *key )
{
	unsigned hashval = hash( (unsigned char*)key, strlen(key) );
	int bucket = hashval % set->num_buckets;
	struct bucket *b = set->buckets[bucket];
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
 * Put a key into the hashset or test for membership. The
 * destination bucket is decided by modding the hash by
 * the number of bucket slots available. This gives us up
 * to 4 billion buckets maximum.
 * @param set the hashset to add to
 * @param key the key to put in there
 * @param id an optional int to store as its value
 * @return 1 if it was added, 0 otherwise (already there)
 */
int hashset_put( hashset *set, char *key, int id )
{
	unsigned hashval = hash( (unsigned char*)key, strlen(key) );
	int bucket = hashval % set->num_buckets;
	if ( set->buckets[bucket] == NULL )
	{
		set->buckets[bucket] = malloc( sizeof(struct bucket) );
		if ( set->buckets[bucket] == NULL )
		{
			printf("failed to allocate store for hashset bucket\n");
			exit( 0 );
		}
		bucket_set_key( set->buckets[bucket], key, id );
		set->buckets[bucket]->next = NULL;
		set->num_keys++;
		return 1;
	}
	else if ( (float)set->num_keys/(float)set->num_buckets > MAX_RATIO )
	{
		hashset_rehash( set );
		return hashset_put( set, key, id );
	}
	else // bucket already present
	{
		struct bucket *b = set->buckets[bucket];
		while ( b != NULL )
		{
			// if key already present, just return
			if ( strcmp(key,b->key)==0 )
				return 0;
			else if ( b->next != NULL )
				b = b->next;
			else
				break;
		}
		// key not found
		b->next = malloc( sizeof(struct bucket) );
		if ( b->next == NULL )
		{
			printf("failed to allocate store for hashset bucket\n");
			exit( 0 );
		}
		bucket_set_key( b, key, id );
		set->num_keys++;
		return 1;
	}
}
/**
 * Get the size of this hashset
 * @param set the hashset to get the size of
 * @return the number of its current entries
 */
int hashset_size( hashset *set )
{
	return set->num_keys;
}
/**
 * Get the keys of this hashset as an array
 * @param array an array just big enough for the keys
 */
void hashset_to_array( hashset *set, char **array )
{
	int i,j;
	for ( j=0,i=0;i<set->num_buckets;i++ )
	{
		struct bucket *b = set->buckets[i];
		while ( b != NULL )
		{
			array[j++] = b->key;
			b = b->next;
		}
	}
}
