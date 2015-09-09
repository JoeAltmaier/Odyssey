//************************************************************************
// FILE:		LogManager.h
//
// PURPOSE:		Defines that class whose instance will be managing
//				message log objects and log metadata objects.
//************************************************************************

#ifndef __LOG_MANAGER_H__
#define __LOG_MANAGER_H__


#include "ObjectManager.h"
class FilterSet;
class MsgQueryLog;

#define	LOG_MANAGER_NAME		"LogManager"

#ifdef WIN32
#pragma pack(4)
#endif

class LogManager : public ObjectManager {
	
	U32					m_outstandingRequests;
	bool				m_isInited;
	static LogManager	*m_pThis;

//************************************************************************
// LogManager:
//
// PURPOSE:		Default constructor
//************************************************************************

LogManager( ListenManager *pListenManager, DdmServices *pParent );


public:


//************************************************************************
// ~LogManager:
//
// PURPOSE:		The destructor
//************************************************************************

~LogManager();


//************************************************************************
// GetName:
//
// PURPOSE:		Returns the name of the manager
//************************************************************************

virtual StringClass GetName() { return StringClass(LOG_MANAGER_NAME); }


//************************************************************************
// Ctor:
//
// PURPOSE:		Creates the manager
//************************************************************************

static ObjectManager* Ctor(	ListenManager			*pLManager, 
							DdmServices				*pParent, 
							StringResourceManager	*pSRManager ){

	return m_pThis? m_pThis : m_pThis = new LogManager( pLManager, pParent );
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

virtual bool Dispatch( ValueSet *pRequestParms, U32 requestCode, SsapiResponder *pResponder);


protected:

//************************************************************************
// ListObjects:
// 
// PURPOSE:		Services a LIST OBJECTS request. 
//
// RECEIVE:		filterSet:		set of filters for pipelining
//				pResponder:		ptr to a responder object
//************************************************************************

virtual bool ListObjects( ValueSet *pFilterSet, SsapiResponder *pResponder );


//************************************************************************
// ListObjects:
// 
// PURPOSE:		Services a PAGED LIST OBJECTS request. 
//
// RECEIVE:		pParms:			set of parms 
//				pResponder:		ptr to a responder object
//************************************************************************

virtual bool ListObjectsPaged( ValueSet *pParms, SsapiResponder *pResponder );


//************************************************************************
// ObjectDeletedCallbackHandler:
//
// PURPOSE:		This method may be provided by subclasses to be notified
//				by events from other object managers. 
//************************************************************************

virtual void ObjectDeletedCallbackHandler( SSAPIEvent *pEvent , bool isLast ) {}


private:

//************************************************************************
//  Outstanding requests management crap
//************************************************************************
void AddOutstandingRequest(){ m_outstandingRequests++; SetIsReadyToServiceRequests(false); }
void RemoveOutstandingRequest(){ if( !--m_outstandingRequests ) SetIsReadyToServiceRequests(true);}


//************************************************************************
// ConvertFilterSet2LogQueryMsg:
//
// PURPOSE:		Converts a filter set into LogMessage format so
//				the LogMaster can work for us.
//
// RETURN:		true:	successfully converted
//				false:	abort!
//************************************************************************

bool ConvertFilterSet2LogQueryMsg( FilterSet& filterSet, MsgQueryLog& msg );


//************************************************************************
// QueryLogReplyCallback:
//
// PURPOSE:		Receives reply from the logmaster. Responsible  for replying
//				to the customer.
//************************************************************************

STATUS QueryLogReplyCallback( Message *pMsg );


//************************************************************************
// CollectIntVectorFiltersForField:
//
// PURPOSE:		Searches the filter set for int vector filters that are
//				to filter based on the field specified by the 'fieldId'.
//				If found, filters are put into the 'container'
//************************************************************************

void CollectIntVectorFiltersForField( Container &container, FilterSet &filterSet, int fieldId );


//************************************************************************
// GetLogMetaDataCallback:
//
// PURPOSE:		Callback for GGET_LOG_META_DATA msg request
//************************************************************************

STATUS GetLogMetaDataCallback( Message *pMsg );


//************************************************************************
//************************************************************************
//************************************************************************
//************************************************************************
};
#endif // __LOG_MANAGER_H__