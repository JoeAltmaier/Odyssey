//************************************************************************
// FILE:		LogManager.cpp
//
// PURPOSE:		Implements that class whose instance will be managing
//				message log objects and log metadata objects.
//************************************************************************

#include "LogManager.h"
#include "LogMasterMessages.h"
#include "FilterSet.h"
#include "IntVectorFilter.h"
#include "SsapiResponder.h"
#include "CtEvent.h"
#include "SlotMap.h"
#include "SSAPIAssert.h"
#include "ListenManager.h"

// managed objects:
#include "LogMessage.h"
#include "LogMetaData.h"

// trace facility hookup
#include "Trace_Index.h"
#ifdef TRACE_INDEX
#undef TRACE_INDEX
#endif
#define TRACE_INDEX TRACE_SSAPI_MANAGERS


#define MAX_OF(a,b) ( (a>b)? a : b )
#define MIN_OF(a,b) ( (a<b)? a : b )


LogManager* LogManager::m_pThis	= NULL;


//************************************************************************
// LogManager:
//
// PURPOSE:		Default constructor
//************************************************************************

LogManager::LogManager( ListenManager *pListenManager, DdmServices *pParent )
:ObjectManager( pListenManager, DesignatorId( RowId(), SSAPI_MANAGER_CLASS_TYPE_LOG_MANAGER), pParent){

	m_outstandingRequests	= 0;
	m_isInited				= false;
	SetIsReadyToServiceRequests( false );
	SSAPI_TRACE( TRACE_L2, "\nLogManager: Initializing..." );

	MsgQueryLogMetaData *pMsg = new MsgQueryLogMetaData(LM_LISTEN_ALL);
	Send(pMsg, REPLYCALLBACK(LogManager, GetLogMetaDataCallback ));

	MsgQueryLog *pMsg_ = new MsgQueryLog( 0, 0, true, LM_LISTEN_NEW );
	Send( pMsg_, NULL, REPLYCALLBACK(LogManager, QueryLogReplyCallback ));
}


//************************************************************************
// ListObjects:
// 
// PURPOSE:		Services a PAGED LIST OBJECTS request. 
//
// RECEIVE:		pParms:			set of parms 
//				pResponder:		ptr to a responder object
//************************************************************************

bool
LogManager::ListObjectsPaged( ValueSet *pParms, SsapiResponder *pResponder ){


	ValueSet			*pFilterSetValues;
	U32					fieldId, direction, pageSize, startValue;
	int					rc = 1;
	FilterSet			*pFS = NULL;
	MsgQueryLog			*pMsg;

	pFilterSetValues = (ValueSet *)pParms->GetValue( SSAPI_OBJECT_MANAGER_PAGED_LIST_FILTER_SET );
	rc &= pParms->GetInt( SSAPI_OBJECT_MANAGER_PAGED_LIST_FIELD_ID, (int *)&fieldId );
	rc &= pParms->GetU32( SSAPI_OBJECT_MANAGER_PAGED_LIST_START_VALUE, &startValue );
	rc &= pParms->GetInt( SSAPI_OBJECT_MANAGER_PAGED_LIST_DIRECTION, (int *)&direction ); // 0 descending
	rc &= pParms->GetInt( SSAPI_OBJECT_MANAGER_PAGED_LIST_PAGE_SIZE, (int *)&pageSize );

	if( !rc ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
		return true;
	}

	SSAPI_TRACE( TRACE_L2, "\nLogManager: got a query, start = ", startValue);
	SSAPI_TRACE( TRACE_L2, " pageSize = ", pageSize );
	SSAPI_TRACE( TRACE_L2, direction? " direction = UP\n" : " direction = DOWN\n" );

	pMsg = new MsgQueryLog( startValue, pageSize, direction? true : false );

	if( pFilterSetValues && pFilterSetValues->GetCount() ){
		pFS = new FilterSet( *pFilterSetValues );
		if( !ConvertFilterSet2LogQueryMsg( *pFS, *pMsg ) ){
			pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
			return true;
		}
		delete pFS;
	}

	Send( pMsg, pResponder, REPLYCALLBACK(LogManager,QueryLogReplyCallback) );

	return true;
}


//************************************************************************
// GetLogMetaDataCallback:
//
// PURPOSE:		Callback for GGET_LOG_META_DATA msg request
//************************************************************************

STATUS 
LogManager::GetLogMetaDataCallback( Message *pMsg ){

	LogMetaData		*pLogMetaData = new LogMetaData( GetListenManager() );
	CoolVector		container; 

	pLogMetaData->BuildYourSelf( (MsgQueryLogMetaData *)pMsg );
	container.Add( (CONTAINER_ELEMENT)pLogMetaData );
	AddObjectsIntoManagedObjectsVector( container );
	printf("\nLog should have %d entries", ((MsgQueryLogMetaData *)pMsg)->GetEntryCount());

	if( !m_isInited ){
		SSAPI_TRACE( TRACE_L2, "\nLogManager: Done!" );
		SetIsReadyToServiceRequests( true );
		m_isInited = true;
	}
	
	delete ((MsgQueryLogMetaData *)pMsg);

	return OK;
}



