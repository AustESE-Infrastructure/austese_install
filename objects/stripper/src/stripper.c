/**
 * Strip XML tags from a file and write them out in AESE format
 * Created on 22/10/2010
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <syslog.h>
#ifdef JNI
#include <jni.h>
#include "calliope_AeseStripper.h"
#include "ramfile.h"
#else
#include "utils.h"
#endif
#include "format.h"
#include "expat.h"
#include "stack.h"
#include "AESE.h"
#include "STIL.h"
#include "hashset.h"
#include "error.h"
#include "range.h"
#include "attribute.h"
#include "simplification.h"
#include "milestone.h"
#include "layer.h"
#include "recipe.h"
#include "dest_file.h"
#include "hashmap.h"
#include "log.h"
#include "memwatch.h"
#include "hh_exceptions.h"
#include "userdata.h"

#define FILE_NAME_LEN 128
#ifdef XML_LARGE_SIZE
#if defined(XML_USE_MSC_EXTENSIONS) && _MSC_VER < 1400
#define XML_FMT_INT_MOD "I64"
#else
#define XML_FMT_INT_MOD "ll"
#endif
#else
#define XML_FMT_INT_MOD "l"
#endif

/** array of available formats - add more here */
static format formats[]={{"STIL",STIL_write_header,STIL_write_tail,
    STIL_write_range,".txt",".json","-stil"},
    {"AESE",AESE_write_header,AESE_write_tail,
    AESE_write_range,".txt",".xml","-aese"}};
/** size of formats array */
static int num_formats = sizeof(formats)/sizeof(format);
typedef struct 
{
    userdata *user_data;
    /** source file */
    char src[FILE_NAME_LEN];
    /** source file minus suffix */
    char barefile[FILE_NAME_LEN];
    /** name of style */
    char *style;
    /** name of recipe file */
    char *recipe_file;
    /** index into the currently selected format */
    int selected_format;
    /** if doing help or version info don't process anything */
    int doing_help;
    /** language code */
    char *language;
    /** hard-hyphen object */
    hh_exceptions *hh_except;
    /** copy of commandline arg */
    char *hh_except_string;
    /** the parser */
    XML_Parser parser;
} stripper;
/**
 * Copy an array of attributes as returned by expat
 * @param atts the attributes
 * @return a NULL terminated array, user must free
 */
static char **copy_atts( const char **atts )
{
	int len = 0;
	int i = 0;
	char **new_atts;
	while ( atts[i] != NULL )
    {
        i += 2;
        len+=2;
    }
    new_atts = calloc( len+2, sizeof(char*) );
    i = 0;
    while ( atts[i] != NULL )
    {
        new_atts[i] = strdup(atts[i]);
        if ( new_atts[i] == NULL )
            fprintf( stderr,
                "stripper: failed to allocate store for attribute key" );
        new_atts[i+1] = strdup( atts[i+1] );
        if ( new_atts[i+1] == NULL )
            fprintf( stderr,
                "stripper: failed to allocate store for attribute value" );
        i += 2;
    }
    return new_atts;
}
/**
 * Start element handler for XML file stripping.
 * @param userData the user data (optional)
 * @param name the name of the element
 * @param atts an array of attributes terminated by a NULL
 * pointer
 */
static void XMLCALL start_element_scan( void *userData,
	const char *name, const char **atts )
{
    range *r;
    userdata *u = userData;
    char **new_atts;
    char *simple_name = (char*)name;
    if ( recipe_has_removal(userdata_rules(u),(char*)name) )
        stack_push( userdata_ignoring(u), (char*)name );
    new_atts = copy_atts( atts );
    if ( stack_empty(userdata_ignoring(u)) && recipe_has_rule(userdata_rules(u),name,atts) )
        simple_name = recipe_simplify( userdata_rules(u), simple_name, new_atts );
    r = range_new( stack_empty(userdata_ignoring(u))?0:1,
        simple_name,
        new_atts,
        userdata_toffset(u) );
    // stack has to set length when we get to the range end
    stack_push( userdata_range_stack(u), r );
    // queue preserves the order of the start elements
    dest_file *df = userdata_get_markup_dest( u, range_get_name(r) );
    dest_file_enqueue( df, r );
}
/**
 * End element handler for XML split
 * @param userData (optional)
 * @param name name of element
 */
