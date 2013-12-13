#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "hh_exceptions.h"
struct hh_exceptions_struct
{
    /** hard-hyphen exceptions: e.g. "underfoot" */
    char **hh_array;
    /** number of hh_exceptiops */
    int hh_size;
};
/**
 * Shell sort the array of strings
 * @param array an array of strings
 * @param N its length
 */

static void hh_extensions_sort( char **array, int N )
{
    int i, j, k, h; char *v;
    int incs[16] = { 1391376, 463792, 198768, 86961, 33936,
        13776, 4592, 1968, 861, 336, 
        112, 48, 21, 7, 3, 1 };
    for ( k = 0; k < 16; k++)
    {
        for ( h=incs[k],i=h;i<=N-1;i++ )
        { 
            v = array[i]; 
            j = i;
            while (j >= h && strcmp(array[j-h],v)>0 )
            { 
                array[j] = array[j-h]; 
                j -= h; 
            }
            array[j] = v; 
        }
    }
}
/**
 * Add a list of merged words to tell stripper when NOT to soft-hyphen
 * compound words
 * @param hee the hh_exceptions object
 * @param list a space-separated list of merged hard-hyphen words
 */
static void hh_extensions_load( hh_exceptions *hhe, char *list )
{
    if ( list != NULL )
    {
        int i = 0;
        int spaces = 0;
        int hlen = strlen(list);
        int was_space = 0;
        for ( i=0;i<hlen;i++ )
        {
            if ( isspace(list[i]) )
            {
                if ( !was_space )
                {
                    spaces++;
                    was_space = 1;
                }
            }
            else
                was_space = 0;
        }
        i = 0;
        hhe->hh_array = calloc( spaces+1, sizeof(char*) );
        if ( hhe->hh_array != NULL )
        {
            hhe->hh_size = spaces+1;
            char *hh_compound = strtok( list, " \t\n\r" );
            while ( hh_compound != NULL )
            {
                hhe->hh_array[i++] = strdup(hh_compound);
                hh_compound = strtok( NULL, " \t\n\r");
                if ( i == spaces+1 )
                    break;
            }
            hh_extensions_sort( hhe->hh_array, hhe->hh_size );
        }
    }
}
/**
 * Create a hard hyphen exceptions object
 * @param hh_list a space-delimited list of compound words
 * @return a sorted string array
 */
hh_exceptions *hh_exceptions_create( char *hh_list )
{
    hh_exceptions *hhe = calloc( 1, sizeof(hh_exceptions) );
    if ( hhe != NULL )
    {
        hh_extensions_load( hhe, hh_list );
    }
    else
        fprintf(stderr,"hh_exceptions: failed to allocate object\n");
    return hhe;
}
/**
 * Dispose of this object and its list
 * @param hhe the hard hyphen exceptions object
 */
void hh_exceptions_dispose( hh_exceptions *hhe )
{
    int i;
    if ( hhe->hh_array != NULL )
    {
        for ( i=0;i<hhe->hh_size;i++ )
        {
            if ( hhe->hh_array[i] != NULL )
                free( hhe->hh_array[i] );
        }
        free( hhe->hh_array );
    }
    free( hhe );
}
/**
 * Lookup a word in the list
 * @param hhe the hh_exceptions object
 * @param combination the combined word e.g. "strikeout"
 * @return 1 if it is preset else 0
 */
int hh_exceptions_lookup( hh_exceptions *hhe, char *combination )
{
    int top = 0;
    int bot=hhe->hh_size-1;
    while ( top <= bot )
    {
        int mid = (top+bot)/2;
        int res = strcmp(combination,hhe->hh_array[mid]);
        if ( res>0 )
            top = mid+1;
        else if ( res < 0 )
            bot = mid-1;
        else
            return 1;
    }
    return 0;
}
#ifdef HH_EXCEPTIONS_TEST
int main( int argc, char **argv )
{
    int i;
    char *str = strdup("scapegoat\tunderfoot   thunderclap\n\roverbearing");
    hh_exceptions *hhe = hh_exceptions_create( str);
    int res = hh_exceptions_lookup( hhe, "scapegoat" );
    if ( res )
        printf("found 'scapegoat\n");
    res = hh_exceptions_lookup( hhe, "underfoot" );
    if ( res )
        printf("found 'underfoot\n");
    res = hh_exceptions_lookup( hhe, "overbearing" );
    if ( res )
        printf("found 'overbearing\n");
    res = hh_exceptions_lookup(hhe, "dogbreath" );
    if ( !res )
        printf("did not find 'dogbreath'\n");
    for ( i=0;i<hhe->hh_size;i++ )
        printf("%s\n",hhe->hh_array[i]);
    hh_exceptions_dispose( hhe );
}
#endif