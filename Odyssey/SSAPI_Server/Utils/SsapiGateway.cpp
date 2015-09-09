//************************************************************************
// FILE:		SsapiGateway.cpp
//
// PURPOSE:		Implements the object who will manage validity of requests
//				received given the current state  of the system.
//************************************************************************

#include "SsapiGateway.h"
#include "UserManager.h"
#include "DdmSSAPI.h"
#include "Ssapi_Codes.h"
#include "SsapiLocalResponder.h"
#include "CtEvent.h"



SSAPI_GATEWAY_SPECIAL_REQ_CELL SsapiGatewaySpecialReqTable[] = {
	{SSAPI_MANAGER_CLASS_TYPE_USER_MANAGER,		SSAPI_USER_MANAGER_LOGIN},
	{SSAPI_MANAGER_CLASS_TYPE_ALARM_MANAGER,	SSAPI_OBJECT_MANAGER_LIST},
	{SSAPI_MANAGER_CLASS_TYPE_ALARM_MANAGER,	SSAPI_OBJECT_MANAGER_ADD_LISTENER},
	{SSAPI_MANAGER_CLASS_TYPE_ALARM_MANAGER,	SSAPI_MANAGED_OBJECT_ADD_LISTENER},
	{SSAPI_MANAGER_CLASS_TYPE_ALARM_MANAGER,	SSAPI_OBJECT_MANAGER_DELETE_LISTENER},
	{SSAPI_MANAGER_CLASS_TYPE_ALARM_MANAGER,	SSAPI_MANAGED_OBJECT_DELETE_LISTENER},
	{SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER,	SSAPI_OBJECT_MANAGER_LIST}
};

#define	SSAPI_GATEWAY_SPECIAL_REQ_COUNT	sizeof(SsapiGatewaySpecialReqTable)/sizeof(SsapiGatewaySpecialReqTable[0])

//************************************************************************
// SsapiGateway:
//
// PURPOSE:		Default constructor
//************************************************************************

SsapiGateway::SsapiGateway( DdmSSAPI *pDdmSSAPI ){

	m_pDdmSSAPI	 = pDdmSSAPI;
}


//************************************************************************
// ~SsapiGateway:
//
// PURPOSE:		The destructor
//************************************************************************

SsapiGateway::~SsapiGateway(){
}


//************************************************************************
// ShouldLetThisRequestThru:
//
// PURPOSE:		The brains of the object. Gets to decide who is allowed
//				and is not.
//
// RETURN:		true:	request is valid given current system state
//				false:	request is invalid given current system state
//
// NOTE:		replies if needed
//************************************************************************

bool 
SsapiGateway::ShouldLetThisRequestThru( U32 requestCode, U32 managerType, SsapiResponder *pResponder ){

	UserManager	*pUManager = (UserManager *)m_pDdmSSAPI->GetObjectManager(SSAPI_MANAGER_CLASS_TYPE_USER_MANAGER);

	if( pResponder->GetSessionID() == SSAPI_LOCAL_SESSION_ID )
		return true;	// local session is all-powerful!

	if( !pUManager->IsValidRequest( requestCode, pResponder ) ){
		//if special request -> ok, else GITR!
		if( IsSpecialRequest( managerType, requestCode ) )
			return true;
		else{
			printf("\n####Not Logged in Yet####, failed request to the ");
			char *p = m_pDdmSSAPI->GetObjectManager(managerType)->GetName().CString();
			printf( p ); printf("\n");
			delete p;
			pResponder->RespondToRequest( SSAPI_EXCEPTION_SECURITY, CTS_SSAPI_EXCEPTION_NOT_LOGGED_IN);
			return false;
		}
	}


	return true;
}


//************************************************************************
// IsSpecialRequest:
//
// PURPOSE:		Checks if given request is in the list of special requests
//************************************************************************

bool 
SsapiGateway::IsSpecialRequest( U32 managerType, U32 requestCode ){

	U32		i;

	for( i = 0; i < SSAPI_GATEWAY_SPECIAL_REQ_COUNT; i++ )
		if( SsapiGatewaySpecialReqTable[i].managerType == managerType )
			if( SsapiGatewaySpecialReqTable[i].requestCode == requestCode )
				return true;

	return false;
}
