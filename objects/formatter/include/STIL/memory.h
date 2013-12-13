/* 
 * File:   memory.h
 * Author: desmond
 *
 * Created on 17 November 2011, 12:35 PM
 */

#ifdef DEBUG_MEMORY
#ifndef MEMORY_H
#define	MEMORY_H
#ifdef	__cplusplus
extern "C" {
#endif
void *dbg_malloc(size_t a,const char *f,int l);
void dbg_free(void *a);
char *dbg_strdup(char *a,const char *f,int l);
void *dbg_calloc(size_t a,size_t b,const char *f,int l);
void memory_print();
void vp( void *p );
#define malloc(a) dbg_malloc(a,__FUNCTION__,__LINE__)
#define free(a) dbg_free(a)
#define strdup(a) dbg_strdup(a,__FUNCTION__,__LINE__)
#define calloc(a,b) dbg_calloc(a,b,__FUNCTION__,__LINE__)
#endif	/* MEMORY_H */
#ifdef	__cplusplus
}
#endif
#endif

