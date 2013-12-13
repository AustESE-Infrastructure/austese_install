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
#include "hashset.h"
#include "error.h"
#include "memwatch.h"
#define MOD_ADLER 65521
#define BLOCK_SIZE 24
#define MAX_RATIO 1.2f
/**
 * Implement a fixed-size hash table with ids for values
 */
struct hashset_struct
{
    int num_keys;
    int num_buckets;
    int id;
    struct hs_bucket **buckets;
};
struct hs_bucket
{
    char *key;
    int id;
    struct hs_bucket *next;
};
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
 * Create a single new bucket
 * @param prop the property name for the bucket
 * @param id its id
 * @return a finished bucket
 */
static struct hs_bucket *hs_bucket_create( char *prop, int id )
{
    struct hs_bucket *b = calloc( 1, sizeof(struct hs_bucket) );
    if ( b != NULL )
    {
        b->key = strdup( prop );
        b->id = id;
    }
    else
        warning("hs_bucket: failed to create bucket\n");
    return b;
}
/**
 * Create a new hashset
 * @return the constructed hashset ready to go
 */
hashset *hashset_create()
{
    hashset *hs = calloc( 1, sizeof(hashset) );
    if ( hs != NULL )
    {
        hs->num_buckets = BLOCK_SIZE;
        hs->id = 1;
        hs->buckets = calloc( hs->num_buckets, sizeof(struct hs_bucket*) );
        if ( hs->buckets == NULL )
        {
            hashset_dispose( hs );
            warning("hashset: failed to allocate %d buckets\n",hs->num_buckets);
            hs = NULL;
        }
    }
    return hs;
}
/**
 * Dispose of a bucket and all its linked siblings
 * @param b the bucket to dispose
 */
static void hs_bucket_dispose( struct hs_bucket *b )
{
    if ( b->key != NULL )
    {
        free( b->key );
        b->key = NULL;
    }
    if ( b->next != NULL )
        hs_bucket_dispose( b->next );
    free( b );
}
/**
 * Dispose of this hashset and all its bucket contents
 * @param hs the hashset in question
 */
void hashset_dispose( hashset *hs )
{
    int i;
    for ( i=0;i<hs->num_buckets;i++ )
    {
        if ( hs->buckets[i] != NULL )
            hs_bucket_dispose( hs->buckets[i] );
    }
    free( hs->buckets );
    hs->buckets = NULL;
    free( hs );
}
/**
 * Reallocate all the keys in a new bucket map that must be
 * 1.5 times bigger than before
 * @param hs the hashset to rehash
 */
static int hashset_rehash( hashset *hs )
{
#ifdef HASHSET_DEBUG
      printf("rehashing...\n");
#endif
	int i,new_size = hs->num_buckets + hs->num_buckets/2;
	struct hs_bucket **new_buckets = calloc( new_size, sizeof(struct hs_bucket*) );
	if ( new_buckets == NULL )
	{
		warning("hashset: failed to resize hash table\n");
        return 0;
	}
	// copy the old keys over
	for ( i=0;i<hs->num_buckets;i++ )
	{
		if ( hs->buckets[i]!=NULL )
        {
            struct hs_bucket *b = hs->buckets[i];		
            while ( b != NULL )
            {
                unsigned slot = hash((unsigned char*)b->key,
                    strlen(b->key))%new_size;
                struct hs_bucket *d = hs_bucket_create(b->key,b->id);
                if ( new_buckets[slot] == NULL )
                    new_buckets[slot] = d;
                else
                {
                    struct hs_bucket *c = new_buckets[slot];
                    while ( c->next != NULL )
                        c = c->next;
                    c->next = d;
                }
                b = b->next;
            }
            // gets rid of all connected buckets b
            hs_bucket_dispose( hs->buckets[i] );
        }
	}
	free( hs->buckets );
    hs->num_buckets = new_size;
    hs->buckets = new_buckets;
	return 1;
}
/**
 * Add a new name to the hashset and allocate it a unique id
 * @param hs the hashset in question
 * @param prop the property to add
 * @return 1 if successful, else 0
 */
