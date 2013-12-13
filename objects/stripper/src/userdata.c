#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <ctype.h>
#ifdef JNI
#include <jni.h>
#include "log.h"
#endif
#include "expat.h"
#include "attribute.h"
#include "simplification.h"
#include "milestone.h"
#include "layer.h"
#include "recipe.h"
#include "stack.h"
#include "hashmap.h"
#include "ramfile.h"
#include "format.h"
#include "range.h"
#include "dest_file.h"
#include "hh_exceptions.h"
#include "userdata.h"
#include "aspell.h"
#include "utils.h"
struct userdata_struct
{
    /** flag to remove multiple white space */
    int last_char_type;
    /** current offset in the text file */
    int current_text_offset;
    /** last token ended in hyphen */
    int hyphen_state;
    /** offset at which hyphen begins in text */
    int hoffset;
    /** last word in line ending in hyphen */
    char *last_word;
    /** the recipe */
    recipe *rules;
    /** stack of potential ranges being maintained
     * as we parse the file */
    stack *range_stack;
    /** if non-empty ignore all tags and text */
    stack *ignoring;
    /** pointer to output stripped text file */
    dest_file *text_dest;
    /** array of markup_dest */
    dest_file **markup_dest;
    /** map of XML names to dest_files */
    hashmap *dest_map;
    /** hard hyphen exceptions */
    hh_exceptions *hhe;
    /** spell check object */
    AspellSpeller *spell_checker;
    AspellConfig *spell_config;
};
/**
 * Open the dest files
 * @return 1 if it worked else 0
 */
static int open_dest_files( userdata *u, char *barefile, format *fmt )
{
    int res = 0;
    u->text_dest = dest_file_create( text_kind, NULL, "", barefile, fmt );
    if ( u->text_dest != NULL )
    {
        res = dest_file_open( u->text_dest );
        if ( res )
        {
            // always at least one markup df is needed
            dest_file *df = dest_file_create( markup_kind, NULL, 
                (char*)fmt->middle_name, barefile, fmt );
            if ( df != NULL && u->rules != NULL )
            {
                // ask the recipe which markup files to create
                int i,n_layers = recipe_num_layers( u->rules );
                u->markup_dest = calloc( n_layers+2, sizeof(dest_file*));
                if ( u->markup_dest != NULL )
                {
                    u->markup_dest[0] = df;
                    for ( i=1;i<=n_layers;i++ )
                    {
                        layer *l = recipe_layer( u->rules, i-1 );
                        char *name = layer_name( l );
                        int mlen = strlen(fmt->middle_name)
                            +strlen(name)+2;
                        char *mid_name = malloc( mlen );
                        if ( mid_name != NULL )
                        {
                            snprintf( mid_name, mlen, "%s-%s", 
                                fmt->middle_name, name );
                            //dest_kind kind, layer *l, char *midname, 
                            //char *name, format *f
                            u->markup_dest[i] = dest_file_create( markup_kind,
                                l, mid_name, barefile, fmt );
                            free( mid_name );
                        }
                        else
                        {
                            fprintf(stderr,
                                "stripper: failed to allocate name\n");
                            break;
                        }
                    }
                    if ( i == n_layers )
                        res = 1;
                }
                else
                    fprintf(stderr,
                        "stripper: failed to allocate markup array\n");
                // now open the markup dest files 
                if ( res )
                {
                    i = 0;
                    while ( u->markup_dest[i] != NULL && res )
                        res = dest_file_open( u->markup_dest[i++] );
                }
            }
        }
    }
    if ( res )
        u->dest_map = hashmap_create();
    return res&&(u->dest_map!=NULL);
}
/**
 * Create a userdata object
 * @param language the language e.g. "en_GB"
 * @param rules recipe file path to recipe file
 * @param fmt the format object containing function pointers
 * @return a complete userdata object or NULL
 */
userdata *userdata_create( const char *language, char *barefile, recipe *rules, 
    format *fmt, hh_exceptions *hhe )
{
    int err = 0;
    userdata *u = calloc( 1, sizeof(userdata) );
    if ( u != NULL )
    {
        u->rules = rules;
        if ( hhe != NULL )
            u->hhe = hhe;
        u->spell_config = new_aspell_config();
        if ( u->spell_config != NULL )
        {
            aspell_config_replace( u->spell_config, "lang", language );
            AspellCanHaveError *possible_err 
                = new_aspell_speller(u->spell_config);
            u->spell_checker = 0;
            if (aspell_error_number(possible_err) != 0)
            {
                fprintf(stderr,"%s\n",aspell_error_message(possible_err));
                err = 1;
            }
            else
            {
                u->spell_checker = to_aspell_speller(possible_err);
                if ( u->spell_checker == NULL )
                {
                    fprintf(stderr,"userdata: failed to initialise speller\n");
                    err = 1;
                }
            }
            u->range_stack = stack_create();
            if ( u->range_stack == NULL )
            {
                err = 1;
                fprintf(stderr, 
                    "stripper: failed to allocate store for range stack" );
            }
            u->ignoring = stack_create();
            if ( u->ignoring == NULL )
            {
                err = 1;
                fprintf(stderr, 
                    "stripper: failed to allocate store for ignore stack" );
            }
            if ( !open_dest_files(u,barefile,fmt) )
            {
                err = 1;
                fprintf(stderr,"stripper: couldn't open dest files\n");
            }
        }
        else
        {
            fprintf(stderr, "userdata: failed to initialise speller\n");
            err = 1;
        }
    }
    else
        fprintf(stderr, "userdata:failed to allocate object\n");
    if ( err )
    {
        userdata_dispose( u );
        u = NULL;
    }
    return u;
}
/**
 * Dispose of a userdata
 * @param u the object to dispose
 */