static void XMLCALL end_element_scan(void *userData, const char *name)
{
	userdata *u = userData;
	range *r = stack_pop( userdata_range_stack(u) );
    int rlen = userdata_toffset(u)-range_get_start(r);
    range_set_len( r, rlen );
	if ( !stack_empty(userdata_ignoring(u)) 
        && strcmp(stack_peek(userdata_ignoring(u)),name)==0 )
		stack_pop( userdata_ignoring(u) );
}
/**
 * Is the given string just whitespace?
 * @param s the string (not null-terminated
 * @param len its length
 * @return 1 is only contains whitespace, else 0
 */
static int is_whitespace( const char *s, int len )
{
    int res = 1;
    int i;
    for ( i=0;i<len;i++ )
        if ( s[i]!='\t' && s[i]!='\n' && s[i]!=' ' )
            return 0;
    return res;
}
/**
 * Compute the length of the string in UTF-8
 * @param text the UTF-8 string
 * @param len its length in BYTES
 * @return its length in CHARS
 */
static int utf8_len( XML_Char *text, int len )
{
     int i;
     int nchars = 0;
     for ( i=0;i<len;i++ )
     {
         unsigned char uc = (unsigned char) text[i];
         // these are just continuation bytes not real "characters"
         if ( uc < 0x80 || uc > 0xBF )
             nchars++;
     }
     return nchars;
}
/**
 * trim leading and trailing white space down to 1 char
 * @param u the userdata struct
 * @param cptr VAR pointer to the string
 * @param len VAR pointer to its length
 */