//************************************************************************
// ~LogManager:
//
// PURPOSE:		The destructor
//************************************************************************

LogManager::~LogManager(){
	
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
LogManager::Dispatch( ValueSet *pRequestParms, U32 requestCode, SsapiResponder *pResponder){


	return ObjectManager::Dispatch( pRequestParms, requestCode, pResponder ) ;
}


//************************************************************************
// ListObjects:
// 
// PURPOSE:		Services a LIST OBJECTS request. 
//
// RECEIVE:		filterSet:		set of filters for pipelining
//				pResponder:		ptr to a responder object
//************************************************************************

bool 
LogManager::ListObjects( ValueSet *pFilterSet, SsapiResponder *pResponder ){

	if( !pFilterSet->GetCount() )	// not supported for now --- too much shit to transfer
		pResponder->RespondToRequest( SSAPI_EXCEPTION_CAPABILITIES, CTS_SSAPI_INTERNAL_EXCEPTION_NOT_SUPPORTED );
	else
		ObjectManager::ListObjects( pFilterSet, pResponder );

	return true;
}


//************************************************************************
// ConvertFilterSet2LogQueryMsg:
//
// PURPOSE:		Converts a filter set into LogMessage format so
//				the LogMaster can work for us.
//
// RETURN:		true:	successfully converted
//				false:	abort!
//************************************************************************

bool 
LogManager::ConvertFilterSet2LogQueryMsg( FilterSet& filterSet, MsgQueryLog& msg ){

	CoolVector		container;
	U32				position, u32;
	bool			passed;
	CoolVector		severityRange, facilityRange, slotRange;
	ValueSet		*pVs = new ValueSet;
	int				min, max, possibleValue, filterNum;
	U16				*pArr, u16;
	TySlot			slots[128];

	// first -- the severity
	min = Event::GetMinSeverity(); 	max = Event::GetMaxSeverity();
	CollectIntVectorFiltersForField( container, filterSet, SSAPI_LOG_MESSAGE_FID_SEVERITY );
	for( possibleValue = min; possibleValue <= max; possibleValue++ ){
		pVs->AddInt( possibleValue, SSAPI_LOG_MESSAGE_FID_SEVERITY );
		for( filterNum = 0, passed = true; filterNum < container.Count(); filterNum++ ){
			container.GetAt( (CONTAINER_ELEMENT &)position, filterNum );
			IntVectorFilter &filter = (IntVectorFilter &) filterSet.GetFilterAt( position );
			if( filter.DoesApply( *pVs ) != Filter::APPLIES )
				passed = false;
		}
		if( passed && container.Count() )
			severityRange.Add( (CONTAINER_ELEMENT)possibleValue);
	}
	//copy
	if( severityRange.Count() ){
		pArr = new U16[ severityRange.Count() ];
		for( position = 0; position < severityRange.Count(); position++ ){
			severityRange.GetAt( (CONTAINER_ELEMENT &)u32, position );
			u16 = u32;
			memcpy( pArr + position, &u16, sizeof( u16 ) );
		}
		msg.QuerySeverity( severityRange.Count(), pArr );
		delete [] pArr;
		severityRange.RemoveAll();
	}
	pVs->Clear();

	// second -- the facility
	min = Event::GetMinFacility(); 	max = Event::GetMaxFacility();
	CollectIntVectorFiltersForField( container, filterSet, SSAPI_LOG_MESSAGE_FID_FACILITY );
	for( possibleValue = min; possibleValue <= max; possibleValue++ ){
		pVs->AddInt( possibleValue, SSAPI_LOG_MESSAGE_FID_FACILITY );
		for( filterNum = 0, passed = true; filterNum < container.Count(); filterNum++ ){
			container.GetAt( (CONTAINER_ELEMENT &)position, filterNum );
			IntVectorFilter &filter = (IntVectorFilter &) filterSet.GetFilterAt( position );
			if( filter.DoesApply( *pVs ) != Filter::APPLIES )
				passed = false;
		}
		if( passed && container.Count() )
			facilityRange.Add( (CONTAINER_ELEMENT)possibleValue);
	}
	if( facilityRange.Count() ){
		pArr = new U16[ facilityRange.Count() ];
		for( position = 0; position < facilityRange.Count(); position++ ){
			facilityRange.GetAt( (CONTAINER_ELEMENT &)u32, position );
			u16 = u32;
			memcpy( pArr + position, &u16, sizeof( u16 ) );
		}
		msg.QueryFacility( facilityRange.Count(), pArr );
		delete [] pArr;
		facilityRange.RemoveAll();
	}
	pVs->Clear();

	// third -- the slot
	min = SlotMap::GetMinSlotNumber(); 	max = SlotMap::GetMaxSlotNumber();
	CollectIntVectorFiltersForField( container, filterSet, SSAPI_LOG_MESSAGE_FID_SLOT );
	for( possibleValue = min; possibleValue <= max; possibleValue++ ){
		pVs->AddInt( possibleValue, SSAPI_LOG_MESSAGE_FID_SLOT );
		for( filterNum = 0, passed = true; filterNum < container.Count(); filterNum++ ){
			container.GetAt( (CONTAINER_ELEMENT &)position, filterNum );
			IntVectorFilter &filter = (IntVectorFilter &) filterSet.GetFilterAt( position );
			if( filter.DoesApply( *pVs ) != Filter::APPLIES )
				passed = false;
		}
		if( passed && container.Count() )
			slotRange.Add( (CONTAINER_ELEMENT)possibleValue);
	}
	if( slotRange.Count() ){
		for( position = 0; position < slotRange.Count(); position++ )
			slotRange.GetAt( (CONTAINER_ELEMENT &)slots[position], position );
		msg.QuerySlot( slotRange.Count(), slots );
		slotRange.RemoveAll();
	}
	pVs->Clear();

	// the last -- the timestamp
	CollectIntVectorFiltersForField( container, filterSet, SSAPI_LOG_MESSAGE_FID_TIME_STAMP );
	if( container.Count() ){
		if( container.Count() == 1 ){ // get specific date
			container.GetAt( (CONTAINER_ELEMENT &)position, 0 );
			IntVectorFilter &filter = (IntVectorFilter &) filterSet.GetFilterAt( position );
			
			msg.QueryTimestamp( filter.GetNextApplicableNumber() );
		}
		else if( container.Count() == 2 ){ // get the range
			I64		min64, max64;

			container.GetAt( (CONTAINER_ELEMENT &)position, 0 );
			IntVectorFilter &filter1 = (IntVectorFilter &) filterSet.GetFilterAt( position );
			container.GetAt( (CONTAINER_ELEMENT &)position, 1 );
			IntVectorFilter &filter2 = (IntVectorFilter &) filterSet.GetFilterAt( position );
			max64 = MAX_OF( filter1.GetNextApplicableNumber(), filter2.GetNextApplicableNumber() );
			min64 = MIN_OF( filter1.GetNextApplicableNumber(), filter2.GetNextApplicableNumber() );

			msg.QueryTimestamp( min64, max64 );
		}
		else
			ASSERT(0);
	}



	delete pVs;

	return true;
}


//************************************************************************
// CollectIntVectorFiltersForField:
//
// PURPOSE;		Searches the filter set for int vector filters that are
//				to filter based on the field specified by the 'fieldId'.
//				If found, filter positions are put into the 'container'
//************************************************************************

void 
LogManager::CollectIntVectorFiltersForField( Container &container, FilterSet &filterSet, int fieldId ){

	U32				i;

	container.RemoveAll();

	for( i = 0; i < filterSet.GetFilterCount(); i++ ){
		Filter &filter = (Filter &)filterSet.GetFilterAt( i );
		if( filter.GetClassType() == SSAPI_FILTER_CLASS_TYPE_INT_VECTOR ){
			IntVectorFilter &intFilter = (IntVectorFilter &)filter;
			if( intFilter.GetFieldId() == fieldId )
				container.AddAt( (CONTAINER_ELEMENT)i, container.Count() );
		}
	}
}



//************************************************************************
// QueryLogReplyCallback:
//
// PURPOSE:		Receives reply from the logmaster. Responsible  for replying
//				to the customer.
//************************************************************************

STATUS 
LogManager::QueryLogReplyCallback( Message *pMsg_ ){

	SsapiResponder	*pResponder = (SsapiResponder *)pMsg_->GetContext();
	MsgQueryLog		*pMsg		= (MsgQueryLog *)pMsg_;
	Event			*pEvents	= NULL;
	U16				count		= pMsg->GetLogEntries( &pEvents), i;
	LogMessage		*pObj;	
	ValueSet		*pObjects = new ValueSet(), *pRc = new ValueSet(), *pResults = new ValueSet();


	if( pResponder ) { // it's a query reply
		for( i = 0; i < count; i++ ){
			pObj = new LogMessage( GetListenManager(), &(pEvents[i]) );
			pObj->BuildYourValueSet();
			pObjects->AddValue( pObj, i );
			delete pObj;
		}

		pRc->AddInt( SSAPI_RC_SUCCESS, SSAPI_RETURN_STATUS );
		pResults->AddValue( pRc, SSAPI_RETURN_STATUS_SET );
		pResults->AddValue( pObjects, SSAPI_OBJECT_MANAGER_PAGED_LIST_OBJECT_VECTOR );

	
		pResponder->Respond( pResults, TRUE );
	}
	else{	// it's a listen
		for( i = 0; i < count; i++ ){
			pObj = new LogMessage( GetListenManager(), &(pEvents[i]) );
			pObj->BuildYourValueSet();
			GetListenManager()->PropagateObjectAddedEvent( GetDesignatorId(), pObj );
			delete pObj;
		}
	}

	delete pObjects;
	delete pRc;
	delete [] pEvents;
	delete pResults;
	
	//if( pMsg->IsLast() )
		delete pMsg;

	return OK;
}



