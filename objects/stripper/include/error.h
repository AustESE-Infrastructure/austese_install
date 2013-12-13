/* 
 * File:   error.h
 * Author: desmond
 *
 * Created on 12 April 2011, 6:20 AM
 */

#ifndef ERROR_H
#define	ERROR_H
#ifdef	__cplusplus
extern "C" {
#endif
void error( const char *fmt, ... );
void warning( const char *fmt, ... );
void report( const char *fmt, ... );
#ifdef	__cplusplus
}
#endif

#endif	/* ERROR_H */

