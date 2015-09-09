//DdmSSAPI.cpp

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include <stdio.h>
#include <String.h>
#include "OsTypes.h"
#include "BuildSys.h"
#include "DdmSSAPI.h"
#include "CoolVector.h"
#include "SsapiAlarms.h"

#include "..\msl\OsHeap.h"


CLASSNAME(DdmSSAPI,SINGLE);  //Class link name used by buildsys
SERVELOCAL(DdmSSAPI, REQ_SSAPI_REQ);

//************************************************************************
// SSAPI MANAGER LAUNCH DETAILS:
//
// IDEA:		The intent is to have different types of manager launch
//				programs. Ideologically speaking, there are 3 distinct 
//				types of SSAPI managers:
//				1.	Always present, do not require a full system initialization
//					to run (do not depend on other managers being ready)
//				2.  Always present. Require that all managers be initialized
//				3.	Launched only on the first request to them. Can only be
//					launched after all managers of type 2. have initialized
//
// IMPLEMENTATION:
//				We use 2 launch tables: SYNC and ASYNC. The SYNC table is
//				used to accomodate managers of types 1. and 2. while the
//				ASYNC table is populated with type 3. managers.
//				All managers of type 1. must have their 'isFlexible' flag
//				equal to TRUE and must be positioned at the *beginning* of
//				the SYNC table. 
//************************************************************************

// The SYNC manager launch table
SSAPI_MANAGER_LAUNCH_CELL	SsapiManagerLaunchSyncTable[] = {
	{ (DdmSSAPI::SSAPI_MANAGER_CTOR) ConfigIdManager::Ctor,		SSAPI_MANAGER_CLASS_TYPE_CONFIG_ID_MANAGER,		TRUE },
	{ (DdmSSAPI::SSAPI_MANAGER_CTOR) LogManager::Ctor,			SSAPI_MANAGER_CLASS_TYPE_LOG_MANAGER,			TRUE },
	{ (DdmSSAPI::SSAPI_MANAGER_CTOR) UserManager::Ctor,			SSAPI_MANAGER_CLASS_TYPE_USER_MANAGER,			TRUE },
	{ (DdmSSAPI::SSAPI_MANAGER_CTOR) PHSDataManager::Ctor,		SSAPI_MANAGER_CLASS_TYPE_PHS_DATA_MANAGER,		FALSE },
	{ (DdmSSAPI::SSAPI_MANAGER_CTOR) DeviceManager::Ctor,		SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER,		FALSE },
	{ (DdmSSAPI::SSAPI_MANAGER_CTOR) StorageManager::Ctor,		SSAPI_MANAGER_CLASS_TYPE_STORAGE_MANAGER,		FALSE },
	{ (DdmSSAPI::SSAPI_MANAGER_CTOR) ProcessManager::Ctor,		SSAPI_MANAGER_CLASS_TYPE_PROCESS_MANAGER,		FALSE },
	{ (DdmSSAPI::SSAPI_MANAGER_CTOR) ConnectionManager::Ctor,	SSAPI_MANAGER_CLASS_TYPE_CONNECTION_MANAGER,	FALSE }, 
	{ (DdmSSAPI::SSAPI_MANAGER_CTOR) HostManager::Ctor,			SSAPI_MANAGER_CLASS_TYPE_HOST_MANAGER,			FALSE },
	{ (DdmSSAPI::SSAPI_MANAGER_CTOR) LunMapManager::Ctor,		SSAPI_MANAGER_CLASS_TYPE_LUN_MANAGER,			FALSE },
	{ (DdmSSAPI::SSAPI_MANAGER_CTOR) AlarmManager::Ctor,		SSAPI_MANAGER_CLASS_TYPE_ALARM_MANAGER,			FALSE },
	{ (DdmSSAPI::SSAPI_MANAGER_CTOR) SoftwareImageManager::Ctor,SSAPI_MANAGER_CLASS_TYPE_SOFTWARE_UPDATE_MANAGER, FALSE }
};

// The ASYNC manager launch table
SSAPI_MANAGER_LAUNCH_CELL	SsapiManagerLaunchASyncTable[] = {
	{ (DdmSSAPI::SSAPI_MANAGER_CTOR) TableStuffManager::Ctor,	SSAPI_MANAGER_TYPE_TABLE_MANAGER,		FALSE },
	{ (DdmSSAPI::SSAPI_MANAGER_CTOR) ProfileManager::Ctor,		SSAPI_MANAGER_CLASS_TYPE_PROFILE_MANAGER,FALSE }
};

#define	MANAGER_SYNC_TABLE_SIZE		sizeof(SsapiManagerLaunchSyncTable)/sizeof(SsapiManagerLaunchSyncTable[0])
#define	MANAGER_ASYNC_TABLE_SIZE	sizeof(SsapiManagerLaunchASyncTable)/sizeof(SsapiManagerLaunchASyncTable[0])








