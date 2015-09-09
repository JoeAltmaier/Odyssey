//******************************************************************************
// FILE:		ProcessManager.h
//
// PURPOSE:		Defines a manager-type object that will me managing processes
//				running in the O2K box
//******************************************************************************

#include "ProcessManager.h"
#include "SSAPITypes.h"
#include "RaidDefs.h"
#include "RmstrCmnds.h"
#include "RmstrEvents.h"
#include "RmstrErrors.h"
#include "SsapiAssert.h"
#include "ClassTypeMap.h"
#include "SsapiLocalResponder.h"
#include "SList.h"

// managed objects
#include "ProcessRaidUtilityVerify.h"
#include "ProcessRaidUtilityHotCopy.h"
#include "ProcessRaidUtilitySmartCopy.h"
#include "ProcessRaidUtilityRegenerate.h"
#include "ProcessRaidUtilityInitialize.h"

#include "Trace_Index.h"
#ifdef TRACE_INDEX
#undef TRACE_INDEX
#endif
#define TRACE_INDEX TRACE_SSAPI_MANAGERS


ProcessManager* ProcessManager::m_pThis	= NULL;

//******************************************************************************
// ProcessManager:
//
// PURPOSE:		Default constructor
//******************************************************************************

ProcessManager::ProcessManager( ListenManager	*pListenManager, DdmServices *pParent )
:ObjectManager( pListenManager, DesignatorId( RowId(), SSAPI_MANAGER_CLASS_TYPE_PROCESS_MANAGER ), pParent){

	m_shouldRebuildAllTables = false;
	m_builtTablesMask	= 0;
	m_pTable = new SSAPI_PROCESS_MGR_JUMP_TABLE_RECORD[SSAPI_PM_NUMBER_OF_TABLES_USED];
	m_outstandingRequests = 0;

	(m_pTable)->pTableName					= RAID_ARRAY_DESCRIPTOR_TABLE;
	(m_pTable)->tableMask					= SSAPI_PM_ARRAY_TABLE;
	(m_pTable)->pRowInsertedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(ProcessManager,ArrayTableRowInsertedCallback);
	(m_pTable)->pRowDeletedCallback			= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(ProcessManager,ArrayTableRowDeletedCallback);
	(m_pTable)->pRowModifiedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(ProcessManager,ArrayTableRowModifiedCallback);
	(m_pTable)->rowSize						= sizeof(RAID_ARRAY_DESCRIPTOR);
	(m_pTable)->pFieldDef					= (fieldDef*)ArrayDescriptorTable_FieldDefs;
	(m_pTable)->fieldDefSize				= sizeofArrayDescriptorTable_FieldDefs;

	(m_pTable +1 )->pTableName				= RAID_UTIL_DESCRIPTOR_TABLE;
	(m_pTable +1 )->tableMask				= SSAPI_PM_UTIL_TABLE;
	(m_pTable +1 )->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(ProcessManager,UtilTableModified);
	(m_pTable +1 )->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(ProcessManager,UtilTableModified);
	(m_pTable +1 )->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(ProcessManager,UtilTableModified);
	(m_pTable +1 )->rowSize					= sizeof(RAID_ARRAY_UTILITY);
	(m_pTable +1 )->pFieldDef				= (fieldDef*)UtilDescriptorTable_FieldDefs;
	(m_pTable +1 )->fieldDefSize			= sizeofUtilDescriptorTable_FieldDefs;

	for( int i = 0; i < SSAPI_PM_NUMBER_OF_TABLES_USED; i++ )
		(m_pTable + i)->pShadowTable	= new ShadowTable(	(m_pTable + i)->pTableName,
															this,
															(m_pTable + i)->pRowInsertedCallback,
															(m_pTable + i)->pRowDeletedCallback,
															(m_pTable + i)->pRowModifiedCallback,
															(m_pTable + i)->rowSize );

	m_isIniting			= true;
	m_tablesToRebuild	= 0;

	SSAPI_TRACE( TRACE_L2, "\nProcessManager: Intializing...." );

	m_pRaidCommandQ = new CmdSender(RMSTR_CMD_QUEUE_TABLE,
									sizeof(RMSTR_CMND_INFO),
									sizeof(RMSTR_EVENT_INFO),
									this);

	m_pRaidCommandQ->csndrInitialize((pInitializeCallback_t)METHOD_ADDRESS(ProcessManager,RaidCmdQInitReply));
	AddOutstandingRequest();

	m_pRaidCommandQ->csndrRegisterForEvents((pEventCallback_t)METHOD_ADDRESS(ProcessManager,RaidEventHandler));


	DefineAllTables();
	AddOutstandingRequest();
}


