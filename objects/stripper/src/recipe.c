/**
 * A recipe is a set of rules for simplifying element+attribute
 * combinations in XML into simple properties in Aese format.
 * Each simplified property equates to a single format in HTML.
 * The recipe file allows us to customise the simplification
 * and also to reverse it. A rule applies iff the element name
 * (or property name in reverse) and its specified attributes match.
 * This module provides one method for loading a recipe and another
 * for simplifying a given xml element and its attributes. It also
 * has a method for testing if a recipe contains a given rule.
 * (c) Desmond Schmidt 2011
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
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "expat.h"
#include "attribute.h"
#include "simplification.h"
#include "milestone.h"
#include "layer.h"
#include "recipe.h"
#include "error.h"
#include "cJSON.h"
#include "memwatch.h"
#ifdef XML_LARGE_SIZE
#if defined(XML_USE_MSC_EXTENSIONS) && _MSC_VER < 1400
#define XML_FMT_INT_MOD "I64"
#else
#define XML_FMT_INT_MOD "ll"
#endif
#else
#define XML_FMT_INT_MOD "l"
#endif
struct recipe_struct
{
    // list of removals
    char **removals;
    // list of simplifications
    simplification **rules;
    /** list of extra layers */
    layer **layers;
};
static simplification *current_rule = NULL;
/**
 * Allocate a totally empty recipe
 * @return the newly allocated recipe
 */
recipe *recipe_new()
{
    recipe *r = calloc(1, sizeof(recipe) );
    if ( r != NULL )
    {
        r->rules = calloc(1,sizeof(simplification*));
        if ( r->rules != NULL )
        {   
            r->removals = calloc( 1, sizeof(char*) );
            if ( r->removals == NULL )
            {
                r = recipe_dispose(r);
                fprintf(stderr, "recipe: failed to allocate removal\n" );
            }
        }
        else
        {
            r = recipe_dispose( r );
            fprintf(stderr, "recipe: failed to allocate rule\n" );
        }
    }
    else
        fprintf(stderr, "recipe: failed to allocate recipe\n" );
    return r;
}
/**
 * Add an attribute to a rule
 * @param s the rule to add it to
 * @param name the name of the attribute
 * @param value its value
 */
static void recipe_add_attribute( simplification *s, const char *name,
    const char *value )
{
    attr1bute *a = attribute_new( name, value );
    simplification_add_attribute( s, a );
}
/**
 * Count the number of simplification rules in a NULL-terminated array
 * @param rules a NULL-terminated array of pointers
 * @return the number of elements in the list
 */
static int count_rules( simplification **rules )
{
    int i = 0;
    while ( rules[i] != NULL )
        i++;
    return i;
}
/**
 * Add a layer to the recipe
 * @param r the recipe to add it to
 * @param l the layer object already created
 * @return 1 if it worked else 0
 */
static int recipe_add_layer( recipe *r, layer *l )
{
    int res = 0;
    if ( r->layers == NULL )
    {
        r->layers = calloc( 2, sizeof(layer*) );
        r->layers[0] = l;
    }
    else
    {
        int i = 0;
        while ( r->layers[i] != NULL )
            i++;
        layer **ll = calloc( i+1, sizeof(layer*) );
        if ( ll != NULL )
        {
            i = 0;
            while ( r->layers[i] != NULL )
            {
                ll[i] = r->layers[i];
                i++;
            }
            ll[i] = l;
            free( r->layers );
            r->layers = ll;
            res = 1;
        }
        else
            fprintf(stderr,"recipe: failed to expand layer array\n");
    }
    return res;
}
/**
 * Add a basic rule
 * @param r the recipe to add it to
 * @param xml_name the name of the xml element
 * @param aese_name the name of the aese property
 * @return the rule added
 */
static simplification *recipe_add_rule( recipe *r,
    const char *xml_name, const char *aese_name )
{
    int i;
    simplification **rules;
    simplification *s = simplification_new( xml_name, aese_name );
    int n_rules = count_rules( r->rules );
    rules = malloc( (n_rules+2)*sizeof(simplification*) );
    if ( rules == NULL )
        error("recipe: failed to reallocate recipe rules\n");
    for ( i=0;i<n_rules;i++ )
        rules[i] = r->rules[i];
    rules[i] = s;
    rules[i+1] = (simplification*)NULL;
    free( r->rules );
    r->rules = rules;
    return s;
}
/**
 * Count the number of removals
 * @param removals a NULL-terminated array of string pointers
 * @return the number of removals
 */
