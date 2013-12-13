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
#include <limits.h>
#include <errno.h>
#include "file_list.h"
#include "error.h"
#include "memwatch.h"
#define BLOCK_SIZE 8
/**
 A file list can be used as here for storing and maintaining a list of files 
 * specified for a single purpose, such as a list of markup files.*/
struct file_list_struct
{
    char **file_names;
    int num_files;
    int num_allocated;
};

/**
 * Create a markup file list
 * @param file_names a list of colon-separated file-names
 * @return an object storing the names or NULL
 */
file_list *file_list_create( const char *file_names )
{
    file_list *fl = calloc( 1, sizeof(file_list) );
    if ( fl != NULL )
    {
        fl->file_names = calloc( BLOCK_SIZE, sizeof(char*));
        if ( fl->file_names != NULL )
        {
            const char *name = strtok( (char*)file_names, ":" );
            while ( name != NULL )
            {
                if ( file_list_add_name(fl,name) )
                    name = strtok( NULL, ":" );
                else
                {
                    file_list_delete( fl );
                    error("file_list: failed to create file list %s\n",
                        file_names );
                    fl = NULL;
                    break;
                }
            }
        }
        else
        {
            free( fl );
            error("file_list: failed to create file list %s\n",file_names );
            fl = NULL;
        }
    }
    return fl;
}
/**
 * Delete a filelist and everything in it
 * @param fl the filelist
 */
void file_list_delete( file_list *fl )
{
    int i;
    for ( i=0;i<fl->num_files;i++ )
    {
        free( fl->file_names[i] );
        fl->file_names[i] = NULL;
    }
    free( fl->file_names );
    fl->file_names = NULL;
    free( fl );
}
/**
 * Add a name to the filelist
 * @param fl the filelist object
 * @param name the name to add
 * @return 1 if it worked, else 0
 */
int file_list_add_name( file_list *fl, const char *name )
{
    if ( fl->num_files >= fl->num_allocated )
    {
        char **temp = calloc(fl->num_allocated+BLOCK_SIZE, sizeof(char*) );
        if ( temp == NULL )
            return 0;
        else
        {
            int i;
            for ( i=0;i<fl->num_files;i++ )
                temp[i] = fl->file_names[i];
            free( fl->file_names );
            fl->file_names = temp;
        }
    }
    fl->file_names[fl->num_files] = strdup( (char*)name );
    if ( fl->file_names[fl->num_files] == NULL )
        return 0;
    else
        fl->num_files++;
    return 1;
}
/**
 * Get the size of this file list
 * @param fl the file list in question
 * @return the size of the list
 */
int file_list_size( file_list *fl )
{
    return fl->num_files;
}
/**
 * Get the file at the specified index
 * @param fl the file list in question
 * @param index its index in the list
 * @return file name or NULL if out of range
 */
char *file_list_get( file_list *fl, int index )
{
    if ( index < fl->num_files && index >= 0 )
        return fl->file_names[index];
    else
        return NULL;
}
/**
 * Check whether a file exists
 * @param file the file to test
 * @return 1 if it does, 0 otherwise
 */
static int file_exists( const char *file )
{
	FILE *EXISTS = fopen( file,"r" );
	if ( EXISTS )
	{
		fclose( EXISTS );
		return 1;
	}
	return 0;
}
/**
 * Do all the files in the list exist?
 * @param l the list in question
 * @param missing VAR param: save name of missing file here
 * @return 1 if they are all there, else 0
 */
int file_list_check( file_list *fl, char **missing )
{
    int i;
    for ( i=0;i<fl->num_files;i++ )
    {
        if ( !file_exists(fl->file_names[i]) )
        {
            *missing = fl->file_names[i];
            return 0;
        }
    }
    return 1;
}
/**
 * Get the length of an open file
 * @param fp an open FILE handle
 * @return file length if successful, else 0
 */
static int get_file_length( FILE *fp )
{
	int length = 0;
	int res = fseek( fp, 0, SEEK_END );
	if ( res == 0 )
	{
		long long_len = ftell( fp );
		if ( long_len > INT_MAX )
			error( "file_list: file too long: %d", long_len );
		length = (int) long_len;
		if ( length != -1 )
			res = fseek( fp, 0, SEEK_SET );
        else
            res = 1;
	}
	if ( res != 0 )
    {
		error( "file_list: failed to read file: error %d",errno );
        length = 0;
    }
	return length;
}
/**
 * Load the contents of a file
 * @param fl the filelist in question
 * @param index the index into the filelist specifying the file
 * @param data pointer to a handle to allocate and copy data to
 * @param len VAR parameter for file length
 * @return 1 if successful, else 0
 */
int file_list_load( file_list *fl, int index, char **data, int *len )
{
    int res = 0;
    if ( index < fl->num_files )
    {
        if ( file_exists(fl->file_names[index]) )
        {
            FILE *fp = fopen( fl->file_names[index], "r" );
            if ( fp != NULL )
            {
                int flen = get_file_length( fp );
                if ( flen > 0 )
                {
                    *data = malloc( flen );
                    if ( *data == NULL )
                        error( "file_list: no memory for markup\n" );
                    else
                    {
                        int n = fread( *data, 1, flen, fp );
                        if ( n != flen )
                        {
                            error( "file_list: failed to read %s\n",
                                fl->file_names[index] );
                            free( *data );
                            *data = NULL;
                            *len = 0;
                        }
                        else
                        {
                            *len = n;
                            res = 1;
                        }
                    }
                }
                fclose( fp );
            }
            else
                error( "file_list: failed to open %s\n",
                    fl->file_names[index] );
        }
    }
    return res;
}
/**
 * Does this file-list contain the given name?
 * @param fl the filelist in question
 * @param name the name we are seeking
 * @return 1 if it is there, else 0
 */
int file_list_contains( file_list *fl, const char *name )
{
    int i;
    for ( i=0;i<fl->num_files;i++ )
        if ( strcmp(fl->file_names[i],name)==0 )
            return 1;
    return 0;
}