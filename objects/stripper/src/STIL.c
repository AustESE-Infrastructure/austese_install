/*
 * STIL.c
 *
 *  Created on: 13/11/2011
 * (c) Desmond Schmidt 2011
 */
/* This file is part of stripper.
 *
 *  stripper is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
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
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "ramfile.h"
#include "format.h"
#include "STIL.h"
#include "error.h"
#include "memwatch.h"

/**
 * Write the header information. 
 * @param arg ignored optional user param
 * @param style the name of the format style (for transformation)
 * @param dst the destination markup file handle
 * @return 1 if successful, 0 otherwise
 */
int STIL_write_header( void *arg, DST_FILE *dst, const char *style )
{
    int n,len,res = 1;
	char *xml_decl = "{\n";
	const char *stil_decl = "  \"style\": \"%s\",\n  \"ranges\": [\n";
	// write header
	len = strlen(xml_decl);
	n = DST_WRITE( xml_decl, len, dst );
	if ( n != len )
		res = 0;
    else
    {
        // write style
        len = strlen( stil_decl );
        n = DST_PRINT( dst, stil_decl, style );
        if ( n != len+strlen(style)-2 )
		{
			res = 0;
		}
    }
	return res;
}
/**
 * Write the tail
 */
int STIL_write_tail( void *arg, DST_FILE *dst )
{
    const char *fmt = "  ]\n}";
    int res = DST_PRINT( dst, "%s", fmt );
    return res == strlen(fmt);
}
/**
 * Convert any double-quotes into escaped double-quotes
 * @param src the string that needs escaping
 * @param len the length of the string
 * @return an allocated string with escaped quotes
 */
static char *escape( char *src, int len )
{
    int i,j;
    char *dst = malloc(len*2 );
    if ( dst != NULL )
    {
        for ( j=0,i=0;i<len;i++ )
        {
            if ( src[i] == '"' )
            {
                dst[j++] = '\\';
                dst[j++] = '"';
            }
            else
                dst[j++] = src[i];
        }
        dst[j] = 0;
    }
    return dst;
}
/**
 * This will be called repeatedly
 * @param name the name of the range
 * @param atts a NULL-terminated array of XML attributes.
 * These get turned into STIL annotations
 * @param reloff relative offset for this range
 * @param len length of the range
 * @param dst the output file handle
 * @param contents the contents of an empty range
 * @param content_len length of the content
 * @param first 1 if this is the first range
 * @param dst the open file descriptor to write to
 */
int STIL_write_range( char *name, char **atts, int removed,
	int reloff, int len, char *contents, int content_len, int first,
    DST_FILE *dst )
{
	int res=1;
	int n;
    //fprintf(stderr,"range=%s\n",name);
	const char *fmt1 = "  {\n";
    const char *fmt2 = "    \"name\": \"%s\",\n";
    const char *fmt3 = "    \"reloff\": %d,\n";
    const char *fmt4 = "    \"len\": %d";
    const char *fmt5 = "    \"content\": \"%s\"";
    const char *fmt6 = "    \"removed\": %s";
    const char *fmt7 = "    \"annotations\": [ ";
    const char *fmt8 = "{ \"%s\": \"%s\" }";
    const char *fmt9 = " ]\n";
	const char *fmt10 = "  }\n";
    // optional comma
    if ( !first )
    {
        n = DST_WRITE( ",", 1, dst );
        if ( n != 1 )
            res = 0;
        // CR
        n = DST_WRITE( "\n", strlen("\n"), dst );
        if ( n != strlen("\n"))
            res = 0;
    }
    n = DST_WRITE( fmt1, strlen(fmt1), dst );
    if ( n != strlen(fmt1) )
        res = 0;
    // name
    if ( res )
    {
        n = DST_PRINT( dst, fmt2, name );
        if ( n != strlen(fmt2)+strlen(name)-2 )
            res = 0;
    }
    // reloff
    if ( res )
    {
        n = DST_PRINT( dst, fmt3, reloff );
        if ( n < strlen(fmt3)-1 )
            res = 0;
    }
    // len
    if ( res )
    {
        n = DST_PRINT( dst, fmt4, len );
        if ( n < strlen(fmt4)-1 )
            res = 0;
        if ( res && (contents != NULL || removed || atts[0] != NULL) )
        {
            n = DST_WRITE( ",", 1, dst );
            if ( n != 1 )
                res = 0;
        }
        if ( res )
        {
            n = DST_WRITE( "\n", 1, dst );
            if ( n != strlen("\n") )
                res = 0;
        }
    }
    // optional contents
    if ( res && contents != NULL )
    {
        char *tmp = escape( contents, content_len );
        if ( tmp != NULL )
        {
            int len5 = strlen(fmt5);
            int lent = strlen(tmp);
            n = DST_PRINT( dst, fmt5, tmp );
            if ( n != len5+lent-2 )
            {
                warning("STIL: failed to write the full string\n");
            }
            free( tmp );
        }
        else
        {
            error( "STIL: failed to escape string\n");
            res = 0;
        }
        if ( res && (removed || atts[0] != NULL) )
        {
            n = DST_WRITE( ",", 1, dst );
            if ( n != 1 )
                res = 0;
        }
        if ( res )
        {
            n = DST_WRITE( "\n", 1, dst );
            if ( n != strlen("\n") )
                res = 0;
        }
    }
    // removed
    if ( res && removed )
    {
        n = DST_PRINT( dst, fmt6, "true" );
        if ( n != strlen(fmt6)+2 )
            res = 0;
        if ( res && atts[0] != NULL )
        {
            n = DST_WRITE( ",", 1, dst );
            if ( n != 1 )
                res = 0;
        }
        if ( res )
        {
            n = DST_WRITE( "\n", 1, dst );
            if ( n != strlen("\n") )
                res = 0;
        }
    }
    // annotations
	if ( res && atts[0] != NULL )
	{
		n = DST_WRITE( fmt7, strlen(fmt7), dst );
        if ( n != strlen(fmt7) )
            res = 0;
        else
        {
            int i = 0;
            //fprintf(stderr,"about to write attributes for %s\n",name);
            while ( res && atts[i] != NULL )
            {
                //fprintf(stderr,"about to print attribute %d %s=%s\n",i,atts[i],atts[i+1]);
                n = DST_PRINT( dst, fmt8, atts[i], atts[i+1] );
                //fprintf(stderr,"printed n=%d\n",n);
                if ( n != strlen(fmt8)+strlen(atts[i])+strlen(atts[i+1])-4 )
                {
                    warning("STIL: failed to write the full string\n");
                }
                else
                {
                    i += 2;
                    //fprintf(stderr,"about to address atts[%d]\n",i);
                    if ( atts[i] != NULL )
                    {
                        //fprintf(stderr,"about to address atts[%d]=%lx\n",i,(long)atts[i]);
                        n = DST_PRINT( dst, "%s", "," );
                        if ( n != 1 )
                            res = 0;
                    }
                }
            }
            //fprintf(stderr,"wrote attributes for %s\n",name);
            // end of array
            n = DST_WRITE( fmt9, strlen(fmt9), dst );
            if ( n != strlen(fmt9) )
                res = 0;
        }
        
	}
    // trailing brace
	n = DST_WRITE( fmt10, strlen(fmt10), dst );
    if ( n != strlen(fmt10) )
        res = 0;
    return res;
}

