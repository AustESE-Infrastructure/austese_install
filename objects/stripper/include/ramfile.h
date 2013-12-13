/* 
 * File:   ramfile.h
 * Author: desmond
 *
 * Created on 3 December 2011, 3:31 PM
 */

#ifndef RAMFILE_H
#define	RAMFILE_H

#ifdef	__cplusplus
extern "C" {
#endif
typedef struct ramfile_struct ramfile;
ramfile *ramfile_create();
void ramfile_dispose( ramfile *rf );
int ramfile_write( ramfile *rf, const char *data, int len );
int ramfile_print( ramfile *rf, const char *fmt, ... );
char *ramfile_get_buf( ramfile *rf );
int ramfile_get_len( ramfile *rf );

#ifdef	__cplusplus
}
#endif

#endif	/* RAMFILE_H */