static int count_removals( char **removals )
{
    int i = 0;
    while ( removals[i] != NULL )
        i++;
    return i;
}
/**
 * Add a removal to the recipe
 * @param r the recipe to add it to
 * @param removal name of an element to remove (and all its descendants)
 */
static void recipe_add_removal( recipe *r, const char *removal )
{
    int i;
    char **removals;
    char *rm = strdup( removal );
    if ( rm == NULL )
        error( "recipe: failed to allocate for removal\n" );
    int n_removals = count_removals( r->removals );
    removals = malloc( sizeof(char*)*(n_removals+2) );
    if ( removals == NULL )
        error( "recipe: failed to reallocate removals\n");
    // copy old removals to new removals
    for ( i=0;i<n_removals;i++ )
        removals[i] = r->removals[i];
    removals[i+1] = NULL;
    removals[i] = rm;
    free( r->removals );
    r->removals = removals;
}
/**
 * Get an attribute from those available
 * @param name the name of the attribute
 * @param atts the expat attribute array
 * @return the attribute's value or NULL if not found
 */
static const char *get_attr( const char *name, const char **atts )
{
    int i = 0;
    while ( atts[i] != NULL )
    {
        if ( strcmp(atts[i],name)==0 )
            return atts[i+1];
        i += 2;
    }
    return NULL;
}
/**
 * Start element handler for XML file stripping. 
 * @param userData the user data (optional)
 * @param name the name of the element
 * @param atts an array of attributes terminated by a NULL
 * pointer
 */
static void XMLCALL start_recipe_element( void *userData,
	const char *name, const char **atts )
{
    recipe *r = (recipe*)userData;
    if ( strcmp(name,"rule")==0 )
    {
        const char *prop_name = get_attr( "prop_name", atts );
        const char *xml_name = get_attr( "xml_name", atts );
        if ( prop_name == NULL || xml_name == NULL )
        {
            warning( "recipe: missing attribute prop_name or "
                "xml_name for rule\n" );
            current_rule = NULL;
        }
        else
            current_rule = recipe_add_rule( r, xml_name, prop_name );
    }
    else if ( strcmp(name,"removal")==0 )
    {
        const char *rem_name = get_attr( "name", atts );
        if ( rem_name == NULL )
            warning( "recipe: missing removal name\n");
        else
            recipe_add_removal( r, rem_name );
    }
    else if ( strcmp(name,"attribute")==0 )
    {
        const char *attr_name = get_attr( "name", atts );
        const char *attr_value = get_attr( "value", atts );
        if ( attr_name == NULL || attr_value == NULL )
            warning( "recipe: missing attribute name or value\n");
        else if ( current_rule != NULL )
            recipe_add_attribute( current_rule, attr_name, attr_value );
    }
}
/**
 * End element handler for XML split
 * @param userData (optional)
 * @param name name of element
 */
static void XMLCALL end_recipe_element(void *userData,
	const char *name )
{
    if ( strcmp(name,"rule")==0 )
        current_rule = NULL;
}
/**
 * Load a recipe from its xml file
 * @param r the recipe object
 * @param buf the XML recipe file
 * @param len its length
 * @return a loaded recipe
 */
static recipe *recipe_load_xml( const char *buf, int len )
{
	recipe *r = recipe_new();
	XML_Parser lparser = XML_ParserCreate( NULL );
	if ( lparser == NULL )
        error("recipe: failed to create parser\n");
    else
    {
        XML_SetElementHandler( lparser, start_recipe_element,
            end_recipe_element );
        XML_SetUserData( lparser, r );
        if ( XML_Parse(lparser,buf,len,1) == XML_STATUS_ERROR )
        {
            printf(
                "%s at line %" XML_FMT_INT_MOD "u\n",
                XML_ErrorString(XML_GetErrorCode(lparser)),
                XML_GetCurrentLineNumber(lparser));
        }
        XML_ParserFree( lparser );
    }
    return r;
}
/**
 * Parse simplification rules
 * @param r the recipe to store them in
 * @param item the item containing the "rules" keyword
 */
