/*
 * AESE.h
 *
 *  Created on: 23/10/2010
 *      Author: desmond
 */

#ifndef AESE_H_
#define AESE_H_

int AESE_write_header(void *arg, DST_FILE *dst, const char *style );
int AESE_write_tail(void *arg, DST_FILE *dst);
int AESE_write_range( char *name, char **atts, int removed,
	int offset, int len, char *content, int content_len, int first, 
    DST_FILE *dst );
#endif /* AESE_H_ */
