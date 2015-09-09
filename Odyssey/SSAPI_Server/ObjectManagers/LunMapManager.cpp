//************************************************************************
// FILE:		LunMapManager.cpp
//
// PURPOSE:		Implements class LunMapManager whose instance will control
//				LUN mapping/unmapping as well as exporting/unexporting
//************************************************************************


#include "LunMapManager.h"
#include "StringResourceManager.h"
#include "SSAPITypes.h"
#include "SList.h"
#include "LunMapEntry.h"
#include "ListenManager.h"
#include "DdmVCMCommands.h"
#include "CmdSender.h"
#include "ListenManager.h"
#include "PhsDataManager.h"
#include "ClassTypeMap.h"
#include "SSAPIEvents.h"
#include "STSData.h"

#include "Trace_Index.h"
#ifdef TRACE_INDEX
#undef TRACE_INDEX
#endif
#define TRACE_INDEX TRACE_SSAPI_MANAGERS

LunMapManager* LunMapManager::m_pThis	= NULL;

//************************************************************************
// LunMapManager:
//
// PURPOSE:		Default constructor
//************************************************************************

LunMapManager::LunMapManager(	ListenManager			*pListenManager,	
								DdmServices				*pParent,
								StringResourceManager	*pSRManager )

			  :ObjectManager(	pListenManager, 
								DesignatorId( RowId(), SSAPI_MANAGER_CLASS_TYPE_LUN_MANAGER ),
								pParent ){

	m_pStringResourceManager	= pSRManager;
	m_builtTablesMask	= 0;
	m_requestsOutstanding = 0;

	m_pTable = new SSAPI_LUN_MAP_MGR_JUMP_TABLE_RECORD[SSAPI_LM_NUMBER_OF_TABLES_USED];

	(m_pTable)->pTableName					= EXPORT_TABLE_USER_INFO_TABLE_NAME;
	(m_pTable)->tableMask					= SSAPI_LM_EXPORT_USER_INFO_TABLE;
	(m_pTable)->pRowInsertedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(LunMapManager,UserInfoRowInserted);
	(m_pTable)->pRowDeletedCallback			= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(LunMapManager,UserInfoRowDeleted);
	(m_pTable)->pRowModifiedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(LunMapManager,UserInfoRowModified);
	(m_pTable)->pCreateObjectsFromRow		= (LunMapManager::CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(LunMapManager,CreateObjectsFromUserInfoRow);
	(m_pTable)->rowSize						= sizeof(ExportTableUserInfoRecord);
	(m_pTable)->pFieldDef					= (fieldDef*)ExportTableUserInfoTable_FieldDefs;
	(m_pTable)->fieldDefSize				= cbExportTableUserInfoTable_FieldDefs;

	(m_pTable + 1)->pTableName				= STORAGE_ROLL_CALL_TABLE;
	(m_pTable + 1)->tableMask				= SSAPI_LM_SRC_TABLE;
	(m_pTable + 1)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(LunMapManager,SrcTableRowInserted);
	(m_pTable + 1)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(LunMapManager,SrcTableRowDeleted);
	(m_pTable + 1)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(LunMapManager,SrcTableRowModified);
	(m_pTable + 1)->pCreateObjectsFromRow	= (LunMapManager::CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(LunMapManager,CreateNoObjects);
	(m_pTable + 1)->rowSize					= sizeof(StorageRollCallRecord);
	(m_pTable + 1)->pFieldDef				= (fieldDef*)StorageRollCallTable_FieldDefs;
	(m_pTable + 1)->fieldDefSize			= cbStorageRollCallTable_FieldDefs;

	(m_pTable + 2)->pTableName				= EXPORT_TABLE;
	(m_pTable + 2)->tableMask				= SSAPI_LM_EXPORT_TABLE;
	(m_pTable + 2)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(LunMapManager,ExportTableRowInserted);
	(m_pTable + 2)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(LunMapManager,ExportTableRowDeleted);
	(m_pTable + 2)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(LunMapManager,ExportTableRowModified);
	(m_pTable + 2)->pCreateObjectsFromRow	= (LunMapManager::CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(LunMapManager,CreateNoObjects);
	(m_pTable + 2)->rowSize					= sizeof(ExportTableEntry);
	(m_pTable + 2)->pFieldDef				= (fieldDef*)ExportTable_FieldDefs;
	(m_pTable + 2)->fieldDefSize			= cbExportTable_FieldDefs;

	(m_pTable + 3)->pTableName				= STS_DATA_TABLE;
	(m_pTable + 3)->tableMask				= SSAPI_LM_ID_INFO_TABLE;
	(m_pTable + 3)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(LunMapManager,IdInfoTableRowInserted);
	(m_pTable + 3)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(LunMapManager,IdInfoTableRowDeleted);
	(m_pTable + 3)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(LunMapManager,IdInfoTableRowModified);
	(m_pTable + 3)->pCreateObjectsFromRow	= (LunMapManager::CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(LunMapManager,CreateNoObjects);
	(m_pTable + 3)->rowSize					= sizeof(StsData);
	(m_pTable + 3)->pFieldDef				= (fieldDef*)STSDataTable_FieldDefs;
	(m_pTable + 3)->fieldDefSize			= cbSTSDataTable_FieldDefs;


	for( int i = 0; i < SSAPI_LM_NUMBER_OF_TABLES_USED; i++ )
		(m_pTable + i)->pShadowTable	= new ShadowTable(	(m_pTable + i)->pTableName,
															this,
															(m_pTable + i)->pRowInsertedCallback,
															(m_pTable + i)->pRowDeletedCallback,
															(m_pTable + i)->pRowModifiedCallback,
															(m_pTable + i)->rowSize );
	AddOutstandingRequest();

	m_isIniting			= true;
	m_tablesToRebuild	= 0;

	// register for OBJECT_ADDED events
	m_pLocalResponder = new SsapiLocalResponder( this, (LOCAL_EVENT_CALLBACK)METHOD_ADDRESS(LunMapManager,ObjectAddedEventCallback) ); 
	GetListenManager()->AddListenerForObjectAddedEvent( SSAPI_LISTEN_OWNER_ID_ANY, m_pLocalResponder->GetSessionID(), CALLBACK_METHOD(m_pLocalResponder, 1) );
	
	m_pObjectModifiedResponder = new SsapiLocalResponder( this, (LOCAL_EVENT_CALLBACK)METHOD_ADDRESS(LunMapManager,ObjectModifiedEventCallback) ); 
	GetListenManager()->AddListenerForObjectModifiedEventForManagers( SSAPI_LISTEN_OWNER_ID_ANY, m_pObjectModifiedResponder->GetSessionID(), CALLBACK_METHOD(m_pObjectModifiedResponder, 1) );


	// init the cmd snder for the VCM
	m_pCmdSender	= new CmdSender(VCM_CONTROL_QUEUE,
									VCM_CONTROL_COMMAND_SIZE,
									VCM_CONTROL_STATUS_SIZE,
									this);

	m_pCmdSender->csndrInitialize((pInitializeCallback_t)METHOD_ADDRESS(LunMapManager,InitVCMCommandQueueCallback));
	m_pCmdSender->csndrRegisterForEvents((pEventCallback_t)METHOD_ADDRESS(LunMapManager,VCMEventHandler));


	SSAPI_TRACE( TRACE_L2, "\nLunMapManager: Intializing...." );
	DefineAllTables();
}



//************************************************************************
// InitVCMCommandQueueCallback:
//
// PURPOSE:		Called by the VCM command queue object to report the end
//				of command q. initialization.
//************************************************************************

void 
LunMapManager::InitVCMCommandQueueCallback( STATUS rc ) { 

	if( rc != OK ){
		ASSERT( 0 ); 
		// TBDGAI log event here
	}
}


//************************************************************************
// ~LunMapManager:
//
// PURPOSE:		The destructor
//************************************************************************

LunMapManager::~LunMapManager(){

	GetListenManager()->DeleteListenerForObjectAddedEvent( SSAPI_LISTEN_OWNER_ID_ANY, m_pLocalResponder->GetSessionID() );
	delete m_pLocalResponder;
	
	GetListenManager()->DeleteListenerForObjectModifiedEventForManagers( SSAPI_LISTEN_OWNER_ID_ANY, m_pObjectModifiedResponder->GetSessionID() );
	delete m_pObjectModifiedResponder;

	for( int i = 0; i < SSAPI_LM_NUMBER_OF_TABLES_USED; i++ )
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
LunMapManager::Dispatch( ValueSet *pRequestParms, U32 requestCode, SsapiResponder *pResponder){

	DesignatorId		lunId;

	if( ObjectManager::Dispatch( pRequestParms, requestCode, pResponder ) )
		return true;

	switch( requestCode ){
		case SSAPI_LUN_MANAGER_EXPORT:
			pRequestParms->GetGenericValue( (char *)&lunId, sizeof(lunId), SSAPI_LUN_MANAGER_EXPORT_LUN_MAP_ENTRY_ID );
			ExportLun( lunId, true, pResponder );
			break;

		case SSAPI_LUN_MANAGER_UNEXPORT:
			pRequestParms->GetGenericValue( (char *)&lunId, sizeof(lunId), SSAPI_LUN_MANAGER_UNEXPORT_LUN_MAP_ENTRY_ID );
			ExportLun( lunId, false, pResponder );
			break;

		case SSAPI_LUN_MANAGER_VERIFY_LUN:

			pResponder->RespondToRequest( SSAPI_EXCEPTION_CAPABILITIES, CTS_SSAPI_EXCEPTION_REQUEST_NOT_IMPLEMENTED );
			break;

		default:
			pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_EXCEPTION_REQUEST_NOT_IMPLEMENTED );
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
LunMapManager::ObjectDeletedCallbackHandler( SSAPIEvent *pEvent , bool isLast ){

	ValueSet				*pObjVs = new ValueSet;
	ClassTypeMap			map;
	bool					isPhsObject;
	int						classType;
	DesignatorId			id;
	CoolVector				container;
	ManagedObject			*pObj;

	*pObjVs	= *(ValueSet *)pEvent->GetValue( SSAPI_EVENT_FID_MANAGED_OBJECT );
	pObjVs->GetInt( SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &classType );
	isPhsObject = map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_PHS_DATA, classType, true );

	if( isPhsObject ){
		pObjVs->GetGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID );
		pObj = GetObjectWithPhsId( id );
		if( pObj )
			pObj->DeletePhsDataItem( id );
	}

	delete pObjVs;
}


//************************************************************************
// AddObject:
//
// PURPOSE:		Adds an object to the system
//
// NOTE:		Must be overridden by object managers that can add objects
//************************************************************************

bool 
LunMapManager::AddObject( ValueSet &objectValues, SsapiResponder *pResponder ){
	
	LunMapEntry				*pLun = new LunMapEntry(GetListenManager(), this );
	LM_INSERT_STRING_CELL	*pCell= new LM_INSERT_STRING_CELL;
	bool					rc;

	*pLun = objectValues;
	pLun->BuildYourselfFromYourValueSet();

	if( !CheckForDuplicateExportInfo( pLun, pResponder ) ){
		delete pLun;
		return true;
	}

	pCell->pThis = this;
	pCell->pResponder = pResponder;
	pCell->outstandingRequests = 0;
	pCell->requestValues = objectValues;
	pCell->hasErrorOccured = false;
	
	if( pLun->GetName().GetLength() ){
		rc = m_pStringResourceManager->WriteString(	pLun->GetName(),
													&pCell->ridName,
													(pTSCallback_t)METHOD_ADDRESS(LunMapManager, AddObjectInsertStringsCallback),
													pCell );
		if( rc ){
			AddOutstandingRequest();
			pCell->outstandingRequests++;
		}
		else{
			pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );
			delete pCell;
			return true;
		}
	}
	else{
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_NAME_IS_BLANK );
		delete pCell;
		return true;
	}

	if( pLun->GetDescription().GetLength() ){
		rc = m_pStringResourceManager->WriteString(	pLun->GetDescription(),
													&pCell->ridDescription,
													(pTSCallback_t)METHOD_ADDRESS(LunMapManager, AddObjectInsertStringsCallback),
													pCell );
		if( rc ){
			AddOutstandingRequest();
			pCell->outstandingRequests++;
		}
		else{
			pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );
			delete pCell;
			return true;
		}
	}
	
	delete pLun;
	return true;
}


STATUS 
LunMapManager::AddObjectInsertStringsCallback( void *pContext, STATUS status ){

	LM_INSERT_STRING_CELL			*pCell = (LM_INSERT_STRING_CELL *)pContext;
	ExportTableUserInfoRecord		row;
	LM_INSERT_USER_INFO_ROW_CELL	*pUICell;
	
	if( status != OK ){
		pCell->hasErrorOccured = true;
		ASSERT(0);
	}	
		
	if( !--pCell->outstandingRequests ){ // ready to insert a UserInfoRow
		if( !pCell->hasErrorOccured ){

			row.size			= sizeof( row );
			row.version			= EXPORT_TABLE_USER_INFO_TABLE_VERSION;
			row.ridDescription	= pCell->ridDescription;
			row.ridName			= pCell->ridName;

			pUICell	= new LM_INSERT_USER_INFO_ROW_CELL;
			pUICell->objectValues	= pCell->requestValues;
			pUICell->pResponder		= pCell->pResponder;
			pUICell->ridName		= pCell->ridName;
			pUICell->ridDescription	= pCell->ridDescription;

			pCell->pThis->GetShadowTable(SSAPI_LM_EXPORT_USER_INFO_TABLE)
				->InsertRow(&row,
							&pUICell->ridUserInfo,
							(pTSCallback_t)METHOD_ADDRESS(LunMapManager, AddObjectInsertUIRowCallback),
							pUICell );

			pCell->pThis->AddOutstandingRequest();
		}
		else {
			pCell->pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );
		}
	}
	pCell->pThis->RemoveOutstandingRequest();
	if( !pCell->outstandingRequests )
		delete pCell;
	return OK;
}


