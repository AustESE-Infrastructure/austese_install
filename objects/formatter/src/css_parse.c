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
 * A very basic CSS parser. Don't use this for serious CSS work.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include "css_selector.h"
#include "css_property.h"
#include "attribute.h"
#include "hashmap.h"
#include "annotation.h"
#include "css_rule.h"
#include "hashset.h"
#include "range.h"
#include "range_array.h"
#include "formatter.h"
#include "css_parse.h"
#include "error.h"
#include "memwatch.h"


/**
 * Get the selectors (or just one) for a css rule.
 * Selectors are separated by commas.
 * @param data the raw data read from the css file
 * @param start the offset at which the selector starts in data
 * @param len the length of the selector's data
 * @param rule the rule to fill in with selectors
 * @return 1 if 
 */
static int get_selectors( const char *data, int start, int len, css_rule *rule )
{
    int res = 0;
	int i = start;
	int end = start + len;
	int last_sel_start = start;
	while ( i < end )
	{
		if ( data[i] == ',' || i == end-1 )
		{
			css_selector *s = css_selector_parse( &data[last_sel_start],
				(i-last_sel_start)+1 );
            if ( s != NULL )
            {
				res = css_rule_add_selector( rule, s );
                if ( !res )
                    break;
            }
			last_sel_start = i+1;
		}
		i++;
	}
    return res;
}
/**
 * Get the properties for a css rule. Properties are
 * separated by semi-colons.
 * @param data the start of the rule body
 * @param len the length of the properties' data
 * @param rule the rule to fill out
 * @return 1 if it succeeded, else 0
 */
static int get_properties( const char *data, int len, css_rule *rule )
{
	int res = 1,i = 0;
	int end = len;
	int last_prop_start = 0;
	while ( i < end )
	{
		if ( data[i] == ';' || i == end-1 )
		{
			css_property *p = css_property_parse( &data[last_prop_start],
                i-last_prop_start );
            if ( p != NULL )
			{
				res = css_rule_add_property( rule, p );
                if ( !res )
                    break;
			}
			last_prop_start = i+1;
		}
		i++;
	}
    return res;
}
/**
 * Read the next rule from the input data
 * @param data the CSS data to read from
 * @param offset VAR param of the offset into data to read from
 * @return the rule or NULL
 */
static css_rule *get_css_rule( const char *data, int *offset )
{
	int pos = *offset;
	int state = 0;
	int bodyStart=-1,bodyEnd=-1;
	css_rule *rule = NULL;
	while ( state >= 0 && data[pos] != 0 )
	{
		switch ( state )
		{
			case 0:	// collecting selector
				if ( data[pos] == '{' )
				{
					bodyStart = pos;
					state = 1;
				}
				pos++;
				break;
			case 1:	// collecting body
				if ( data[pos] == '}' )
				{
					bodyEnd = pos;
					state = -1;
				}
				pos++;
				break;
		}
	}
	if ( (bodyStart == -1 || bodyEnd == -1)&&state!=0 )
	{
		warning("failed to find css rule body", 0 );
		return NULL;
	}
	else
	{
		rule = css_rule_create();
		if ( rule != NULL )
        {
            int res = get_selectors( data, *offset, bodyStart-(*offset), rule );
            if ( res )
                res = get_properties( &data[bodyStart+1], (bodyEnd-bodyStart), 
                    rule );
        }
        // update pos even if rule-reading fails
        *offset = pos;
        if ( css_rule_valid(rule) )
            return rule;
        else if ( rule != NULL )
            css_rule_dispose( rule );
        return NULL;
	}
}
/**
 * Print a css-node
 * @param key a css_rule in disguise
static void print_css_rule( void *key )
{
    css_rule *rule = key;
    char *element = css_rule_get_element( rule );
    char *class_name = css_rule_get_class( rule );
    //fprintf( stderr, "css_rule element=%s class=%s\n", element, class_name );
}
 */
/**
 * Parse a CSS file. Add css rules to the tree if they are mentioned as 
 * properties in the markup file.
 * @param data the css data to parse
 * @param len its length
 * @param props the properties from all the markup files
 * @param css store the css rules in here
 * @return 1 if it succeeded, else 0
 */
int css_parse( const char *data, int len, hashset *props, hashmap *css )
{
	int offset = 0;
    do
    {
        css_rule *rule = get_css_rule( data, &offset );
        if ( rule != NULL )
        {
            char *class_name = css_rule_get_class(rule);
            // only put into the css properties seen in the markup
            if ( hashset_contains(props,class_name) )
            {
                //fprintf(stderr,"adding class %s\n",class_name);
                if ( hashmap_contains(css,class_name) )
                {
                    css_rule *r = hashmap_get(css,class_name);
                    css_rule_dispose( r );
                }
                hashmap_put( css, class_name, rule );
            }
            else
            {
                //fprintf(stderr,"disposing of class %s\n",class_name);
                css_rule_dispose( rule );
                //fprintf(stderr,"ended dispose\n");
            }
        }
        // point beyond closing brace
        offset++;
    } while ( offset < len );
    //fprintf( stderr, "contents of css rule hashmap:\n" );
    //hashmap_print( css, (print_value)css_rule_print );
    return 1;
}