static void trim( userdata *u, char **cptr, int *len )
{
    char *text = *cptr;
    int length = *len;
    int i;
    int state = userdata_last_char_type(u);
    // trim front of string
    for ( i=0;i<length;i++ )
    {
        switch ( state )
        {
            case 0: // last char was non-space/non-CR 
                if ( text[i] == ' ' || text[i] == '\t' )
                    state = 1;
                else if ( text[i] == '\r' )
                    state = 2;
                else if ( text[i] == '\n' )
                    state = 3;
                else
                    state = -1;
                break;
            case 1: // last char was a space
                if ( isspace(text[i]) )
                {
                    (*cptr)++;
                    (*len)--;
                    if ( text[i] == '\n' )
                        state = 3;
                    else if ( text[i] == '\r' )
                        state = 2;
                }
                else
                    state = -1;
                break;
            case 2: // last char was a CR
                if ( text[i] == '\n' )
                    state = 3;
                else if ( isspace(text[i]) )
                {
                    (*cptr)++;
                    (*len)--;
                }
                else
                    state = -1;
                break;
            case 3: // last char was a LF
                if ( isspace(text[i]) )
                {
                    (*cptr)++;
                    (*len)--;
                }
                else
                    state = -1;
                break;
            
        }
        if ( state < 0 )
            break;
    }
    // trim rear of string
    length = *len;
    text = *cptr;
    state = 0;
    for ( i=length-1;i>=0;i-- )
    {
        switch ( state )
        {
            case 0: // initial state or TEXT
                if ( text[i] == ' ' || text[i] == '\t' )
                    state = 1;
                else if ( text[i] == '\r' )
                    state = 2;
                else if ( text[i] == '\n' )
                    state = 3;
                else
                {
                    if ( text[i] == '-' )
                    {
                        int ulen = length;
                        userdata_update_last_word(u,text,length);
                        if ( strlen(userdata_last_word(u))>0 )
                        {
                            userdata_set_hoffset(u,userdata_toffset(u)+ulen-1);
                            userdata_set_hyphen_state(u,HYPHEN_ONLY);
                        }
                    }
                    else
                    {
                        userdata_clear_last_word(u);
                        userdata_set_hyphen_state(u,HYPHEN_NONE);
                    }
                    state = -1;
                    userdata_set_last_char_type(u,CHAR_TYPE_TEXT);
                }
                break;
            case 1: // last char was space
                if ( text[i] == ' ' || text[i] == '\t' )
                    (*len)--;
                else if ( text[i] == '\n' )
                {
                    (*len)--;
                    state = 3;
                }
                else if ( text[i] == '\r' )
                {
                    (*len)--;
                    state = 2;
                }
                else
                {
                    state = -1;
                    userdata_set_last_char_type(u,CHAR_TYPE_SPACE);
                }
                break;
            case 2: // last char was CR
                if ( isspace(text[i]) )
                {
                    (*len)--;
                }
                else
                {
                    if ( text[i] == '-' )
                    {
                        // remove trailing LF
                        (*len)--;
                        userdata_set_hyphen_state(u,HYPHEN_LF);
                        userdata_set_hoffset(u,userdata_toffset(u)
                            +length/*utf8_len(text,length)*/-2);
                        userdata_update_last_word(u,text,length);
                        userdata_set_last_char_type(u,CHAR_TYPE_TEXT);
                    }
                    else
                    {
                        userdata_clear_last_word(u);
                        userdata_set_last_char_type(u,CHAR_TYPE_CR);
                    }
                    state = -1;
                }
                break;
            case 3: // last char was LF
                if ( isspace(text[i]) )
                {
                    (*len)--;
                }
                else
                {
                    if ( text[i] == '-' )
                    {
                        // remove trailing LF
                        (*len)--;
                        userdata_set_hyphen_state(u,HYPHEN_LF);
                        userdata_set_hoffset(u,userdata_toffset(u)
                            +length/*utf8_len(text,length)*/-2);
                        userdata_update_last_word(u,text,length);
                        userdata_set_last_char_type(u,CHAR_TYPE_TEXT);
                    }
                    else
                    {
                        userdata_clear_last_word(u);
                        userdata_set_last_char_type(u,CHAR_TYPE_LF);
                    }
                    state = -1;
                }
                break;
        }
        if ( state == -1 )
            break;
    }
    if ( state != -1 && (*len)>0 )
        userdata_set_last_char_type(u,state);
}
/**
 * Duplicate the first word of a text fragment
 * @param text the text to pop the word off of
 * @param len its length
 * @return an allocated word in UTF8 or NULL
 */
static XML_Char *first_word( XML_Char *text, int len )
{
    int i;
    // point to first non-space
    for ( i=0;i<len;i++ )
    {
        if ( !isspace(text[i]) )
            break;
    }
    int j = i;
    for ( ;i<len;i++ )
    {
        if ( !isalpha(text[i])||text[i]=='-' )
            break;
    }
    char *word = calloc( 1+i-j, sizeof(XML_Char) );
    if ( word != NULL )
    {
        int nbytes = (i-j)*sizeof(XML_Char);
        memcpy( word, &text[j], nbytes );
    }
    return word;
}
/**
 * Combine two words by returning an allocated concatenation
 * @param w1 the first word
 * @param w2 the second word
 * @return the combined word, caller to dispose
 */
static XML_Char *combine_words( XML_Char *w1, XML_Char *w2 )
{
    XML_Char *w3 = calloc( strlen(w1)+strlen(w2)+1,1 );
    if ( w3 != NULL )
    {
        strcpy( w3,w1 );
        strcat( w3, w2 );
    }
    return w3;
}
/**
 * Add markup for the detected hyphen 
 * @param u the userdata
 * @param text the current text after the hyphen
 * @param len its length
 */
