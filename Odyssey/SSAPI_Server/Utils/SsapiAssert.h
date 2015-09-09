//************************************************************************
// FILE:		SSAPIAssert.h
//
// PURPOSE:		Provides ASSERT() macro for debugging
//************************************************************************

#ifndef __SSAPI_ASSERT_H__
#define	__SSAPI_ASSERT_H__


void SSAPI_TRACE( int l, char *pString );
void SSAPI_TRACE( int l, char *pString, unsigned number ) ;


#ifdef _DEBUG

#ifdef WIN32
#include "Windows.h"
#define ASSERT( a ) if(a==0) MessageBox( NULL, "A value passed in == 0", "SSAPI Debug Assert", MB_OK | MB_APPLMODAL );
#else
#define	ASSERT( a ) ;// if(a==0) TRACE_STRING( TRACE_L1, "!!!!!!!!!!!!!!!!!!!!\nSSAPI Debug Assert!!!! \nThe value passed in == 0!\n\n");
#endif

#else // else RELEASE

#define ASSERT( a )

#endif // _DEBUG


#endif //__SSAPI_ASSERT_H__