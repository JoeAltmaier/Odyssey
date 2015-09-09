//************************************************************************
// FILE:		AlarmManager.cpp
//
// PURPOSE:		Implements the manager that manages SSAPI alarm objects
//************************************************************************

#include "AlarmManager.h"
#include "AlarmCmdQueue.h"
#include "AlarmEvents.h"
#include "SSAPITypes.h"
#include "AlarmMasterMessages.h"
#include "AlarmLogTable.h"
#include "AlarmRecordTable.h"
#include "Alarm.h"
#include "AlarmEvents.h"
#include "SList.h"
#include "SsapiLocalResponder.h"
#include "SsapiEvents.h"
#include "UserManager.h"
#include "DdmSSAPI.h"

#include "Trace_Index.h"
#ifdef TRACE_INDEX
#undef TRACE_INDEX
#endif
#define TRACE_INDEX TRACE_SSAPI_MANAGERS


AlarmManager* AlarmManager::m_pThis = NULL;

//************************************************************************
// AlarmManager:
//
// PURPOSE:		Default constructor
//************************************************************************

AlarmManager::AlarmManager( ListenManager *pListenManager, DdmServices *pParent )
:ObjectManager( pListenManager, DesignatorId(RowId(), SSAPI_MANAGER_CLASS_TYPE_ALARM_MANAGER), pParent ){

	m_isInited	= false;
	SetIsReadyToServiceRequests( false );
	SSAPI_TRACE( TRACE_L2, "\nAlarmManager: Initializing..." );


	// init the cmd snder for the VCM
	m_pCmdSender	= new CmdSender(AMSTR_CMD_QUEUE_TABLE,
									sizeof(AMSTR_CMND_INFO),
									sizeof(AMSTR_EVENT_INFO),
									this);

	m_pCmdSender->csndrInitialize((pInitializeCallback_t)METHOD_ADDRESS(AlarmManager,InitAMCommandQueueCallback));
	m_pCmdSender->csndrRegisterForEvents((pEventCallback_t)METHOD_ADDRESS(AlarmManager,AlarmEventHandler));

	// get the alarms!
	MsgQueryAlarms	*pMsg = new MsgQueryAlarms( ALL_ALARMS );
	Send( pMsg, NULL, REPLYCALLBACK(AlarmManager, QueryAMReplyCallback ) );
}




//************************************************************************
// ~AlarmManager:
//
// PURPOSE:		Default destructor
//************************************************************************

AlarmManager::~AlarmManager(){

	delete m_pCmdSender;
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
AlarmManager::Dispatch( ValueSet *pRequestParms, U32 requestCode, SsapiResponder *pResponder){

	DesignatorId		alarmId;
	UnicodeString		us;
	Alarm				*pAlarm;

	if( ObjectManager::Dispatch( pRequestParms, requestCode, pResponder ) )
		return true;

	switch(requestCode){

		case SSAPI_ALARM_MANAGER_ACKNOWLEDGE_ALARM:
			pRequestParms->GetGenericValue( (char *)&alarmId, sizeof(alarmId), SSAPI_ALARM_MANAGER_ACKNOWLEDGE_ALARM_ALARM_ID );
			pRequestParms->GetString( SSAPI_ALARM_MANAGER_ACKNOWLEDGE_ALARM_COMMENTS, &us );
			SendCommandToAM( AMSTR_CMND_ACKNOWLEDGE_ALARM, alarmId, us, pResponder );
			break;

		case SSAPI_ALARM_MANAGER_UNACKNOWLEDGE_ALARM:
			pRequestParms->GetGenericValue( (char *)&alarmId, sizeof(alarmId), SSAPI_ALARM_MANAGER_UNACKNOWLEDGE_ALARM_ALARM_ID );
			pRequestParms->GetString( SSAPI_ALARM_MANAGER_UNACKNOWLEDGE_ALARM_COMMENTS, &us );
			SendCommandToAM( AMSTR_CMND_UNACKNOWLEDGE_ALARM, alarmId, us, pResponder );
			break;

		case SSAPI_ALARM_MANAGER_CLEAR_ALARM:
			pRequestParms->GetGenericValue( (char *)&alarmId, sizeof(alarmId), SSAPI_ALARM_MANAGER_CLEAR_ALARM_ALARM_ID );
			pRequestParms->GetString( SSAPI_ALARM_MANAGER_CLEAR_ALARM_COMMENTS, &us );
			pAlarm = (Alarm *)GetManagedObject( &alarmId );
			ASSERT( pAlarm );
			if( pAlarm )
				if( !pAlarm->GetIsClearable() )
					pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_EXCEPTION_ALARM_NOT_CLEARABLE );

			SendCommandToAM( AMSTR_CMND_REMIT_ALARM_FROM_USER, alarmId, us, pResponder );
			break;

		case SSAPI_ALARM_MANAGER_ADD_NOTES:
			pRequestParms->GetGenericValue( (char *)&alarmId, sizeof(alarmId), SSAPI_ALARM_MANAGER_ADD_NOTES_ALARM_ID );
			pRequestParms->GetString( SSAPI_ALARM_MANAGER_ADD_NOTES_NOTES, &us );
			SendCommandToAM( AMSTR_CMND_NOTIFY_ALARM, alarmId, us, pResponder );
			break;

		default:
			ASSERT(0);
			break;
	}

	return true;
}


