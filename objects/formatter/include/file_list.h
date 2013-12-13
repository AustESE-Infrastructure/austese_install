/* 
 * File:   file_list.h
 * Author: desmond
 *
 * Created on 13 June 2011, 9:09 AM
 */

#ifndef FILE_LIST_H
#define	FILE_LIST_H
#ifdef	__cplusplus
extern "C" {
#endif
typedef struct file_list_struct file_list;
file_list *file_list_create( const char *file_names );
void file_list_delete( file_list *fl );
int file_list_add_name( file_list *fl, const char *name );
int file_list_check( file_list *fl, char **missing );
int file_list_load( file_list *fl, int index, char **data, int *len );
int file_list_contains( file_list *fl, const char *name );
int file_list_size( file_list *fl );
char *file_list_get( file_list *fl, int index );
#ifdef	__cplusplus
}
#endif
#endif	/* FILE_LIST_H */

