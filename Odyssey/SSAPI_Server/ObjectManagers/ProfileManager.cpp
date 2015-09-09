//************************************************************************
// FILE:		ProfileManager.cpp
//
// PURPOSE:		Implements the object to be the interface to the OS' DDM
//				Profiler via SSAPI infrostructure
//************************************************************************

#include "ProfileManager.h"
#include "SsapiAssert.h"
#include "RqProfile.h"
#include "SsapiResponder.h"
#include "CtEvent.h"
#include "RqLeak.h"

#include "Trace_Index.h"
#ifdef TRACE_INDEX
#undef TRACE_INDEX
#endif
#define TRACE_INDEX TRACE_SSAPI_MANAGERS

ProfileManager*		ProfileManager::m_pThis;

#ifdef WIN32
#define H_TO_N( a )	(	((a & 0xFF000000) >> 24) |\
						((a & 0x00FF0000) >> 8)  |\
						((a & 0x0000FF00) << 8)  |\
						((a & 0x000000FF) << 24 ) ) 
#else
#define	H_TO_N( a ) a
#endif

//************************************************************************
// ProfileManager:
//
// PURPOSE:		Default constructor
//************************************************************************

ProfileManager::ProfileManager( ListenManager *pLM, DdmServices *pParent )
	:ObjectManager( pLM, DesignatorId(RowId(), SSAPI_MANAGER_CLASS_TYPE_PROFILE_MANAGER), pParent ){

	SetIsReadyToServiceRequests( true );
	SSAPI_TRACE( TRACE_L2, "Mr. Profiler is alive and ready" );
}


//************************************************************************
// ~ProfileManager:
//
// PURPOSE:		The destructor
//************************************************************************

ProfileManager::~ProfileManager(){
}


//************************************************************************
// Dispatch:
//
// PURPOSE:		Dispatches a request to whoever should service it.
//				
// RECEIVE:		requestParms:		value set with request parms
//				reuestCode:			code of the request
//				pResponder:			a wrapper object that knows how to 
//									respond
//
// NOTE:		All sub-classes that override this method MUST call it on
//				their superclass before processing!!! If this method 
//				returned false, only then should they tray to handle 
//				a request.
//************************************************************************

bool 
ProfileManager::Dispatch( ValueSet *pRequestParms, U32 requestCode, SsapiResponder *pResponder){

	switch( requestCode ){
		case SSAPI_PROFILE_MANAGER_START:
			ServiceStart( pRequestParms, pResponder );
			break;

		case SSAPI_PROFILE_MANAGER_STOP:
			ServiceStop( pRequestParms, pResponder );
			break;

		case SSAPI_PROFILE_MANAGER_CLEAR:
			ServiceClear( pRequestParms, pResponder );
			break;

		case SSAPI_PROFILE_MANAGER_DELIVER:
			ServiceDeliver( pRequestParms, pResponder );
			break;

		case SSAPI_PROFILE_MANAGER_HEAP_CLEAR:
			ServiceHeapClear( pRequestParms, pResponder );
			break;

		case SSAPI_PROGILE_MANAGER_HEAP_DELIVER:
			ServiceHeapDeliver( pRequestParms, pResponder );
			break;

		default:
			ASSERT(0);
			pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION);
			break;
	}

	return false;
}


//************************************************************************
//
//************************************************************************

void 
ProfileManager::ServiceStart( ValueSet *pParms, SsapiResponder *pResponder ){

	I64			interval;
	TySlot		slot;

	pParms->GetInt64( SSAPI_PROFILE_MANAGER_START_INTERVAL, &interval );
	pParms->GetInt( SSAPI_PROFILE_MANAGER_START_IOP_SLOT, (int *)&slot );

	Send(	slot,
			new RqProfileStart( interval ),
			pResponder,
			(ReplyCallback)METHOD_ADDRESS( ProfileManager ,ServiceStartCallback ) );
}


STATUS  
ProfileManager::ServiceStartCallback( Message *pReply ){

	RqProfileStart		*pMsg = (RqProfileStart *)pReply;
	SsapiResponder		*pResponder = (SsapiResponder *)pReply->GetContext();

	if( pMsg->DetailedStatusCode == OK )
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	else
		pResponder->RespondToRequest(  SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );

	delete pMsg;
	return OK;
}

//************************************************************************
//
//************************************************************************

void  
ProfileManager::ServiceStop( ValueSet *pParms, SsapiResponder *pResponder ){

	TySlot		slot;

	pParms->GetInt( SSAPI_PROFILE_MANAGER_STOP_IOP_SLOT, (int *)&slot );

	Send(	slot,
			new RqProfileStop(),
			pResponder,
			(ReplyCallback)METHOD_ADDRESS( ProfileManager ,ServiceStopCallback ) );
}


STATUS  
ProfileManager::ServiceStopCallback( Message *pReply ){

	RqProfileStop		*pMsg = (RqProfileStop *)pReply;
	SsapiResponder		*pResponder = (SsapiResponder *)pReply->GetContext();

	if( pMsg->DetailedStatusCode == OK )
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	else
		pResponder->RespondToRequest(  SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );

	delete pMsg;
	return OK;
}

//************************************************************************
//
//************************************************************************

void  
ProfileManager::ServiceClear( ValueSet *pParms, SsapiResponder *pResponder ){

	TySlot		slot;

	pParms->GetInt( SSAPI_PROFILE_MANAGER_CLEAR_IOP_SLOT, (int *)&slot );

	Send(	slot,
			new RqProfileClear(),
			pResponder,
			(ReplyCallback)METHOD_ADDRESS( ProfileManager ,ServiceClearCallback ) );
}


