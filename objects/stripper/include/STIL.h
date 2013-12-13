/*
 * STIL.h
 *
 *  Created on: 23/10/2010
 *      Author: desmond
 */

#ifndef STIL_H_
#define STIL_H_

int STIL_write_header(void *arg, DST_FILE *dst, const char *style );
int STIL_write_tail(void *arg, DST_FILE *dst);
int STIL_write_range( char *name, char **atts, int removed,
	int offset, int len, char *content, int content_len, int first, 
    DST_FILE *dst );
#endif /* STIL_H_ */
