#include "SsapiAssert.h"

extern "C" int Tracef(char *pFmt, ...);

#ifdef _DEBUG
void SSAPI_TRACE( int l, char *pString )	{ if(l<=2) Tracef( pString ); }
void SSAPI_TRACE( int l, char *pString, unsigned number ) { if(l<=2) Tracef("%s%d", pString, number); }
#else
void SSAPI_TRACE( int l, char *pString )	{  }
void SSAPI_TRACE( int l, char *pString, unsigned number ) { }

#endif