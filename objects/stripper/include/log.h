/* 
 * File:   log.h
 * Author: desmond
 *
 * Created on August 18, 2012, 9:43 AM
 */

#ifndef LOG_H
#define	LOG_H

#ifdef	__cplusplus
extern "C" {
#endif

void initlog();
void tmplog( const char *fmt, ... );


#ifdef	__cplusplus
}
#endif

#endif	/* LOG_H */