void userdata_dispose( userdata *u )
{
    if ( u != NULL )
    {
        if ( u->rules != NULL )
            u->rules = recipe_dispose( u->rules );
        if ( u->ignoring != NULL )
            stack_delete( u->ignoring );
        if ( u->range_stack != NULL )
            stack_delete( u->range_stack );
        if ( u->spell_config != NULL )
            delete_aspell_config(u->spell_config);
        if ( u->spell_checker != NULL )
            delete_aspell_speller(u->spell_checker);
        if ( u->last_word != NULL )
            free( u->last_word );
        if ( u->dest_map != NULL )
            hashmap_dispose( u->dest_map );
        // we don't own the hh_exceptions
        free( u );
    }
}
#ifdef JNI
static int set_string_field( JNIEnv *env, jobject obj, 
    const char *field_name, char *value )
{
    int res = 0;
    jfieldID fid;
    jstring jstr;
    initlog();
    //printf("setting string field\n");
    jclass cls = (*env)->GetObjectClass(env, obj);
    fid = (*env)->GetFieldID(env, cls, field_name, "Ljava/lang/String;");
    if (fid != NULL) 
    {
        jstr = (*env)->NewStringUTF( env, value );
        if (jstr != NULL) 
        {
            (*env)->SetObjectField(env, obj, fid, jstr);
            res = 1;
        }
    }
    return res;
}
static int add_layer( JNIEnv *env, jobject obj, char *value )
{
    int res = 0;
    jstring jstr;
    jstr = (*env)->NewStringUTF( env, value );
    if (jstr != NULL) 
    {
        jclass cls = (*env)->GetObjectClass( env, obj );
        jmethodID mid = (*env)->GetMethodID( env, cls,"addLayer",
            "(Ljava/lang/String;)V");
        if ( mid == 0 )
        {
            tmplog("stripper: failed to find method addLayer\n");
            res = 0;
        }
        else
        {
            (*env)->ExceptionClear( env );
            (*env)->CallVoidMethod( env, obj, mid, jstr);
            if((*env)->ExceptionOccurred(env)) 
            {
                fprintf(stderr,"stripper: couldn't add layer\n");
                (*env)->ExceptionDescribe( env );
                (*env)->ExceptionClear( env );
            }
            else
                res = 1;
        }
    }
    return res;
}
/**
 * Write out the text and markup files to JNI objects
 * @param env the JNI environment
 * @param u userdata
 * @param text text object
 * @param markup markup object
 */
void userdata_write_files( JNIEnv *env, userdata *u, jobject text, 
    jobject markup )
{
    // write out text
    dest_file_close( u->text_dest, 0 );
    int tlen = dest_file_len( u->text_dest );
    // save result to text and markup objects
    int res = set_string_field( env, text, "body", 
        ramfile_get_buf(dest_file_dst(u->text_dest)) );
    dest_file_dispose( u->text_dest );
    int i=0;
    while ( u->markup_dest[i] != NULL && res )
    {
        dest_file_close( u->markup_dest[i], tlen );
        if ( res ) 
        {
            DST_FILE *df = dest_file_dst(u->markup_dest[i]);
            if ( i == 0 )
            {
                res = set_string_field( env, markup, "body", 
                    ramfile_get_buf(df) );
            }
            else
            {
                res = add_layer( env, markup, ramfile_get_buf(df) );
            }
        }
        dest_file_dispose( u->markup_dest[i++] );
    }
}
#else
void userdata_write_files( userdata *u )
{
    int tlen = 0;
    if ( u->text_dest != NULL )
    {
        tlen = dest_file_len( u->text_dest );
        dest_file_close( u->text_dest, 0 );
        dest_file_dispose( u->text_dest );
    }
    // write body for markup files
    int i=0;
    if ( u->markup_dest != NULL )
    {
        while ( u->markup_dest[i] != NULL )
        {
            dest_file_close( u->markup_dest[i], tlen );
            dest_file_dispose( u->markup_dest[i++] );
        }
        free( u->markup_dest );
    }
}
#endif
/**
 * Get the text destination file
 * @param u the userdata object
 * @return the dest file for text
 */
