/* 
 * File:   attribute.h
 * Author: desmond
 *
 * Created on 14 April 2011, 3:45 PM
 */

#ifndef ATTRIBUTE_H
#define	ATTRIBUTE_H

typedef struct attribute_struct attr1bute;
attr1bute *attribute_new( const char *name, const char *value );
char *attribute_get_name( attr1bute *a );
char *attribute_get_value( attr1bute *a );
int attribute_present( attr1bute *a, char **attrs );
void attribute_remove( attr1bute *a, char **attrs );
void attribute_delete( attr1bute *a );
#endif	/* ATTRIBUTE_H */