STATUS  
ProfileManager::ServiceClearCallback( Message *pReply ){

	RqProfileClear		*pMsg = (RqProfileClear *)pReply;
	SsapiResponder		*pResponder = (SsapiResponder *)pReply->GetContext();

	if( pMsg->DetailedStatusCode == OK )
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	else
		pResponder->RespondToRequest(  SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );

	delete pMsg;
	return OK;
}

//************************************************************************
//
//************************************************************************

void  
ProfileManager::ServiceDeliver( ValueSet *pParms, SsapiResponder *pResponder ){
	TySlot		slot;
	U32			address;
	void		*pTemp = new char[1024*8];

	pParms->GetU32( SSAPI_PROFILE_MANAGER_DELIVER_START_ADDRESS, &address );
	pParms->GetInt( SSAPI_PROFILE_MANAGER_DELIVER_IOP_SLOT, (int *)&slot );

	Message	*pMsg = new RqProfileDeliver( address );
	pMsg->AddSgl( 0, pTemp, 1024*8, SGL_REPLY );

	Send(	slot,
			pMsg,
			pResponder,
			(ReplyCallback)METHOD_ADDRESS( ProfileManager ,ServiceDeliverCallback ) );

	delete []pTemp;
}


STATUS  
ProfileManager::ServiceDeliverCallback( Message *pReply ){

	RqProfileDeliver	*pMsg = (RqProfileDeliver *)pReply;
	SsapiResponder		*pResponder = (SsapiResponder *)pReply->GetContext();

	if( pMsg->DetailedStatusCode == OK ){
		ValueSet		*pRc = new ValueSet(), *pReturn = new ValueSet;
		void			*pBlob = NULL;
		U32				blobSize = 0;

		pMsg->GetSgl( 0, &pBlob, &blobSize );

		for( U32 i = 0; i < blobSize/4; i++ )
			*((U32 *)pBlob + i) = H_TO_N(*((U32 *)pBlob + i));

		pRc->AddInt( SSAPI_RC_SUCCESS, SSAPI_RETURN_STATUS );
		pReturn->AddValue( pRc, SSAPI_RETURN_STATUS_SET );
		pReturn->AddGenericValue( (char *)pBlob, blobSize, SSAPI_PROFILE_MANAGER_DELIVER_BLOB );

		pResponder->Respond( pReturn, true );

		delete pRc;
		delete pReturn;
	}
	else
		pResponder->RespondToRequest(  SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );

	pMsg->CleanAllSgl();
	delete pMsg;
	return OK;
}


//************************************************************************
//
//************************************************************************

void 
ProfileManager::ServiceHeapClear( ValueSet *pParms, SsapiResponder *pResponder ){

	TySlot		slot;

	pParms->GetInt( SSAPI_PROFILE_MANAGER_HEAP_CLEAR_IOP_SLOT, (int *)&slot );

	Send(	slot,
			new RqLeakClear(),
			pResponder,
			(ReplyCallback)METHOD_ADDRESS( ProfileManager ,ServiceHeapClearCallback ) );
}


STATUS 
ProfileManager::ServiceHeapClearCallback( Message *pReply ){

	RqLeakClear			*pMsg = (RqLeakClear *)pReply;
	SsapiResponder		*pResponder = (SsapiResponder *)pReply->GetContext();

	if( pMsg->DetailedStatusCode == OK )
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	else
		pResponder->RespondToRequest(  SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );


	delete pMsg;
	return OK;
}


//************************************************************************
//
//************************************************************************

void 
ProfileManager::ServiceHeapDeliver( ValueSet *pParms, SsapiResponder *pResponder ){

	TySlot		slot;
	U32			address;
	void		*pTemp = new char[1024*8];

	pParms->GetU32( SSAPI_PROGILE_MANAGER_HEAP_DELIVER_START_ADDRESS, &address );
	pParms->GetInt( SSAPI_PROGILE_MANAGER_HEAP_DELIVER_IOP_SLOT, (int *)&slot );

	Message	*pMsg = new RqLeakDeliver( address );
	pMsg->AddSgl( 0, pTemp, 1024*8, SGL_REPLY );

	Send(	slot,
			pMsg,
			pResponder,
			(ReplyCallback)METHOD_ADDRESS( ProfileManager ,ServiceHeapDeliverCallback ) );

	delete []pTemp;
}


STATUS 
ProfileManager::ServiceHeapDeliverCallback( Message *pReply ){

	RqLeakDeliver		*pMsg = (RqLeakDeliver *)pReply;
	SsapiResponder		*pResponder = (SsapiResponder *)pReply->GetContext();

	if( pMsg->DetailedStatusCode == OK ){
		ValueSet		*pRc = new ValueSet(), *pReturn = new ValueSet;
		void			*pBlob = NULL;
		U32				blobSize = 0;

		pMsg->GetSgl( 0, &pBlob, &blobSize );

		for( U32 i = 0; i < blobSize/4; i++ )
			*((U32 *)pBlob + i) = H_TO_N(*((U32 *)pBlob + i));

		pRc->AddInt( SSAPI_RC_SUCCESS, SSAPI_RETURN_STATUS );
		pReturn->AddValue( pRc, SSAPI_RETURN_STATUS_SET );
		pReturn->AddGenericValue( (char *)pBlob, blobSize, SSAPI_PROGILE_MANAGER_HEAP_DELIVER_BLOB );

		pResponder->Respond( pReturn, true );

		delete pRc;
		delete pReturn;
	}
	else
		pResponder->RespondToRequest(  SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );

	pMsg->CleanAllSgl();
	delete pMsg;
	return OK;
}
