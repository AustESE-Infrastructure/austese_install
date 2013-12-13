#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "attribute.h"
#include "hashmap.h"
#include "annotation.h"
#include "range.h"
#include "range_array.h"
#include "hashset.h"
#include "formatter.h"
#include "master.h"
#include "AESE/AESE.h"
#include "STIL/STIL.h"
#include "error.h"

#include "memwatch.h"
static format formats[]={{"AESE",load_aese_markup},{"STIL",load_stil_markup}};
static int num_formats = sizeof(formats)/sizeof(format);
static char error_string[128] = "";
struct master_struct
{
    char *text;
    int tlen;
    int html_len;
    int has_css;
    int has_markup;
    int has_text;
    int selected_format;
    formatter *f;
};
/**
 * Create a aese formatter
 * @param text the text to format
 * @param len the length of the text
 * @return an initialised master instance
 */
master *master_create( char *text, int len )
{
    master *hf = calloc( 1, sizeof(master) );
    if ( hf != NULL )
    {
        hf->has_text = 0;
        hf->has_css = 0;
        hf->has_markup = 0;
        if ( text != NULL )
        {
            hf->tlen = len;
            if ( hf->tlen > 0 )
            {
                hf->f = formatter_create( hf->tlen );
                hf->text = text;
                hf->has_text = 1;
            }
        }
        
    }
    else
        error("master: failed to allocate instance\n");
    return hf;
}
/**
 * Dispose of a aese formatter
 */
void master_dispose( master *hf )
{
    if ( hf->f != NULL )
        formatter_dispose( hf->f );
    free( hf );
}
/**
 * Look up a format in our list.
 * @param fmt_name the format's name
 * @return its index in the table or 0
 */
static int master_lookup_format( const char *fmt_name )
{
    int i;
    for ( i=0;i<num_formats;i++ )
    {
        if ( strcmp(formats[i].name,fmt_name)==0 )
            return i;
    }
    return 0;
}
/**
 * Load the markup file (possibly one of several)
 * @param hf the master in question
 * @param markup a markup string
 * @param mlen the length of the markup
 * @param fmt the format
 * return 1 if successful, else 0
 */
int master_load_markup( master *hf, const char *markup, int mlen, 
    const char *fmt )
{
    int res = 0;
    //fprintf(stderr,"mlen=%d markup=%s\n",mlen,markup);
    hf->selected_format = master_lookup_format( fmt );
    if ( hf->selected_format >= 0 )
    {
        res = formatter_load_markup( hf->f, 
            formats[hf->selected_format].lm, markup, mlen );
        if ( res && !hf->has_markup )
            hf->has_markup = 1;
    }
    return res;
}
/**
 * Load a css file
 * @param hf the master in question
 * @param css the css data
 * @param len its length
 * return 1 if successful, else 0
 */
int master_load_css( master *hf, const char *css, int len )
{
    int res = formatter_css_parse( hf->f, css, len );
    if ( res && !hf->has_css )
        hf->has_css = 1;
    return res;
}
/**
 * Convert the specified text to HTML
 * @param hf the master in question
 * @return a HTML string
 */
char *master_convert( master *hf )
{
    char *str = NULL;
    if ( hf->has_text && hf->has_css && hf->has_markup )
    {
        if ( formatter_cull_ranges(hf->f,hf->text,&hf->tlen) )
        {
            int res = formatter_make_html( hf->f, hf->text, hf->tlen );
            if ( res )
                str = formatter_get_html( hf->f, &hf->html_len );
            else
            {
                const char *error = "<html><body><p>Error: conversion "
                    "failed</p></body></html>";
                strcpy( error_string, error );
                str = error_string;
                hf->html_len = strlen(error_string);
            }
        }
        else
        {
            snprintf( error_string, 128,
                "<html><body><p>Error: failed to remove ranges</p></body></html>"
                );
            str = error_string;
        }
    }
    else
    {
        const char *hntext = (hf->has_text)?"":"no text ";
        const char *hnmarkup = (hf->has_markup)?"":"no markup ";
        const char *hncss = (hf->has_css)?"":"no css ";
        snprintf(error_string,128,
            "<html><body><p>Error: %s%s%s</p></body></html>",
            hntext,hnmarkup,hncss );
        hf->html_len = strlen( error_string );
        str = error_string;
    }
    return str;
}
/**
 * Get the length of the just processed html
 * @param hf the master in question
 * @return the html text length
 */
int master_get_html_len( master *hf )
{
    return hf->html_len;
}
/**
 * List the formats registered with the main program. If the user
 * defines another format he/she must call the register routine to
 * register it. Then this command returns a list of dynamically
 * registered formats.
 * @return the available format names
*/
char *master_list()
{
	int i;
    error_string[0] = 0;
	for ( i=0;i<num_formats;i++ )
	{
        int left = 128 - strlen(error_string)+2;
        strncat( error_string, formats[i].name, left );
        strncat( error_string, "\n", left );
	}
    return error_string;
}