STATUS 
LunMapManager::AddObjectInsertUIRowCallback( void *pContext, STATUS status ){

	LM_INSERT_USER_INFO_ROW_CELL	*pCell = (LM_INSERT_USER_INFO_ROW_CELL *)pContext;
	VCRequest						request;
	VCCreateCommand					*pCmd = &request.u.VCCreateParms;
	LunMapEntry						*pLun = new LunMapEntry(GetListenManager(), this );

	if( status == OK ){
		*pLun = pCell->objectValues;
		pLun->BuildYourselfFromYourValueSet();
		pLun->SetRidUserInfo( pCell->ridUserInfo );
		pLun->SetRidDescription( pCell->ridDescription );
		pLun->SetRidName( pCell->ridName );

		pLun->PopulateVCMCreateCommand( *pCmd );
		request.eCommand = k_eCreateVC;
		m_pCmdSender->csndrExecute(	&request,
									(pCmdCompletionCallback_t)METHOD_ADDRESS(LunMapManager, AddObjectCallback),
									pCell->pResponder);
		AddOutstandingRequest();
	}
	else {
		pCell->pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );
	}

	RemoveOutstandingRequest();
	delete pLun;
	delete pCell;
	return OK;
}


void   
LunMapManager::AddObjectCallback(	STATUS			completionCode,
									void			*pResultData,
									void			*pCmdData,
									void			*pCmdContext ){

	SsapiResponder		*pResponder	= (SsapiResponder *)pCmdContext;
	VCStatus			*pStatus = (VCStatus *)pResultData;

	if( pStatus->status == OK ){
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	}
	else{
		// TBDGAI detailed status
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INTERNAL, pStatus->status );
	}

	RemoveOutstandingRequest();
}