//******************************************************************************
// ~ProcessManager:
//
// PURPOSE:		The destructor
//******************************************************************************

ProcessManager::~ProcessManager(){

	delete m_pRaidCommandQ;

	for( int i = 0; i < SSAPI_PM_NUMBER_OF_TABLES_USED; i++ )
		delete (m_pTable + i)->pShadowTable;

	delete[] m_pTable;

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
ProcessManager::Dispatch( ValueSet *pRequestParms, U32 requestCode, SsapiResponder *pResponder){

	if( ObjectManager::Dispatch( pRequestParms, requestCode, pResponder ) )
		return true;

	return true;
}



//************************************************************************
// AddObject:
//
// PURPOSE:		Adds an object to the system
//
// NOTE:		Must be overridden by object managers that can add objects
//************************************************************************

bool 
ProcessManager::AddObject( ValueSet &objectValues, SsapiResponder *pResponder ){

	int					classType;
	ClassTypeMap		map;

	objectValues.GetInt( SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &classType );
	if( map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_RAID_UTILITY, classType, true ) )
		return StartRaidUtility( objectValues, pResponder );
	else
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
	
	return true;
}


//************************************************************************
// DefineAllTables:
//
// PURPOSE:		Issues requests to the PTS to define all tables if not yet
//				defined
//************************************************************************

STATUS 
ProcessManager::DefineAllTables(){

	STATUS	status = OK;

	for( int i = 0; i < SSAPI_PM_NUMBER_OF_TABLES_USED; i++ )
		status	|= (m_pTable + i)->pShadowTable->DefineTable(	(m_pTable + i)->pFieldDef,
																(m_pTable + i)->fieldDefSize,
																32,
																(pTSCallback_t)METHOD_ADDRESS(ProcessManager,DefineAllTablesReplyHandler),
																(void *)(m_pTable + i)->tableMask );

	if( status != OK ){
	}	// GAIEVENT

	return OK;
}


//************************************************************************
// DefineAllTablesReplyHandler:
//
// PURPOSE:		Handles replies to requests issued in DefineAllTables()
//************************************************************************

STATUS 
ProcessManager::DefineAllTablesReplyHandler( void *pContext, STATUS status ){

	U32			mask	= (U32)pContext;
	
	m_builtTablesMask	|= mask;
	if( (m_builtTablesMask & SSAPI_PM_ALL_TABLES_BUILT) == SSAPI_PM_ALL_TABLES_BUILT ){
		m_builtTablesMask = 0;
		InitializeAllTables();
	}

	return OK;
}


//************************************************************************
// InitializeAllTables:
//
// PURPOSE:		Sends requests to the PTS to init ShadowTable objects
//				for all the tables used.
//************************************************************************

STATUS 
ProcessManager::InitializeAllTables(){

	STATUS status = OK;

	for( int i = 0; i < SSAPI_PM_NUMBER_OF_TABLES_USED; i++ )
		status |= (m_pTable + i)->pShadowTable->Initialize(	(pTSCallback_t)METHOD_ADDRESS(ProcessManager,InitializeAllTablesReplyHandler), 
															(void *)(m_pTable + i)->tableMask );

	if( status != OK ){
	}	// GAIEVENT

	return OK;
}


//************************************************************************
// InitializeAllTablesReplyHandler:
//
// PURPOSE:		Handles replies to requests to the PTS sent from 
//				InitializeAllTables()
//************************************************************************

STATUS 
ProcessManager::InitializeAllTablesReplyHandler( void *pContext, STATUS status ){

	U32			mask	= (U32)pContext;
	
	m_builtTablesMask	|= mask;
	if( (m_builtTablesMask & SSAPI_PM_ALL_TABLES_BUILT) == SSAPI_PM_ALL_TABLES_BUILT ){
		m_builtTablesMask = 0;
		EnumerateAllTables();
	}

	return OK;
}


//************************************************************************
// EnumerateAllTables:
//
// PUPROSE:		Issues requests to the PTS to enumerate all tables used
//************************************************************************

STATUS 
ProcessManager::EnumerateAllTables(){

	STATUS		status = OK;
	
	for( int i = 0; i < SSAPI_PM_NUMBER_OF_TABLES_USED; i++ )
		status |= (m_pTable + i)->pShadowTable->Enumerate(	&(m_pTable + i)->pTempTable,
															(pTSCallback_t)METHOD_ADDRESS(ProcessManager,EnumerateAllTablesReplyHandler),
															(void *)(m_pTable + i)->tableMask);

	if( status != OK ){
	}	// GAIEVENT

	return OK;
}