static void process_hyphen( userdata *u, XML_Char *text, int len )
{
    XML_Char *next = first_word(text,len);
    if ( next != NULL && strlen(next)>0 )
    {
        char *force = "weak";
        XML_Char *last = userdata_last_word(u);
        XML_Char *combined = combine_words(last,next);
        if ( (userdata_has_word(u,last)
            && userdata_has_word(u,next)
            && combined!=NULL
            && (!userdata_has_word(u,combined)
                ||userdata_has_hh_exception(u,combined)))
            || (strlen(next)>0&&isupper(next[0])) )
        {
            force = "strong";
            //printf("strong: %s-%s\n",last,next);
        }
        //else
        //    printf("weak: %s\n",combined);
        // create a range to describe a hard hyphen
        char **atts = calloc(1,sizeof(char*));
        if ( atts != NULL )
        {
            range *r = range_new( 0, force, atts, userdata_hoffset(u) );
            if ( r != NULL )
            {
                dest_file *df = userdata_get_markup_dest( u, force );
                userdata_set_hyphen_state(u,HYPHEN_NONE);
                range_set_len( r, 1 );
                dest_file_enqueue( df, r );
            }
        }
        else
            fprintf(stderr,"stripper: failed to create hyphen range\n");
        if ( combined != NULL )
            free( combined );
    }
    if ( next != NULL )
        free( next );
}
/**
 * Handle characters during stripping. We basically write
 * everything to all the files currently identified by
 * current_bitset.
 * @param userData a userdata object or NULL
 * @param s the character data
 * @param its length
 */
static void XMLCALL charhndl( void *userData, const XML_Char *s, int len )
{
	userdata *u = userData;
	if ( stack_empty(userdata_ignoring(u)) )
	{
		size_t n;
        if ( len == 1 && s[0] == '&' )
		{
			n = dest_file_write( userdata_text_dest(u), "&amp;", 5 );
			userdata_inc_toffset( u, 5 );
            userdata_set_last_char_type(u,CHAR_TYPE_TEXT);
		}
        else 
		{
			char *text = (char*)s;
            if ( userdata_hyphen_state(u) == HYPHEN_LF 
                && !is_whitespace(text,len) )
                process_hyphen(u,text,len);
            trim( u, &text, &len );
            if ( len == 1 && (text[0]=='\n'||text[0]=='\r') 
                && userdata_hyphen_state(u)==HYPHEN_ONLY )
            {
                userdata_set_hyphen_state(u,HYPHEN_LF);
                len=0;
            }
            if ( len > 0 )
            {
                //int ulen = utf8_len(text,len);
                n = dest_file_write( userdata_text_dest(u), text, len );
                userdata_inc_toffset( u, len );
                //if ( ulen != len )
                //    printf("ulen!=len\n");
                if ( n != len )
                    error( "stripper: write error on text file" );
            }
		}
	}
    else if ( !is_whitespace(s,len) )
    {
        range *r = stack_peek( userdata_range_stack(u) );
        if ( len == 1 && s[0] == '&' )
        {
			range_add_content( r, "&amp;", 5 );
        }
        else
            range_add_content( r, s, len );
    }
    // else it's inter-element white space
}
/**
 * Scan the source file, looking for tags to send to
 * the tags file and text to the text file.
 * @param buf the data to parse
 * @param len its length
 * @param s the stripper object
 * @return 1 if it succeeded, 0 otherwise
 */
static int scan_source( const char *buf, int len, stripper *s )
{
	int res = 1;
	userdata_set_last_char_type(s->user_data, CHAR_TYPE_LF);
    s->parser = XML_ParserCreate( NULL );
    if ( s->parser != NULL )
    {
        XML_SetElementHandler( s->parser, start_element_scan,
            end_element_scan );
        XML_SetCharacterDataHandler( s->parser, charhndl );
        XML_SetUserData( s->parser, s->user_data );
        if ( XML_Parse(s->parser,buf,len,1) == XML_STATUS_ERROR )
        {
            error(
                "stripper: %s at line %" XML_FMT_INT_MOD "u\n",
                XML_ErrorString(XML_GetErrorCode(s->parser)),
                XML_GetCurrentLineNumber(s->parser));
            return 0;
        }
        XML_ParserFree( s->parser );
        
    }
    else
    {
        fprintf(stderr,"stripper: failed to create parser\n");
        res = 0;
    }
	return res;
}
/**
 * Look up a format in our list.
 * @param fmt_name the format's name
 * @return its index in the table or -1
 */