STATUS // for test only!
LunMapManager::AddObjectTestCallback( void *pContext, STATUS status ){
	SsapiResponder	*pResponder = (SsapiResponder *)pContext;
	pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	RemoveOutstandingRequest();
	return OK;
}


//************************************************************************
// DefineAllTables:
//
// PURPOSE:		Issues requests to the PTS to define all tables if not yet
//				defined
//************************************************************************

STATUS 
LunMapManager::DefineAllTables(){

	STATUS	status = OK;

	for( int i = 0; i < SSAPI_LM_NUMBER_OF_TABLES_USED; i++ )
		status	|= (m_pTable + i)->pShadowTable->DefineTable(	(m_pTable + i)->pFieldDef,
																(m_pTable + i)->fieldDefSize,
																32,
																(pTSCallback_t)METHOD_ADDRESS(LunMapManager,DefineAllTablesReplyHandler),
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
LunMapManager::DefineAllTablesReplyHandler( void *pContext, STATUS status ){

	U32			mask	= (U32)pContext;
	
	m_builtTablesMask	|= mask;
	if( (m_builtTablesMask & SSAPI_LM_ALL_TABLES_BUILT) == SSAPI_LM_ALL_TABLES_BUILT ){
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
LunMapManager::InitializeAllTables(){

	STATUS status = OK;

	for( int i = 0; i < SSAPI_LM_NUMBER_OF_TABLES_USED; i++ )
		status |= (m_pTable + i)->pShadowTable->Initialize(	(pTSCallback_t)METHOD_ADDRESS(LunMapManager,InitializeAllTablesReplyHandler), 
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
LunMapManager::InitializeAllTablesReplyHandler( void *pContext, STATUS status ){

	U32			mask	= (U32)pContext;
	
	m_builtTablesMask	|= mask;
	if( (m_builtTablesMask & SSAPI_LM_ALL_TABLES_BUILT) == SSAPI_LM_ALL_TABLES_BUILT ){
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
LunMapManager::EnumerateAllTables(){
	
	STATUS		status = OK;
	
	for( int i = 0; i < SSAPI_LM_NUMBER_OF_TABLES_USED; i++ )
		status |= (m_pTable + i)->pShadowTable->Enumerate(	&(m_pTable + i)->pTempTable,
															(pTSCallback_t)METHOD_ADDRESS(LunMapManager,EnumerateAllTablesReplyHandler),
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
LunMapManager::EnumerateAllTablesReplyHandler( void *pContext, STATUS rc ){

	U32					mask = (U32)pContext, index;
	SList				container;
	int					i, descrNum;
	char				*pDescr;
	ExportTableEntry	*pExportEntry;

	for( i = 0; i < SSAPI_LM_NUMBER_OF_TABLES_USED; i++ ){
		if( mask == (m_pTable + i)->tableMask ){
			if( m_tablesToRebuild & (m_pTable + i)->tableMask ){
				m_tablesToRebuild &= ~(m_pTable + i)->tableMask;
				delete (m_pTable + i)->pTempTable;
				return (m_pTable + i)->pShadowTable->Enumerate(	&(m_pTable + i)->pTempTable, (pTSCallback_t)METHOD_ADDRESS(LunMapManager,EnumerateAllTablesReplyHandler), (void *)(m_pTable + i)->tableMask);
			}
			else if( mask != SSAPI_LM_EXPORT_TABLE ){
				for( descrNum = 0, pDescr = (char *)(m_pTable + i)->pTempTable; descrNum < (m_pTable + i)->pShadowTable->GetNumberOfRows(); descrNum++, pDescr+=(m_pTable + i)->rowSize )
					m_descriptorCollector.Add( pDescr, (m_pTable + i)->rowSize );
			}

			m_builtTablesMask |= mask;
		}
	}

	if( (m_builtTablesMask & SSAPI_LM_ALL_TABLES_BUILT) == SSAPI_LM_ALL_TABLES_BUILT ){
		// save lun id info records
		StsData	*pIdRow = (StsData *)GetTempTableData(SSAPI_LM_ID_INFO_TABLE );
		for( index = 0; index < GetShadowTable(SSAPI_LM_ID_INFO_TABLE)->GetNumberOfRows(); index++, pIdRow++ )
			m_idInfoCollector.Add( pIdRow, sizeof(StsData) );

		// create objects for the first time
		pExportEntry = (ExportTableEntry *)GetTempTableData( SSAPI_LM_EXPORT_TABLE );
		for( index = 0; index < GetShadowTable(SSAPI_LM_EXPORT_TABLE)->GetNumberOfRows(); index++, pExportEntry++ )
			if( RowId( pExportEntry->ridAltExportRec ).IsClear() )
				CreateObjectsFromExportTableRow( pExportEntry, container );
		
		// add objects to the list of managed objects
		AddObjectsIntoManagedObjectsVector( container );

		// delete all temp tables
		for( i = 0; i < SSAPI_LM_NUMBER_OF_TABLES_USED; i++ )
			delete (m_pTable + i)->pTempTable;

		// set ready!
		m_isIniting = false;
		SSAPI_TRACE( TRACE_L2, "\nLunMapManager: ...Done! Objects built: ", GetManagedObjectCount() );
		RemoveOutstandingRequest();
	}

	return OK;
}


//************************************************************************
// PTS callbacks for Storage Roll Call Table
//************************************************************************

STATUS 
LunMapManager::SrcTableRowInserted( void *pRows_, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_LM_SRC_TABLE;
		return OK;
	}
	
	StorageRollCallRecord	*pRow = (StorageRollCallRecord *)pRows_;

	for( U32 i = 0; i < numberOfRows; i++, pRow++ )
		m_descriptorCollector.Add( pRow, sizeof(StorageRollCallRecord) );

	return OK;
}


STATUS 
LunMapManager:: SrcTableRowDeleted( void *pRows_, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_LM_SRC_TABLE;
		return OK;
	}

	StorageRollCallRecord	*pRow = (StorageRollCallRecord *)pRows_;

	for( U32 i = 0; i < numberOfRows; i++, pRow++ )
		m_descriptorCollector.Delete( pRow->rid );

	return OK;
}


STATUS 
LunMapManager::SrcTableRowModified( void *pRows_, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_LM_SRC_TABLE;
		return OK;
	}

	StorageRollCallRecord	*pRow = (StorageRollCallRecord *)pRows_;

	for( U32 i = 0; i < numberOfRows; i++, pRow++ ){
		m_descriptorCollector.Delete( pRow->rid );
		m_descriptorCollector.Add( pRow, sizeof(StorageRollCallRecord) );
	}

	return OK;
}


//************************************************************************
// PTS callbacks for ExportTableUserInfo Table
//************************************************************************

STATUS 
LunMapManager::UserInfoRowInserted( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_LM_EXPORT_USER_INFO_TABLE;
		return OK;
	}

	ExportTableUserInfoRecord	*pRow = (ExportTableUserInfoRecord *)pRows;

	for( U32 i = 0; i < numberOfRows; i++, pRow++ )
		m_descriptorCollector.Add( pRow, sizeof(ExportTableUserInfoRecord) );

	return OK;
}


STATUS 
LunMapManager::UserInfoRowDeleted( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_LM_EXPORT_USER_INFO_TABLE;
		return OK;
	}

	ExportTableUserInfoRecord	*pRow = (ExportTableUserInfoRecord *)pRows;

	for( U32 i = 0; i < numberOfRows; i++, pRow++ )
		m_descriptorCollector.Delete( pRow->rid );

	return OK;
}


STATUS 
LunMapManager::UserInfoRowModified( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_LM_EXPORT_USER_INFO_TABLE;
		return OK;
	}

	ExportTableUserInfoRecord	*pRow = (ExportTableUserInfoRecord *)pRows;

	for( U32 i = 0; i < numberOfRows; i++, pRow++ ){
		m_descriptorCollector.Delete( pRow->rid );
		m_descriptorCollector.Add( pRow, sizeof(ExportTableUserInfoRecord) );
	}

	return OK;
}



//************************************************************************
// PTS callbacks for Export Table
//************************************************************************

STATUS 
LunMapManager:: ExportTableRowInserted( void *pRows_, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_LM_EXPORT_TABLE;
		return OK;
	}
	// nothing to do, we should get the event from VCM
	BumpUpConfigId(); 
	return OK;
}


STATUS 
LunMapManager:: ExportTableRowDeleted( void *pRows_, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_LM_EXPORT_TABLE;
		return OK;
	}
	// nothing to do, we should get the event from VCM
	BumpUpConfigId();
	return OK;
}


STATUS 
LunMapManager:: ExportTableRowModified( void *pRows_, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_LM_EXPORT_TABLE;
		return OK;
	}
	
	DesignatorId				id;
	LunMapEntry					*pLun;
	U32							index;
	ExportTableEntry			*pRow = (ExportTableEntry *) pRows_;

	SSAPI_TRACE( TRACE_L3, "\nLunMapManager::ExportTableRowModified" );

	for( index = 0; index < numberOfRows; index++, pRow++ ){
		id = DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_LUN_MAP_ENTRY );
		pLun = (LunMapEntry *)GetManagedObject( &id );
		if( pLun ){
			if((pLun->GetRidStatus() !=  pRow->ridStatusRec) ||
				(pLun->GetRidPerformance() != pRow->ridPerformanceRec) ){
				pLun->BuildYourselfFromExportTableRow( pRow );
				BuildPhsDataIdVector( pLun, true );
			}
			else
				pLun->BuildYourselfFromExportTableRow( pRow );
			GetListenManager()->PropagateObjectModifiedEventForManagers( GetDesignatorId(), pLun );
		}
	}
	
	BumpUpConfigId();


	return OK;
}