//************************************************************************
// EnumerateAllTablesReplyHandler:
//
// PURPOSE:		Handles replies to requests sent by EnumerateAllTables().
//				When all table have been enumerated, sets "ready for 
//				service flag"
//************************************************************************

STATUS 
ProcessManager::EnumerateAllTablesReplyHandler( void *pContext, STATUS rc ){

	U32				mask = (U32)pContext;
	SList			container;

	for( int i = 0; i < SSAPI_PM_NUMBER_OF_TABLES_USED; i++ ){
		if( mask == (m_pTable + i)->tableMask ){
			if( m_tablesToRebuild & (m_pTable + i)->tableMask ){
				m_tablesToRebuild &= ~(m_pTable + i)->tableMask;
				delete (m_pTable + i)->pTempTable;
				return (m_pTable + i)->pShadowTable->Enumerate(	&(m_pTable + i)->pTempTable, (pTSCallback_t)METHOD_ADDRESS(ProcessManager,EnumerateAllTablesReplyHandler), (void *)(m_pTable + i)->tableMask);
			}
			else{
				m_builtTablesMask |= mask;
				break;
			}
		}
	}
	
	if( (m_builtTablesMask & SSAPI_PM_ALL_TABLES_BUILT) == SSAPI_PM_ALL_TABLES_BUILT ){

		if( m_shouldRebuildAllTables ){
			m_builtTablesMask = 0;
			ClearAllTempTables();
			EnumerateAllTables();
		}
		else
			AllTablesHaveBeenEnumerated();
	}

	return OK;
}


//******************************************************************************
// AllTablesHaveBeenEnumerated
//
// PURPOSE:		Called when all tables have been successfully enumed and
//				we can start building objects
//******************************************************************************

void 
ProcessManager::AllTablesHaveBeenEnumerated(){

	SList					container;
	U32						i;
	RAID_ARRAY_DESCRIPTOR	*pDescr = (RAID_ARRAY_DESCRIPTOR *)GetTempTableData( SSAPI_PM_ARRAY_TABLE );

	m_isIniting = false;
	
	// store all descriptors
	for( i = 0; i < GetShadowTable( SSAPI_PM_ARRAY_TABLE )->GetNumberOfRows(); i++, pDescr++ )
		m_descriptorCollector.Add( pDescr, sizeof(RAID_ARRAY_DESCRIPTOR) );

	CreateRaidUtils( container, NULL, 0 );
	AddObjectsIntoManagedObjectsVector( container );

	SSAPI_TRACE( TRACE_L2, "\nProcessManager: ...Done! Objects built: ", GetManagedObjectCount() );
	RemoveOutstandingRequest();
	ClearAllTempTables();
}


//******************************************************************************
// PTS Callbacks for the ArrayDescriptor
//******************************************************************************

STATUS 
ProcessManager::ArrayTableRowInsertedCallback( void *pRows, U32 numberOfRows, ShadowTable*){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_PM_ARRAY_TABLE;
		return OK;
	}

	U32						i;
	RAID_ARRAY_DESCRIPTOR	*pDescr = (RAID_ARRAY_DESCRIPTOR *)pRows;

	for( i = 0; i < numberOfRows; i++, pDescr++ )
		m_descriptorCollector.Add( pDescr, sizeof(RAID_ARRAY_DESCRIPTOR) );

	SSAPI_TRACE( TRACE_L3, "\nProcessManager::ArrayTableRowInsertedCallback" );

	return OK;
}


STATUS 
ProcessManager::ArrayTableRowModifiedCallback( void *pRows, U32 numberOfRows, ShadowTable*){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_PM_ARRAY_TABLE;
		return OK;
	}

	SSAPI_TRACE( TRACE_L3, "\nProcessManager::ArrayTableRowModifiedCallback" );
	
	U32						i;
	RAID_ARRAY_DESCRIPTOR	*pDescr = (RAID_ARRAY_DESCRIPTOR *)pRows;

	for( i = 0; i < numberOfRows; i++, pDescr++ ){
		m_descriptorCollector.Delete( RowId(pDescr->thisRID) );
		m_descriptorCollector.Add( pDescr, sizeof(RAID_ARRAY_DESCRIPTOR) );
	}

	return OK;
}