static int lookup_format( const char *fmt_name )
{
	int i;
	for ( i=0;i<num_formats;i++ )
	{
		if ( strcmp(formats[i].name,fmt_name)==0 )
			return i;
	}
	return -1;
}
/**
 * Dispose of a stripper
 * @param s the stripper perhaps partly completed
 */
void stripper_dispose( stripper *s )
{
    if ( s->user_data != NULL )
        userdata_dispose( s->user_data );
    if ( s->hh_except_string != NULL )
        free( s->hh_except_string );
    if ( s->hh_except != NULL )
        hh_exceptions_dispose( s->hh_except );
    if ( s != NULL )
        free( s );
}
/**
 * Create a stripper object. We need this for thread safety
 * @return  a finished stripper object
 */
stripper *stripper_create()
{
    stripper *s = calloc( 1, sizeof(stripper) );
    if ( s != NULL )
    {
        s->language = "en_GB";
        s->style = "TEI";
    }
    else
        fprintf(stderr,"stripper: failed to allocate object\n");
    return s;
}
#ifdef JNI
static void unload_string( JNIEnv *env, jstring jstr, const char *cstr, 
    jboolean copied )
{
    if ( copied )
        (*env)->ReleaseStringUTFChars( env, jstr, cstr );
}
static const char *load_string( JNIEnv *env, jstring jstr, jboolean *copied )
{
    return (*env)->GetStringUTFChars(env, jstr, copied);  
}
/*
 * Class:     calliope_AeseStripper
 * Method:    strip
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Lcalliope/json/JSONResponse;Lcalliope/json/JSONResponse;)I
 */