//************************************************************************
// ObjectAddedEventCallback:
//
// PURPOSE:		This is a callback method for all OBJECT_ADDED events 
//				coming from the listen manager.
//************************************************************************

void 
LunMapManager::ObjectAddedEventCallback( ValueSet *pVs, bool isLast, int eventObjectId ){
}


//************************************************************************
// ObjectModifiedEventCallback:
//
// PURPOSE:		This is a callback method for all OBJECT_MODIFIED events 
//				coming from the listen manager.
//************************************************************************

void 
LunMapManager::ObjectModifiedEventCallback( ValueSet *pVs, bool isLast, int eventObjectId ){
}


//************************************************************************
// CreateObjectsFromExportTableRow:
//
// PURPOSE:		Creates a lun map entry object off an export table row
//************************************************************************

bool 
LunMapManager::CreateObjectsFromExportTableRow(	ExportTableEntry	*pRow, 
												Container			&container ){

	LunMapEntry			*pLun = new LunMapEntry(GetListenManager(), this );

	pLun->BuildYourselfFromExportTableRow(	pRow,
											m_descriptorCollector,
											m_idInfoCollector,
											(DdmSSAPI &)*pParentDdmSvs);
	BuildPhsDataIdVector( pLun );

	ReadEntryString( pLun->GetRidDescription(), LM_DESCRIPTION, *pLun );
	ReadEntryString( pLun->GetRidName(), LM_NAME, *pLun );
	container.Add( (CONTAINER_ELEMENT)pLun );
	return true;
}