static void recipe_parse_rules( recipe *r, cJSON *item )
{
    cJSON *obj = item->child;
    while ( obj != NULL )
    {
        const char *prop_name = NULL;
        const char *xml_name = NULL;
        const char *attr_name = NULL;
        const char *attr_value = NULL;
        cJSON *field = obj->child;
        while ( field != NULL )
        {
            if ( strcmp(field->string,"xml_name")==0 )
            {
                xml_name = field->valuestring;
            }
            else if ( strcmp(field->string,"prop_name")==0 )
            {
                prop_name = field->valuestring;
            }
            else if ( strcmp(field->string,"attribute")==0 )
            {
                if ( field->child != NULL )
                {
                    attr_name = field->child->string;
                    attr_value = field->child->valuestring;
                }
                else
                    warning("empty attribute\n");
            }
            field = field->next;
        }
        if ( prop_name == NULL || xml_name == NULL )
        {
            warning( "recipe: missing attribute prop_name or "
                "xml_name for rule\n" );
            current_rule = NULL;
        }
        else
        {
            current_rule = recipe_add_rule( r, xml_name, prop_name );
            if ( current_rule != NULL && attr_name != NULL 
                && attr_value != NULL )
                recipe_add_attribute( current_rule, attr_name, 
                    attr_value );
        }
        obj = obj->next;
    }
}
/**
 * Parse a milestone set
 * @param item the parent of the milestone set
 * @return a list of milestones all in the set
 */
static milestone *parse_milestone_set( cJSON *item )
{
    milestone *list = NULL;
    cJSON *obj = item->child;
    while ( obj != NULL )
    {
        cJSON *field = obj->child;
        while ( field != NULL )
        {
            if ( strcmp(field->string,"xml_name")==0 )
            {
                milestone *m = milestone_create(field->valuestring);
                if ( m != NULL )
                {
                    if ( list == NULL )
                        list = m;
                    else
                        milestone_append(list,m);
                }
                else    // error reported in milestone
                    break;
            }
            field = field->next;
        }
        obj = obj->next;
    }
    return list;
}
/**
 * Parse the layers element
 * @param r the recipe to store the layers in
 * @param item the item containing the "layers" element
 */
static void recipe_parse_layers( recipe *r, cJSON *item )
{
    cJSON *obj = item->child;
    while ( obj != NULL )
    {
        cJSON *field = obj->child;
        char *name = NULL;
        milestone *milestones = NULL;
        while ( field != NULL )
        {
            if ( strcmp(field->string,"name")==0 )
                name = field->valuestring;
            else if ( strcmp(field->string,"milestones")==0 )
                milestones = parse_milestone_set( field );
            field = field->next;
        }
        // now build the layer
        if ( name != NULL && milestones != NULL )
        {
            layer *l = layer_create( name, milestones );
            if ( l != NULL )
            {
                if ( !recipe_add_layer(r,l) )
                    break;
                milestones = NULL;
                name = NULL;
            }
            else
                break;
        }
        obj = obj->next;
    }
}
/**
 * Parse a json recipe file. Should be simple.
 * @param root the root element of the JSON tree
 * @param r a recipe object to fill in
 */
static void recipe_parse_json( recipe *r, cJSON *root )
{
    cJSON *item = root->child;
    while ( item != NULL )
    {
        if ( strcmp(item->string,"rules")==0 )
            recipe_parse_rules( r, item );
        else if ( strcmp(item->string,"layers")==0 )
            recipe_parse_layers( r, item );
        else if ( strcmp(item->string,"type")==0 )
        {
            if ( item->valuestring==NULL
                ||strcmp(item->valuestring,"stripper")!=0 )
            {
                error("incorrect recipe type %s\n",item->valuestring);
                break;
            }
            // else ignore it
        }
        else if ( strcmp(item->string,"removals")==0 )
        {
        	cJSON *child = item->child;
            while ( child != NULL )
            {
                if ( child->valuestring != NULL )
                    recipe_add_removal( r, child->valuestring );
                child = child->next;
            }
        }
		item = item->next;
    }
}
/**
 * Load a recipe from its json file
 * @param r the recipe object
 * @param buf the JSON recipe file as a string
 * @return a loaded recipe or NULL 
 */
