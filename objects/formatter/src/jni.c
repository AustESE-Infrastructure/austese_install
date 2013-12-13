#ifdef JNI
#include <jni.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "calliope_AeseFormatter.h"
#include "master.h"
#include "memwatch.h"


static int set_string_field( JNIEnv *env, jobject obj, 
    const char *field_name, char *value )
{
    int res = 0;
    jfieldID fid;
    jstring jstr;
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
/**
 * Change this to report differently when a library or commandline tool
 */
void jni_report( const char *fmt, ... )
{
    va_list ap;
    char message[128];
    va_start( ap, fmt );
    vsnprintf( message, 128, fmt, ap );
    FILE *db = fopen("/tmp/formatter-debug.txt","a+");
    if ( db != NULL )
    {
        fprintf( db, "%s", message );
        fclose( db );
    }
    va_end( ap );
}

/*
 * Class:     calliope_AeseFormatter
 * Method:    format
 * Signature: ([B[Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;Lcalliope/json/JSONResponse;)I
 */
JNIEXPORT jint JNICALL Java_calliope_AeseFormatter_format
  (JNIEnv *env, jobject obj, jbyteArray text, jobjectArray markup, 
    jobjectArray css, jobjectArray formats, jobject jsonHtml)
{
    int res=0;
    jsize i,len,flen;
    char *html;
    jboolean isCopy=0;
    //jni_report("entered format\n");
    jbyte *t_data = (*env)->GetByteArrayElements(env, text, &isCopy);
    int t_len = (*env)->GetArrayLength( env, text );
    if ( t_data != NULL && markup != NULL && css != NULL && formats != NULL )
    {
        jboolean isMarkupCopy,isFormatCopy;
        master *hf = master_create( (char*)t_data, t_len );
        if ( hf != NULL )
        {
            len = (*env)->GetArrayLength(env, markup);
            flen = (*env)->GetArrayLength(env, formats);
            for ( i=0;i<len&&i<flen;i++ )
            {
                res = 1;
                jstring markup_str = (jstring)(*env)->GetObjectArrayElement(
                    env, markup, i );
                jstring format_str = (jstring)(*env)->GetObjectArrayElement(
                    env, formats, i );
                const char *markup_data = (*env)->GetStringUTFChars(env, 
                    markup_str, &isMarkupCopy);
                const char *format_data = (*env)->GetStringUTFChars(env, 
                    format_str, &isFormatCopy);
                if ( markup_data != NULL && format_data != NULL )
                {
                    res = master_load_markup( hf, markup_data, 
                        (int)strlen(markup_data), format_data );
                }    
                if ( markup_data != NULL && isMarkupCopy==JNI_TRUE )
                    (*env)->ReleaseStringUTFChars( env, markup_str, markup_data );
                if ( format_data != NULL && isFormatCopy==JNI_TRUE )
                    (*env)->ReleaseStringUTFChars( env, format_str, format_data );
                if ( !res )
                    break;
            }
            if ( res )
            {
                len = (*env)->GetArrayLength(env, css);
                for ( i=0;i<len;i++ )
                {
                    jboolean isCssCopy;
                    jstring css_str = (jstring)(*env)->GetObjectArrayElement(
                        env, css, i);
                    const char *css_data = (*env)->GetStringUTFChars(env, css_str, 
                        &isCssCopy);
                    if ( css_data != NULL )
                    {
                        res = master_load_css( hf, css_data, (int)strlen(css_data) );
                        if ( isCssCopy==JNI_TRUE )
                            (*env)->ReleaseStringUTFChars( env, css_str, css_data );
                        if ( !res )
                            break;
                    }
                }
                if ( res )
                {
                    //jni_report( "about to call master_convert\n" );
                    html = master_convert( hf );
                    //jni_report( "finished calling master_convert\n" );
                    if ( html != NULL )
                        res = set_string_field( env, jsonHtml, "body", html );
                }
            }
            master_dispose( hf );
        }
    }
    if ( t_data != NULL )
        (*env)->ReleaseByteArrayElements( env, text, t_data, JNI_ABORT );
#ifdef DEBUG_MEMORY
        memory_print();
#endif
    return res;
}
#endif