JNIEXPORT jint JNICALL Java_calliope_AeseStripper_strip
  (JNIEnv *env, jobject obj, jstring xml, jstring rules, jstring format, 
    jstring style, jstring language, jstring hexcepts, jobject text, 
    jobject markup)
{
	int res = 1;
    jboolean x_copied,r_copied=JNI_FALSE,f_copied,s_copied,h_copied,l_copied=JNI_FALSE;
    const char *x_str = load_string( env, xml, &x_copied );
    //fprintf(stderr,"x_str=%s r_str\n",x_str);
    const char *r_str = (rules!=NULL)?load_string(env,rules,&r_copied):NULL;
    //fprintf(stderr,"r_str=%s\n",r_str);
    const char *f_str = load_string( env, format, &f_copied );
    //fprintf(stderr,"f_str=%s\n",f_str);
    const char *s_str = load_string( env, style, &s_copied );
    //fprintf(stderr,"s_str=%s\n",s_str);
    const char *l_str = (language==NULL)?"en_GB"
        :load_string( env, language, &l_copied );
    //fprintf(stderr,"l_str=%s\n",l_str);
    const char *h_str = (hexcepts==NULL)?NULL
        :load_string( env, hexcepts, &h_copied );
    //fprintf(stderr,"h_str=%s\n",h_str);
    stripper *s = stripper_create();
    if ( s != NULL )
    {
        recipe *ruleset;
        s->hh_except_string = (h_str==NULL)?NULL:strdup(h_str);
        s->selected_format = lookup_format( f_str );
        // load or initialise rule set
        if ( rules == NULL )
            ruleset = recipe_new();
        else
            ruleset = recipe_load(r_str,strlen(r_str));
        hh_exceptions *hhe = hh_exceptions_create( s->hh_except_string );
        if ( hhe != NULL )
        {
            s->user_data = userdata_create( s->language, s->barefile, 
                ruleset, &formats[s->selected_format], hhe );
            if ( s->user_data != NULL )
            {
                // write header
                int i=0;
                while ( res && userdata_markup_dest(s->user_data,i)!= NULL )
                {
                    res = formats[s->selected_format].hfunc( NULL, 
                        dest_file_dst(
                            userdata_markup_dest(s->user_data,i)), s_str );
                    i++;
                }
                // parse XML
                if ( res )
                {
                    int xlen = strlen( x_str );
                    res = scan_source( x_str, xlen, s );
                    if ( res )
                        userdata_write_files( env, s->user_data, text, markup );
                }
                else
                    tmplog("write header failed\n");
            }
        }
        stripper_dispose( s );
        unload_string( env, xml, x_str, x_copied );
        unload_string( env, rules, r_str, r_copied );
        unload_string( env, format, f_str, f_copied );
        unload_string( env, style, s_str, s_copied );
        unload_string( env, language, l_str, l_copied );
        if ( h_str != NULL )
            unload_string( env, hexcepts, h_str, h_copied );
    }
    return res;
}
/*
 * Class:     AeseStripper
 * Method:    version
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_aeseserver_AeseStripper_version
  (JNIEnv *env, jobject obj )
{
	return (*env)->NewStringUTF(env, 
		"stripper version 1.1 (c) Desmond Schmidt 2011" );
}

/*
 * Class:     calliope_AeseStripper
 * Method:    formats
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_calliope_AeseStripper_formats
(JNIEnv *env, jobject obj )
{
	jobjectArray ret = (jobjectArray)(*env)->NewObjectArray(
		env,
        num_formats,
 		(*env)->FindClass(env,"java/lang/String"),
		(*env)->NewStringUTF(env,""));
	if ( ret != NULL )
	{
		int i;
        for( i=0;i<num_formats;i++ ) 
		{
		    (*env)->SetObjectArrayElement(env,
				ret,i,(*env)->NewStringUTF(env, formats[i].name));
		}
	}
	return ret;
}
#elif COMMANDLINE
/**
 * Print a simple help message. If we get time we can
 * make a man page later.
 */
static void print_help()
{
	printf(
		"usage: stripper [-h] [-v] [-s style] [-l] [-f format] "
        "[-r recipe] XML-file\n"
		"stripper removes tags from an XML file and saves "
			"them to a separate file\n"
		"in a standoff markup format. The original text is "
			"also preserved and is\n"
		"written to another file. Options are: \n"
		"-h print this help message\n"
		"-v print the version information\n"
        "-s style. Specify a style name (default \"TEI\")\n"
		"-f format. Specify a supported format\n"
		"-l list supported formats\n"
        "-e hh_exceptions ensure these space-delimited compound words ARE "
        "hyphenated\nIF both halves are words and the compound is also, e.g. safeguard\n"
		"-r recipe-file specifying removals and simplifications in XML or JSON\n"
		"XML-file the only real argument is the name of an XML "
			"file to split.\n");
}
/**
 * Check whether a file exists
 * @param file the file to test
 * @return 1 if it does, 0 otherwise
 */
static int file_exists( const char *file )
{
	FILE *EXISTS = fopen( file,"r" );
	if ( EXISTS )
	{
		fclose( EXISTS );
		return 1;
	}
	return 0;
}
/**
 * List the formats registered with the main program. If the user
 * defines another format he/she must call the register routine to 
 * register it. Then this command returns a list of dynamically
 * registered formats.
*/
static void list_formats()
{
	int i;
	for ( i=0;i<num_formats;i++ )
	{
		printf( "%s\n",formats[i].name );
	}
}
/**
 * Check the commandline arguments
 * @param argc number of commandline args+1
 * @param argv array of arguments, first is program name
 * @param s the stripper object containing local vars
 * @return 1 if they were OK, 0 otherwise
 */
