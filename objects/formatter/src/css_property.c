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
#include <ctype.h>
#include <stdio.h>
#include "css_property.h"
#include "error.h"
#include "memwatch.h"

#define AESE_PREFIX "-aese-"
#define AESE_PREFIX_LEN 6

/**
 * A css property is a property like font-weight: bold. Except that we
 * extend it by prefixing "-aese-xmlname: htmlname. This is for copying 
 * over attributes from XML to HTML.
 */
struct css_property_struct
{
	/* name of the xml-attribute */
	char *xml_name;
	/* if NULL then we only match not replace */
	char *html_name;
    /* property value to be set */
    char *html_value;
};

/**
 * Delete a property and free its memory
 * @param p the property in question
 */
void css_property_dispose( css_property *p )
{
    if ( p->xml_name != NULL )
    {
        free( p->xml_name );
        p->xml_name = NULL;
    }
    if ( p->html_name != NULL )
    {
        free( p->html_name );
        p->html_name = NULL;
    }
    if ( p->html_value != NULL )
    {
        free( p->html_value );
        p->html_name = NULL;
    }
    free( p );
    p = NULL;
}
/**
 * Clone a property deeply
 * @param p the original one to copy
 * @return the clone
 */
css_property *css_property_clone( css_property *p )
{
    css_property *copy = calloc( 1, sizeof(css_property) );
    if ( copy == NULL )
        error( "css_property: failed to duplicate property struct\n");
    if ( p->xml_name != NULL )
    {
        copy->xml_name = strdup(p->xml_name);
        if ( copy->xml_name == NULL )
            error("css_property: failed to duplicate xml_name "
                "field during clone\n");
    }
    if ( p->html_name != NULL )
    {
        copy->html_name = strdup(p->html_name);
        if ( copy->html_name == NULL )
            error("css_property: failed to duplicate xml_name "
                "field during clone\n");
    }
    if ( p->html_value != NULL )
    {
        copy->html_value = strdup(p->html_value);
        if ( copy->html_value == NULL )
            error("css_property: failed to duplicate html_value "
                "field during clone\n");
    }
    return copy;
}
/**
 * Get the property's html name
 * @param p the property in question
 * @return a string being the html attribute name
 */
char *css_property_get_html_name( css_property *p )
{
    return p->html_name;
}
/**
 * Get the property's xml name
 * @param p the property in question
 * @return a string being the xml attribute name
 */
char *css_property_get_xml_name( css_property *p )
{
    return p->xml_name;
}/**
 * Get the html property value
 * @param p the property in question
 * @return a string being the property value
 */
char *css_property_get_html_value( css_property *p )
{
    return p->html_value;
}
/**
 * Remove all instances of the given char from the string in situ
 * @param str the string to remove it from
 * @param c the char to remove
 */
static void strip_char( char *str, char c )
{
    int i = 0;
    int j = 0;
    while ( str[i] != 0 )
    {
        if ( str[i] != c )
        {
            if ( i > j )
                str[j] = str[i];
            j++;
        }
        i++;
    }
    str[j] = 0;
}
/**
 * Parse a single property from the raw CSS data. Ignore any property
 * not beginning with "-aese-". Such properties specify
 * the xml attribute name and its corresponding html name. The attribute
 * value is the same in both cases. Such properties are supposed to be
 * ignored by browser-based css parsers
 * @param data the raw CSS data read from the file
 * @param len the length of the property in data
 * @return an allocated css_property (caller must eventually dispose it)
 */
css_property *css_property_parse( const char *data, int len )
{
    // format: -aese-xml_name: html_name;
    // copy the attribute value over from the xml unchanged (not here)
	int i,start=0, end=len;
    css_property *prop_temp = NULL;
    // the property name had an escaped ":"
	int escaped = 0;
    while ( start<end && isspace(data[start]) )
    {
		start++;
        end--;
    }
	if ( strncmp(&data[start],AESE_PREFIX,AESE_PREFIX_LEN)==0 )
    {
        start += AESE_PREFIX_LEN;
        i = start;
        while ( i < end )
        {
            if ( data[i]=='\\' )
            {
                escaped = 1;
                i+=2;
            }
            else if ( data[i] == ':' )
            {
                // parse left hand side
                prop_temp = calloc( 1, sizeof(css_property) );
                if ( prop_temp != NULL )
                {
                    int lhs_len = i-start;
                    prop_temp->xml_name = calloc( 1, lhs_len+1 );
                    if ( prop_temp->xml_name != NULL )
                    {
                        strncpy( prop_temp->xml_name, &data[start], lhs_len );
                        if ( escaped )
                            strip_char(prop_temp->xml_name,'\\');
                        // parse right hand side
                        i++;
                        while ( i<end && isspace(data[i]) )
                            i++;
                        while ( end>i && isspace(data[end]) )
                            end--;
                        if ( i < end )
                        {
                            int rhs_len = (end-i)+1;
                            prop_temp->html_name = calloc( 1, rhs_len+1 );
                            if ( prop_temp->html_name != NULL )
                            {
                                strncpy( prop_temp->html_name, &data[i], 
                                    rhs_len );
                                break;
                            }
                            else
                            {
                                css_property_dispose( prop_temp );
                                warning("css_property: failed to allocate"
                                    "html name\n");
                                prop_temp = NULL;
                            }
                        }
                        else
                        {
                            warning("css_property: missing html name\n");
                            css_property_dispose( prop_temp );
                            prop_temp = NULL;
                        }
                    }
                    else
                    {
                        css_property_dispose( prop_temp );
                        warning( "css_property: failed to allocate xml_name\n");
                        prop_temp = NULL;
                    }
                }
                else
                {
                    warning("css_property: failed to allocate css_property\n");
                }
            }
            i++;
        }
    }
	return prop_temp;
}
/**
 * Set the html value of this property
 * @param p the property in question
 * @param value the value for the HTML attribute
 */
void css_property_set_html_value( css_property *p, char *value )
{
    p->html_value = strdup( value );
    if ( p->html_value == NULL )
        warning( "css_property: failed to duplicate html value\n" );
}