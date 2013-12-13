/*
 * format.h
 *
 *  Created on: 22/10/2010
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

#ifndef FORMAT_H_
#define FORMAT_H_
#ifdef JNI
#define DST_FILE ramfile
#define DST_WRITE(p,n,f) ramfile_write( f, p, n )
#define DST_PRINT(s,f,...) ramfile_print( s, f, __VA_ARGS__ )
#else
#define DST_FILE FILE
#define DST_WRITE(p,n,f) fwrite( p, 1, n, f )
#define DST_PRINT(s,f,...) fprintf( s, f, __VA_ARGS__ )
#endif
typedef int (*format_write_header)(void *arg, DST_FILE *dst, 
        const char *format );
typedef int (*format_write_tail)(void *arg, DST_FILE *dst);
typedef int (*format_write_range)( char *name, char **atts, int removed,
	int offset, int len, char *content, int content_len, int final, 
    DST_FILE *dst );
/* this has to be public so we can initialise it in main */
typedef struct
{
	const char *name;
	format_write_header hfunc;
	format_write_tail tfunc;
	format_write_range rfunc;
	const char *text_suffix;
	const char *markup_suffix;
	const char *middle_name;
} format;
#endif /* FORMAT_H_ */