//************************************************************************
// GetShadowTable:
//
// PURPOSE:		Performs a look up of the table by the table mask
//************************************************************************

ShadowTable* 
LunMapManager::GetShadowTable( U32 tableMask ){

	for( U32 i = 0; i < SSAPI_LM_NUMBER_OF_TABLES_USED; i++ )
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
LunMapManager::GetTempTableData( U32 tableMask ){

	for( U32 i = 0; i < SSAPI_LM_NUMBER_OF_TABLES_USED; i++ )
		if( m_pTable[i].tableMask == tableMask )
			return m_pTable[i].pTempTable;


	ASSERT(0);
	return NULL;
}


//************************************************************************
// ModifyObject:
//
// PURPOSE:		Responsible for modifying some info of a lun map entry
//				(like name, description)
//************************************************************************

bool 
LunMapManager::ModifyObject( ValueSet &, SsapiResponder * ){

	// WTD is this method???
	ASSERT(0);

	return true;
}


//************************************************************************
// ModifyObjectStringsCallback:
//
// PURPOSE:		Called by the string resource manager to inform the object
//				that string resources have been updated
//************************************************************************

STATUS 
LunMapManager::ModifyObjectStringsCallback( void *pContext, STATUS rc ){

	SsapiResponder *pResponder = (SsapiResponder *)pContext;

	if( rc == OK )
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	else
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);

	return OK;
}


//************************************************************************
// ModifyLunMap:
//
// PURPOSE:		Modifies LunMap info. 
//************************************************************************

bool 
LunMapManager::ModifyLunMap( ValueSet &objectValues, SsapiResponder *pResponder ){

	UnicodeString	name, description;
	LunMapEntry		*pLun = new LunMapEntry( GetListenManager(), this );
	int				rc = 0;
	DesignatorId	id;

	*pLun = objectValues;
	pLun->BuildYourselfFromYourValueSet();

	if( !CheckForDuplicateExportInfo( pLun, pResponder ) )
		return true;

	id = pLun->GetDesignatorId();
	delete pLun;
	pLun = (LunMapEntry *)GetManagedObject( &id );

	if( objectValues.GetString( SSAPI_LUN_MAP_ENTRY_FID_NAME, &name ) )
		if( !UpdateEntryName( *pLun, name, pResponder ) )
			return true;
	

	if( objectValues.GetString( SSAPI_LUN_MAP_ENTRY_FID_DESCRIPTION, &description ) )
		if( !UpdateEntryDescription( *pLun, description, pResponder ) )
			return true;

	return true;
}


//************************************************************************
// ModifyLunMapReplyCallback:
//
// PURPOSE:		Called by the lun map entry to inform the manager 
//				the strings have been stored
//************************************************************************

STATUS 
LunMapManager::ModifyLunMapReplyCallback( void *pContext, STATUS rc ){

	SsapiResponder	*pResponder = (SsapiResponder *)pContext;
	
	RemoveOutstandingRequest();

	if( rc == OK )
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	else
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );

	return OK;
}


//************************************************************************
// DeleteLunMap:
// 
// PURPOSE:		Deletes LunMap entry
//************************************************************************

bool 
LunMapManager::DeleteLunMap( DesignatorId id, SsapiResponder *pResponder ){

	LunMapEntry		*pLun = (LunMapEntry *)GetManagedObject( &id );
	VCRequest		request;
	VCDeleteCommand	*pCmd = &request.u.VCDeleteParms;

	memset( &request, 0, sizeof(request) );
	pCmd->ridVcId = pLun->GetRidHandle().GetRowID();
	request.eCommand		= k_eDeleteVC;

	m_pCmdSender->csndrExecute(	&request,
								(pCmdCompletionCallback_t)METHOD_ADDRESS(LunMapManager, DeleteLunMapCallback),
								pResponder );
	AddOutstandingRequest();
	return true;
}