dest_file *userdata_text_dest( userdata *u )
{
    return u->text_dest;
}
/**
 * Work out which dest file to send a named range to
 * @param s the stripper object
 * @param range_name the range name
 * @return the appropriate dest_file object
 */
dest_file *userdata_get_markup_dest( userdata *u, char *range_name )
{
    if ( hashmap_contains(u->dest_map,range_name) )
    {
        return (dest_file*)hashmap_get( u->dest_map, range_name );
    }
    else
    {
        int i=1;
        while ( u->markup_dest[i] != NULL )
        {
            layer *l = dest_file_layer(u->markup_dest[i]);
            if ( l!=NULL&& layer_has_milestone(l,range_name) )
            {
                // remember for future calls
                hashmap_put( u->dest_map,range_name,
                    u->markup_dest[i] );
                return u->markup_dest[i];
            }
            i++;
        }
        hashmap_put( userdata_dest_map(u), range_name, u->markup_dest[0] );
        return u->markup_dest[0];
    }
}
/**
 * Get the userdata's range stack
 * @param u the userdata object
 * @return a stack
 */
stack *userdata_range_stack( userdata *u )
{
    return u->range_stack;
}
/**
 * Get the ignoring stack
 * @param u the userdata object
 * @return the stack of tags to ignore
 */
stack *userdata_ignoring( userdata *u )
{
    return u->ignoring;
}
/**
 * Get the stripping rules
 * @param u the userdata object
 * @return the current stripping rule set
 */
recipe *userdata_rules( userdata *u )
{
    return u->rules;
}
/**
 * Get a markup destination by its index
 * @param u the userdata object
 * @param i the index
 * @return the dest_file
 */
dest_file *userdata_markup_dest( userdata *u, int i )
{
    return u->markup_dest[i];
}
/**
 * Get the destination map of markup files
 * @param u the userdata object
 * @return the map of markup files
 */
hashmap *userdata_dest_map( userdata *u )
{
    return u->dest_map;
}
/**
 * Does the current spell-checker have this word?
 * @param u the userdata object
 * @param word the word to lookup
 * @return 1 if it was there else 0
 */
int userdata_has_word( userdata *u, XML_Char *word )
{
    int correct = aspell_speller_check(u->spell_checker, word, 
        strlen((char*)word));
    return correct;
}
/**
 * Get the character offset (not byte offset!)
 * @param u the userdata object in question
 * @return the character offset 
 */
int userdata_toffset( userdata *u )
{
    return u->current_text_offset;
}
int userdata_last_char_type( userdata *u )
{
    return u->last_char_type;
}
void userdata_inc_toffset( userdata *u, int inc )
{
    u->current_text_offset += inc;
}
void userdata_set_last_char_type( userdata *u, int ctype )
{
    u->last_char_type = ctype;
}
void userdata_set_rules( userdata *u, recipe *r )
{
    if ( u->rules != NULL )
        recipe_dispose( u->rules );
    u->rules = r;
}
int userdata_hyphen_state( userdata *u )
{
    return u->hyphen_state;
}
void userdata_set_hyphen_state( userdata *u, int hstate )
{
    u->hyphen_state = hstate;
}
int userdata_has_hh_exception( userdata *u, char *combination )
{
    if ( u->hhe != NULL )
        return hh_exceptions_lookup(u->hhe,combination);
    else
        return 0;
}
/**
 * Duplicate the last word of a text fragment
 * @param text the text to pop the word off of
 * @param len its length
 * @return an allocated word in UTF8 or the empty string
 */
static XML_Char *last_word( XML_Char *text, int len )
{
    int size,i=len-1;
    if ( len > 0 )
    {
        int start = 0;
        // point beyond trailing hyphen
        if ( text[len-1] == '-' )
        {
            len--;
            i--;
        }
        else
        {
            // point to last non-space
            for ( ;i>0;i-- )
            {
                if ( !isspace(text[i]) )
                    break;
            }
        }
        int j = i;
        for ( ;i>0;i-- )
        {
            if ( !isalpha(text[i]) )
            {
                start = i+1;
                size = j-i;
                break;
            }
        }
        if ( i==0 )
            size = (j-i)+1;
        XML_Char *word = calloc( 1+size, sizeof(XML_Char) );
        if ( word != NULL )
        {
            int nbytes = size*sizeof(XML_Char);
            memcpy( word, &text[start], nbytes );
        }
        return word;
    }
    else
        return "";
}
void userdata_update_last_word( userdata *u, char *line, int len )
{
    if ( u->last_word != NULL )
        free( u->last_word );
    u->last_word = last_word( line, len );
}
XML_Char *userdata_last_word( userdata *u )
{
    return u->last_word;
}
void userdata_clear_last_word( userdata *u )
{
    if ( u->last_word != NULL )
        free( u->last_word );
    u->last_word = NULL;
}
int userdata_hoffset( userdata *u )
{
    return u->hoffset;
}
void userdata_set_hoffset( userdata *u, int hoffset )
{
    u->hoffset = hoffset;
}