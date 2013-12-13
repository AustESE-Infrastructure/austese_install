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
#include "hashmap.h"
#include "attribute.h"
#include "annotation.h"
#include "css_property.h"
#include "css_selector.h"
#include "css_rule.h"
#include "error.h"
#include "memwatch.h"

/**
 * An annotation is a former XML attribute, applied to a range.
 */
struct annotation_struct
{
    char *name;
    char *value;
    annotation *next;
};
/**
 * Create an annotation using a simple name value pair
 * @param name the annotation name
 * @param value the annotation value
 * @return the annotation
 */
annotation *annotation_create_simple( char *name, char *value )
{
    annotation *a = calloc( 1, sizeof(annotation) );
    if ( a != NULL )
    {
        a->name = strdup( name );
        a->value = strdup( value );
    }
    else
        warning("annotation: failed to allocate annotation\n");
    return a;
}
/**
 * Create a new annotation
 * @param atts a NULL-terminated list of a single name-value pair
 * @return a list of annotation object or NULL
 */
annotation *annotation_create( const char **atts )
{
    annotation *head = calloc( 1, sizeof(annotation) );
    annotation *a = head;
    if ( a != NULL )
    {
        int i = 0;
        while ( atts[i] != NULL )
        {
            if ( strcmp(atts[i],"name")==0 )
            {
                a->name = strdup( (char*)atts[i+1] );
                if ( a->name == NULL )
                {
                    annotation_dispose( a );
                    if ( head == a )
                        head = NULL;
                    a = NULL;
                    break;
                }
            }
            else if ( strcmp(atts[i],"value")==0 )
            {
                a->value = strdup( (char*)atts[i+1] );
                if ( a->value == NULL )
                {
                    annotation_dispose( a );
                    if ( head == a )
                        head = NULL;
                    a = NULL;
                    break;
                }
            }
            i += 2;
            if ( head != a )
                annotation_append(head,a);
        }
    }
    else
        warning("annotation: failed to allocate annotation\n");
    return head;
}
/**
 * Clone an annotation
 * @param a the annotation to clone
 * @return the clone
 */
annotation *annotation_clone( annotation *a )
{
    annotation *b = calloc( 1, sizeof(annotation) );
    if ( b != NULL )
    {
        b->name = strdup( a->name );
        b->value = strdup( a->value );
        if ( b->value == NULL || b->name == NULL )
        {
            annotation_dispose( b );
            warning("annotation: failed to duplicate name/value pair\n");
            b = NULL;
        }
    }
    else
        warning("annotation: failed to duplicate annotation\n");
    return b;
}
/**
 * Dispose of the memory of an annotation safely
 * @param a the annotation to dispose of
 */
void annotation_dispose( annotation *a )
{
    if ( a->name != NULL )
    {
        free( a->name );
        a->name = NULL;
    }
    if ( a->value != NULL )
    {
        free( a->value );
        a->value = NULL;
    }
    a->next = NULL;
    free( a );
    a = NULL;
}
/**
 * Get the name of this annotation
 * @param a the annotation in question
 * @return a string being its name
 */
char *annotation_get_name( annotation *a )
{
    return a->name;
}
/**
 * Get the value of this annotation
 * @param a the annotation in question
 * @return a string being its value
 */
char *annotation_get_value( annotation *a )
{
    return a->value;
}
/**
 * Get the next annotation in the list
 * @param a the annotation in question
 * @return the next annotation
 */
annotation *annotation_get_next( annotation *a )
{
    return a->next;
}
/**
 * Print an annotation
 * @param a the annotation to print
 */
void annotation_print( annotation *a )
{
    fprintf( stderr,"   name: %s value: %s\n",a->name,a->value);
}
/**
 * Add one annotation onto the end of this one
 * @param a the first annotation
 * @param b the one to append
 */
void annotation_append( annotation *a, annotation *b )
{
    if ( a->next == NULL )
        a->next = b;
    else
        annotation_append( a->next, b );
}
/**
 * Convert an annotation to an HTML attribute via css rules
 * @param xml_name the xml name of the property
 * @param a the annotation to convert
 * @param css_rules the css rules to use for transformation
 * @return a shiny new attribute or NULL if we failed
 */
attribute *annotation_to_attribute( annotation *a, char *xml_name, 
    hashmap *css_rules )
{
    css_rule *rule = hashmap_get( css_rules, xml_name );
    if ( rule != NULL )
    {
        css_property *prop = css_rule_get_property( rule, a->name );
        if ( prop != NULL )
        {
            char *html_name = css_property_get_html_name( prop );
            //warning("annotation: creating attribute %s:%s\n",html_name,a->value);
            return attribute_create( html_name, annotation_get_name(a), a->value );
        }
        // ignore this
        //else
        //    warning("annotation: failed to find property for %s:%s:%s\n",
        //        xml_name,a->name,a->value);
    }
    else
        warning("annotation: failed to find css rule %s\n",xml_name);
    return NULL;
}