void 
LunMapManager::DeleteLunMapCallback(STATUS			completionCode,
									void			*pResultData,
									void			*pCmdData,
									void			*pCmdContext ){

	SsapiResponder	*pResponder = (SsapiResponder *)pCmdContext;
	VCStatus		*pStatus	= (VCStatus *)pResultData;

	if( pStatus->status == OK ){
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	}
	else{
		// TBDGAI dsetailed status
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );
	}
	RemoveOutstandingRequest();
}


//************************************************************************
// CheckForDuplicateExportInfo:
//
// PURPOSE:		Checks for duplicate data:
//				-- LUN and ID
//				-- storage ID
//				If a duplicate is detected, replies with an error. else
//				does not reply
//
// RETURN:		true:		may proceed
//				false:		replied with an error
//************************************************************************

bool 
LunMapManager::CheckForDuplicateExportInfo( LunMapEntry *, SsapiResponder * ){
#if 0
	LunMapEntry		*pObj;
	U32				i;

	pLun->BuildYourselfFromYourValueSet();

	for( i = 0; i < GetManagedObjectCount(); i++ ){
		pObj = (LunMapEntry *)GetManagedObject( i );
		if( pLun->GetStorageElementId() == pObj->GetStorageElementId() ){
			pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER , CTS_SSAPI_INVALID_PARAM_EXCEPTION_STORAGE_IN_USE);
			return false;
		}

		// this check is not exact, we need to do it on the loop basis! TBDGAI
		if( (pLun->GetTargetLun() == pObj->GetTargetLun() ) && (pLun->GetTargetId() == pObj->GetTargetId() ) ){
			pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER , CTS_SSAPI_INVALID_PARAM_EXCEPTION_LUN_ID_IN_USE);
			return false;
		}
	}
#endif
	return true;
}


//************************************************************************
// ReadEntryString:
//
// PURPOSE:		Reads in the string associated with the entry
//************************************************************************

void 
LunMapManager::ReadEntryString( RowId rid, LM_STRING_TYPE type, LunMapEntry &entry ){

	LM_READ_STRING_CELL		*pCell;
	bool					rc;

	if( rid.IsClear() == false ){
		pCell = new LM_READ_STRING_CELL;

		pCell->pThis	= this;
		pCell->entryId	= entry.GetDesignatorId();
		pCell->stringType = type;	

		rc = m_pStringResourceManager->ReadString(	&pCell->string,
													rid,
													(pTSCallback_t)METHOD_ADDRESS(LunMapManager, ReadEntryStringCallback),
													pCell );
		if( rc )
			AddOutstandingRequest();
	}
}


STATUS 
LunMapManager::ReadEntryStringCallback( void *pContext, STATUS status ){
	
	LM_READ_STRING_CELL	*pCell = (LM_READ_STRING_CELL *)pContext;
	LunMapEntry			*pEntry;

	if( status == OK ){
		pEntry = (LunMapEntry *)pCell->pThis->GetManagedObject( &pCell->entryId );

		switch( pCell->stringType ){
			case LM_NAME:
				pEntry->SetName( pCell->string );
				break;

			case LM_DESCRIPTION:
				pEntry->SetDescription( pCell->string );
				break;

			default:
				ASSERT(0);
				break;
		}
	}

	pCell->pThis->RemoveOutstandingRequest();
	delete pCell;
	return OK;
}

//************************************************************************
//
//************************************************************************

void 
LunMapManager::RemoveOutstandingRequest(){ 

	if( m_requestsOutstanding )
		if( !--m_requestsOutstanding ) 
			SetIsReadyToServiceRequests( true ); 
}


//************************************************************************
// UpdateEntryName:
//
// PURPOSE:		Updates entry's name
//************************************************************************

bool 
LunMapManager::UpdateEntryName( LunMapEntry &entry, UnicodeString &newName, SsapiResponder *pResponder ){


	LM_WRITE_STRING_CELL		*pCell;
	bool						rc;


	if( !newName.GetLength() ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_NAME_IS_BLANK );
		return false;
	}

	if( entry.GetRidName().IsClear() == false )
		m_pStringResourceManager->DeleteString(	entry.GetRidName(),
												(pTSCallback_t)METHOD_ADDRESS(LunMapManager, DummyCallback),
												NULL );
	
	pCell = new LM_WRITE_STRING_CELL;
	pCell->pThis	= this;
	pCell->entryId	= entry.GetDesignatorId();
	pCell->string	= newName;
	pCell->pResponder = pResponder;

	rc = m_pStringResourceManager->WriteString(	newName,
												&pCell->ridString,
												(pTSCallback_t)METHOD_ADDRESS(LunMapManager, UpdateEntryNameCallback),
												pCell );

	if( rc )
		AddOutstandingRequest();
	else{
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);
		delete pCell;
		return false;
	}

	return true;
}


STATUS 
LunMapManager::UpdateEntryNameCallback( void *pContext, STATUS status ){
	
	LM_WRITE_STRING_CELL	*pCell = (LM_WRITE_STRING_CELL *)pContext;
	LunMapEntry				*pEntry = (LunMapEntry *)GetManagedObject( &pCell->entryId );

	if( status == OK ){
		if( pEntry ){
			pEntry->SetName( pCell->string );
			pEntry->SetRidName( pCell->ridString );
			pCell->pThis->GetShadowTable( SSAPI_LM_EXPORT_USER_INFO_TABLE )
				->ModifyField(	pEntry->GetRidUserInfo(),
								TFN_ETUI_RIDNAME,
								&pCell->ridString,
								sizeof(pCell->ridString),
								(pTSCallback_t)METHOD_ADDRESS(LunMapManager, DummyCallback),
								NULL );
		}
		pCell->pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	}
	else
		pCell->pResponder->RespondToRequest(SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);

	pCell->pThis->RemoveOutstandingRequest();
	delete pCell;
	return OK;
}