DdmSSAPI::DdmSSAPI(DID did) : DdmMaster(did) {
	m_pListenManager	= NULL;
	m_pNotReadyManagers = new CoolVector;
	m_pRequestQueue		= new CoolVector;
	m_isInited			= false;
}

Ddm* DdmSSAPI::Ctor(DID did) {
	return new DdmSSAPI(did);
}

STATUS DdmSSAPI::Initialize(Message* pMsg) {
	DispatchRequest(REQ_SSAPI_REQ, REPLYCALLBACK( DdmSSAPI, ProcessRequest) );

#ifdef WIN32
	Sleep( 5000 );
#endif

	m_pListenManager = new ListenManager();
	m_pStringResourceManager = new StringResourceManager( this, (pTSCallback_t)METHOD_ADDRESS(DdmSSAPI, StringResourceManagerInitCallback) );

	m_apObjectTable = new ObjectTableElement*[ MANAGER_SYNC_TABLE_SIZE + MANAGER_ASYNC_TABLE_SIZE ];
	m_iObjectTableSize = MANAGER_SYNC_TABLE_SIZE;	
	
	for( U32 index = 0; index < m_iObjectTableSize; index++ )
		m_apObjectTable[index] = new ObjectTableElement();
	

	// start the chain reaction of the initialization process
	m_apObjectTable[0]->m_pObjectManager = SsapiManagerLaunchSyncTable[0].pCtor( m_pListenManager, this, m_pStringResourceManager );
	m_apObjectTable[0]->m_iObjectRequestCode = m_apObjectTable[0]->m_pObjectManager->GetDesignatorId().GetClassId();

	Reply(pMsg,OK);
	m_pSsapiGateway = new SsapiGateway( this );
	return OK;
}

STATUS DdmSSAPI::Enable(Message* pMsg) {
	Reply(pMsg,OK);
	return OK;
}
	

//************************************************************************
// ProcessRequest:
//
// PURPOSE:		Responsible for the following tasks:
//				1.	Starting ssapi managers of type 3.
//				2.	Queueing up requested to managers who are not ready
//					to process them
//				3.	Making the intellegent decision if a given manager can
//					service a request based on the manager's launch type.
//************************************************************************

STATUS DdmSSAPI::ProcessRequest(Message* pReqMsg) {
	SsapiRequestMessage			*pMsg		= (SsapiRequestMessage*) pReqMsg;
	bool						found		= false;		
	int							i;
	SSAPI_MANAGER_LAUNCH_CELL	*pCell;
 
	// check if this is for manager type 1.
	for(i = 0, pCell = SsapiManagerLaunchSyncTable;
		( i < MANAGER_SYNC_TABLE_SIZE ) && ( pCell->isFlexible ) && !m_isInited;
		i++, pCell++) {

		if( (pCell->classType == pMsg->m_iObjectCode) && m_apObjectTable[i]->m_pObjectManager ){
			if( m_apObjectTable[i]->m_pObjectManager->IsReadyToServiceRequests() ){
				SsapiResponder		*pResponder	= new SsapiResponder(this, pMsg);
				// go thru the gateway....
				if( !m_pSsapiGateway->ShouldLetThisRequestThru( pMsg->m_iRequestCode, pMsg->m_iObjectCode, pResponder ) )
					return OK;
				m_apObjectTable[i]->m_pObjectManager->Dispatch(pMsg->m_pValueSet,pMsg->m_iRequestCode,pResponder);
				return OK;
			}
		}
	}

	// if this is for manager type 2. or 3. -> queue up for later execution 
	if( !m_isInited || m_pNotReadyManagers->Count() ){ 
		m_pRequestQueue->AddAt( (CONTAINER_ELEMENT)pReqMsg, m_pRequestQueue->Count() );
	}
	else {
		//dispatch -> find object by object code
		for( i=0;i<m_iObjectTableSize;i++) {
			if(m_apObjectTable[i]->m_iObjectRequestCode == pMsg->m_iObjectCode) {
				SsapiResponder		*pResponder	= new SsapiResponder(this, pMsg);

				found = true;
				// go thru the gateway....
				if( !m_pSsapiGateway->ShouldLetThisRequestThru( pMsg->m_iRequestCode, pMsg->m_iObjectCode, pResponder ) )
					break;
				m_apObjectTable[i]->m_pObjectManager->Dispatch(pMsg->m_pValueSet,pMsg->m_iRequestCode,pResponder);
				break;
			}
		}

		if( !found ) {
			// check if for manager type 3. --> instantiate the sucker!
			for( i = 0; i < MANAGER_ASYNC_TABLE_SIZE; i++ ){
				if( pMsg->m_iObjectCode == SsapiManagerLaunchASyncTable[i].classType ){
					printf("\n_______________________________________________________");
					printf("\n SSAPI got a request to an uninitialized manager ");
					printf("\n Performing friendly 'On-Demand' deployment....");
					printf("\n_______________________________________________________");
					m_apObjectTable[m_iObjectTableSize] = new ObjectTableElement();
					m_apObjectTable[m_iObjectTableSize]->m_pObjectManager = SsapiManagerLaunchASyncTable[i].pCtor( m_pListenManager, this, m_pStringResourceManager );
					m_apObjectTable[m_iObjectTableSize]->m_iObjectRequestCode = m_apObjectTable[m_iObjectTableSize]->m_pObjectManager->GetDesignatorId().GetClassId();
					m_iObjectTableSize++;
					// now recurse to process the request!!!
					return ProcessRequest(pReqMsg);
				}
			}

			if(pMsg->m_iObjectCode == SSAPI_SESSION_TERMINATED) {
				HandleSessionExpiredEvent(pMsg->m_iSessionID);
				Reply(pMsg, OK);
			}
			else {
				SsapiResponder		*pResponder	= new SsapiResponder(this, pMsg);
				pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION , TRUE);
			}
		}
	}
	return OK;
}


