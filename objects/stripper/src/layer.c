#include <stdlib.h>
#include <string.h>
#include "milestone.h"
#include "layer.h"
/**
 * A layer of markup is created from a list of milestones. There may be 
 * several and their fields may overlap etc.
 */
struct layer_struct 
{
    char *name;
    milestone *milestones;
};
/**
 * Create a layer
 * @param name its name
 * @param milestones a linked list of milestone objects
 * @return an initialised layer object
 */
layer *layer_create( char *name, milestone *milestones )
{
    layer *l = calloc( 1, sizeof(layer) );
    if ( l != NULL )
    {
        l->name = strdup( name );
        if ( l->name==NULL )
        {
            l = layer_dispose( l );
        }
        l->milestones = milestones;
    }
    return l;
}
/**
 * Dispose of this layer and all its children
 * @param l the layer to dispose
 * @return NULL
 */
layer *layer_dispose( layer *l )
{
    if ( l->name != NULL )
        free( l->name );
    if ( l->milestones != NULL )
        milestone_dispose( l->milestones );
    free( l );
    return NULL;
}
/**
 * Get this layer's name
 * @param l the layer in question
 * @return its name as a string
 */
char *layer_name( layer *l )
{
    return l->name;
}
/**
 * Does this layer have a particular milestone?
 * @param l the layer in question
 * @param mname the desired milestone name (xml name)
 * @return 1 if present else 0
 */
int layer_has_milestone( layer *l, char *mname )
{
    return milestone_contains( l->milestones, mname );
}