//************************************************************************
// UpdateEntryDescription;
//
// PURPOSE:		Updates entry's description
//************************************************************************

bool 
LunMapManager::UpdateEntryDescription( LunMapEntry &entry, UnicodeString &description, SsapiResponder *pResponder ){

	LM_WRITE_STRING_CELL		*pCell;
	bool						rc;


	if( !description.GetLength() ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_DESCRIPTION_IS_BLANK );
		return false;
	}

	if( entry.GetRidDescription().IsClear() == false )
		m_pStringResourceManager->DeleteString(	entry.GetRidDescription(),
												(pTSCallback_t)METHOD_ADDRESS(LunMapManager, DummyCallback),
												NULL );
	
	pCell = new LM_WRITE_STRING_CELL;
	pCell->pThis	= this;
	pCell->entryId	= entry.GetDesignatorId();
	pCell->string	= description;
	pCell->pResponder = pResponder;

	rc = m_pStringResourceManager->WriteString(	description,
												&pCell->ridString,
												(pTSCallback_t)METHOD_ADDRESS(LunMapManager, UpdateEntryDescriptionCallabck),
												pCell );

	if( rc )
		AddOutstandingRequest();
	else{
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);
		delete pCell;
		return false;
	}

	return true;
}


STATUS 
LunMapManager::UpdateEntryDescriptionCallabck( void *pContext, STATUS status ){

	LM_WRITE_STRING_CELL	*pCell = (LM_WRITE_STRING_CELL *)pContext;
	LunMapEntry				*pEntry = (LunMapEntry *)GetManagedObject( &pCell->entryId );

	if( status == OK ){
		if( pEntry ){
			pEntry->SetDescription( pCell->string );
			pEntry->SetRidDescription( pCell->ridString );
			pCell->pThis->GetShadowTable( SSAPI_LM_EXPORT_USER_INFO_TABLE )
				->ModifyField(	pEntry->GetRidUserInfo(),
								TFN_ETUI_RIDDESCRIPTION,
								&pCell->ridString,
								sizeof(pCell->ridString),
								(pTSCallback_t)METHOD_ADDRESS(LunMapManager, DummyCallback),
								NULL );
		}
		pCell->pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	}
	else
		pCell->pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );

	pCell->pThis->RemoveOutstandingRequest();
	delete pCell;
	return OK;
}


//************************************************************************
// ExportLun:
//
// PURPOSE:		Attempts to export the lun entry specified. 
//				Always responds.
//************************************************************************

void 
LunMapManager::ExportLun( DesignatorId &lunId, bool shouldExport, SsapiResponder *pResponder ){

	LunMapEntry			*pLun	= (LunMapEntry *)GetManagedObject( &lunId );
	VCRequest			request;
	VCExportCtlCommand	*pCmd = (VCExportCtlCommand *)&request.u.VCExportParms;

	if( !pLun ){
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_OBJECT_DOES_NOT_EXIST_EXCEPTION);
		return;
	}

	if( pLun->GetIsExported() == shouldExport ){
		pResponder->RespondToRequest(	SSAPI_EXCEPTION_INVALID_PARAMETER,
										shouldExport? 
											CTS_SSAPI_LUN_ALREADY_EXPORTED
											:
											CTS_SSAPI_LUN_ALREADY_UNEXPORTED);
		return;
	}

	request.eCommand		= k_eVCExportCtl;
	pCmd->fAllVCs			= false;
	pCmd->ridVcId			= pLun->GetRidHandle().GetRowID();
	pCmd->fExport			= shouldExport;
	AddOutstandingRequest();
	m_pCmdSender->csndrExecute(	&request,
								(pCmdCompletionCallback_t)METHOD_ADDRESS(LunMapManager, ExportLunCallback),
								pResponder );
}


void 
LunMapManager::ExportLunCallback(	STATUS			completionCode,
									void			*pResultData,
									void			*pCmdData,
									void			*pCmdContext ){

	SsapiResponder		*pResponder = (SsapiResponder *)pCmdContext;
	VCStatus			*pStatus	= (VCStatus *)pResultData;

	if( pStatus->status == OK )
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	else{
		// TBDGAI detailed status
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );
	}

	RemoveOutstandingRequest();
}


//************************************************************************
// VCMEventHandler:
//
// PURPOSE:		Called by the VCM to inform of async' events
//************************************************************************

void 
LunMapManager::VCMEventHandler( STATUS eventCode, void *pEventData ){

	VCStatus			*pStatus = (VCStatus *)pEventData;
	SList				container;
	ExportTableEntry	*pEntry;
	LunMapEntry			*pLun;

	switch( pStatus->eVCEvent ){

		case VCCreated:
			SSAPI_TRACE( TRACE_L2, "\nLunMapManager: Event from VCM -> Circuit created" );

			pEntry	 = (ExportTableEntry *)&pStatus->u.VCCreateResults.VCExportRecRet[0];
			CreateObjectsFromExportTableRow( pEntry, container );
			container.GetAt( (CONTAINER_ELEMENT &)pLun, 0 );
			AddObjectsIntoManagedObjectsVector( container );
			pLun->SetRidHandle( pStatus->ridVcId );
			break;

		case VCDeleted:
			SSAPI_TRACE( TRACE_L2, "\nLunMapManager: Event from VCM -> Circuit deleted" );

			pLun = GetEntryByRidHandle( pStatus->ridVcId );
			ASSERT( pLun );
			if( pLun ){
				container.Add( (CONTAINER_ELEMENT)pLun );
				DeleteObjectsFromTheSystem( container );
			}
			break;

		case VCQuiesced:
			SSAPI_TRACE( TRACE_L2, "\nLunMapManager: Event from VCM -> Circuit quiesced" );

			pLun = GetEntryByRidHandle( pStatus->ridVcId );
			ASSERT( pLun );
			if( pLun ){
				pLun->SetIsQuiesced( true );
			}
			break;

		case VCUnQuiesced:
			SSAPI_TRACE( TRACE_L2, "\nLunMapManager: Event from VCM -> Circuit unquiesced" );

			pLun = GetEntryByRidHandle( pStatus->ridVcId );
			ASSERT( pLun );
			if( pLun ){
				pLun->SetIsQuiesced( false );
			}
			break;

		case VCExported:
			SSAPI_TRACE( TRACE_L2, "\nLunMapManager: Event from VCM -> Circuit exported" );

			pLun = GetEntryByRidHandle( pStatus->ridVcId );
			ASSERT( pLun );
			if( pLun )
				pLun->SetIsExported( true );

			break;
		
		case VCUnExported:
			SSAPI_TRACE( TRACE_L2, "\nLunMapManager: Event from VCM -> Circuit unexported" );

			pLun = GetEntryByRidHandle( pStatus->ridVcId );
			ASSERT( pLun );
			if( pLun )
				pLun->SetIsExported( false );

			break;

		default:
			ASSERT(0);
			break;
	}
}