// returns ptr to the necessary manager or NULL if no such dude is hanging around
ObjectManager* 
DdmSSAPI::GetObjectManager( U32 managerClassType ){
	
	for( int index = 0; index < m_iObjectTableSize; index++ )
		if( m_apObjectTable[index]->m_iObjectRequestCode == (int)managerClassType )
			return m_apObjectTable[index]->m_pObjectManager;

	return NULL;
}

// Performs necessary clean-up procedures
void 
DdmSSAPI::HandleSessionExpiredEvent( SESSION_ID sessionId ){

	m_pListenManager->DeleteAllListeners( sessionId );
	((UserManager *)GetObjectManager(SSAPI_MANAGER_CLASS_TYPE_USER_MANAGER))->CleanUpForThisSession( sessionId );
}


// Adds/removes managers that are busy and cannot service requests
void 
DdmSSAPI::SetManagerReady( U32 managerClassType, bool ready ){

	CONTAINER_ELEMENT		element;
	static U32				lastLaunchedManager = 0, i;

	if( ready )
		m_pNotReadyManagers->Remove( (CONTAINER_KEY)managerClassType );
	else{
		if( !m_pNotReadyManagers->Get( element, (CONTAINER_KEY)managerClassType ) )
			m_pNotReadyManagers->Add( (CONTAINER_ELEMENT)managerClassType, (CONTAINER_KEY)managerClassType );
	}

	if( !m_pNotReadyManagers->Count() && !m_isInited ){
		if( lastLaunchedManager == MANAGER_SYNC_TABLE_SIZE - 1 ){
			m_isInited = true;
			RecoverAlarms();
			printf("\n**************************************************************************");
			printf("\n*");
			printf("\n* -----> All SSAPI managers have successfully initialized. SSAPI is ready." );
			printf("\n*");
			printf("\n**************************************************************************");
		}
		else{
			lastLaunchedManager++;
			m_apObjectTable[lastLaunchedManager]->m_pObjectManager = SsapiManagerLaunchSyncTable[lastLaunchedManager].pCtor( m_pListenManager, this, m_pStringResourceManager );
			m_apObjectTable[lastLaunchedManager]->m_iObjectRequestCode = m_apObjectTable[lastLaunchedManager]->m_pObjectManager->GetDesignatorId().GetClassId();
		}
	}

	// if all managers are ready and there are queued up requests, start processing the queued requests
	while( !m_pNotReadyManagers->Count() && m_pRequestQueue->Count() ){
		SsapiRequestMessage	*pMsg;

		if( m_isInited ){
			m_pRequestQueue->GetAt( (CONTAINER_ELEMENT &)pMsg, 0 );
			m_pRequestQueue->RemoveAt( 0 );
			ProcessRequest( pMsg );
		}
	}
}	


// This callback is called once per recovered alarm. 
void 	
DdmSSAPI::cbRecoverAlarm(void *pAlarmContext_, STATUS status){

	SsapiAlarmContext	*pContext = (SsapiAlarmContext *)pAlarmContext_;

	if( status == OK ){
		for( int index = 0; index < m_iObjectTableSize; index++ )
			if( m_apObjectTable[index]->m_iObjectRequestCode == (int)pContext->GetManagerType() ){
				char *p = m_apObjectTable[index]->m_pObjectManager->GetName().CString();
				printf("\nDdmSSAPI: Recovered an alarm for the %s", p );
				delete p;
				m_apObjectTable[index]->m_pObjectManager->HandleAlarmRecovered( pContext );
			}
	}
}
