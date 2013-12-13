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
/**
 * Represent a css rule, which has selectors, like element.class, and
 * properties, like font-size:12px;
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "css_property.h"
#include "css_selector.h"
#include "hashmap.h"
#include "attribute.h"
#include "annotation.h"
#include "css_rule.h"
#include "error.h"
#include "memwatch.h"

struct css_rule_struct
{
	css_selector **selectors;
	css_property **properties;
	int num_selectors;
	int num_properties;
	/* index into selectors of chosen selector */
	int index;
};
/**
 * Create a css rule.
 */
css_rule *css_rule_create()
{
	css_rule *rule = calloc( 1, sizeof(css_rule) );
	if ( rule == NULL )
		error( "css_rule: failed to allocate css rule\n" );
	return rule;
}
/**
 * Delete a rule
 * @param rule the rule to delete
 */
void css_rule_dispose( css_rule *rule )
{
    int i;
    for ( i=0;i<rule->num_selectors;i++ )
        css_selector_dispose( rule->selectors[i] );
	for ( i=0;i<rule->num_properties;i++ )
        css_property_dispose( rule->properties[i] );
	if ( rule->selectors != NULL )
        free( rule->selectors );
	if ( rule->properties != NULL )
        free( rule->properties );
	free( rule );
}
/**
 * Clone a css rule
 */
css_rule *css_rule_clone( css_rule *old )
{
	int i;
	css_rule *rule = calloc( 1, sizeof(css_rule) );
	if ( rule == NULL )
		error( "css_rule: failed to allocate css rule\n" );
	rule->selectors = calloc( old->num_selectors, sizeof(css_selector*) );
	if ( rule->selectors == NULL )
		error( "css_rule: failed to allocate css rule selectors\n" );
	rule->properties = calloc( old->num_properties, sizeof(css_property*) );
	if ( rule->properties == NULL )
		error( "css_rule: failed to allocate css rule properties\n" );
	rule->index = 0;
	rule->num_properties = old->num_properties;
	rule->num_selectors = old->num_selectors;
	for ( i=0;i<rule->num_selectors;i++ )
		rule->selectors[i] = css_selector_clone(old->selectors[i]);
	for ( i=0;i<rule->num_properties;i++ )
		rule->properties[i] = css_property_clone(old->properties[i]);
	return rule;
}
/**
 * Is this css rule valid?
 * @return 1 if it is, 0 otherwise
 */
int css_rule_valid( css_rule *rule )
{
	return rule->selectors != NULL;
}
/**
 * Add a property to the rule
 * @param rule the rule to add
 * @param prop the property to add
 * @return 1 if it succeeded, else 0
 */
int css_rule_add_property( css_rule *rule, css_property *prop )
{
	int i,res = 0;
	css_property **new_props;
	css_property **p = rule->properties;
	new_props = calloc( rule->num_properties+1,sizeof(css_property*) );
	if ( new_props != NULL )
    {
        if ( rule->num_properties > 0 )
        {
            p = rule->properties;
            // copy old properties over
            for ( i=0;i<rule->num_properties;i++ )
                new_props[i] = p[i];
            // copy in the new property
            free( rule->properties );
            rule->properties = NULL;
        }
        new_props[rule->num_properties] = prop;
        rule->num_properties++;
        rule->properties = new_props;
        res = 1;
    }
    else
        warning( "css_rule: failed to allocate new properties in css rule\n" );
    return res;
}
/**
 * Add a selector to the rule
 * @param rule the rule to add
 * @param sel the selector to add
 * @return 1 if successful, else 0
 */