static recipe *recipe_load_json( const char *buf )
{
    recipe *r = recipe_new();
    if ( r != NULL )
    {
        cJSON *root = cJSON_Parse( buf );
        if ( root != NULL )
        {
            recipe_parse_json( r, root );
            cJSON_Delete( root );
        }
        else
            warning("parse of JSON config failed!\n");
    }
    return r;
}
/**
 * Is the first non-whitespace character a '<' or '{' etc?
 * @param buf the string to test
 * @param first the first character
 * @return 1 or 0
 */
static int begins_with( const char *buf, char first )
{
    int i = 0;
    while ( buf[i] != 0 )
    {
        if ( !isspace(buf[i]) )
        {
            if ( buf[i] == first )
                return 1;
            else
                return 0;
        }
        i++;
    }
    return 0;
}
/**
 * Load a recipe from a json OR xml file
 * @param r the recipe object
 * @param buf the recipe file as a string
 * @return a loaded recipe or NULL 
 */
recipe *recipe_load( const char *buf, int len )
{
    if ( begins_with(buf,'<') )
        return recipe_load_xml( buf, len );
    else if ( begins_with(buf,'{') )
        return recipe_load_json( buf );
    else
    {
        warning("invalid config format\n");
        return NULL;
    }
}
/**
 * Simplify an XML element and its attributes. Assume that the recipe
 * already matches the element.
 * @param r the recipe to use
 * @param name the name of the XML element
 * @param attrs copy of its attributes (will be modified!)
 * @return the name of the Aese property
 */
char *recipe_simplify( recipe *r, char *name, char **attrs )
{
    simplification *s = recipe_has_rule( r, (const char*)name,
        (const char**)attrs );
    if ( s != NULL )
    {
        simplification_remove_attribute( s, attrs );
        return simplification_get_prop_name(s);
    }
    return NULL;
}
/**
 * Does this recipe contain a rule for the current element?
 * @param r the recipe to use
 * @param name the name of the XML element
 * @param attrs its attributes from expat
 * @return pointer to the rule for this element, else NULL
 */
simplification *recipe_has_rule( recipe *r, const char *name,
    const char **attrs )
{
    int i=0;
    while ( r->rules[i] != NULL )
    {
        if ( strcmp(simplification_get_xml_name(r->rules[i]),name)==0
            && simplification_contains(r->rules[i],(char**)attrs) )
            return r->rules[i];
        i++;
    }
    return NULL;
}
/**
 * Count the number of layers
 * @param r the recipe to count layers for
 * @return the number of layers defined in the recipe
 */
int recipe_num_layers( recipe *r )
{
    int i = 0;
    if ( r->layers != NULL )
    {
        while ( r->layers[i] != NULL )
            i++;
    }
    return i;
}       
/**
 * Get a specific layer;
 * @param r the recipe
 * @param i its index
 * @return the layer
 */
layer *recipe_layer( recipe *r, int i )
{
    return r->layers[i];
}
/**
 * Dispose of a recipe and all its children
 * @param r the recipe
 * @return NULL;
 */
recipe *recipe_dispose( recipe *r )
{
    int i;
    if ( r->removals != NULL )
    {
        i = 0;
        while ( r->removals[i] != NULL )
            free( r->removals[i++] );
        free( r->removals );
    }
    if ( r->rules != NULL )
    {
        i = 0;
        while ( r->rules[i] != NULL )
            simplification_delete( r->rules[i++] );
        free( r->rules );
    }
    if ( r->layers != NULL )
    {
        i = 0;
        while ( r->layers[i] != NULL )
            layer_dispose( r->layers[i++] );
        free( r->layers );
    }
    free( r );
    return NULL;
}
/**
 * Do we have the given element in our removals list?
 * @param r the recipe in question
 * @param removal the name of the element that may be removed
 * @return 1 if it is slated for removal, 0 otherwise
 */
int recipe_has_removal( recipe *r, const char *removal )
{
    int i = 0;
    while ( r->removals[i] != NULL )
    {
        if ( strcmp(removal,r->removals[i++])==0 )
            return 1;
    }
    return 0;
}