STATUS 
ProcessManager::ArrayTableRowDeletedCallback( void *pRows, U32 numberOfRows, ShadowTable*){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_PM_ARRAY_TABLE;
		return OK;
	}

	SSAPI_TRACE( TRACE_L3, "\nProcessManager::ArrayTableRowDeletedCallback" );

	U32						i;
	RAID_ARRAY_DESCRIPTOR	*pDescr = (RAID_ARRAY_DESCRIPTOR *)pRows;

	for( i = 0; i < numberOfRows; i++, pDescr++ )
		m_descriptorCollector.Delete( RowId(pDescr->thisRID) );


	return OK;
}


//******************************************************************************
// PTS callbacks (one for all!)
//******************************************************************************

STATUS 
ProcessManager::UtilTableModified( void*, U32, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_PM_UTIL_TABLE;
		return OK;
	}

	SSAPI_TRACE( TRACE_L3, "\nProcessManager::UtilTableModified" );

	return OK;
}


//************************************************************************
// GetShadowTable:
//
// PURPOSE:		Performs a look up of the table by the table mask
//************************************************************************

ShadowTable* 
ProcessManager::GetShadowTable( U32 tableMask ){

	for( U32 i = 0; i < SSAPI_PM_NUMBER_OF_TABLES_USED; i++ )
		if( m_pTable[i].tableMask == tableMask )
			return m_pTable[i].pShadowTable;


	ASSERT(0);
	return NULL;
}


//************************************************************************
// GetTempTableData:
// 
// PURPOSE:		Retrieves pointer to temp table data
//************************************************************************

void* 
ProcessManager::GetTempTableData( U32 tableMask ){

	for( U32 i = 0; i < SSAPI_PM_NUMBER_OF_TABLES_USED; i++ )
		if( m_pTable[i].tableMask == tableMask )
			return m_pTable[i].pTempTable;


	ASSERT(0);
	return NULL;
}


//************************************************************************
// ObjectDeletedCallbackHandler:
//
// PURPOSE:		This method may be provided by subclasses to be notified
//				by events from other object managers. 
//************************************************************************

void 
ProcessManager::ObjectDeletedCallbackHandler( SSAPIEvent *pEvent , bool isLast ){
}


//************************************************************************
// ClearAllTempTables:
//
// PURPOSE:		Deallocates memory taken by temp tables 
//************************************************************************

void 
ProcessManager::ClearAllTempTables(){

	for( U32 i = 0; i < SSAPI_PM_NUMBER_OF_TABLES_USED; i++ ){
		delete m_pTable[i].pTempTable;
		m_pTable[i].pTempTable = NULL;
	}
}


//******************************************************************************
// CreateRaidUtils:
//
// PURPOSE:		Creates RAID util objects and stores them in the 'container'
//
// NOTE:		if pDescr == NULL, data will be fetched from temp table data,
//				else, the descriptor must be in collector
//******************************************************************************

void 
ProcessManager::CreateRaidUtils( Container &container, RAID_ARRAY_UTILITY *pDescr, U32 count ){

	ProcessRaidUtility		*pUtil;
	U32						i;
	RAID_ARRAY_DESCRIPTOR	*pArray;
	DesignatorId			id;

	if( !pDescr ){	// fetch data from the TempTableData
		pDescr	= (RAID_ARRAY_UTILITY *)GetTempTableData( SSAPI_PM_UTIL_TABLE );
		count	= GetShadowTable(SSAPI_PM_UTIL_TABLE)->GetNumberOfRows();
	}

	for( i = 0; i < count; i++, pDescr++ ){
		if( !m_descriptorCollector.Get( (RowId &)pDescr->targetRID, (void *&)pArray ) ){
			ASSERT(0);
			continue;
		}

		// TBDGAI the right way would be to get the id from the StorageManager
		switch(pArray->raidLevel){
			case RAID0:
				id = DesignatorId( RowId(pArray->SRCTRID), SSAPI_OBJECT_CLASS_TYPE_ARRAY_0_STORAGE_ELEMENT);
				break;

			case RAID1:
				if( pArray->createPolicy.StartHotCopyWithAutoBreak )
					id = DesignatorId( RowId(pArray->SRCTRID), SSAPI_OBJECT_CLASS_TYPE_ARRAY_HOT_COPY_AUTO );
				else if( pArray->createPolicy.StartHotCopyWithManualBreak )
					id = DesignatorId( RowId(pArray->SRCTRID), SSAPI_OBJECT_CLASS_TYPE_ARRAY_HOT_COPY_MANUAL );
				else
					id = DesignatorId( RowId(pArray->SRCTRID), SSAPI_OBJECT_CLASS_TYPE_ARRAY_1_STORAGE_ELEMENT);
				break;

			default:
				ASSERT(0);
				continue;
		}

		switch( pDescr->utilityCode ){
			case RAID_UTIL_VERIFY:
				pUtil = new ProcessRaidUtilityVerify( GetListenManager(), m_pRaidCommandQ, id );
				break;
			
			case RAID_UTIL_REGENERATE:
				pUtil = new ProcessRaidUtilityRegenerate( GetListenManager(), m_pRaidCommandQ, id );
				break;

			case RAID_UTIL_LUN_HOTCOPY:
				pUtil = new ProcessRaidUtilityHotCopy( GetListenManager(), m_pRaidCommandQ, id );
				break;

			case RAID_UTIL_MEMBER_HOTCOPY:
				pUtil = new ProcessRaidUtilitySmartCopy( GetListenManager(), m_pRaidCommandQ, id );
				break;

			case RAID_UTIL_BKGD_INIT:
				pUtil = new ProcessRaidUtilityInitialize( GetListenManager(), m_pRaidCommandQ, id );
				break;

			case RAID_UTIL_EXPAND:
			default:
				ASSERT(0);
				continue;
				break;
		}

		pUtil->BuildYourselfFromPtsRow( pDescr );
		container.Add( (CONTAINER_ELEMENT) pUtil );
	}
}