int css_rule_add_selector( css_rule *rule, css_selector *sel )
{
	int i,res=1;
	css_selector **new_sels;
	css_selector **s = rule->selectors;
	new_sels = malloc( sizeof(css_selector*)*(rule->num_selectors+1) );
	if ( new_sels == NULL )
    {
		warning( "css_rule: failed to allocate new selectors in css rule\n" );
        res = 0;
    }
    else
    {
        s = rule->selectors;
        rule->num_selectors++;
        // copy old selectors over
        for ( i=0;i<rule->num_selectors-1;i++ )
            new_sels[i] = s[i];
        // copy in the new property
        new_sels[rule->num_selectors-1] = sel;
        if ( rule->selectors != NULL )
        {
            free( rule->selectors );
            rule->selectors = NULL;
        }
        rule->selectors = new_sels;
    }
    return res;
}
/**
 * Look for a matching property in the rule set
 * @param r the rule to test
 * @param xml_name the xml name of the attribute
 * @return NULL (usually) unless you find a matching property
 */
css_property *css_rule_get_property( css_rule *r, char *xml_name )
{
    int i=0;
    for ( i=0;i<r->num_properties;i++ )
    {
        if ( strcmp(xml_name,css_property_get_xml_name(r->properties[i]))==0 )
            return r->properties[i];
    }
    return NULL;
}
/**
 * Does the class name of one of the selectors match the given property?
 * @param rule the rule to test
 * @param name the xml_name to match
 * @return 1 if a match, 0 otherwise
 */
int css_rule_match( css_rule *rule, char *name )
{
	int i;
	for ( i=0;i<rule->num_selectors;i++ )
	{
        char *class_name = css_selector_get_class( rule->selectors[i] );
		if ( strcmp(class_name,name)==0 )
		{
			rule->index = i;
			return 1;
		}
	}
	return 0;
}
/**
 * Get the rule's html name
 * @param rule the rule in question
 * @return a string
 */
char *css_rule_get_element( css_rule *rule )
{
	return css_selector_get_element(rule->selectors[rule->index]);
}
/**
 * Get the rule's xml name or html class name
 * @param rule the rule in question
 * @return a string
 */
char *css_rule_get_class( css_rule *rule )
{
	return css_selector_get_class(rule->selectors[rule->index]);
}
/**
 * Get a property given its index
 * @param r the rule in question
 * @param index the index of the property
 * @return NULL if not there, else the property
 */
css_property *css_rule_get_property_by_index( css_rule *r, int index )
{
    if ( index < r->num_properties )
        return r->properties[index];
    else
        return NULL;
}
/**
 * Get the number of properties specified by this rule
 * @param r the rule in question
 * @return the number of properties
 */
int css_rule_get_num_properties( css_rule *r )
{
    return r->num_properties;
}
/**
 * Print a single css property. This is really simple
 * because we only parse custom properties. This is NOT a
 * general CSS parser. This is just for debugging.
 * @param p the property to print
 */
static void css_rule_print_property( css_property *p )
{
	fprintf( stderr, "    -aese-%s: %s;\n", css_property_get_xml_name(p),
        css_property_get_html_name(p) );
}
/**
 * Print a single selector. Since there may be several
 * to a rule we don't output CRs. This is just for debug.
 * @param s the selector to print
 */
static void css_rule_print_selector( css_selector *s )
{
	if ( strlen(css_selector_get_element(s)) > 0 )
		fprintf( stderr, "%s",css_selector_get_element(s) );
	if ( strlen(css_selector_get_class(s))>0 )
		fprintf( stderr,".%s",css_selector_get_class(s) );
}
/**
 * Print a single css rule to stdout.
 * @param the rule to print
 */
void css_rule_print( css_rule *rule )
{
	int i = 0;
	for ( i=0;i<rule->num_selectors;i++ )
	{
		css_rule_print_selector( rule->selectors[i++] );
		if ( i<rule->num_selectors-1 )
			fprintf( stderr,", ");
	}
	fprintf( stderr,"\n{\n");
	for ( i=0;i<rule->num_properties;i++ )
		css_rule_print_property( rule->properties[i] );
	fprintf( stderr,"}\n");
}
/**
 * Compare two css rules
 * @param key1 the first css rule
 * @param key2 the second css rule
 * @return 1 if key1>key2, else if equal 0, else -1
 */
int css_rule_compare( void *key1, void *key2 )
{
    char *class1 = css_rule_get_class( key1 );
    char *class2 = css_rule_get_class( key2 );
    return strcmp( class1, class2 );
}
