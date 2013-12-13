/* 
 * File:   utils.h
 * Author: desmond
 *
 * Created on August 1, 2013, 7:45 PM
 */

#ifndef UTILS_H
#define	UTILS_H

#ifdef	__cplusplus
extern "C" {
#endif

int file_size( const char *file_name );
const char *read_file( const char *file_name, int *len );

#ifdef	__cplusplus
}
#endif

#endif	/* UTILS_H */

