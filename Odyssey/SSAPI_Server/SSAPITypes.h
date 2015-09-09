//************************************************************************
// FILE:		SSAPITypes.h
//
// PURPOSE:		Defines types used by the SSAPI layer
//************************************************************************

#ifndef __SSAPI_TYPE_H__
#define	__SSAPI_TYPE_H__

#include "CtTypes.h"

#pragma pack(4)


typedef	U32			LocalizedString;
typedef	I64			LocalizedDateTime;


#ifdef WIN32			// WIn32
#define METHOD_ADDRESS(clas,method)	&clas::method
#elif defined(__ghs__)  // Green Hills
#define METHOD_ADDRESS(clas,method)	&clas::method
#else					// MetroWerks
#define METHOD_ADDRESS(clas,method)	&clas::method
#endif



#endif //	__SSAPI_TYPE_H__