//************************************************************************
// GetEntryByRidHandle:
//
// PURPOSE:		Performs a look up of entries by the VCM rid handle
//************************************************************************

LunMapEntry* 
LunMapManager::GetEntryByRidHandle( RowId ridHandle ){

	LunMapEntry		*pObj;
	U32				index;

	for( index = 0; index < GetManagedObjectCount(); index++ ){
		pObj	= (LunMapEntry *)GetManagedObject( index );
		if( pObj->GetRidHandle() == ridHandle )
			return pObj;
	}

	ASSERT(0);
	return NULL;
		
}


//************************************************************************
// BuildPhsDataIdVector:
//
// PURPOSE:		Builds the PHS id vector for the element
//************************************************************************

void 
LunMapManager::BuildPhsDataIdVector( LunMapEntry *pElement, bool alwaysRebuild ){

	ManagedObject		*pOldObject;
	DesignatorId		id = pElement->GetDesignatorId(), *pId;
	PHSDataManager		*pMgr = (PHSDataManager *)GetObjectManager( SSAPI_MANAGER_CLASS_TYPE_PHS_DATA_MANAGER );
	SList				container, rowidContainer;
	RowId				*pRowId;

	if( alwaysRebuild )
		pElement->PurgeAllPhsIds();

	if( ((!alwaysRebuild) && (pOldObject = GetManagedObject( &id ) ) != NULL )){
		// this object has been created before
		if( pOldObject->GetPhsDataItemCount() != pElement->GetPhsDataItemCount() )
			RecoverPhsIds( pElement );
	}
	else{
		// this is the first time the object has been created
		pElement->GetPhsDataRowId( rowidContainer );
		while( rowidContainer.Count() ){
			rowidContainer.GetAt( (CONTAINER_ELEMENT &)pRowId, 0 );
			pMgr->GetObjectIdsByRowId( container, *pRowId );
			while( container.Count() ){
				container.GetAt( (CONTAINER_ELEMENT &)pId, 0 );
				container.RemoveAt( 0 );
				pElement->AddPhsDataItem( *pId, alwaysRebuild? false : true );
				delete pId;
			}
			delete pRowId;
			rowidContainer.RemoveAt(0);
		}
	}
}


//************************************************************************
// DoesBelongToThisElement:
//
// PURPOSE:		Checks if a given phs object id is claimed by the 
//				storage element specified.
//************************************************************************

bool 
LunMapManager::DoesBelongToThisElement( LunMapEntry *pElement, DesignatorId& id ){

	SList				container;
	RowId				*pRowId;
	bool				rc = false;

	pElement->GetPhsDataRowId( container );
	while( container.Count() ){
		container.GetAt( (CONTAINER_ELEMENT &)pRowId, 0 );
		container.RemoveAt( 0 );
		if( id.GetRowId() == *pRowId )
			rc = true;

		delete pRowId;
	}
	return rc;
}


//************************************************************************
// PTS callbacks for IdInfo Table
//************************************************************************

STATUS 
LunMapManager::IdInfoTableRowInserted( void *pRows_, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_LM_ID_INFO_TABLE;
		return OK;
	}

	StsData			*pRow = (StsData *)pRows_;
	U32				i;

	for( i = 0; i < numberOfRows; i++, pRow++ )
		m_idInfoCollector.Add( pRow, sizeof(StsData) );

	return OK;
}


STATUS 
LunMapManager::IdInfoTableRowDeleted( void *pRows_, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_LM_ID_INFO_TABLE;
		return OK;
	}  

	StsData			*pRow = (StsData *)pRows_;
	U32				i;

	for( i = 0; i < numberOfRows; i++, pRow++ )
		m_idInfoCollector.Delete( pRow->rid );

	return OK;
}


STATUS 
LunMapManager::IdInfoTableRowModified( void *pRows_, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_LM_ID_INFO_TABLE;
		return OK;
	}

	StsData			*pRow = (StsData *)pRows_;
	U32				i;
	LunMapEntry		*pLun;

	for( i = 0; i < numberOfRows; i++, pRow++ ){
		m_idInfoCollector.Delete( pRow->rid );
		m_idInfoCollector.Add( pRow, sizeof(StsData) );
		if( (pLun = GetLunByVd(pRow->vdSTS) ) != NULL )
			pLun->BuildYourIdInfo( pRow, true );
	}

	return OK;
}


//************************************************************************
// GetLunByVd:
//
// PURPOSE:		Searches existing LUNs for the one whose Next VD is 
//				specified.
//
// RETURN:		success:	ptr the object
//				failure:	NULL
//************************************************************************

LunMapEntry* 
LunMapManager::GetLunByVd( VDN vd ){

	LunMapEntry		*pLun;
	U32				i;

	for( i = 0; i < GetManagedObjectCount(); i++ ){
		pLun = (LunMapEntry *)GetManagedObject( i );
		if( pLun->GetNextVd() == vd )
			return pLun;
	}
	return NULL;
}


//************************************************************************
// IsThisPathInUse:
//
// PURPOSE:		Checks if any of the lun entries are exportes on the data 
//				path specified .
//************************************************************************

bool 
LunMapManager::IsThisPathInUse( const DesignatorId &pathId ){

	LunMapEntry		*pLun;

	for( U32 i = 0; i < GetManagedObjectCount(); i++ ){
		pLun = (LunMapEntry *)GetManagedObject( i );
		if( pLun->GetHostConnectionId() == pathId )
			return true;
	}

	return false;
}