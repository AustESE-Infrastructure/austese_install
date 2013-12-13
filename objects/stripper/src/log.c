#include <stdarg.h>
#include <stdio.h>
#include <string.h>
void initlog()
{
    FILE *fp = fopen("/tmp/stripper.log","w");
    fclose( fp );
}
void tmplog( const char *fmt, ... )
{
    char message[256];
    va_list l;
    va_start(l,fmt);
    vsnprintf( message, 255, fmt, l );
    va_end(l);
    FILE *fp = fopen("/tmp/stripper.log","a+");
    fwrite(message,1,strlen(message),fp);
    fclose(fp);
}

