/* 
 * File:   milestone.h
 * Author: desmond
 *
 * Created on August 11, 2012, 9:34 AM
 */

#ifndef MILESTONE_H
#define	MILESTONE_H

#ifdef	__cplusplus
extern "C" {
#endif
typedef struct milestone_struct milestone;
milestone *milestone_create( char *name );
milestone *milestone_dispose( milestone *m );
void milestone_append( milestone *parent, milestone *child );
int milestone_contains( milestone *head, char *name );
char *milestone_name( milestone *m );
milestone *milestone_next( milestone *m );

#ifdef	__cplusplus
}
#endif

#endif	/* MILESTONE_H */