//******************************************************************************
// StartRaidUtility:
//
// PURPOSE:		Starts a Raid utility
//******************************************************************************

bool 
ProcessManager::StartRaidUtility( ValueSet &utilValues, SsapiResponder *pResponder ){

	int					classType;
	ProcessRaidUtility	*pUtil;
	bool				rc;


	utilValues.GetInt( SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &classType );
	switch( classType ){
		case SSAPI_OBJECT_CLASS_TYPE_RAID_VERIFY:
			pUtil = new ProcessRaidUtilityVerify(	GetListenManager(),
													m_pRaidCommandQ,
													DesignatorId() );
			break;

		case SSAPI_OBJECT_CLASS_TYPE_RAID_REGENERATE:
		case SSAPI_OBJECT_CLASS_TYPE_RAID_HOT_COPY:
		default:
			pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALIDPARM_EXCEPTION_CANT_START_PROCESS );
			ASSERT(0);
			return false;
	}
	
	*pUtil = utilValues;
	pUtil->BuildYourselfFromYourValueSet();

	rc = ((Process *)pUtil)->Start( this, pResponder );
	delete pUtil;
	
	if( rc ){
		AddOutstandingRequest();
		SSAPI_TRACE( TRACE_L2, "\nProcessManager::Sent START_UTIL to Raid Master" );
	}
	else{
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );
	}

	return rc;
}


//******************************************************************************
// RaidCommandCompletionReply:
//
// PURPOSE:		Gets all command completetion replies from the 
//				Raid Master
//******************************************************************************

void 
ProcessManager::RaidCommandCompletionReply(	STATUS				completionCode,
											void				*pStatusData,
											void				*pCmdData,
											void				*pCmdContext ){

	RMSTR_CMND_INFO				*pInfo = (RMSTR_CMND_INFO *)pCmdData;
	RMSTR_CMND_PARAMETERS		*pParams = &pInfo->cmdParams;
	RMSTR_START_UTIL_INFO		*pStartUtilInfo = (RMSTR_START_UTIL_INFO *)&pParams->startUtilInfo;
	RMSTR_ABORT_UTIL_INFO		*pAbortUtilInfo = (RMSTR_ABORT_UTIL_INFO *)&pParams->abortUtilInfo;
	RMSTR_CHANGE_PRIORITY_INFO	*pChangePriorityInfo =	(RMSTR_CHANGE_PRIORITY_INFO *)&pParams->changePriorityInfo;
	SsapiResponder				*pResponder;


	SSAPI_TRACE( TRACE_L2, "\nProcessManager::Got ReplyFrom Raid Master" );

	switch( pInfo->opcode ){

		case RMSTR_CMND_START_UTIL:
			pResponder = (SsapiResponder *)pCmdContext;
			if( completionCode == OK ){
				if( pResponder )
					pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
			}
			else
				pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, m_errorConvertor.GetSsapiException(completionCode) );

			RemoveOutstandingRequest();
			break;

		case RMSTR_CMND_ABORT_UTIL:
			pResponder = (SsapiResponder *)pCmdContext;
			if( completionCode == OK ){
				if( pResponder )
					pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
			}
			else
				pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, m_errorConvertor.GetSsapiException(completionCode) );

			RemoveOutstandingRequest();
			break;

		case RMSTR_CMND_CHANGE_UTIL_PRIORITY:
			pResponder = (SsapiResponder *)pCmdContext;
			if( completionCode == OK ){
				if( pResponder )
					pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
			}
			else
				pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, m_errorConvertor.GetSsapiException(completionCode) );

			RemoveOutstandingRequest();
			break;

		default:
			ASSERT(0);
			break;
	}

}

