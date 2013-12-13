#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "milestone.h"

/**
 * Milestone elements in XML typically cover a range but are used 
 * because overlap prevents them from enclosing content. We can fix 
 * this because overlap is allowed in standoff properties. So any 
 * declared milestone ranges will be extended until the next instance 
 * of that milestone. Also milestones can belong to a set of other 
 * milestones which will be stored in the same file. Milestone sets 
 * have their own markup file.
 */
struct milestone_struct
{
    char *name;
    milestone *next;
};

/**
 * Create a milestone
 * @return an initialised milestone
 */
milestone *milestone_create( char *name )
{
    milestone *m = calloc( 1, sizeof(milestone) );
    if ( m != NULL )
    {
        m->name = strdup(name);
        if ( m->name == NULL )
            m = milestone_dispose( m );
    }
    if ( m == NULL )
        fprintf(stderr,"milestone: failed to allocate object\n");
    return m;
}
/**
 * Dispose of a milestone and its descendants
 * @param m the milestone to dispose
 * @return NULL 
 */
milestone *milestone_dispose( milestone *m )
{
    if ( m->name != NULL )
        free( m->name );
    if ( m->next != NULL )
        milestone_dispose( m->next );
    free( m );
    return NULL;
}
/**
 * Add a milestone to the list
 * @param parent the head of the list
 * @param child the milestone to append at the end
 */
void milestone_append( milestone *parent, milestone *child )
{
    while ( parent->next != NULL )
        parent = parent->next;
    parent->next = child;
}
/**
 * Does this milestone list contain the given element?
 * @param head the head of the list
 * @param name the name to search for
 * @return 1 if it does
 */
int milestone_contains( milestone *head, char *name )
{
    milestone *temp = head;
    while ( temp != NULL )
    {
        if ( strcmp(temp->name,name)==0 )
            return 1;
        else
            temp = temp->next;
    }
    return 0;
}
/*
 * Get the milestone's name
 * @param m the milestone in question
 * @return its name
 */
char *milestone_name( milestone *m )
{
    return m->name;
}
/**
 * What is the next milestone in this list
 * @param m the next milestone or NULL
 * @return 
 */
milestone *milestone_next( milestone *m )
{
    return m->next;
}