/* 
 * File:   recipe.h
 * Author: desmond
 *
 * Created on 12 April 2011, 5:03 AM
 * (c) Desmond Schmidt 2011
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
#ifndef RECIPE_H
#define	RECIPE_H
typedef struct recipe_struct recipe;
recipe *recipe_new();
recipe *recipe_load( const char *buf, int len );
recipe *recipe_dispose( recipe *r );
char * recipe_simplify( recipe *r, char *name, char **attrs );
simplification *recipe_has_rule( recipe *r, const char *name,
    const char **attrs );
int recipe_has_removal( recipe *r, const char *removal );
int recipe_num_layers( recipe *r );
layer *recipe_layer( recipe *r, int i );
#endif	/* RECIPE_H */