//************************************************************************
// ObjectDeletedCallbackHandler:
//
// PURPOSE:		This method may be provided by subclasses to be notified
//				by events from other object managers. 
//************************************************************************

void 
AlarmManager::ObjectDeletedCallbackHandler( SSAPIEvent *pEvent , bool isLast ){

	ValueSet				*pObjVs = new ValueSet;
	int						classType;
	DesignatorId			id;
	Alarm					*pAlarm;
	U32						index;
	SList					container;

	*pObjVs	= *(ValueSet *)pEvent->GetValue( SSAPI_EVENT_FID_MANAGED_OBJECT );
	pObjVs->GetGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID );
	pObjVs->GetInt( SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &classType );

	// see if we have alarms whose source is this object
	if( classType != SSAPI_OBJECT_CLASS_TYPE_ALARM ){ // to avoid recursion
		for( index = 0; index < GetManagedObjectCount(); index++ ){
			pAlarm = (Alarm *)GetManagedObject( index );
			if( id == pAlarm->GetSourceId() )
				container.Add( (CONTAINER_ELEMENT)pAlarm ); // collect for deletion
		}
	}

	// purge alarms whose source has been deleted from the system
	for( index = 0; index < container.Count(); index++ ){
		container.GetAt( (CONTAINER_ELEMENT &)pAlarm, index );

		SendCommandToAM(	AMSTR_CMND_KILL_ALARM, 
							pAlarm->GetDesignatorId(),
							UnicodeString(StringClass("")),
							new SsapiLocalResponder( this, (LOCAL_EVENT_CALLBACK)METHOD_ADDRESS(AlarmManager, LocalResponderDummyCallback) ) );
	}
	if( container.Count() )
		DeleteObjectsFromTheSystem( container );

	delete pObjVs;
}



//************************************************************************
// QueryAMReplyCallback:
//
// PURPOSE:		Callback for a alarm query. We need to check for 
//				the context. If it's not null, it's the ptr to RowID of the
//				new/modified alarm which we need to add/modify
//************************************************************************

STATUS 
AlarmManager::QueryAMReplyCallback( Message *pMsg_ ){

	MsgQueryAlarms	*pMsg = (MsgQueryAlarms *)pMsg_;
	RowId			*pRowId = (RowId *)pMsg->GetContext();
	AlarmRecord		*pAlarmRecord = NULL;
	AlarmLogRecord	*pAlarmLog = NULL;
	U32				alarm;
	Alarm			*pAlarm;
	SList			container;


	if( pRowId ){	// new one or modified one
		pMsg->GetAlarms( (void **) &pAlarmRecord );
		pMsg->GetAlarmHistory( (void **) &pAlarmLog );

		for( alarm = 0; alarm < pMsg->GetNumberOfAlarms(); alarm++ ){
			if( *pRowId == (pAlarmRecord + alarm)->rid ){
				pAlarm = new Alarm( GetListenManager(), (DdmSSAPI *)pParentDdmSvs );
				pAlarm->BuildYourselfFromPtsRow( pAlarmRecord + alarm, pAlarmLog, pMsg->GetNumberOfAlarmLogEntries() );
				if( pAlarm->GetIsSourceFound() )
					container.Add( (CONTAINER_ELEMENT) pAlarm );
				else
					SendCommandToAM(AMSTR_CMND_KILL_ALARM, 
									pAlarm->GetDesignatorId(),
									UnicodeString(StringClass("")),
									new SsapiLocalResponder( this, (LOCAL_EVENT_CALLBACK)METHOD_ADDRESS(AlarmManager, LocalResponderDummyCallback) ) );
				break;
			}
		}
		if( container.Count() )
			AddObjectsIntoManagedObjectsVector( container );
	}
	else {			// first time ever
		pMsg->GetAlarms( (void **) &pAlarmRecord );
		pMsg->GetAlarmHistory( (void **) &pAlarmLog );

		for( alarm = 0; alarm < pMsg->GetNumberOfAlarms(); alarm++ ){
			pAlarm = new Alarm( GetListenManager(), (DdmSSAPI *)pParentDdmSvs );
			pAlarm->BuildYourselfFromPtsRow( pAlarmRecord + alarm, pAlarmLog, pMsg->GetNumberOfAlarmLogEntries() );
			if( pAlarm->GetIsSourceFound() )
				container.Add( (CONTAINER_ELEMENT) pAlarm );
			else
				SendCommandToAM(AMSTR_CMND_KILL_ALARM, 
								pAlarm->GetDesignatorId(),
								UnicodeString(StringClass("")),
								new SsapiLocalResponder( this, (LOCAL_EVENT_CALLBACK)METHOD_ADDRESS(AlarmManager, LocalResponderDummyCallback) ) );
		}
		if( container.Count() )
			AddObjectsIntoManagedObjectsVector( container );

		// Put it after we have inited
		SSAPI_TRACE( TRACE_L2, "\nAlarmManager:...Done! Objects built: ", GetManagedObjectCount() );
		m_isInited = true;
		SetIsReadyToServiceRequests( true );
	}

	delete pAlarmRecord;
	delete pAlarmLog;
	delete pRowId;
	delete pMsg;
	return OK;
}


