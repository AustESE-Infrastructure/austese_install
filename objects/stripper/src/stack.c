/**
 * a stack of objects
 *
 * Created on 20/10/2010
 * (c) Desmond Schmidt 2010
 */
/* This file is part of stripper.
 *
 *  stripper is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  stripper is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with stripper.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include <stdio.h>
#include "stack.h"
#include "memwatch.h"
#define INITIAL_ELEMENTS 12
struct stack_struct
{
	void **elements;
	int n_elements;
	/** points to the next pop position */
	int top;
};
stack *stack_create()
{
	struct stack_struct *s = malloc(sizeof(struct stack_struct) );
	if ( s != NULL )
	{
		s->elements = malloc(sizeof(void*)*INITIAL_ELEMENTS);
		if ( s->elements != NULL )
		{
			s->n_elements = INITIAL_ELEMENTS;
			s->top = -1;
		}
		else
		{
			printf("error: failed to allocate stack\n");
			free( s );
			s = NULL;
		}
	}
	return s;
}
/**
 * Dispose of a stack
 */
void stack_delete( stack *s )
{
    free( s->elements );
    free( s );
}
/**
 * Resize the stack
 * @param stack the stack to resize
 */
static void stack_resize( stack *s )
{
	int i,newSize = s->n_elements+(s->n_elements/2);
	void **temp = malloc(sizeof(void*)*newSize );
	if ( temp == NULL )
	{
		printf("error: failed to resize stack\n");
		exit( 0 );
	}
	for ( i=0;i<=s->top;i++ )
		temp[i] = s->elements[i];
	free( s->elements );
	s->elements = temp;
	s->n_elements = newSize;
}
/**
 * Push an object onto the stack
 */
void stack_push( stack *s, void *obj )
{
	if ( s->top == s->n_elements-1 )
		stack_resize( s );
	else
	{
		s->top++;
		s->elements[s->top] = obj;
	}
}
/**
 * Pop an object off the stack
 */
void *stack_pop( stack *s )
{
	void *obj=NULL;
	if ( s->top >= 0 )
	{
		obj = s->elements[s->top];
		s->top--;
	}
	else
	{
		printf("error: attempt to pop empty stack\n");
		exit( 0 );
	}
	return obj;
}
/**
 * Is the stack empty?
 * @params s the stack to test
 */
int stack_empty( stack *s )
{
	return s->top < 0;
}
/**
 * Get the topmost element without popping
 * @params s the stack to peek
 * @return the topmost element or NULL if it is empty
 */
void *stack_peek( stack *s )
{
	if ( s->top >= 0 )
        return s->elements[s->top];
    else
        return NULL;
}