static int check_args( int argc, char **argv, stripper *s )
{
	char *dot_pos;
	int sane = 1;
	if ( argc < 2 )
		sane = 0;
	else
	{
		int i,rlen;
        const char *rdata = NULL;
		for ( i=1;i<argc;i++ )
		{
			if ( strlen(argv[i])==2 && argv[i][0]=='-' )
			{
				switch ( argv[i][1] )
				{
					case 'v':
						printf( "stripper version 1.1 (c) "
								"Desmond Schmidt 2011\n");
						s->doing_help = 1;
						break;
					case 'h':
						print_help();
						s->doing_help = 1;
						break;
					case 'f':
						if ( i < argc-2 )
						{
							s->selected_format = lookup_format( argv[i+1] );
							if ( s->selected_format == -1 )
								error("stripper: format %s not supported.\n",
                                    argv[i+1]);
						}
						else
							sane = 0;
						break;
					case 'l':
						list_formats();
						s->doing_help = 1;
						break;
                    case 'r':
                        s->recipe_file = argv[i+1];
                        break;
                    case 's':
                        s->style = argv[i+1];
                        break;
                    case 'e':
                        s->hh_except_string = strdup(argv[i+1]);
                        break;
				}
			}
			if ( !sane )
				break;
		}
		if ( !s->doing_help )
		{
			strncpy( s->src, argv[argc-1], FILE_NAME_LEN );
			sane = file_exists( s->src );
			if ( !sane )
				fprintf(stderr,"stripper: can't find file %s\n",s->src );
            else
            {
                strncpy(s->barefile, s->src, FILE_NAME_LEN );
                dot_pos = strrchr( s->barefile, '.' );
                if ( dot_pos != NULL )
                    dot_pos[0] = 0;
            }
		}
	}
	return sane;
}
/**
 * Tell them how to use the program.
 */
static void usage()
{
	printf( "usage: stripper [-h] [-v] [-s style] [-l] [-f format] "
        "[-r recipe] [-e hh_exceptions] XML-file\n" );
}
/**
 * The main entry point
 * @param argc number of commandline args+1
 * @param argv array of arguments, first is program name
 * @return 0 to the system
 */
int main( int argc, char **argv )
{
    stripper *s = stripper_create();
    if ( s != NULL )
    {
        int res = 1;
        if ( check_args(argc,argv,s) )
		{
            recipe *rules;
            if ( s->recipe_file == NULL )
                rules = recipe_new();
            else
            {
                int rlen;
                const char *rdata = read_file( s->recipe_file, &rlen );
                if ( rdata != NULL )
                {
                    rules = recipe_load(rdata, rlen);
                    free( (char*)rdata );
                }
            }
            if ( rules != NULL )
            {
                hh_exceptions *hhe = hh_exceptions_create( s->hh_except_string );
                s->user_data = userdata_create( s->language, s->barefile, 
                    rules, &formats[s->selected_format], hhe );
                if ( s->user_data == NULL )
                {
                    fprintf(stderr,"stripper: failed to initialise userdata\n");
                    res = 0;
                }
                if ( res && !s->doing_help )
                {
                    int i=0;
                    userdata *u = s->user_data;
                    while ( userdata_markup_dest(u,i) )
                    {
                        res = formats[s->selected_format].hfunc( NULL, 
                            dest_file_dst(userdata_markup_dest(u,i)), s->style );
                        i++;
                    }
                    // parse XML, prepare body for writing
                    if ( res )
                    {
                        int len;
                        const char *data = read_file( s->src, &len );
                        if ( data != NULL )
                        {
                            res = scan_source( data, len, s );
                            free( (char*)data );
                        }
                    }
                }
                // save the files in a separate step
                userdata_write_files( s->user_data );
            }
        }
        else
            usage();
        stripper_dispose( s );
    }
	return 0;
}
#endif
