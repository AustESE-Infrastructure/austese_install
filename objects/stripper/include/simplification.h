#ifndef SIMPLIFICATION_H
#define SIMPLIFICATION_H
typedef struct simplification_struct simplification;
simplification *simplification_new( const char *xml_name,
    const char *prop_name );
int simplification_contains( simplification *s, char **attrs );
void simplification_delete( simplification *s );
void simplification_add_attribute( simplification *s, attr1bute *a );
void simplification_remove_attribute( simplification *s, char ** attrs );
char *simplification_get_xml_name( simplification *s );
char *simplification_get_prop_name( simplification *s );
#endif