//************************************************************************
// RaidEventHandler:
//
// PURPOSE:		Called by the Raid command Q on command completetion and
//				to report events
//************************************************************************

void 
ProcessManager::RaidEventHandler( STATUS comletionCode, void *pStatusData ){

	if( m_isIniting ){
		m_shouldRebuildAllTables = true;
		return;
	}

	RMSTR_EVT_UTIL_STARTED_STATUS		*pEvtUtilStarted = NULL;
	RMSTR_EVT_UTIL_STOPPED_STATUS		*pEvtUtilStopped = NULL;
	RMSTR_PRIORITY_CHANGED_STATUS		*pEvtPriorityChanged = NULL;
	RMSTR_PERCENT_COMPLETE_STATUS		*pEvtPercentComplete = NULL;
	RAID_ARRAY_UTILITY					*pUtilRow;
	ProcessRaidUtility					*pUtil;
	SList								container;
	DesignatorId						id;

	switch( comletionCode ){
		case RMSTR_EVT_UTIL_STARTED:
			SSAPI_TRACE( TRACE_L2, "\nProcessManager: Got RaidEvent: Util Started" );
			pEvtUtilStarted = (RMSTR_EVT_UTIL_STARTED_STATUS *)pStatusData;
			pUtilRow		= (RAID_ARRAY_UTILITY *)&pEvtUtilStarted->utilityData;

			CreateRaidUtils( container, pUtilRow, 1 );
			AddObjectsIntoManagedObjectsVector( container );
			break;

		case RMSTR_EVT_UTIL_PRIORITY_CHANGED:
			SSAPI_TRACE( TRACE_L2, "\nProcessManager: Got RaidEvent: Util priority changed" );
			pEvtPriorityChanged = (RMSTR_PRIORITY_CHANGED_STATUS *)pStatusData;
			pUtilRow			= (RAID_ARRAY_UTILITY *)&pEvtPriorityChanged->utilityData;
			pUtil				= GetRaidUtility( pUtilRow->thisRID );
			if( pUtil )
				pUtil->SetPriority( pUtilRow->priority );
			break;

		case RMSTR_EVT_UTIL_PERCENT_COMPLETE:
			pEvtPercentComplete = (RMSTR_PERCENT_COMPLETE_STATUS *)pStatusData;
			pUtilRow			= (RAID_ARRAY_UTILITY *)&pEvtPercentComplete->utilityData;
			pUtil				= GetRaidUtility( pUtilRow->thisRID );
			if( pUtil )
				pUtil->SetPercentComplete( pUtilRow->percentComplete );
			break;

		case RMSTR_EVT_UTIL_STOPPED:
			pEvtUtilStopped = (RMSTR_EVT_UTIL_STOPPED_STATUS *)pStatusData;
			pUtilRow		= (RAID_ARRAY_UTILITY *)&pEvtUtilStopped->utilityData;
			pUtil			= GetRaidUtility( pUtilRow->thisRID );
			if( pUtil ){
				container.Add( (CONTAINER_ELEMENT)pUtil );
				DeleteObjectsFromTheSystem( container );
			}
			break;

		default:
			break;
	}
}


//******************************************************************************
// GetRaidUtility:
//
// PURPOSE:		Looks up a raid util by rid in the raid util descriptor
//******************************************************************************

ProcessRaidUtility* 
ProcessManager::GetRaidUtility( RowId rid ){

	ManagedObject		*pObj;
	U32					i;

	for( i = 0; i < GetManagedObjectCount(); i++ ){
		pObj = GetManagedObject( i );
		if( pObj->GetDesignatorId().GetRowId() == rid )
			return (ProcessRaidUtility *)pObj;
	}

	SSAPI_TRACE( TRACE_L1, "\nProcessManager: Utility referenced was not found!" );
	ASSERT(0);
	return NULL;
}