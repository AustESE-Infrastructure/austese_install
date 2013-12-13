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

#if COMMANDLINE
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include "css_property.h"
#include "css_selector.h"
#include "hashmap.h"
#include "attribute.h"
#include "annotation.h"
#include "range.h"
#include "css_rule.h"
#include "hashset.h"
#include "range_array.h"
#include "formatter.h"
#include "css_parse.h"
#include "file_list.h"
#include "AESE/AESE.h"
#include "STIL/STIL.h"
#include "error.h"
#include "master.h"
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
#define DEFAULT_HTML_FILE "output.html"
/*
 * Combine a stripped standoff markup file (initially only in STIL),
 * a plain text file and a CSS file into a HTML file.
 * Created 26/10/2010
 * Revised 17/6/2011
 * (c) Desmond Schmidt 2011
 */
static file_list *css_files;
static file_list *markup_files;
static file_list *text_file;
static char *format_name="STIL";
static char html_file_name[FILE_NAME_LEN];

/** if doing help or version info don't process anything */
static int doing_help = 0;


/**
 * Print a simple help message. If we get time we can
 * make a man page later.
 */
static void print_help()
{
	fprintf( stderr,
		"usage: formatter [-h] [-v] [-l] [-w] [-f format] -c css-files "
			"-m markup-files -t text-file [html-file]\n"
		"formatter combines a plain text file, its stripped "
			"markup file and a\nCSS file into HTML. "
		"Options are: \n"
		"-h print this help message\n"
		"-v print the version information\n"
		"-f the markup format\n"
		"-l list supported formats\n"
		"-c colon-separated list of css files (required)\n"
		"-m colon-separated list of markup file names (required)\n"
		"-t file the name of the base text file (required)\n");
}
/**
 * Check the commandline arguments
 * @param argc number of commandline args+1
 * @param argv array of arguments, first is program name
 * @return 1 if they were OK, 0 otherwise
 */
static int check_args( int argc, char **argv )
{
	int sane = 1;
	markup_files = NULL;
    css_files = NULL;
    text_file = NULL;
	if ( argc < 7 )
		sane = 0;
	else
	{
		int i;
		for ( i=1;i<argc;i++ )
		{
			if ( strlen(argv[i])==2 && argv[i][0]=='-' )
			{
				switch ( argv[i][1] )
				{
					case 'v':
						fprintf( stderr, "formatter version 2.0 (c) "
								"Desmond Schmidt 2011\n");
						doing_help = 1;
						break;
					case 'h':
						print_help();
						doing_help = 1;
						break;
					case 'f':
						if ( i < argc-1 )
							format_name = argv[i+1];
						else
							sane = 0;
						break;
					case 'l':
						printf("%s",master_list());
						doing_help = 1;
						break;
					case 'c':
						if ( i < argc-1 )
                            css_files = file_list_create( argv[i+1] );
						else
							sane = 0;
						break;
					case 'm':
						if ( i < argc-1 )
                            markup_files = file_list_create( argv[i+1] );
						else
							sane = 0;
						break;
					case 't':
						if ( i < argc-1 )
							text_file = file_list_create( argv[i+1] );
						else
							sane = 0;
						break;
				}
			}
			if ( !sane )
				break;
		}
		if ( !doing_help )
		{
			if ( text_file==NULL||markup_files==NULL||css_files==NULL)
				sane = 0;
			else
			{
                char *missing;
				if ( !file_list_check(css_files,&missing) )
					warning("can't find css file %s\n",missing );
				if ( !file_list_check(markup_files,&missing) )
					warning("can't find markup file %s\n",missing );
                if ( !file_list_check(text_file,&missing) )
					warning("can't find text file %s\n",missing );
                if ( !file_list_contains(css_files,argv[argc-1])
                    && !file_list_contains(text_file,argv[argc-1])
                    && !file_list_contains(markup_files,argv[argc-1]) )
                    strncpy( html_file_name, argv[argc-1], FILE_NAME_LEN );
                else
                    strncpy( html_file_name, DEFAULT_HTML_FILE, 
                        FILE_NAME_LEN );
			}
		}
	}
	return sane;
}
/**
 * Tell the user how to use the program
 */
static void usage()
{
	fprintf( stderr,"usage: formatter [-h] [-v] [-l] [-w] [-f format] -c css "
		"-m markup -t text-file [html-file]\n"
		"type: \"formatter -h\" for help\n");
}
/**
 * Main entry point
 */
int main( int argc, char **argv )
{
    FILE *output;
	int res = 0;
    if ( check_args(argc,argv) )
	{
		if ( !doing_help )
		{
            char *data,*text;
            int i,len;
            if ( file_list_load(text_file,0,&text,&len) )
            {
                master *hf = master_create( text, len );
                for ( i=0;i<file_list_size(markup_files);i++ )
                {
                    res = file_list_load(markup_files,i,&data,&len);
                    if ( res )
                    {
                        res = master_load_markup( hf, data, len, format_name );
                        free( data );
                        data = NULL;
                    }
                }
                if ( res )
                {
                    for ( i=0;i<file_list_size(css_files);i++ )
                    {
                        res = file_list_load(css_files,i,&data,&len);
                        if ( res )
                        {
                            res = master_load_css( hf, data, len );
                            free( data );
                            data = NULL;
                        }
                    }
                    output = fopen( html_file_name, "w" );
                    if ( output != NULL )
                    {
                        char *html = master_convert( hf );
                        fwrite( html, 1, master_get_html_len(hf), output );
                        fclose( output );
                    }
                }
                master_dispose( hf );
                free( text );
                text = NULL;
            }
            if ( css_files != NULL )
                file_list_delete( css_files );
            if ( markup_files != NULL )
                file_list_delete( markup_files );
            if ( text_file != NULL )
                file_list_delete( text_file );
        }
	}
	else
		usage();
#ifdef DEBUG_MEMORY
    memory_print();
#endif
	return res;
}
#endif