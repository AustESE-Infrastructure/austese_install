/* 
 * File:   css_property.h
 * Author: desmond
 *
 * Created on 3 June 2011, 6:42 AM
 */

#ifndef CSS_PROPERTY_H
#define	CSS_PROPERTY_H

typedef struct css_property_struct css_property;
void css_property_dispose( css_property *p );
css_property *css_property_clone( css_property *p );
char *css_property_get_html_name( css_property *p );
char *css_property_get_xml_name( css_property *p );
css_property *css_property_parse( const char *data, int len );
void css_property_set_html_value( css_property *p, char *value );
#endif	/* CSS_PROPERTY_H */

