/*
 * css_rule.h
 *
 *  Created on: 06/11/2010
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
#ifndef CSS_RULE_H_
#define CSS_RULE_H_
typedef struct css_rule_struct css_rule;

css_rule *css_rule_create();
void css_rule_dispose( css_rule *rule );
css_rule *css_rule_clone( css_rule *r );
int css_rule_valid( css_rule *rule );
int css_rule_add_property( css_rule *rule, css_property *prop );
int css_rule_add_selector( css_rule *rule, css_selector *sel );
int css_rule_match( css_rule *rule, char *name );
char *css_rule_get_element( css_rule *rule );
char *css_rule_get_class( css_rule *rule );
css_property *css_rule_get_property( css_rule *r, char *xml_name );
css_property *css_rule_get_property_by_index( css_rule *r, int index );
void css_rule_print( css_rule *rule );
int css_rule_compare( void *key1, void *key2 );
int css_rule_get_num_properties( css_rule *r );
#endif /* CSS_RULE_H_ */
