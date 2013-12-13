/*
 * ruleset.h
 *
 *  Created on: 26/10/2010
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

#ifndef RULESET_H_
#define RULESET_H_
typedef struct ruleset_struct ruleset;
ruleset *ruleset_create();
void ruleset_destroy( ruleset *rs );
void ruleset_add( ruleset *rule_array, css_rule *rule );
void ruleset_to_array( ruleset *rule_array, css_rule **rules );
int ruleset_get_size( ruleset *rule_array );
#endif /* RULESET_H_ */
