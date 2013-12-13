#include <stdlib.h>
#include <string.h>
#include "attribute.h"
#include "simplification.h"
#include "error.h"
#include "memwatch.h"
struct simplification_struct
{
    char *xml_name;
    char *prop_name;
    attr1bute **attributes;
};
/**
 * Create a new rule to simplify xml to aese properties
 * @param xml_name the xml name of the simplification
 * @param prop_name the aese property name
 * @return a simplification rule
 */
simplification *simplification_new( const char *xml_name,
    const char *prop_name )
{
    simplification *s = malloc( sizeof(simplification) );
    if ( s == NULL )
        error( "recipe: failed to allocate rule\n" );
    s->xml_name = malloc(strlen(xml_name)+1 );
    if ( s->xml_name == NULL )
        error( "recipe: failed to allocate xml name\n");
    s->prop_name = malloc( strlen(prop_name)+1 );
    if ( s->prop_name == NULL )
        error( "recipe: failed to allocate aese name\n");
    strcpy( s->xml_name, xml_name );
    strcpy( s->prop_name, prop_name );
    s->attributes = calloc( 1, sizeof(attr1bute*) );
    if ( s->attributes == NULL )
        error( "recipe: failure to allocate attributes\n");
    return s;
}
/**
 * Does a simplification rule contain a set of attributes?
 * @param s the simplification rule
 * @param attrs the list of attributes in expat fprmat
 * @return 1 if all the attributes in the rule are present in the list, else 0
 */
int simplification_contains( simplification *s, char **attrs )
{
    int i = 0;
    while ( s->attributes[i] != NULL )
    {
        if ( !attribute_present(s->attributes[i++],attrs) )
            return 0;
    }
    return 1;
}
/**
 * Delete a simplification
 * @param s the simplification to delete
 */
void simplification_delete( simplification *s )
{
    int i = 0;
    free( s->xml_name );
    free( s->prop_name );
    while ( s->attributes[i] != NULL )
        attribute_delete( s->attributes[i++] );
    free( s->attributes );
    free( s );
}
/**
 * Add an attribute to a simplification rule
 * @param s the simplification rule
 * @param a the new attribute to add to it
 */
void simplification_add_attribute( simplification *s, attr1bute *a )
{
    attr1bute **attrs;
    int i = 0;
    while ( s->attributes[i] != NULL )
        i++;
    attrs = calloc( i+2,sizeof(attr1bute*) );
    if ( attrs == NULL )
        error( "recipe: failed to reallocate attributes for rule\n");
    // copy existing rules
    i = 0;
    while ( s->attributes[i] != NULL )
    {
        attrs[i] = s->attributes[i];
        i++;
    }
    attrs[i] = a;
    free( s->attributes );
    s->attributes = attrs;
}
/**
 * Remove an attribute from a copy of an expat list of attributes
 * @param s the simplification rule in question
 * @param attrs the expat attribute list (copied)
 */
void simplification_remove_attribute( simplification *s, char **attrs )
{
    int i = 0;
    while ( s->attributes[i] != NULL )
        attribute_remove( s->attributes[i++], attrs );
}
/**
 * Get the xml name of a rule
 * @param s the simplification rule in question
 * @return its xml name
 */
char *simplification_get_xml_name( simplification *s )
{
    return s->xml_name;
}
/**
 * Get the aese name of a rule
 * @param s the simplification rule in question
 * @return its xml name
 */
char *simplification_get_prop_name( simplification *s )
{
    return s->prop_name;
}