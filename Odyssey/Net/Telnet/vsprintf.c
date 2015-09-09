/****************************************************************
	Copyright 1990 Software Development Systems, Inc.

	vsprintf routine
****************************************************************/
#include "clib.h"
#include "stdarg.h"
#include "stdio.h"


int _uprint( int (*put)(int,char *),char *,const char *,va_list );

int _sputc( int, char ** );

int _vsprintf( char *where,
			  const char *fmt,
			  va_list varg ) 

{
    return _uprint( (int(*)(int,char*))_sputc, (char *)&where, fmt, varg ); 
}
