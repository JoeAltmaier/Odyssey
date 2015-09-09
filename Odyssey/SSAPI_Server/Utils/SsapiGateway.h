//************************************************************************
// FILE:		SsapiGateway.h
//
// PURPOSE:		Defines the object who will manage validity of requests
//				received given the current state  of the system.
//************************************************************************

#ifndef __SSAPI_GATEWAY_H__
#define	__SSAPI_GATEWAY_H__


class DdmSSAPI;
class SsapiResponder;

#include "CtTypes.h"

#ifdef WIN32
#pragma pack(4)
#endif



class SsapiGateway{
	
	DdmSSAPI		*m_pDdmSSAPI;

public:

//************************************************************************
// SsapiGateway:
//
// PURPOSE:		Default constructor
//************************************************************************

SsapiGateway( DdmSSAPI *pDdmSSAPI );


//************************************************************************
// ~SsapiGateway:
//
// PURPOSE:		The destructor
//************************************************************************

~SsapiGateway();


//************************************************************************
// ShouldLetThisRequestThru:
//
// PURPOSE:		The brains of the object. Gets to decide who is allowed
//				and is not.
//
// RETURN:		true:	request is valid given current system state
//				false:	request is invalid given current system state
//************************************************************************

bool ShouldLetThisRequestThru( U32 requestCode, U32 managerType, SsapiResponder *pResponder );


private:


//************************************************************************
// IsSpecialRequest:
//
// PURPOSE:		Checks if given request is in the list of special requests
//************************************************************************

bool IsSpecialRequest( U32 managerType, U32 requestCode );

};


struct SSAPI_GATEWAY_SPECIAL_REQ_CELL{
	U32				managerType;
	U32				requestCode;
};

#endif // __SSAPI_GATEWAY_H__