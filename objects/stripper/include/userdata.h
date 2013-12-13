/* 
 * File:   userdata.h
 * Author: desmond
 *
 * Created on July 30, 2013, 12:24 PM
 */

#ifndef USERDATA_H
#define	USERDATA_H

#ifdef	__cplusplus
extern "C" {
#endif
typedef struct userdata_struct userdata;

#define CHAR_TYPE_TEXT 0
#define CHAR_TYPE_SPACE 1
#define CHAR_TYPE_CR 2
#define CHAR_TYPE_LF 3
#define HYPHEN_NONE 0
#define HYPHEN_ONLY 1
#define HYPHEN_LF 2

userdata *userdata_create( const char *language, char *barefile, recipe *rules, 
        format *fmt, hh_exceptions *hhe );
void userdata_dispose( userdata *u );
int userdata_toffset( userdata *u );
int userdata_last_char_type( userdata *u );
int userdata_has_word( userdata *u, XML_Char *word );
void userdata_inc_toffset( userdata *u, int inc );
void userdata_set_last_char_type( userdata *u, int ctype );
void userdata_set_rules( userdata *u, recipe *r );
int userdata_hyphen_state( userdata *u );
void userdata_set_hyphen_state( userdata *u, int hstate );
void userdata_update_last_word( userdata *u, char *line, int len );
void userdata_clear_last_word( userdata *u );
XML_Char *userdata_last_word( userdata *u );
int userdata_hoffset( userdata *u );
void userdata_set_hoffset( userdata *u, int hoffset );
hashmap *userdata_dest_map( userdata *u );
dest_file *userdata_markup_dest( userdata *u, int i );
recipe *userdata_rules( userdata *u );
stack *userdata_ignoring( userdata *u );
stack *userdata_range_stack( userdata *u );
dest_file *userdata_get_markup_dest( userdata *u, char *range_name );
int userdata_has_hh_exception( userdata *u, char *combination );
dest_file *userdata_text_dest( userdata *u );
#ifdef JNI
void userdata_write_files( JNIEnv *env, userdata *u, jobject text, 
    jobject markup );
#else
void userdata_write_files( userdata *u );
#endif
#ifdef	__cplusplus
}
#endif

#endif	/* USERDATA_H */

