#include <stdlib.h>
#include <string.h>
#include "attribute.h"
#include "error.h"
#include "memwatch.h"
struct attribute_struct
{
    char *name;
    char *value;
};
/**
 * Create a new attribute
 * @param name name of attribute
 * @param value value of attribute
 * @return the new attribute
 */
attr1bute *attribute_new( const char *name, const char *value )
{
    char *new_name = strdup( name );
    char *new_value = strdup( value );
    attr1bute *a = calloc( 1, sizeof(attr1bute) );
    if ( a == NULL || new_name == NULL || new_value == NULL )
    {
        if( new_name != NULL )
            free( new_name );
        if ( new_value != NULL )
            free( new_value );
        error( "recipe: failed to allocate new attribute\n");
    }
    else
    {
        a->name = new_name;
        a->value = new_value;
    }
    return a;
}
/**
 * Free a single attribute name-value pair
 * @param attr the attribute to delete
 */
void attribute_delete( attr1bute *attr )
{
    if ( attr->name != NULL )
        free( attr->name );
    if ( attr->value != NULL)
        free( attr->value );
    free( attr );
}
/**
 * Does the given attribute exist in the given list of attributes?
 * @param a the attribute to test
 * @param attrs the expat list of attributes (name, value pairs)
 * @return 1 if present, else 0
 */
int attribute_present( attr1bute *a, char **attrs )
{
    int i = 0;
    int res = 0;
    while ( attrs[i] != NULL )
    {
        if ( strcmp(attrs[i],attribute_get_name(a))==0
            &&strcmp(attrs[i+1],attribute_get_value(a))==0 )
        {
            res = 1;
            break;
        }
        i += 2;
    }
    return res;
}
/**
 * Get an attribute's value
 * @param a the attribute in question
 * @return the attribute value
 */
char *attribute_get_name( attr1bute *a )
{
    return a->name;
}
/**
 * Get an attribute's value
 * @param a the attribute in question
 * @return the attribute value
 */
char *attribute_get_value( attr1bute *a )
{
    return a->value;
}
/**
 * Remove an attribute from a copy of the expat attribute list
 * @param a the attribute to apply to he list
 * @param attrs the copy of the expat attribute list (updated)
 */
void attribute_remove( attr1bute *a, char **attrs )
{
    int i = 0;
    while ( attrs[i] != NULL )
    {
        if ( strcmp(attribute_get_name(a),attrs[i])==0
            && strcmp(attribute_get_value(a),attrs[i+1])==0 )
        {
            free( attrs[i] );
            free( attrs[i+1] );
            int j = i;
            while ( attrs[j] != NULL )
            {
                attrs[j] = attrs[j+2];
                attrs[j+1] = attrs[j+3];
                j+=2;
            }
            break;
        }
        i += 2;
    }
    
}
