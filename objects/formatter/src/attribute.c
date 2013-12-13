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
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "attribute.h"
#include "error.h"
#include "memwatch.h"
struct attribute_struct
{
    char *name;
    char *value;
    // needed to convert back to annotation
    char *prop_name;
    attribute *next;
};
/**
 * Create an attribute
 * @param name its name
 * @param prop_name its original property name
 * @param value its value
 * @return the finished attribute or NULL
 */
attribute *attribute_create( char *name, char *prop_name, char *value )
{
    attribute *attr = calloc( 1, sizeof(attribute) );
    if ( attr != NULL )
    {
        // don't duplicate or dispose of this
        attr->prop_name = prop_name;
        attr->name = strdup( name );
        if ( attr->name == NULL )
        {
            attribute_dispose( attr );
            warning("attribute: failed to allocate attribute name\n");
            attr = NULL;
        }
        else
        {
            attr->value = strdup( value );
            if ( attr->value == NULL )
            {
                attribute_dispose( attr );
                warning("attribute: failed to allocate attribute value\n");
                attr = NULL;
            }
        }
    }
    else
        warning("attribute: failed to allocate attribute struct\n");
    return attr;
}
/**
 * Get the xml (property name)
 * @param attr the attribute
 * @return its property (xml) name
 */
char *attribute_prop_name( attribute *attr )
{
    return attr->prop_name;
}
/**
 * Add a suffix to an attribute value
 * @param attr the attribute
 * @param suffix the suffix
 * @return 1 if it worked else 0
 */
int attribute_append_value( attribute *attr, char *suffix )
{
    int vlen = strlen(attr->value);
    char *val1 = malloc( vlen+2 );
    if ( val1 != NULL )
    {
        strcpy( val1, attr->value );
        strcat( val1, suffix );
        free( attr->value );
        attr->value = val1;
        return 1;
    }
    else
    {
        fprintf(stderr,"attribute: failed to copy attr\n");
        return 0;
    }
}
/**
 * Convert a base 24 string to an int
 * @param str the string
 * @return its value
 */
static int from_base_24( char *str )
{
    int value = 0;
    int i,len = strlen( str );
    for ( i=0;i<len;i++ )
    {
        value *= 24;
        value += str[i] - 'a';
    }
    return value;
}
/**
 * Compute the length of a=the string representing a base 24 number (a=0,aa=24)
 * @param value the int value
 * @return its string length
 */
static int base_24_len( int value )
{
    int len = 0;
    do
    {
        value = value/24;
        len++;
    }
    while ( value > 0 );
    return len;
}
/**
 * Convert a number to a base-24 string
 * @param value the number
 * @param dst the location for the string (must be large enough)
 */
static void to_base_24( int value, char *dst )
{
	int len = base_24_len(value);
    int i = len-1;
    do
    {
        dst[i--] = value%24+'a';
        value /= 24;
    }
    while ( value > 0 && i >= 0 );
	dst[len] = 0;
}
/**
 * Increment a value by incrementing an *existing* suffix
 * @param attr the attribute in question
 * @return its value allocated (to be freed) with a new suffix
 */
char *attribute_inc_value( attribute *attr )
{
    int i=strlen(attr->value)-1;
    while ( i>0 )
    {
        if ( attr->value[i-1]>='a'&&attr->value[i-1]<='z' )
            i--;
        else
            break;
    }
    // so we're pointing to the first suffix char
    char *suffix = &attr->value[i];
    int base_len = strlen(attr->value)-strlen(suffix);
    int old = from_base_24( suffix );
    int new_suffix_len = base_24_len( old+1 );
    char *new_value = malloc( base_len+new_suffix_len+1 );
    strncpy( new_value, attr->value, base_len );
    to_base_24( old+1, &new_value[base_len] );
    new_value[base_len+new_suffix_len] = 0;
    return new_value;
}
/**
 * Clone an existing attribute
 * @param attr the attribute to clone
 * @return the attribute or NULL
 */
attribute *attribute_clone( attribute *attr )
{
    attribute *new_attr = NULL;
    if ( strcmp(attr->name,"id")==0 )
    {
        // inc suffix
        int vlen = strlen(attr->value);
        int i = vlen-1;
        while ( i > 0 && isalpha(attr->value[i]) )
            i--;
        int res = 1;
        if ( i == vlen-1 )
            res = attribute_append_value( attr, "a" );
        if ( res )
        {
            char *value = attribute_inc_value( attr );
            if ( value != NULL )
            {
                new_attr = attribute_create( attr->name, attr->prop_name, value );
                free( value );
            }
            else
                fprintf(stderr,"attribute: failed to inc value\n");
        }
        else
            fprintf(stderr,"attribute: failed to append value\n");
    }
    else
        new_attr = attribute_create( attr->name, attr->prop_name, attr->value );
    return new_attr;
}
/**
 * Dispose of an attribute
 * @param attr the attr to throw away
 */
void attribute_dispose( attribute *attr )
{
    if ( attr->next != NULL )
        attribute_dispose( attr->next );
    if ( attr->name != NULL )
    {
        free( attr->name );
        attr->name = NULL;
    }
    if ( attr->value != NULL )
    {
        free( attr->value );
        attr->value = NULL;
    }
    free( attr );
    attr = NULL;
}
/**
 * Add one attribute onto the end of the list of which we are a part
 * @param attrs the list of attributes
 * @param attr the new attribute to append to the end
 */
void attribute_append( attribute *attrs, attribute *attr )
{
    while ( attrs->next != NULL )
        attrs = attrs->next;
    attrs->next = attr;
}
char *attribute_get_name( attribute *attr )
{
    return attr->name;
}
char *attribute_get_value( attribute *attr )
{
    return attr->value;
}
attribute *attribute_get_next( attribute *attr )
{
    return attr->next;
}
int attribute_count( attribute *attr )
{
    int count = 0;
    while ( attr != NULL )
    {
        count++;
        attr = attr->next;
    }
    return count;
}