int hashset_put( hashset *hs, char *prop )
{
    unsigned slot;
    struct hs_bucket *b;
    if ( (float)hs->num_keys/(float)hs->num_buckets > MAX_RATIO )
    {
        if ( !hashset_rehash(hs) )
            return 0;
    }
    slot = hash((unsigned char*)prop,strlen(prop))%hs->num_buckets;
    b = hs->buckets[slot];
    if ( b == NULL )
    {
        hs->buckets[slot] = hs_bucket_create(prop,hs->id++);
        if ( hs->buckets[slot] == NULL )
            return 0;
    }
    else
    {
        do
        {
            // if key already present, just return
            if ( strcmp(prop,b->key)==0 )
                return 0;
            else if ( b->next != NULL )
                b = b->next;
        }
        while ( b->next != NULL );
        // key not found
        b->next = hs_bucket_create(prop,hs->id++);
        if ( b->next == NULL )
            return 0;
    }
    hs->num_keys++;
    return 1;
}
/**
 * Get the id of the given property or 0 if not there
 * @param hs the hashset in question
 * @param prop the property to find
 * @return id &gt; 0 if found, else 0
 */
int hashset_get( hashset *hs, char *prop )
{
    unsigned slot = hash((unsigned char*)prop,
        strlen(prop))%(unsigned)hs->num_buckets;
    if ( hs->buckets[slot] == NULL )
        return 0;
    else
    {
        struct hs_bucket *b = hs->buckets[slot];
        while ( b != NULL )
        {
            if ( strcmp(b->key,prop)==0 )
                return b->id;
            b = b->next;
        }
        return 0;
    }
}
/**
 * Get the number of elements stored here
 * @param hs the hashset in question
 * @return the number of items
 */
int hashset_size( hashset *hs )
{
    return hs->num_keys;
}
/**
 * Convert the keys to an array
 * @param hs the hashset in question
 * @param items an array of items big enough
 */
void hashset_to_array( hashset *hs, char **items )
{
    int i,j;
    for ( j=0,i=0;i<hs->num_buckets;i++ )
    {
        struct hs_bucket *b = hs->buckets[i];
        while ( b != NULL )
        {
            items[j++] = b->key;
            b = b->next;
        }
    }
    if ( hs->num_keys != j )
        warning("hashset: expected %d items but found only %d\n",hs->num_keys,j);
}
/**
 * Does this hashset contain the given key?
 * @param hs the hashset in question
 * @param key the key we seek
 * @return 1 if it was there else 0
 */
int hashset_contains( hashset *hs, char *key )
{
    unsigned slot = hash((unsigned char*)key,strlen(key))%hs->num_buckets;
    struct hs_bucket *b = hs->buckets[slot];
    while ( b != NULL )
    {
        if ( strcmp(b->key,key)==0 )
            return 1;
        b = b->next;
    }
    return 0;
}
/**
 * Print a hashset for debugging to the console
 * @param hs the hashset in question
 */
void hashset_print( hashset *hs )
{
    int i;
    for ( i=0;i<hs->num_buckets;i++ )
    {
        struct hs_bucket *b = hs->buckets[i];
        while ( b != NULL )
        {
            warning( "%s: %d\n",b->key,b->id );
            b = b->next;
        }
    }
}
#ifdef HASHSET_DEBUG
int main( int argc, char **argv )
{
	hashset *hs = hashset_create();
      if ( hs != NULL )
      {
           hashset_put( hs, "banana" );
           hashset_put( hs, "apple" );
           hashset_put( hs, "pineapple" );
           hashset_put( hs, "guava" );
           hashset_put( hs, "watermelon" );
           hashset_put( hs, "orange" );
           hashset_put( hs, "starfruit" );
           hashset_put( hs, "durian" );
           hashset_put( hs, "cherry" );
           hashset_put( hs, "apricot" );
           hashset_put( hs, "peach" );
           hashset_put( hs, "pear" );
           hashset_put( hs, "nectarine" );
           hashset_put( hs, "plum" );
           hashset_put( hs, "grape" );
           hashset_put( hs, "mandarin" );
           hashset_put( hs, "lemon" );
           hashset_put( hs, "clementine" );
           hashset_put( hs, "cumquat" );
           hashset_put( hs, "custard apple" );
           hashset_put( hs, "asian pear" );
           hashset_put( hs, "jakfruit" );
           hashset_put( hs, "rambutan" );
           hashset_put( hs, "lime" );
           hashset_put( hs, "lychee" );
           hashset_put( hs, "mango" );
           hashset_put( hs, "mangosteen" );
           hashset_put( hs, "avocado" );
           hashset_put( hs, "grandilla" );
           hashset_put( hs, "grumichama" );
           hashset_put( hs, "breadfruit" );
		// repeats
		hashset_put( hs, "banana" );
           hashset_put( hs, "apple" );
           hashset_put( hs, "pineapple" );
           hashset_put( hs, "guava" );
           hashset_put( hs, "watermelon" );
           hashset_print( hs );
		printf("number of elements in set=%d\n",hashset_size(hs));
      }
}
#endif
