/* 
 * File:   master.h
 * Author: desmond
 *
 * Created on 19 November 2011, 9:20 AM
 */
#ifndef MASTER_H
#define	MASTER
#ifdef	__cplusplus
extern "C" {
#endif
typedef struct master_struct master;

master *master_create( char *text, int len );
void master_dispose( master *hf );
int master_load_markup( master *hf, const char *markup, int len, 
    const char *fmt ); 
int master_get_html_len( master *hf );
int master_load_css( master *hf, const char *css, int len );
char *master_convert( master *hf );
char *master_list();
#ifdef	__cplusplus
}
#endif
#endif	/* MASTER_H */
