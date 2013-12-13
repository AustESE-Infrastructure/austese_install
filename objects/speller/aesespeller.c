#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "calliope_AeseSpeller.h"
#include "aspell.h"
/** spell check object */
typedef struct checker_struct checker;
struct checker_struct
{
    AspellSpeller *spell_checker;
    AspellConfig *spell_config;
    char lang[24];
    checker *next;
};
struct dict_info_struct
{
    const char *name;
    const char *code;
    struct dict_info_struct *next;
};
typedef struct dict_info_struct dict_info;
static checker *checkers = NULL;

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
static void checker_dispose( checker *c )
{
    if ( c->spell_config != NULL )
        delete_aspell_config(c->spell_config);
    if ( c->spell_checker != NULL )
        delete_aspell_speller(c->spell_checker);
    if ( c->next != NULL )
        checker_dispose( c->next );
    free( c );
}
/**
 * Create a spell checker for a language
 * @param language the language code e.g. en_GB or it
 * @return a checker or NULL
 */
static checker *checker_create( const char *language )
{
    int err = 0;
    checker *c = calloc( 1, sizeof(checker) );
    if ( c != NULL )
    {
        strncpy(c->lang,language,24);
        c->spell_config = new_aspell_config();
        if ( c->spell_config != NULL )
        {
            aspell_config_replace( c->spell_config, "lang", language );
            AspellCanHaveError *possible_err 
                = new_aspell_speller(c->spell_config);
            c->spell_checker = 0;
            if (aspell_error_number(possible_err) != 0)
            {
                fprintf(stderr,"%s\n",aspell_error_message(possible_err));
                err = 1;
            }
            else
            {
                c->spell_checker = to_aspell_speller(possible_err);
                if ( c->spell_checker == NULL )
                {
                    fprintf(stderr,"checker: failed to initialise speller\n");
                    err = 1;
                }
            }
            if ( err )
            {
                checker_dispose( c );
                c = NULL;
            }
        }
        else
            fprintf(stderr,"checker: failed to create speller\n");
    }
    else
        fprintf(stderr,"checker: failed to create object\n");
    return c;
}
/*
 * Class:     calliope_AeseSpeller
 * Method:    cleanup
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_calliope_AeseSpeller_cleanup
  (JNIEnv *env, jobject obj)
{
    if ( checkers != NULL )
    {
        checker_dispose( checkers );
        checkers = NULL;
    }
}
JNIEXPORT jboolean JNICALL Java_calliope_AeseSpeller_initialise
  (JNIEnv *env, jobject obj, jstring lang)
{
    jboolean copied;
    if ( checkers == NULL )
    {    
        const char *language = load_string( env, lang, &copied );
        checkers = checker_create( language );
        unload_string( env, lang, language, copied );
    }
    return checkers != NULL;
}
/*
 * Class:     calliope_AeseSpeller
 * Method:    hasWord
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_calliope_AeseSpeller_hasWord
  (JNIEnv *env, jobject obj, jstring jword, jstring lang)
{
    int correct = 0;
    jboolean copied1, copied2;
    checker *c = checkers;
    const char *word = load_string( env, jword, &copied1 );
    const char *language = load_string( env, lang, &copied2 );
    while ( c != NULL )
        if ( strcmp(language,c->lang)!=0 )
            c = c->next;
        else
            break;
    if ( c == NULL )
    {
        c = checker_create( language );
        if ( c != NULL )
        {
            if ( checkers == NULL )
                checkers = c;
            else
            {
                checker *temp = checkers;
                while ( temp->next != NULL )
                    temp = temp->next;
                temp->next = c;
            }
        }
    }
    if ( c != NULL )
    {
        correct = aspell_speller_check(c->spell_checker, word, 
            strlen(word));
    }
    else
        fprintf(stderr,"checker: no dict for language %s\n",language);
    if ( copied1 )
        unload_string( env, jword, word, copied1 );
    if ( copied2 )
        unload_string( env, lang, language, copied2 );
    return correct;
}
/*
 * Return an array of strings, each being the dict's code:name
 * Class:     calliope_AeseSpeller
 * Method:    listDicts
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_calliope_AeseSpeller_listDicts
  (JNIEnv *env, jobject obj)
{
    AspellConfig *config;
    AspellDictInfoList *dlist;
    AspellDictInfoEnumeration *dels;
    const AspellDictInfo *entry;
    config = new_aspell_config();
    /* the returned pointer should _not_ need to be deleted */
    dlist = get_aspell_dict_info_list(config);
    /* config is no longer needed */
    delete_aspell_config(config);
    jobjectArray ret=NULL;  
    int i=0;
    dels = aspell_dict_info_list_elements(dlist);
    if ( dels != NULL )
    {
        dict_info *head = NULL;
        dict_info *di,*tail = NULL;
        int size = 0;
        while ( (entry = aspell_dict_info_enumeration_next(dels)) != 0)
        {
            size++;
            di = calloc( 1, sizeof(dict_info) );
            if ( di != NULL )
            {
                di->name = entry->name;
                di->code = entry->code;
                if ( head == NULL )
                    tail = head = di;
                else
                {
                    tail->next = di;
                    tail = di;
                }
            }
        }
        ret = (jobjectArray)(*env)->NewObjectArray(env,size,  
             (*env)->FindClass(env,"java/lang/String"),  
             (*env)->NewStringUTF(env,""));
        di = head;
        i = 0;
        while ( di != NULL )
        {
            dict_info *next = di->next;
            int slen = strlen(di->code)+strlen(di->name)+2;
            char *str = calloc(slen,1);
            if ( str != NULL )
            {
                snprintf(str,slen,"%s\t%s",di->code,di->name);
                (*env)->SetObjectArrayElement(env,ret,i++,
                    (*env)->NewStringUTF(env,str));  
                free( str );
            }
            free( di );
            di = next;
        }
        delete_aspell_dict_info_enumeration(dels);
    }
    return ret;
}