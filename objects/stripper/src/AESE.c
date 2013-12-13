/*
 * AESE.c
 *
 *  Created on: 23/10/2010
 * (c) Desmond Schmidt 2010
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
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "ramfile.h"
#include "format.h"
#include "AESE.h"
#include "error.h"
#include "memwatch.h"

/**
 * Write the header information. IN AESE this will produce an
 * opening x:document tag which will only be balanced at the end
 * by the tail.
 * @param arg ignored optional user param
 * @param style the name of the format style (for transformation)
 * @param dst the destination markup file handle
 * @return 1 if successful, 0 otherwise
 */
int AESE_write_header( void *arg, DST_FILE *dst, const char *style )
{
	int n,len,res = 1;
	char *xml_decl = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	const char *hrit_decl = "<aese-markup style=\"%s\">\n";
	// write header
	len = strlen(xml_decl);
	n = DST_WRITE( xml_decl, len, dst );
	if ( n != len )
		res = 0;
	// write lmnl document declaration
	len = strlen( hrit_decl );
	n = DST_PRINT( dst, hrit_decl, style );
	if ( n != len+strlen(style)-2 )
		res = 0;
	return res;
}
/**
 * Write the tail
 */
int AESE_write_tail( void *arg, DST_FILE *dst )
{
	const char *fmt = "</aese-markup>";
	int len = strlen( fmt );
	int n = DST_WRITE( fmt, len, dst );
	return (n==len);
}
/**
 * This will be called repeatedly
 * @param name the name of the range
 * @param atts a NULL-terminated array of XML attributes.
 * These get turned into AESE annotations
 * @param reloff relative offset for this range
 * @param len length of the range
 * @param dst the output file handle
 * @param first 1 if this is the first range (ignored)
 * @param queue the output queue to get them in order
 */
int AESE_write_range( char *name, char **atts, int removed,
	int reloff, int len, char *contents, int content_len, int first, 
    DST_FILE *dst )
{
	int i=0;
	int n1;
	const char *fmt1 = "<range name=\"%s\" reloff=\"%d\" len=\"%d\"";
	const char *fmt2 = "</range>\n";
	const char *fmt3 = "/>\n";
	const char *
    fmt4 = ">\n";
    const char *fmt5 = "<content>";
    const char *fmt6 = "</content>\n";
    n1 = DST_PRINT( dst, fmt1, name, reloff, len );
    if ( removed )
        DST_PRINT( dst, " removed=\"%s\"", "true" );
	if ( atts[0] != NULL || content_len > 0 )
	{
		DST_WRITE( fmt4, strlen(fmt4), dst );
		while ( atts[i] != NULL )
		{
			DST_PRINT( dst, "<annotation name=\"%s\" value=\"%s\"/>\n",
				atts[i], atts[i+1] );
			i += 2;
		}
        if ( content_len > 0 )
        {
            DST_WRITE( fmt5, strlen(fmt5), dst );
            DST_WRITE( contents, content_len, dst );
            DST_WRITE( fmt6, strlen(fmt6), dst );
        }
		DST_WRITE( fmt2, strlen(fmt2), dst );
	}
	else
		DST_WRITE( fmt3, strlen(fmt3), dst );
	return (n1 > 0);
}
