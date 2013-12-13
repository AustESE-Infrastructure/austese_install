/*
 * end_tag_stack.h
 *
 *  Created on: 07/11/2010
 *  (c) Desmond Schmidt 2010
 */
/* This file is part of formatter.
 *
 *  formatter is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  formatter is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with formatter.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef STACK_H_
#define STACK_H_
typedef struct stack_struct stack;

stack *stack_create();
void stack_delete( stack *stack );
void stack_push( stack *stack, void *item );
void *stack_pop( stack *stack );
void *stack_peek( stack *stack );
int stack_empty( stack *stack );
#endif /* END_TAG_STACK_H_ */