//************************************************************************
// AlarmEventHandler:
//
// PURPOSE:		Receives and handles events from the alarm master
//
// FUNCTIONALITY:	For all events but KILLED, we need to query the 
//					alarm master.
//************************************************************************

void 
AlarmManager::AlarmEventHandler( STATUS eventCode, void *pEventData ){

	MsgQueryAlarms		*pMsg;
	RowId				*pRid;
	DesignatorId		id;
	AMSTR_EVENT_INFO	*pEvent = (AMSTR_EVENT_INFO *)pEventData;
	ManagedObject		*pObj;
	SList				container;
	

	switch(eventCode){

		case AMSTR_EVT_ALARM_SUBMITTED:
			pRid = new RowId( pEvent->alarmSubmitted.rid );
			break;

		case AMSTR_EVT_ALARM_REMITTED:
			pRid = new RowId( pEvent->alarmRemitted.rid );
			break;

		case AMSTR_EVT_ALARM_ACKNOWLEDGED:
			pRid = new RowId( pEvent->alarmAcknowledged.rid );
			break;

		case AMSTR_EVT_ALARM_UNACKNOWLEDGED:
			pRid = new RowId( pEvent->alarmUnacknowledged.rid );
			break;

		case AMSTR_EVT_ALARM_NOTIFIED:
			pRid = new RowId( pEvent->alarmNotified.rid );
			break;

		case AMSTR_EVT_ALARM_KILLED:
			if( GetDesignatorIdByRowId( pEvent->alarmKilled.rid, id ) ){
				pObj = GetManagedObject( &id );
				if( pObj ){
					container.Add( (CONTAINER_ELEMENT) pObj );
					DeleteObjectsFromTheSystem( container );
				}
			}
			return;
			break;

		default:
			ASSERT(0);
			break;
	}
	
	// send a query 
	pMsg = new MsgQueryAlarms( ALL_ALARMS );
	Send( pMsg, pRid, REPLYCALLBACK(AlarmManager, QueryAMReplyCallback ) );
}


//************************************************************************
// SendCommandToAM:
//
// PURPOSE:		Builds and sends a command to the Alarm Master.
//				Always responds.
//************************************************************************

void 
AlarmManager::SendCommandToAM( U32 command, DesignatorId alarmId, UnicodeString &us, SsapiResponder *pResponder ){

	AMSTR_CMND_INFO		cmdBuff;
	UserManager			*pUManager = (UserManager *)((DdmSSAPI *)pParentDdmSvs)->GetObjectManager(SSAPI_MANAGER_CLASS_TYPE_USER_MANAGER);


	if( !GetManagedObject( &alarmId ) )
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_OBJECT_DOES_NOT_EXIST_EXCEPTION );

	memset( &cmdBuff, 0, sizeof(cmdBuff) );

	cmdBuff.opcode	= command;
	cmdBuff.cmdParams.rid = alarmId.GetRowId().GetRowID();
	us.CString( &cmdBuff.cmdParams.notes, sizeof(cmdBuff.cmdParams.notes) );
	pUManager->GetUserNameBySessionId(pResponder->GetSessionID()).CString( &cmdBuff.cmdParams.userName, sizeof(cmdBuff.cmdParams.userName) );
	
	m_pCmdSender->csndrExecute(	&cmdBuff,
								(pCmdCompletionCallback_t)METHOD_ADDRESS(AlarmManager,SendCommandToAMCallback),
								pResponder );
	
}


void 
AlarmManager::SendCommandToAMCallback(	STATUS			completionCode,
										void			*pResultData,
										void			*pCmdData,
										void			*pCmdContext ){

	SsapiResponder		*pResponder = (SsapiResponder *)pCmdContext;

	if( completionCode == OK )
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	else{
		ASSERT(0);
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );
	}
}
