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
 * The purpose of this module is to render a particular standoff markup
 * format called "AESE", with the addition of the plain base text and a
 * CSS file, into HTML. There might be formats other than AESE, so this
 * file needs to be called in an interchangeable way. The ranges read and
 * translated here must already be non-overlapping. Overlap should have
 * been resolved before the only public routine, load_markup, is called.
 * @author Desmond Schmidt (c) 2011
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "expat.h"
#include "css_property.h"
#include "css_selector.h"
#include "hashmap.h"
#include "attribute.h"
#include "annotation.h"
#include "range.h"
#include "css_rule.h"
#include "range_array.h"
#include "hashset.h"
#include "formatter.h"
#include "AESE/AESE.h"
#include "plain_text.h"
#include "error.h"

#ifdef XML_LARGE_SIZE
#if defined(XML_USE_MSC_EXTENSIONS) && _MSC_VER < 1400
#define XML_FMT_INT_MOD "I64"
#else
#define XML_FMT_INT_MOD "ll"
#endif
#else
#define XML_FMT_INT_MOD "l"
#endif

struct userdata_struct
{
    range_array *ranges;
    hashset *props;
    int absolute_off;
    range *current;
};
/** the parser */
static XML_Parser parser;


/**
 * Go through the list of attributes and return the
 * value of the named attribute.
 * @param atts a NULL-terminated list of attributes
 * @param name the name of the desired attribute
 * @return the attribute's value or NULL if not found
 */
static char *get_attr_value( const char **atts, const char *name )
{
	int i = 0;
	while ( atts[i] != NULL )
	{
		if ( strcmp(atts[i],name)==0 )
			return (char*)atts[i+1];
		i += 2;
	}
	return NULL;
}
/**
 * Get the int value of an attribute
 * @param atts a NULL-terminated list of attributes
 * @param name the name of the desired attribute
 * @return the integer value of the attribute
static int get_attr_int_value( const char **atts, const char *name )
{
	char *value = get_attr_value( atts, name );
	if ( value == NULL )
		error(" failed to find attribute %s", 1, name );
	return atoi( value );
}
 */
/**
 * Start element handler for the markup file.
 * @param userData a userdata_struct
 * @param name the name of the element
 * @param atts an array of attributes terminated by a NULL pointer
 */
static void XMLCALL start_element_scan( void *userData, const char *name,
    const char **atts )
{
    struct userdata_struct *u = userData;
    if ( strcmp(name,"range")==0 )
	{
        char *removed = get_attr_value( atts,"removed" );
        // clear value from last range
        if ( removed == NULL || strcmp(removed,"true")!= 0 )
        {
            u->current = range_create_atts( atts );
            if ( u->current != NULL )
            {
                u->absolute_off += range_get_reloff( u->current );
                range_set_absolute( u->current, u->absolute_off );
            }
            else
                warning("AESE: invalid range\n");
        }
        else
            range_set_removed( u->current, 1 );
	}
    else if ( strcmp("annotation",name)==0 )
	{
		annotation *a = annotation_create( atts );
        if ( a != NULL && u->current != NULL )
            range_add_annotation( u->current, a );
	}
}

/**
 * End element handler for formatter. When we encounter a
 * range end we can always write out the start-element
 * @param userData (optional)
 * @param name name of element
 */
static void XMLCALL end_element_scan( void *userData, const char *name )
{
    struct userdata_struct *u = userData;
	if ( strcmp(name,"range")==0 )
	{
        // why would this work in XML?
        if ( u->current != NULL )
        {
            char *prop_name = range_name( u->current );
            range_array_add( u->ranges, u->current );
            if ( hashset_contains(u->props, prop_name)==0 )
                hashset_put( u->props, prop_name );
        }
	}
}
/**
 * Load the markup file with NON-overlapping ranges, reading it using expat.
 * @param mdata the overlapping markup data
 * @param mlen its length
 * @param ranges the loaded standoff ranges sorted on absolute offsets
 * @param props store in here the names of all the properties
 * @return 1 if it loaded successfully, else 0
 */
int load_aese_markup( const char *mdata, int mlen, range_array *ranges, hashset *props )
{
    int res = 0;
    struct userdata_struct userdata;
    userdata.props = props;
    userdata.ranges = ranges;
    userdata.absolute_off = 0;
    parser = XML_ParserCreate( NULL );
    if ( parser != NULL )
    {
        XML_SetElementHandler( parser, start_element_scan, end_element_scan );
        XML_SetUserData( parser, &userdata );
        if ( XML_Parse(parser,mdata,mlen,1) == XML_STATUS_ERROR )
        {
            warning(
                "AESE: %s at line %" XML_FMT_INT_MOD "u\n",
                XML_ErrorString(XML_GetErrorCode(parser)),
                XML_GetCurrentLineNumber(parser));
        }
        else
            res = 1;
        XML_ParserFree( parser );
    }
    else
        warning("AESE: failed to create parser\n");
    //hashset_print( props );
    return res;
}
