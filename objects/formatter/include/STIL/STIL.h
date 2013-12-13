/* 
 * File:   STIL.h
 * Author: desmond
 *
 * Created on 18 April 2011, 8:08 AM
 */

#ifndef STIL_H
#define	STIL_H
#ifdef	__cplusplus
extern "C" {
#endif
int load_stil_markup( const char *data, int len, range_array *ranges, hashset *props );
#ifdef	__cplusplus
}
#endif
#endif	/* STIL_H */

