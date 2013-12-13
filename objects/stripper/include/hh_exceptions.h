/* 
 * File:   newfile.h
 * Author: desmond
 *
 * Created on August 5, 2013, 1:46 PM
 */

#ifndef HH_EXCEPTIONS_H
#define	HH_EXCEPTIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct hh_exceptions_struct hh_exceptions;
    hh_exceptions *hh_exceptions_create( char *hh_list );
    void hh_exceptions_dispose( hh_exceptions *hhe );
    int hh_exceptions_lookup( hh_exceptions *hhe, char *combination );

#ifdef	__cplusplus
}
#endif

#endif	/* HH_EXCEPTIONS_H */

