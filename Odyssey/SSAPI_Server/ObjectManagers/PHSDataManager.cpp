//************************************************************************
// FILE:		PHSDataManager.cpp
//
// PURPOSE:		Implements class PHSDataManager that will be responsible
//				for all PHS data items in the SSAPI layer. 
//************************************************************************



#include "PHSDataManager.h"
#include "ShadowTable.h"
#include "ListenManager.h"
#include "DesignatorId.h"
#include "SSAPITypes.h"
#include "CoolVector.h"
#include "SSAPIEvents.h"
#include "ArrayPerformanceTable.h"
#include "ArrayStatusTable.h"
#include "SsdStatusTable.h"
#include "SsdPerformanceTable.h"
#include "PHSDataSpecificObjects.h"
#include "STSPerfTable.h"
#include "STSStatTable.h"

#include "Trace_Index.h"
#include "Odyssey_Trace.h"
#ifdef TRACE_INDEX
#undef TRACE_INDEX
#endif
#define TRACE_INDEX TRACE_SSAPI_MANAGERS

PHSDataManager* PHSDataManager::m_pThis	= NULL;

//************************************************************************
// PHSDataManager:
//
// PURPOSE:		Default constructor
//************************************************************************

PHSDataManager::PHSDataManager( ListenManager *pListenManager, DdmServices *pParent )
				:ObjectManager( pListenManager, DesignatorId( RowId(), SSAPI_MANAGER_CLASS_TYPE_PHS_DATA_MANAGER ), pParent ){
	


	m_tableMask			= 0;
	m_tablesToRebuild	= 0;

	m_pTable = new SSAPI_PHS_JUMP_TABLE_RECORD[SSAPI_PHS_NUMBER_OF_TABLES_USED];

	m_pTable->pTableName					= EVC_STATUS_TABLE;
	m_pTable->tableMask						= SSAPI_PHS_EVC_TABLE;
	m_pTable->pRowInsertedCallback			= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,EvcTableRowInsertedCallback);
	m_pTable->pRowDeletedCallback			= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,EvcTableRowDeletedCallback);
	m_pTable->pRowModifiedCallback			= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,EvcTableRowModifiedCallback);
	m_pTable->pCreateObjectsFromRow			= (CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(PHSDataManager,CreateObjectsFromEvcRow);
	m_pTable->rowSize						= sizeof(EVCStatusRecord);
	m_pTable->pFieldDef						= (fieldDef*)aEvcStatusTable_FieldDefs;
	m_pTable->fieldDefSize					= cbEvcStatusTable_FieldDefs;

	(m_pTable + 1)->pTableName				= CT_IOPST_TABLE_NAME;
	(m_pTable + 1)->tableMask				= SSAPI_PHS_IOP_TABLE;
	(m_pTable + 1)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,IopTableRowInsertedCallback);
	(m_pTable + 1)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,IopTableRowDeletedCallback);
	(m_pTable + 1)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,IopTableRowModifiedCallback);
	(m_pTable + 1)->pCreateObjectsFromRow	= (CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(PHSDataManager,CreateObjectsFromIopRow);
	(m_pTable + 1)->rowSize					= sizeof(IOPStatusRecord);
	(m_pTable + 1)->pFieldDef				= (fieldDef*)aIopStatusTable_FieldDefs;
	(m_pTable + 1)->fieldDefSize			= cbIopStatusTable_FieldDefs;

	(m_pTable + 2)->pTableName				= CT_DPT_TABLE_NAME;
	(m_pTable + 2)->tableMask				= SSAPI_PHS_DISK_PERF_TABLE;
	(m_pTable + 2)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,DiskPerfTableRowInsertedCallback);
	(m_pTable + 2)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,DiskPerfTableRowDeletedCallback);
	(m_pTable + 2)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,DiskPerfTableRowModifiedCallback);
	(m_pTable + 2)->pCreateObjectsFromRow	= (CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(PHSDataManager,CreateObjectsFromDiskPerfRow);
	(m_pTable + 2)->rowSize					= sizeof(DiskPerformanceRecord);
	(m_pTable + 2)->pFieldDef				= (fieldDef*)aDiskPerformanceTable_FieldDefs;
	(m_pTable + 2)->fieldDefSize			= cbDiskPerformanceTable_FieldDefs;

	(m_pTable + 3)->pTableName				= CT_DST_TABLE_NAME;
	(m_pTable + 3)->tableMask				= SSAPI_PHS_DISK_STATUS_TABLE;
	(m_pTable + 3)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,DiskStatusTableRowInsertedCallback);
	(m_pTable + 3)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,DiskStatusTableRowDeletedCallback);
	(m_pTable + 3)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,DiskStatusTableRowModifiedCallback);
	(m_pTable + 3)->pCreateObjectsFromRow	= (CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(PHSDataManager,CreateObjectsFromDiskStatusRow);
	(m_pTable + 3)->rowSize					= sizeof(DiskStatusRecord);
	(m_pTable + 3)->pFieldDef				= (fieldDef*)aDiskStatusTable_FieldDefs;
	(m_pTable + 3)->fieldDefSize			= cbDiskStatusTable_FieldDefs;
	
	(m_pTable + 4)->pTableName				= CT_ARRAYPT_TABLE;
	(m_pTable + 4)->tableMask				= SSAPI_PHS_ARRAY_PERF_TABLE;
	(m_pTable + 4)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,TableRowInsertedCallback);
	(m_pTable + 4)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,TableRowDeletedCallback);
	(m_pTable + 4)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,TableRowModifiedCallback);
	(m_pTable + 4)->pCreateObjectsFromRow	= (CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(PHSDataManager,CreateObjectsFromArrayPerfRow);
	(m_pTable + 4)->rowSize					= sizeof(ArrayPerformanceRecord);
	(m_pTable + 4)->pFieldDef				= (fieldDef*)aArrayPerformanceTable_FieldDefs;
	(m_pTable + 4)->fieldDefSize			= cbArrayPerformanceTable_FieldDefs;

	(m_pTable + 5)->pTableName				= CT_ARRAYST_TABLE;
	(m_pTable + 5)->tableMask				= SSAPI_PHS_ARRAY_STATUS_TABLE;
	(m_pTable + 5)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,TableRowInsertedCallback);
	(m_pTable + 5)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,TableRowDeletedCallback);
	(m_pTable + 5)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,TableRowModifiedCallback);
	(m_pTable + 5)->pCreateObjectsFromRow	= (CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(PHSDataManager,CreateObjectsFromArrayStatusRow);
	(m_pTable + 5)->rowSize					= sizeof(ArrayStatusRecord);
	(m_pTable + 5)->pFieldDef				= (fieldDef*)aArrayStatusTable_FieldDefs;
	(m_pTable + 5)->fieldDefSize			= cbArrayStatusTable_FieldDefs;

	(m_pTable + 6)->pTableName				= CT_SSDST_TABLE;
	(m_pTable + 6)->tableMask				= SSAPI_PHS_SSD_STATUS_TABLE;
	(m_pTable + 6)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,TableRowInsertedCallback);
	(m_pTable + 6)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,TableRowDeletedCallback);
	(m_pTable + 6)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,TableRowModifiedCallback);
	(m_pTable + 6)->pCreateObjectsFromRow	= (CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(PHSDataManager,CreateObjectsFromSsdStatusRow);
	(m_pTable + 6)->rowSize					= sizeof(SSDStatusRecord);
	(m_pTable + 6)->pFieldDef				= (fieldDef*)aSSDStatusTable_FieldDefs;
	(m_pTable + 6)->fieldDefSize			= cbSSDStatusTable_FieldDefs;

	(m_pTable + 7)->pTableName				= CT_SSDPT_TABLE;
	(m_pTable + 7)->tableMask				= SSAPI_PHS_SSD_PERF_TABLE;
	(m_pTable + 7)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,TableRowInsertedCallback);
	(m_pTable + 7)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,TableRowDeletedCallback);
	(m_pTable + 7)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,TableRowModifiedCallback);
	(m_pTable + 7)->pCreateObjectsFromRow	= (CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(PHSDataManager,CreateObjectsFromSsdPerfRow);
	(m_pTable + 7)->rowSize					= sizeof(SSDPerformanceRecord);
	(m_pTable + 7)->pFieldDef				= (fieldDef*)aSSDPerformanceTable_FieldDefs;
	(m_pTable + 7)->fieldDefSize			= cbSSDPerformanceTable_FieldDefs;

	(m_pTable + 8)->pTableName				= CT_STSST_TABLE_NAME;
	(m_pTable + 8)->tableMask				= SSAPI_PHS_STS_STATUS_TABLE;
	(m_pTable + 8)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,TableRowInsertedCallback);
	(m_pTable + 8)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,TableRowDeletedCallback);
	(m_pTable + 8)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,TableRowModifiedCallback);
	(m_pTable + 8)->pCreateObjectsFromRow	= (CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(PHSDataManager,CreateObjectsFromSTSStatusRow);
	(m_pTable + 8)->rowSize					= sizeof(STSStatRecord);
	(m_pTable + 8)->pFieldDef				= (fieldDef*)aSTSStatTable_FieldDefs;
	(m_pTable + 8)->fieldDefSize			= cbSTSStatTable_FieldDefs;

	(m_pTable + 9)->pTableName				= CT_STSPT_TABLE_NAME;
	(m_pTable + 9)->tableMask				= SSAPI_PHS_STS_PERF_TABLE;
	(m_pTable + 9)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,TableRowInsertedCallback);
	(m_pTable + 9)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,TableRowDeletedCallback);
	(m_pTable + 9)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(PHSDataManager,TableRowModifiedCallback);
	(m_pTable + 9)->pCreateObjectsFromRow	= (CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(PHSDataManager,CreateObjectsFromSTSPerfRow);
	(m_pTable + 9)->rowSize					= sizeof(STSPerfRecord);
	(m_pTable + 9)->pFieldDef				= (fieldDef*)aSTSPerfTable_FieldDefs;
	(m_pTable + 9)->fieldDefSize			= cbSTSPerfTable_FieldDefs;

	for( int i = 0; i < SSAPI_PHS_NUMBER_OF_TABLES_USED; i++ )
		(m_pTable + i)->pShadowTable	= new ShadowTable(	(m_pTable + i)->pTableName,
															this,
															(m_pTable + i)->pRowInsertedCallback,
															(m_pTable + i)->pRowDeletedCallback,
															(m_pTable + i)->pRowModifiedCallback,
															(m_pTable + i)->rowSize );

	SetIsReadyToServiceRequests( false );

	SSAPI_TRACE( TRACE_L2, "\nPHSDataManager: Initializing.....");
	DefineAllTables();

}


//************************************************************************
// ~PHSDataManager:
//
// PURPOSE:		The destructor
//************************************************************************

PHSDataManager::~PHSDataManager(){

	for( int i = 0; i < SSAPI_PHS_NUMBER_OF_TABLES_USED; i++ )
		delete (m_pTable + i)->pShadowTable;

	delete[] m_pTable;
}


//************************************************************************
// Dispatch:
//
// PURPOSE:		Responsible for dispatching a request. Must either let
//				the base class handle it (if that one can) or handle it
//				itself or .... fail it!
//************************************************************************

bool 
PHSDataManager::Dispatch( ValueSet *pRequestParms, U32 requestCode, SsapiResponder *pResponder){

	if( ObjectManager::Dispatch( pRequestParms, requestCode, pResponder ) )
		return true;

	return true;
}


//************************************************************************
// ObjectDeletedCallbackHandler:
//
// PURPOSE:		This method may be provided by subclasses to be notified
//				by events from other object managers. 
//************************************************************************

void 
PHSDataManager::ObjectDeletedCallbackHandler( SSAPIEvent *pEvent , bool isLast ){

}



//************************************************************************
// DefineAllTables:
//
// PURPOSE:		Defines all PTS table the manager is going to use. This is
//				done to be able to listen on subsequent changes
//************************************************************************

STATUS 
PHSDataManager::DefineAllTables(){

	STATUS status = OK;

	m_isIniting = true;

	for( int i = 0; i < SSAPI_PHS_NUMBER_OF_TABLES_USED; i++ )
		status	|= (m_pTable + i)->pShadowTable->DefineTable(	(m_pTable + i)->pFieldDef,
																(m_pTable + i)->fieldDefSize,
																32,
																(pTSCallback_t)METHOD_ADDRESS(PHSDataManager,DefineAllTablesReplyHandler),
																(void *)(m_pTable + i)->tableMask );
	if( status != OK ){
		// GAIEVENT
		SSAPI_TRACE( TRACE_L1, "\nPHSDataManager: failed to define a table(s)" );
	}
	return status;
}


//************************************************************************
// DefineAllTablesReplyHandler:
//
// PURPOSE:		Handles callback from the PTS for DefineAllTables() method
//************************************************************************

STATUS 
PHSDataManager::DefineAllTablesReplyHandler( void *pContext, STATUS rc ){
	
	U32		mask = (U32) pContext;

	m_tableMask |= mask;

	if( (m_tableMask & SSAPI_PHS_ALL_TABLES ) == SSAPI_PHS_ALL_TABLES ){
		m_tableMask = 0;
		return InitializeAllTables();
	}

	return OK;
}


//************************************************************************
// InitializeAllTables:
//
// PURPOSE:		Issues ShadowTable::Initialize() to all tables used
//************************************************************************

STATUS 
PHSDataManager::InitializeAllTables(){

	STATUS		status = OK;


	for( int i = 0; i < SSAPI_PHS_NUMBER_OF_TABLES_USED; i++ )
		status |= (m_pTable + i)->pShadowTable->Initialize(	(pTSCallback_t)METHOD_ADDRESS(PHSDataManager,TableInitializeReplyHandler), 
															(void *)(m_pTable + i)->tableMask );

	if( status != OK ){
		// GAIEVENT
		SSAPI_TRACE( TRACE_L1, "\nPHSDataManager: Failed to issue table Initialize()" );
	}

	return status;
}


//************************************************************************
// TableInitializeReplyHandler:
//
// PURPOSE:		Handles replies from the PTS, issues commands to enumerate
//				all tables used by the manager
//************************************************************************

STATUS 
PHSDataManager::TableInitializeReplyHandler( void *pContext, STATUS rc ){
	
	U32			mask = (U32) pContext;

	if( rc != OK ){
		// GAIEVENT
		SSAPI_TRACE( TRACE_L1, "\nPHSDataManager: Failed to intialize a table" );
		return OK;
	}

	m_tableMask	|= mask;

	if( (m_tableMask & SSAPI_PHS_ALL_TABLES ) == SSAPI_PHS_ALL_TABLES ){
		m_tableMask = 0;
		return EnumerateAllTables();
	}

	return OK;
}


//************************************************************************
// EnumerateAllTables:
//
// PURPOSE:		Issues requests to the PTS (thru ShadowTables) to enumerate
//				all tables used by the manager.
//************************************************************************

STATUS 
PHSDataManager::EnumerateAllTables(){

	STATUS		status = OK;

	m_tableMask	= 0;
	m_tablesToRebuild = 0;
	

	for( int i = 0; i < SSAPI_PHS_NUMBER_OF_TABLES_USED; i++ )
		status |= (m_pTable + i)->pShadowTable->Enumerate(	&(m_pTable + i)->pTempTable,
															(pTSCallback_t)METHOD_ADDRESS(PHSDataManager,EnumerateAllTablesReplyHandler),
															(void *)(m_pTable + i)->tableMask);
	if( status != OK ){
		// GAIEVENT
		SSAPI_TRACE( TRACE_L1, "\nPHSDataManager: failed to issue EnumerateTable()" );
	}

	return OK;
}


//************************************************************************
// EnumerateAllTablesReplyHandler:
//
// PURPOSE:		Responsible for handling PTS replies to the EnumerateTable
//				requests. The method must re-issue enumerate command if the
//				table has been modified to this time already. When all
//				tables have been enumerated, the method will set the
//				'readyToServiceRequests' flag and the manager is out of
//				coma.
//************************************************************************

STATUS 
PHSDataManager::EnumerateAllTablesReplyHandler( void *pContext, STATUS rc ){

	U32			mask = (U32)pContext;
	U32			index;
	CoolVector	container;

	if( rc != OK ){
		// GAIEVENT
		SSAPI_TRACE( TRACE_L1, "\nPHSDataManager: failed to enumerate a table" );
	}
	
	for( int i = 0; i < SSAPI_PHS_NUMBER_OF_TABLES_USED; i++ ){
		if( mask == (m_pTable + i)->tableMask ){
			if( m_tablesToRebuild & (m_pTable + i)->tableMask ){
				m_tablesToRebuild &= ~(m_pTable + i)->tableMask;
				delete (m_pTable + i)->pTempTable;
				return (m_pTable + i)->pShadowTable->Enumerate(	&(m_pTable + i)->pTempTable, (pTSCallback_t)METHOD_ADDRESS(PHSDataManager,EnumerateAllTablesReplyHandler), (void *)(m_pTable + i)->tableMask);
			}
			m_tableMask |= mask;
			if( rc == OK ){
				for( index = 0; index < (m_pTable + i)->pShadowTable->GetNumberOfRows(); index++ )
					(this->*(m_pTable + i)->pCreateObjectsFromRow)( ((char *)(m_pTable + i)->pTempTable) + index*(m_pTable + i)->rowSize, container );

				delete (m_pTable + i)->pTempTable;
				AddObjectsIntoManagedObjectsVector( container );
			}
			else{
				delete (m_pTable + i)->pTempTable;
				return OK;
			}
		}
	}

	if( (m_tableMask & SSAPI_PHS_ALL_TABLES) == SSAPI_PHS_ALL_TABLES ){
		m_isIniting = false;

		SSAPI_TRACE( TRACE_L2, "\nPHSDataManager: ...Done! Objects built: ", GetManagedObjectCount());
		SetIsReadyToServiceRequests( true );
	}

	return OK;
}


//************************************************************************
// CreateObjectsFromEvcRow:
//
// PURPOSE:		Creates all PHS data objects from an EVCStatusRecord.
//				Puts new objects into the container
//************************************************************************

void 
PHSDataManager::CreateObjectsFromEvcRow( EVCStatusRecord *pRow, Container &container ){

	PHSDataTemperature		*pObj;
	PHSDataVoltage			*pVoltage;
	PHSDataCurrent			*pCurrent;
	PHSDataInt				*pInt;
	U16						ownerNumberInEvc = 0, i;

	// Chassis: Exit Air Temp1
	ownerNumberInEvc = 0;
	pObj = new PHSDataTemperature( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TEMPERATURE, ownerNumberInEvc | 0x0100 ) );
	pObj->SetIsSettable( false );
	pObj->SetValue( pRow->ExitAirTemp[0] );
	pObj->SetNonCriticalThreshold( pRow->ExitTempFanNormThresh );
	pObj->SetCriticalThreashold( pRow->ExitTempFanUpThresh );
	pObj->SetName( CTS_SSAPI_PHS_DATA_NAME_CHASSIS_EXIT_TEMP1 );
	container.Add( (CONTAINER_ELEMENT) pObj );
	
	// Chassis: Exit Air Temp2
	ownerNumberInEvc = 0;
	pObj = new PHSDataTemperature( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TEMPERATURE, ownerNumberInEvc | 0x0200 ) );
	pObj->SetIsSettable( false );
	pObj->SetValue( pRow->ExitAirTemp[1] );
	pObj->SetNonCriticalThreshold( pRow->ExitTempFanNormThresh );
	pObj->SetCriticalThreashold( pRow->ExitTempFanUpThresh );
	pObj->SetName( CTS_SSAPI_PHS_DATA_NAME_CHASSIS_EXIT_TEMP2 );
	container.Add( (CONTAINER_ELEMENT) pObj );

	// fan speed
	for( ownerNumberInEvc = 1; ownerNumberInEvc <= 4; ownerNumberInEvc++ ){
		pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, ownerNumberInEvc | 0x1500 ), PHSData::RATIO ,SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT );
		pInt->SetValue( pRow->FanSpeed[ownerNumberInEvc-1] );
		pInt->SetName(  CTS_SSAPI_PHS_DATA_NAME_FAN_SPEED );
		container.Add( (CONTAINER_ELEMENT) pInt );
	}

	// PS:  combined voltage (for all 3 )
	// PS1
	ownerNumberInEvc = 5;
	pVoltage = new PHSDataVoltage( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_VOLTAGE, ownerNumberInEvc | 0x1300 ), PHSData::SNAPSHOT );
	pVoltage->SetIsSettable( false );
	pVoltage->SetValue( pRow->SMP48Voltage ); 
	pVoltage->SetName( CTS_SSAPI_PHS_NAME_SMP48VOLTAGE );
	container.Add( (CONTAINER_ELEMENT)pVoltage );
	//PS2
	ownerNumberInEvc = 6;
	pVoltage = new PHSDataVoltage( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_VOLTAGE, ownerNumberInEvc | 0x1300 ), PHSData::SNAPSHOT );
	pVoltage->SetIsSettable( false );
	pVoltage->SetValue( pRow->SMP48Voltage ); 
	pVoltage->SetName( CTS_SSAPI_PHS_NAME_SMP48VOLTAGE );
	container.Add( (CONTAINER_ELEMENT)pVoltage );
	//PS3
	ownerNumberInEvc = 7;
	pVoltage = new PHSDataVoltage( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_VOLTAGE, ownerNumberInEvc | 0x1300 ), PHSData::SNAPSHOT );
	pVoltage->SetIsSettable( false );
	pVoltage->SetValue( pRow->SMP48Voltage ); 
	pVoltage->SetName( CTS_SSAPI_PHS_NAME_SMP48VOLTAGE );
	container.Add( (CONTAINER_ELEMENT)pVoltage );

	// Disk PSs: 
	ownerNumberInEvc = 8; 
	for( i = 0; (i <= 1) && (pRow->afEvcReachable[i]); i++, ownerNumberInEvc++ ){
		// 3.3V current
		pCurrent = new PHSDataCurrent( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_VOLTAGE, ownerNumberInEvc | 0x0400 ), PHSData::SNAPSHOT );
		pCurrent->SetIsSettable(false);
		pCurrent->SetValue( pRow->DCtoDC33Current[i] );
		pCurrent->SetName(CTS_SSAPI_PHS_NAME_CURRENT33);
		container.Add( (CONTAINER_ELEMENT)pCurrent );
		// 5.0V current
		pCurrent = new PHSDataCurrent( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_VOLTAGE, ownerNumberInEvc | 0x0500 ), PHSData::SNAPSHOT );
		pCurrent->SetIsSettable(false);
		pCurrent->SetValue( pRow->DCtoDC5Current[i] );
		pCurrent->SetName(CTS_SSAPI_PHS_NAME_CURRENT5);
		container.Add( (CONTAINER_ELEMENT)pCurrent );
		//12V A current
		pCurrent = new PHSDataCurrent( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_VOLTAGE, ownerNumberInEvc | 0x0600 ), PHSData::SNAPSHOT );
		pCurrent->SetIsSettable(false);
		pCurrent->SetValue( pRow->DCtoDC12ACurrent[i] );
		pCurrent->SetName(CTS_SSAPI_PHS_NAME_CURRENT12A);
		container.Add( (CONTAINER_ELEMENT)pCurrent );
		// 12V B current
		pCurrent = new PHSDataCurrent( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_VOLTAGE, ownerNumberInEvc | 0x0700 ), PHSData::SNAPSHOT );
		pCurrent->SetIsSettable(false);
		pCurrent->SetValue( pRow->DCtoDC12BCurrent[i] );
		pCurrent->SetName(CTS_SSAPI_PHS_NAME_CURRENT12B);
		container.Add( (CONTAINER_ELEMENT)pCurrent );
		// 12V C current
		pCurrent = new PHSDataCurrent( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_VOLTAGE, ownerNumberInEvc | 0x0800 ), PHSData::SNAPSHOT );
		pCurrent->SetIsSettable(false);
		pCurrent->SetValue( pRow->DCtoDC12CCurrent[i] );
		pCurrent->SetName(CTS_SSAPI_PHS_NAME_CURRENT12C);
		container.Add( (CONTAINER_ELEMENT)pCurrent );
		// 3.3V temperature
		pObj = pObj = new PHSDataTemperature( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TEMPERATURE, ownerNumberInEvc | 0x0900 ) );
		pObj->SetIsSettable(false);
		pObj->SetValueInCelcius(pRow->DCtoDC33Temp[i]);
		pObj->SetName(CTS_SSAPI_PHS_NAME_TEMP33);
		container.Add( (CONTAINER_ELEMENT)pObj );
		// 5V temperature
		pObj = pObj = new PHSDataTemperature( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TEMPERATURE, ownerNumberInEvc | 0x0A00 ) );
		pObj->SetIsSettable(false);
		pObj->SetValueInCelcius(pRow->DCtoDC5Temp[i]);
		pObj->SetName(CTS_SSAPI_PHS_NAME_TEMP5);
		container.Add( (CONTAINER_ELEMENT)pObj );	
		// 12V A temperature
		pObj = pObj = new PHSDataTemperature( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TEMPERATURE, ownerNumberInEvc | 0x0b00 ) );
		pObj->SetIsSettable(false);
		pObj->SetValueInCelcius(pRow->DCtoDC12ATemp[i]);
		pObj->SetName(CTS_SSAPI_PHS_NAME_TEMP12A);
		container.Add( (CONTAINER_ELEMENT)pObj );	
		// 12V B temperature
		pObj = pObj = new PHSDataTemperature( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TEMPERATURE, ownerNumberInEvc  | 0x0c00) );
		pObj->SetIsSettable(false);
		pObj->SetValueInCelcius(pRow->DCtoDC12BTemp[i]);
		pObj->SetName(CTS_SSAPI_PHS_NAME_TEMP12B);
		container.Add( (CONTAINER_ELEMENT)pObj );
		// 12V C temperature
		pObj = pObj = new PHSDataTemperature( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TEMPERATURE, ownerNumberInEvc | 0x0d00 ) );
		pObj->SetIsSettable(false);
		pObj->SetValueInCelcius(pRow->DCtoDC12CTemp[i]);
		pObj->SetName(CTS_SSAPI_PHS_NAME_TEMP12C);
		container.Add( (CONTAINER_ELEMENT)pObj );
	}
	// Disk PS, reading reported by the two different EVCs
	// EVC1's readings
	ownerNumberInEvc = 8; 
	// 3.3 Voltage
	pVoltage = new PHSDataVoltage( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_VOLTAGE, ownerNumberInEvc | 0x0e00 ), PHSData::SNAPSHOT );
	pVoltage->SetIsSettable( false );
	pVoltage->SetValue(pRow->DCtoDC33Voltage);
	pVoltage->SetName(CTS_SSAPI_PHS_NAME_VOLTAGE33);
	container.Add( (CONTAINER_ELEMENT)pVoltage );
	// 5 Voltage
	pVoltage = new PHSDataVoltage( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_VOLTAGE, ownerNumberInEvc | 0x0f00 ), PHSData::SNAPSHOT );
	pVoltage->SetIsSettable( false );
	pVoltage->SetValue(pRow->DCtoDC5Voltage );
	pVoltage->SetName(CTS_SSAPI_PHS_NAME_VOLTAGE5);
	container.Add( (CONTAINER_ELEMENT)pVoltage );
	// 12V Voltage
	pVoltage = new PHSDataVoltage( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_VOLTAGE, ownerNumberInEvc | 0x1000 ), PHSData::SNAPSHOT );
	pVoltage->SetIsSettable( false );
	pVoltage->SetValue(pRow->DCtoDC12Voltage );
	pVoltage->SetName(CTS_SSAPI_PHS_NAME_VOLTAGE12);
	container.Add( (CONTAINER_ELEMENT)pVoltage );

	// EVC2's readings
	ownerNumberInEvc = 9; 
	// 3.3 Voltage
	pVoltage = new PHSDataVoltage( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_VOLTAGE, ownerNumberInEvc | 0x0e00 ), PHSData::SNAPSHOT );
	pVoltage->SetIsSettable( false );
	pVoltage->SetValue(pRow->DCtoDC33Voltage);
	pVoltage->SetName(CTS_SSAPI_PHS_NAME_VOLTAGE33);
	container.Add( (CONTAINER_ELEMENT)pVoltage );
	// 5 Voltage
	pVoltage = new PHSDataVoltage( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_VOLTAGE, ownerNumberInEvc | 0x0f00 ), PHSData::SNAPSHOT );
	pVoltage->SetIsSettable( false );
	pVoltage->SetValue(pRow->DCtoDC5Voltage );
	pVoltage->SetName(CTS_SSAPI_PHS_NAME_VOLTAGE5);
	container.Add( (CONTAINER_ELEMENT)pVoltage );
	// 12V Voltage
	pVoltage = new PHSDataVoltage( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_VOLTAGE, ownerNumberInEvc | 0x1000 ), PHSData::SNAPSHOT );
	pVoltage->SetIsSettable( false );
	pVoltage->SetValue(pRow->DCtoDC12Voltage );
	pVoltage->SetName(CTS_SSAPI_PHS_NAME_VOLTAGE12);
	container.Add( (CONTAINER_ELEMENT)pVoltage );

	// Batteries
	ownerNumberInEvc = 10; 
	for( i =0; i <= 1; i++, ownerNumberInEvc++ ){
		pObj = new PHSDataTemperature( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TEMPERATURE, ownerNumberInEvc | 0x1100 ) );
		pObj->SetIsSettable(false);
		pObj->SetValueInCelcius( pRow->BatteryTemperature[i] );
		pObj->SetName(CTS_SSAPI_PHS_NAME_BATTERY_TEMP);
		container.Add( (CONTAINER_ELEMENT) pObj );

		pCurrent = new PHSDataCurrent( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_VOLTAGE, ownerNumberInEvc | 0x1400 ), PHSData::SNAPSHOT );
		pCurrent->SetIsSettable(false);
		pCurrent->SetValue( pRow->BatteryCurrent[i] );
		pCurrent->SetName(CTS_SSAPI_PHS_NAME_BATTERY_CURRENT);
		container.Add( (CONTAINER_ELEMENT)pCurrent );
	}

}


//************************************************************************
// The following methods are PTS listener callbacks for the EVCStatus table
//
// RECEIVE:		pRows:			ptr to array with rows affected
//				numberOfRows:	count of the entries in the array
//************************************************************************

STATUS 
PHSDataManager::EvcTableRowInsertedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){

	Container		*pContainer;
	EVCStatusRecord	*pRow = (EVCStatusRecord *)pRows;
	U32				index;
	
	if( m_isIniting ){
		m_tablesToRebuild	|= SSAPI_PHS_EVC_TABLE;		
		return OK;
	}

	SSAPI_TRACE( TRACE_L3, "\nPHSDataManager::EvcTableRowInsertedCallback" );
	
	pContainer = new CoolVector;
	for( index = 0; index < numberOfRows; index++, pRow++ )
		CreateObjectsFromEvcRow( pRow, *pContainer );

	AddObjectsIntoManagedObjectsVector( *pContainer );

	delete pContainer;
	return OK;
}



STATUS 
PHSDataManager::EvcTableRowDeletedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild	|= SSAPI_PHS_EVC_TABLE;		
		return OK;
	}

	Container		*pContainer = new CoolVector;
	EVCStatusRecord	*pRow = (EVCStatusRecord *)pRows;
	U32				index;
	PHSData			*pObj;

	SSAPI_TRACE( TRACE_L3, "\nPHSDataManager::EvcTableRowDeletedCallback" );

	for( index = 0; index < numberOfRows; index++, pRow++ )
		CreateObjectsFromEvcRow( pRow, *pContainer );

	DeleteObjectsFromTheSystem( *pContainer );
	
	// clean-up
	while( pContainer->Count() ){
		pContainer->GetAt( (CONTAINER_ELEMENT &)pObj, 0 );
		pContainer->RemoveAt( 0 );
		delete pObj;
	}

	delete pContainer;
	return OK;
}



STATUS 
PHSDataManager::EvcTableRowModifiedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){
	
	if( m_isIniting ){
		m_tablesToRebuild	|= SSAPI_PHS_EVC_TABLE;		
		return OK;
	}
	SSAPI_TRACE( TRACE_L3, "\nPHSDataManager::EvcTableRowModifiedCallback" );

	return EvcTableRowInsertedCallback( pRows, numberOfRows, NULL );
} 


//************************************************************************
// ObjectAddedEventCallback:
//
// PURPOSE:		This is a callback method for all OBJECT_ADDED events 
//				coming from the listen manager.
//************************************************************************

void 
PHSDataManager::ObjectAddedEventCallback( ValueSet *pVs, bool isLast, int eventObjectId ){

}


//************************************************************************
// GetAllAvailableObjectIds:
//
// PURPOSE:		Returns a vector of all available to this momemt managed 
//				objects. This method provided for other object managers
//				that need to phs data objects as their children.
//
// NOTE:		The caller must go thru all objects in the container 
//				(ptr to DesignatorId) and delete() 'em!
//************************************************************************

void 
PHSDataManager::GetAllAvailableObjectIds( Container &bag ){

	ManagedObject		*pObj;

	bag.RemoveAll();

	for( U32 index = 0; index < GetManagedObjectCount(); index ++ ){
		pObj = GetManagedObject( index );
		bag.Add( (CONTAINER_ELEMENT) new DesignatorId( pObj->GetDesignatorId() ) );
	}

}


//************************************************************************
// The following methods are PTS listener callbacks for the IOPStatus table
//
// RECEIVE:		pRows:			ptr to array with rows affected
//				numberOfRows:	count of the entries in the array
//************************************************************************

STATUS 
PHSDataManager::IopTableRowInsertedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){

	Container		*pContainer;
	IOPStatusRecord	*pRow = (IOPStatusRecord *)pRows;
	U32				index;
	
	if( m_isIniting ){
		m_tablesToRebuild	|= SSAPI_PHS_IOP_TABLE;		
		return OK;
	}

	SSAPI_TRACE( TRACE_L3, "\nPHSDataManager::IopTableRowInsertedCallback" );
	
	pContainer = new CoolVector;
	for( index = 0; index < numberOfRows; index++, pRow++ )
		CreateObjectsFromIopRow( pRow, *pContainer );

	AddObjectsIntoManagedObjectsVector( *pContainer );

	delete pContainer;
	return OK;
}


STATUS 
PHSDataManager::IopTableRowDeletedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild	|= SSAPI_PHS_IOP_TABLE;		
		return OK;
	}

	Container		*pContainer = new CoolVector;
	IOPStatusRecord	*pRow = (IOPStatusRecord *)pRows;
	U32				index;
	PHSData			*pObj;

	SSAPI_TRACE( TRACE_L3, "\nPHSDataManager::IopTableRowDeletedCallback" );

	for( index = 0; index < numberOfRows; index++, pRow++ )
		CreateObjectsFromIopRow( pRow, *pContainer );

	DeleteObjectsFromTheSystem( *pContainer );
	
	// clean-up
	while( pContainer->Count() ){
		pContainer->GetAt( (CONTAINER_ELEMENT &)pObj, 0 );
		pContainer->RemoveAt( 0 );
		delete pObj;
	}

	delete pContainer;
	return OK;
}


STATUS 
PHSDataManager::IopTableRowModifiedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild	|= SSAPI_PHS_IOP_TABLE;		
		return OK;
	}
	SSAPI_TRACE( TRACE_L3, "\nPHSDataManager::IopTableRowModifiedCallback" );

	return IopTableRowInsertedCallback( pRows, numberOfRows, NULL );
}


//************************************************************************
// CreateObjectsFromIopRow:
//
// PURPOSE:		Creates all PHS data objects from an IOPStatusRecord.
//				Puts new objects into the container
//************************************************************************

void 
PHSDataManager::CreateObjectsFromIopRow( IOPStatusRecord *pRow, Container &container ){

	switch( pRow->eIOPCurrentState ){
		case IOPS_UNKNOWN:
		case IOPS_EMPTY:
		case IOPS_BLANK:
			return;
	}

	PHSDataTemperature	*pObj	= new PHSDataTemperature( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TEMPERATURE ) );
	pObj->SetIsSettable( false );
	pObj->SetValue( pRow->Temp );
	pObj->SetNonCriticalThreshold( pRow->TempNormThreshold );
	pObj->SetCriticalThreashold( pRow->TempHiThreshold );

	pObj->SetName( CTS_SSAPI_PHS_DATA_NAME_CPU_TEMP );
	container.Add( (CONTAINER_ELEMENT) pObj );
}


//************************************************************************
// The following methods are PTS listener callbacks for the Disk Status
// table
//
// RECEIVE:		pRows:			ptr to array with rows affected
//				numberOfRows:	count of the entries in the array
//************************************************************************

STATUS 
PHSDataManager::DiskStatusTableRowInsertedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){

	Container			*pContainer;
	DiskStatusRecord	*pRow = (DiskStatusRecord *)pRows;
	U32					index;
	
	if( m_isIniting ){
		m_tablesToRebuild	|= SSAPI_PHS_DISK_STATUS_TABLE;		
		return OK;
	}

	SSAPI_TRACE( TRACE_L3, "\nPHSDataManager::DiskStatusTableRowInsertedCallback" );
	
	pContainer = new CoolVector;
	for( index = 0; index < numberOfRows; index++, pRow++ )
		CreateObjectsFromDiskStatusRow( pRow, *pContainer );

	AddObjectsIntoManagedObjectsVector( *pContainer );

	delete pContainer;
	return OK;
}

STATUS 
PHSDataManager::DiskStatusTableRowDeletedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild	|= SSAPI_PHS_DISK_STATUS_TABLE;		
		return OK;
	}

	Container			*pContainer = new CoolVector;
	DiskStatusRecord	*pRow = (DiskStatusRecord *)pRows;
	U32					index;
	PHSData				*pObj;

	SSAPI_TRACE( TRACE_L3, "\nPHSDataManager::DiskStatusTableRowDeletedCallback" );

	for( index = 0; index < numberOfRows; index++, pRow++ )
		CreateObjectsFromDiskStatusRow( pRow, *pContainer );

	DeleteObjectsFromTheSystem( *pContainer );
	
	// clean-up
	while( pContainer->Count() ){
		pContainer->GetAt( (CONTAINER_ELEMENT &)pObj, 0 );
		pContainer->RemoveAt( 0 );
		delete pObj;
	}

	delete pContainer;
	return OK;
}


STATUS 
PHSDataManager:: DiskStatusTableRowModifiedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild	|= SSAPI_PHS_DISK_STATUS_TABLE;		
		return OK;
	}
	SSAPI_TRACE( TRACE_L3, "\nPHSDataManager::DiskStatusTableRowModifiedCallback" );

	return DiskStatusTableRowInsertedCallback( pRows, numberOfRows, NULL );
} 


//************************************************************************
// The following methods are PTS listener callbacks for the Disk Performance
// table
//
// RECEIVE:		pRows:			ptr to array with rows affected
//				numberOfRows:	count of the entries in the array
//************************************************************************

STATUS 
PHSDataManager::DiskPerfTableRowInsertedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){

	Container				*pContainer;
	DiskPerformanceRecord	*pRow = (DiskPerformanceRecord *)pRows;
	U32						index;
	
	if( m_isIniting ){
		m_tablesToRebuild	|= SSAPI_PHS_DISK_PERF_TABLE;		
		return OK;
	}

	SSAPI_TRACE( TRACE_L3, "\nPHSDataManager::DiskPerfTableRowInsertedCallback" );
	
	pContainer = new CoolVector;
	for( index = 0; index < numberOfRows; index++, pRow++ )
		CreateObjectsFromDiskPerfRow( pRow, *pContainer );

	AddObjectsIntoManagedObjectsVector( *pContainer );

	delete pContainer;
	return OK;
}


STATUS 
PHSDataManager::DiskPerfTableRowDeletedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild	|= SSAPI_PHS_DISK_PERF_TABLE;		
		return OK;
	}

	Container				*pContainer = new CoolVector;
	DiskPerformanceRecord	*pRow = (DiskPerformanceRecord *)pRows;
	U32						index;
	PHSData					*pObj;

	SSAPI_TRACE( TRACE_L3, "\nPHSDataManager::DiskPerfTableRowDeletedCallback" );

	for( index = 0; index < numberOfRows; index++, pRow++ )
		CreateObjectsFromDiskPerfRow( pRow, *pContainer );

	DeleteObjectsFromTheSystem( *pContainer );
	
	// clean-up
	while( pContainer->Count() ){
		pContainer->GetAt( (CONTAINER_ELEMENT &)pObj, 0 );
		pContainer->RemoveAt( 0 );
		delete pObj;
	}

	delete pContainer;
	return OK;
}


STATUS 
PHSDataManager::DiskPerfTableRowModifiedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild	|= SSAPI_PHS_DISK_PERF_TABLE;		
		return OK;
	}
	SSAPI_TRACE( TRACE_L3, "\nPHSDataManager::DiskPerfTableRowModifiedCallback" );

	return DiskPerfTableRowInsertedCallback( pRows, numberOfRows, NULL );
} 


//************************************************************************
// CreateObjectsFromDiskStatusRow:
//
// PURPOSE:		Creates all PHS data objects from an DiskStatusRecord.
//				Puts new objects into the container
//************************************************************************

void 
PHSDataManager::CreateObjectsFromDiskStatusRow( DiskStatusRecord *pRow, Container &container ){

	U16					itemNumber;
	PHSDataInt			*pObj;
	PHSDataThroughput	*pObjT;
	PHSData				*pPhs;
	U32					i;

	itemNumber = 0;
	pObj	= new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pObj->SetValue( pRow->num_recoverable_media_errors_no_delay );
	pObj->SetName( CTS_SSAPI_PHS_DATA_MEDIA_ERRORS_NO_DELAY );
	container.Add( (CONTAINER_ELEMENT) pObj );

	itemNumber = 1;
	pObj	= new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pObj->SetValue( pRow->num_recoverable_media_errors_delay );
	pObj->SetName( CTS_SSAPI_PHS_DATA_MEDIA_ERRORS_DELAY );
	container.Add( (CONTAINER_ELEMENT) pObj );

	itemNumber = 2;
	pObj	= new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pObj->SetValue( pRow->num_recoverable_media_errors_by_retry );
	pObj->SetName( CTS_SSAPI_PHS_DATA_MEDIA_ERRORS_W_RETRY );
	container.Add( (CONTAINER_ELEMENT) pObj );

	itemNumber = 3;
	pObj	= new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pObj->SetValue( pRow->num_recoverable_media_errors_by_ecc );
	pObj->SetName( CTS_SSAPI_PHS_DATA_MEDIA_ERRORS_W_ECC );
	container.Add( (CONTAINER_ELEMENT) pObj );

	itemNumber = 4;
	pObj	= new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pObj->SetValue( pRow->num_recoverable_nonmedia_errors );
	pObj->SetName( CTS_SSAPI_PHS_DATA_NON_MEDIA_ERRORS );
	container.Add( (CONTAINER_ELEMENT) pObj );

	itemNumber = 5;
	pObjT	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::COUNTER );
	pObjT->SetValue( pRow->num_bytes_processed_total );
	pObjT->SetName( CTS_SSAPI_PHS_DATA_BYTES_PROCESSED_TOTAL );
	container.Add( (CONTAINER_ELEMENT) pObjT );

	itemNumber = 6;
	pObj	= new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pObj->SetValue( pRow->num_unrecoverable_media_errors );
	pObj->SetName( CTS_SSAPI_PHS_UNRECOVERABLE_MEDIA_ERRORS );
	container.Add( (CONTAINER_ELEMENT) pObj );

	for( i = 0; i < container.Count(); i++ ){
		container.GetAt( (CONTAINER_ELEMENT&)pPhs, i );
		pPhs->SetSampleRate( pRow->RefreshRate );
	}
}


//************************************************************************
// CreateObjectsFromDiskPerfRow:
//
// PURPOSE:		Creates all PHS data objects from an DiskPerformanceRecord.
//				Puts new objects into the container
//************************************************************************

void 
PHSDataManager::CreateObjectsFromDiskPerfRow( DiskPerformanceRecord *pRow, Container &container ){

	U16					itemNumber;
	PHSDataInt			*pObj;
	PHSDataTime			*pObjTime;
	PHSDataThroughput	*pObjT;
	PHSData				*pPhs;
	U32					i;

	itemNumber = 0;
	pObjTime	= new PHSDataTime( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TIME, itemNumber ), PHSData::COUNTER );
	pObjTime->SetValue( pRow->UpTime );
	pObjTime->SetName( CTS_SSAPI_PHS_DISK_UP_TIME );
	container.Add( (CONTAINER_ELEMENT)pObjTime );

	itemNumber = 1;
	pObj	= new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pObj->SetValue( pRow->AvgReadsPerSec );
	pObj->SetName( CTS_SSAPI_PHS_DISK_AVG_READS_NUMBER );
	container.Add( (CONTAINER_ELEMENT) pObj );
	
	itemNumber = 2;
	pObj	= new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pObj->SetValue( pRow->AvgWritesPerSec );
	pObj->SetName( CTS_SSAPI_PHS_DISK_AVG_WRITES_NUMBER );
	container.Add( (CONTAINER_ELEMENT) pObj );

	itemNumber = 3;
	pObj	= new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pObj->SetValue( pRow->MaxReadsPerSec );
	pObj->SetName( CTS_SSAPI_PHS_DISK_MAX_READS_NUMBER );
	container.Add( (CONTAINER_ELEMENT) pObj );

	itemNumber = 4;
	pObj	= new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pObj->SetValue( pRow->MaxWritesPerSec );
	pObj->SetName( CTS_SSAPI_PHS_DISK_MAX_WRITES_NUMBER );
	container.Add( (CONTAINER_ELEMENT) pObj );

	itemNumber = 5;
	pObj	= new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pObj->SetValue( pRow->MinReadsPerSec );
	pObj->SetName( CTS_SSAPI_PHS_DISK_MIN_READS_NUMBER );
	container.Add( (CONTAINER_ELEMENT) pObj );

	itemNumber = 6;
	pObj	= new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pObj->SetValue( pRow->MinWritesPerSec );
	pObj->SetName( CTS_SSAPI_PHS_DISK_MIN_WRITES_NUMBER );
	container.Add( (CONTAINER_ELEMENT) pObj );

	itemNumber = 7;
	pObj	= new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pObj->SetValue( pRow->AvgTransferPerSec );
	pObj->SetName( CTS_SSAPI_PHS_DISK_AVG_XFER );
	container.Add( (CONTAINER_ELEMENT) pObj );

	itemNumber = 8;
	pObj	= new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pObj->SetValue( pRow->MaxTransferPerSec );
	pObj->SetName( CTS_SSAPI_PHS_DISK_MAX_XFER );
	container.Add( (CONTAINER_ELEMENT) pObj );

	itemNumber = 9;
	pObj	= new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pObj->SetValue( pRow->MinTransferPerSec );
	pObj->SetName( CTS_SSAPI_PHS_DISK_MIN_XFER );
	container.Add( (CONTAINER_ELEMENT) pObj );

	itemNumber = 10;
	pObjT	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pObjT->SetValue( pRow->AvgBytesReadPerSec );
	pObjT->SetName( CTS_SSAPI_PHS_DISK_AVG_BYTES_READ );
	container.Add( (CONTAINER_ELEMENT) pObjT );
	
	itemNumber = 11;
	pObjT	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pObjT->SetValue( pRow->MaxBytesReadPerSec );
	pObjT->SetName( CTS_SSAPI_PHS_DISK_MAX_BYTES_READ );
	container.Add( (CONTAINER_ELEMENT) pObjT );

	itemNumber = 12;
	pObjT	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pObjT->SetValue( pRow->MinBytesReadPerSec );
	pObjT->SetName( CTS_SSAPI_PHS_DISK_MIN_BYTES_READ );
	container.Add( (CONTAINER_ELEMENT) pObjT );


	itemNumber = 13;
	pObjT	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pObjT->SetValue( pRow->AvgBytesWrittenPerSec );
	pObjT->SetName( CTS_SSAPI_PHS_DISK_AVG_BYTES_WRITTEN );
	container.Add( (CONTAINER_ELEMENT) pObjT );

	itemNumber = 14;
	pObjT	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pObjT->SetValue( pRow->MaxBytesWrittenPerSec );
	pObjT->SetName( CTS_SSAPI_PHS_DISK_MAX_BYTES_WRITTEN );
	container.Add( (CONTAINER_ELEMENT) pObjT );

	itemNumber = 16;
	pObjT	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pObjT->SetValue( pRow->MinBytesWrittenPerSec );
	pObjT->SetName( CTS_SSAPI_PHS_DISK_MIN_BYTES_WRITTEN );
	container.Add( (CONTAINER_ELEMENT) pObjT );

	itemNumber = 17;
	pObjT	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pObjT->SetValue( pRow->AvgBytesTransferredPerSec );
	pObjT->SetName( CTS_SSAPI_PHS_DISK_AVG_BYTES_XFERED );
	container.Add( (CONTAINER_ELEMENT) pObjT );

	itemNumber = 18;
	pObjT	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pObjT->SetValue( pRow->MaxBytesTransferredPerSec );
	pObjT->SetName( CTS_SSAPI_PHS_DISK_MAX_BYTES_XFERED );
	container.Add( (CONTAINER_ELEMENT) pObjT );

	itemNumber = 19;
	pObjT	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pObjT->SetValue( pRow->MinBytesTransferredPerSec );
	pObjT->SetName( CTS_SSAPI_PHS_DISK_MIN_BYTES_XFERED );
	container.Add( (CONTAINER_ELEMENT) pObjT );

	itemNumber = 20;
	pObjT	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pObjT->SetValue( pRow->AvgReadSize );
	pObjT->SetName( CTS_SSAPI_PHS_DISK_AVG_READ_SIZE );
	container.Add( (CONTAINER_ELEMENT) pObjT );

	itemNumber = 21;
	pObjT	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pObjT->SetValue( pRow->MaxReadSize );
	pObjT->SetName( CTS_SSAPI_PHS_DISK_MAX_READ_SIZE );
	container.Add( (CONTAINER_ELEMENT) pObjT );

	itemNumber = 22;
	pObjT	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pObjT->SetValue( pRow->MinReadSize );
	pObjT->SetName( CTS_SSAPI_PHS_DISK_MIN_READ_SIZE );
	container.Add( (CONTAINER_ELEMENT) pObjT );

	itemNumber = 23;
	pObjT	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pObjT->SetValue( pRow->AvgWriteSize );
	pObjT->SetName( CTS_SSAPI_PHS_DISK_AVG_WRITE_SIZE );
	container.Add( (CONTAINER_ELEMENT) pObjT );

	itemNumber = 24;
	pObjT	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pObjT->SetValue( pRow->MaxWriteSize );
	pObjT->SetName( CTS_SSAPI_PHS_DISK_MAX_WRITE_SIZE );
	container.Add( (CONTAINER_ELEMENT) pObjT );

	itemNumber = 25;
	pObjT	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pObjT->SetValue( pRow->MinWriteSize );
	pObjT->SetName( CTS_SSAPI_PHS_DISK_MIN_WRITE_SIZE );
	container.Add( (CONTAINER_ELEMENT) pObjT );

	itemNumber = 26;
	pObjT	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pObjT->SetValue( pRow->AvgTransferSize );
	pObjT->SetName( CTS_SSAPI_PHS_DISK_AVG_XFER_SIZE );
	container.Add( (CONTAINER_ELEMENT) pObjT );

	itemNumber = 27;
	pObjT	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pObjT->SetValue( pRow->MaxTransferSize );
	pObjT->SetName( CTS_SSAPI_PHS_DISK_MAX_XFER_SIZE );
	container.Add( (CONTAINER_ELEMENT) pObjT );

	itemNumber = 28;
	pObjT	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pObjT->SetValue( pRow->MinTransferSize );
	pObjT->SetName( CTS_SSAPI_PHS_DISK_MIN_XFER_SIZE );
	container.Add( (CONTAINER_ELEMENT) pObjT );

	itemNumber = 29;
	pObjTime	= new PHSDataTime( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TIME, itemNumber ), PHSData::SNAPSHOT );
	pObjTime->SetValue( pRow->AvgMicroSecPerRead );
	pObjTime->SetName( CTS_SSAPI_PHS_DISK_AVG_MICROSEC_PER_READ );
	container.Add( (CONTAINER_ELEMENT)pObjTime );

	itemNumber = 30;
	pObjTime	= new PHSDataTime( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TIME, itemNumber ), PHSData::SNAPSHOT );
	pObjTime->SetValue( pRow->MaxMicroSecPerRead );
	pObjTime->SetName( CTS_SSAPI_PHS_DISK_MAX_MICROSEC_PER_READ );
	container.Add( (CONTAINER_ELEMENT)pObjTime );

	itemNumber = 31;
	pObjTime	= new PHSDataTime( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TIME, itemNumber ), PHSData::SNAPSHOT );
	pObjTime->SetValue( pRow->MinMicroSecPerRead );
	pObjTime->SetName( CTS_SSAPI_PHS_DISK_MIN_MICROSEC_PER_READ );
	container.Add( (CONTAINER_ELEMENT)pObjTime );

	itemNumber = 32;
	pObjTime	= new PHSDataTime( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TIME, itemNumber ), PHSData::SNAPSHOT );
	pObjTime->SetValue( pRow->AvgMicroSecPerWrite );
	pObjTime->SetName( CTS_SSAPI_PHS_DISK_AVG_MICROSEC_PER_WRITE );
	container.Add( (CONTAINER_ELEMENT)pObjTime );

	itemNumber = 33;
	pObjTime	= new PHSDataTime( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TIME, itemNumber ), PHSData::SNAPSHOT );
	pObjTime->SetValue( pRow->MaxMicroSecPerWrite );
	pObjTime->SetName( CTS_SSAPI_PHS_DISK_MAX_MICROSEC_PER_WRITE );
	container.Add( (CONTAINER_ELEMENT)pObjTime );

	itemNumber = 34;
	pObjTime	= new PHSDataTime( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TIME, itemNumber ), PHSData::SNAPSHOT );
	pObjTime->SetValue( pRow->MinMicroSecPerWrite );
	pObjTime->SetName( CTS_SSAPI_PHS_DISK_MIN_MICROSEC_PER_WRITE );
	container.Add( (CONTAINER_ELEMENT)pObjTime );

	itemNumber = 35;
	pObjTime	= new PHSDataTime( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TIME, itemNumber ), PHSData::SNAPSHOT );
	pObjTime->SetValue( pRow->AvgMicroSecPerTransfer );
	pObjTime->SetName( CTS_SSAPI_PHS_DISK_AVG_MICROSEC_PER_XFER );
	container.Add( (CONTAINER_ELEMENT)pObjTime );

	itemNumber = 36;
	pObjTime	= new PHSDataTime( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TIME, itemNumber ), PHSData::SNAPSHOT );
	pObjTime->SetValue( pRow->MaxMicroSecPerTransfer );
	pObjTime->SetName( CTS_SSAPI_PHS_DISK_MAX_MICROSEC_PER_XFER );
	container.Add( (CONTAINER_ELEMENT)pObjTime );

	itemNumber = 37;
	pObjTime	= new PHSDataTime( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TIME, itemNumber ), PHSData::SNAPSHOT );
	pObjTime->SetValue( pRow->MinMicroSecPerTransfer );
	pObjTime->SetName( CTS_SSAPI_PHS_DISK_MIN_MICROSEC_PER_XFER );
	container.Add( (CONTAINER_ELEMENT)pObjTime );


	for( i = 0; i < container.Count(); i++ ){
		container.GetAt( (CONTAINER_ELEMENT&)pPhs, i );
		pPhs->SetSampleRate( pRow->SampleRate );
	}
}


//************************************************************************
// GetObjectIdsByRowId:
//
// PURPOSE:		Returns a vector of object ids that were built from the
//				row with the row id specified
//
// NOTE:		The caller must go thru all objects in the container 
//				(ptr to DesignatorId) and delete() 'em!
//************************************************************************

void 
PHSDataManager::GetObjectIdsByRowId( Container &bag, RowId &rid ){

	ManagedObject	*pObj;
	U32				i;
	DesignatorId	id;
	
	for( i = 0; i < GetManagedObjectCount(); i++ ){
		pObj = GetManagedObject( i );
		id = pObj->GetDesignatorId();
		if( id.GetRowId() == rid )
			bag.Add( (CONTAINER_ELEMENT) new DesignatorId( id ) );
	}
}


//************************************************************************
// Pts callback for the Array && SSD (Status && Performance) tables
//************************************************************************
STATUS 
PHSDataManager::TableRowInsertedCallback( void *pRows, U32 numberOfRows, ShadowTable *p ){

	Container				*pContainer;
	char					*pRow = (char *)pRows;
	U32						index;
	
	if( m_isIniting ){
		m_tablesToRebuild	|= GetJumpCellByTable(p)->tableMask;		
		return OK;
	}

	pContainer = new CoolVector;
	for( index = 0; index < numberOfRows; index++, pRow += p->GetBytesPerRow() )
		(this->*(GetJumpCellByTable(p)->pCreateObjectsFromRow))( pRow, *pContainer );

	AddObjectsIntoManagedObjectsVector( *pContainer );

	delete pContainer;
	return OK;
}


STATUS 
PHSDataManager::TableRowDeletedCallback( void *pRows, U32 numberOfRows, ShadowTable *p ){

	if( m_isIniting ){
		m_tablesToRebuild	|= GetJumpCellByTable(p)->tableMask;		
		return OK;
	}

	Container			*pContainer = new CoolVector;
	char				*pRow = (char *)pRows;
	U32					index;
	PHSData				*pObj;

	for( index = 0; index < numberOfRows; index++, pRow += p->GetBytesPerRow() )
		(this->*(GetJumpCellByTable(p)->pCreateObjectsFromRow))( pRow, *pContainer );

	DeleteObjectsFromTheSystem( *pContainer );
	
	// clean-up
	while( pContainer->Count() ){
		pContainer->GetAt( (CONTAINER_ELEMENT &)pObj, 0 );
		pContainer->RemoveAt( 0 );
		delete pObj;
	}

	delete pContainer;
	return OK;
}


STATUS 
PHSDataManager::TableRowModifiedCallback( void *pRows, U32 numberOfRows, ShadowTable *p ){

	if( m_isIniting ){
		m_tablesToRebuild	|= GetJumpCellByTable(p)->tableMask;		
		return OK;
	}

	return TableRowInsertedCallback( pRows, numberOfRows, p);
}


//************************************************************************
// CreateObjectsFromSsdPerfRow:
//
// PURPOSE:		Creates object from the SSDPerformance record
//************************************************************************

void 
PHSDataManager::CreateObjectsFromSsdPerfRow( void *pRow_, Container &container ){


	SSDPerformanceRecord	*pRow = (SSDPerformanceRecord *)pRow_;
	PHSDataInt				*pInt;
	PHSDataThroughput		*pByte;
	U16						itemNumber;
	U32						i;
	PHSData					*pPhs;

	itemNumber = 1;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->AvgNumPagesReadPerSec );
	pInt->SetName( CTS_SSAPI_PHS_SSD_AVG_PAGES_READ );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 2;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->MaxNumPagesReadPerSec );
	pInt->SetName( CTS_SSAPI_PHS_SSD_MAX_PAGES_READ );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 3;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->MinNumPagesReadPerSec );
	pInt->SetName( CTS_SSAPI_PHS_SSD_MIN_PAGES_READ );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 4;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->AvgNumPagesReadCacheHitPerSec );
	pInt->SetName( CTS_SSAPI_PHS_SSD_AVG_PAGES_READ_HIT_CACHE );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 5;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->MaxNumPagesReadCacheHitPerSec );
	pInt->SetName( CTS_SSAPI_PHS_SSD_MAX_PAGES_READ_HIT_CACHE );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 6;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->MinNumPagesReadCacheHitPerSec );
	pInt->SetName( CTS_SSAPI_PHS_SSD_MIN_PAGES_READ_HIT_CACHE );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 7;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->AvgNumPagesReadCacheMissPerSec );
	pInt->SetName( CTS_SSAPI_PHS_SSD_AVG_PAGES_READ_MISS_CACHE );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 8;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->MaxNumPagesReadCacheMissPerSec );
	pInt->SetName( CTS_SSAPI_PHS_SSD_MAX_PAGES_READ_MISS_CACHE );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 9;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->MinNumPagesReadCacheMissPerSec );
	pInt->SetName( CTS_SSAPI_PHS_SSD_MIN_PAGES_READ_MISS_CACHE );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 10;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->AvgNumPagesWritePerSec );
	pInt->SetName( CTS_SSAPI_PHS_SSD_AVG_PAGES_WRITTEN );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 11;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->MaxNumPagesWritePerSec );
	pInt->SetName( CTS_SSAPI_PHS_SSD_MAX_PAGES_WRITTEN );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 12;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->MinNumPagesWritePerSec );
	pInt->SetName( CTS_SSAPI_PHS_SSD_MIN_PAGES_WRITTEN );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 14;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->AvgNumPagesWriteCacheHitPerSec );
	pInt->SetName( CTS_SSAPI_PHS_SSD_AVG_PAGES_WRITTEN_HIT_CACHE );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 15;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->MaxNumPagesWriteCacheHitPerSec );
	pInt->SetName( CTS_SSAPI_PHS_SSD_MAX_PAGES_WRITTEN_HIT_CACHE );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 16;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->MinNumPagesWriteCacheHitPerSec );
	pInt->SetName( CTS_SSAPI_PHS_SSD_MIN_PAGES_WRITTEN_HIT_CACHE );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 17;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->AvgNumPagesWriteCacheMissPerSec );
	pInt->SetName( CTS_SSAPI_PHS_SSD_AVG_PAGES_WRITTEN_MISS_CACHE );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 18;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->MaxNumPagesWriteCacheMissPerSec );
	pInt->SetName( CTS_SSAPI_PHS_SSD_MAX_PAGES_WRITTEN_MISS_CACHE );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 19;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->MinNumPagesWriteCacheMissPerSec );
	pInt->SetName( CTS_SSAPI_PHS_SSD_MIN_PAGES_WRITTEN_MISS_CACHE );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 20;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->AvgNumErasePagesAvailable );
	pInt->SetName( CTS_SSAPI_PHS_SSD_AVG_ERASE_PAGES );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 21;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->MaxNumErasePagesAvailable );
	pInt->SetName( CTS_SSAPI_PHS_SSD_MAX_ERASE_PAGES );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 22;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->MinNumErasePagesAvailable );
	pInt->SetName( CTS_SSAPI_PHS_SSD_MIN_ERASE_PAGES );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 23;
	pByte	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pByte->SetValue( pRow->AvgNumReadBytesTotalPerSec );
	pByte->SetName( CTS_SSAPI_PHS_SSD_AVG_BYTES_READ );
	container.Add( (CONTAINER_ELEMENT) pByte );

	itemNumber = 24;
	pByte	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pByte->SetValue( pRow->MaxNumReadBytesTotalPerSec );
	pByte->SetName( CTS_SSAPI_PHS_SSD_MAX_BYTES_READ );
	container.Add( (CONTAINER_ELEMENT) pByte );

	itemNumber = 25;
	pByte	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pByte->SetValue( pRow->MinNumReadBytesTotalPerSec );
	pByte->SetName( CTS_SSAPI_PHS_SSD_MIN_BYTES_READ );
	container.Add( (CONTAINER_ELEMENT) pByte );

	itemNumber = 26;
	pByte	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pByte->SetValue( pRow->AvgNumWriteBytesTotalPerSec );
	pByte->SetName( CTS_SSAPI_PHS_SSD_AVG_BYTES_WRITTEN );
	container.Add( (CONTAINER_ELEMENT) pByte );

	itemNumber = 24;
	pByte	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pByte->SetValue( pRow->MaxNumWriteBytesTotalPerSec );
	pByte->SetName( CTS_SSAPI_PHS_SSD_MAX_BYTES_WRITTEN );
	container.Add( (CONTAINER_ELEMENT) pByte );

	itemNumber = 25;
	pByte	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pByte->SetValue( pRow->MinNumWriteBytesTotalPerSec );
	pByte->SetName( CTS_SSAPI_PHS_SSD_MIN_BYTES_WRITTEN );
	container.Add( (CONTAINER_ELEMENT) pByte );

	for( i = 0; i < container.Count(); i++ ){
		container.GetAt( (CONTAINER_ELEMENT&)pPhs, i );
		pPhs->SetSampleRate( pRow->SampleRate );
	}
}


//************************************************************************
// CreateObjectsFromSsdStatusRow:
//
// PURPOSE:		Creates object from the SSDStatus record
//************************************************************************

void 
PHSDataManager::CreateObjectsFromSsdStatusRow( void *pRow_, Container &container ){

	PHSDataInt		*pInt;
	PHSData			*pPhs;
	U16				itemNumber;
	U32				i;
	SSDStatusRecord	*pRow = (SSDStatusRecord *)pRow_;


	itemNumber = 1;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->NumReplacementPagesAvailable );
	pInt->SetName( CTS_SSAPI_PHS_SSD_NUM_REPLACEMENT_PAGES );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 2;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->PageTableSize );
	pInt->SetName( CTS_SSAPI_PHS_SSD_PAGE_TABLE_SIZE );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 3;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::RATIO );
	pInt->SetValue( pRow->PercentDirtyPages );
	pInt->SetName( CTS_SSAPI_PHS_SSD_PERCENT_DIRTY );
	container.Add( (CONTAINER_ELEMENT) pInt );

	for( i = 0; i < container.Count(); i++ ){
		container.GetAt( (CONTAINER_ELEMENT&)pPhs, i );
		pPhs->SetSampleRate( pRow->RefreshRate );
	}
}


//************************************************************************
// CreateObjectsFromArrayPerfRow:
//
// PURPOSE:		Creates object from the ArrayPerformance record
//************************************************************************

void 
PHSDataManager::CreateObjectsFromArrayPerfRow( void *pRow_, Container &container ){

	PHSDataInt				*pInt;
	PHSData					*pPhs;
	U16						itemNumber;
	U32						i;
	ArrayPerformanceRecord	*pRow = (ArrayPerformanceRecord *)pRow_;

	itemNumber = 1;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->NumReadsAveragePerSec );
	pInt->SetName( CTS_SSAPI_PHS_RAID_AVG_READS );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 2;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->NumReadsMaximumPerSec );
	pInt->SetName( CTS_SSAPI_PHS_RAID_MAX_READS );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 3;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->NumReadsMinimumPerSec );
	pInt->SetName( CTS_SSAPI_PHS_RAID_MIN_READS );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 4;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->NumWritesAveragePerSec );
	pInt->SetName( CTS_SSAPI_PHS_RAID_AVG_WRITES );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 5;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->NumWritesMaximumPerSec );
	pInt->SetName( CTS_SSAPI_PHS_RAID_MAX_WRITES );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 6;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->NumWritesMinimumPerSec );
	pInt->SetName( CTS_SSAPI_PHS_RAID_MIN_WRITES );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 7;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->NumBlocksReadAveragePerSec );
	pInt->SetName( CTS_SSAPI_PHS_RAID_AVG_BLOCK_READ );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 8;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->NumBlocksReadMaximumPerSec );
	pInt->SetName( CTS_SSAPI_PHS_RAID_MAX_BLOCK_READ );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 9;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->NumBlocksReadMinimumPerSec );
	pInt->SetName( CTS_SSAPI_PHS_RAID_MIN_BLOCK_READ );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 10;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->NumBlocksWrittenAveragePerSec );
	pInt->SetName( CTS_SSAPI_PHS_RAID_AVG_BLOCK_WRITTEN );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 11;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->NumBlocksWrittenMaximumPerSec );
	pInt->SetName( CTS_SSAPI_PHS_RAID_MAX_BLOCK_WRITTEN );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 12;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->NumBlocksWrittenMinimumPerSec );
	pInt->SetName( CTS_SSAPI_PHS_RAID_MIN_BLOCK_WRITTEN );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 13;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->NumOverwritesAveragePerSec );
	pInt->SetName( CTS_SSAPI_PHS_RAID_AVG_OVERWRITES );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 14;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->NumOverwritesMaximumPerSec );
	pInt->SetName( CTS_SSAPI_PHS_RAID_MAX_OVERWRITES );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 15;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->NumOverwritesMinimumPerSec );
	pInt->SetName( CTS_SSAPI_PHS_RAID_MIN_OVERWRITES );
	container.Add( (CONTAINER_ELEMENT) pInt );


	for( i = 0; i < container.Count(); i++ ){
		container.GetAt( (CONTAINER_ELEMENT&)pPhs, i );
		pPhs->SetSampleRate( pRow->RefreshRate );
	}
}


//************************************************************************
// CreateObjectsFromArrayPerfRow:
//
// PURPOSE:		Creates object from the ArrayStatus record
//************************************************************************

void 
PHSDataManager::CreateObjectsFromArrayStatusRow( void *pRow_, Container &container ){

	PHSDataInt				*pInt;
	PHSData					*pPhs;
	U16						itemNumber;
	U32						i;
	ArrayStatusRecord		*pRow = (ArrayStatusRecord *)pRow_;

	itemNumber = 1;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->NumRaidReassignedSuccess );
	pInt->SetName( CTS_SSAPI_PHS_RAID_SUCCESS_REASSIGN );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 2;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->NumRaidReassignedFailed );
	pInt->SetName( CTS_SSAPI_PHS_RAID_FAILED_REASSIGN );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 3;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->NumRetriesTotal );
	pInt->SetName( CTS_SSAPI_PHS_RAID_NUM_RETRIES );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 4;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->NumRecoveredErrorsTotal );
	pInt->SetName( CTS_SSAPI_PHS_RAID_NUM_RECOVERED_ERRORS );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 5;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->NumRecoveredErrorsTotal );
	pInt->SetName( CTS_SSAPI_PHS_RAID_NUM_RECOVERED_ERRORS );
	container.Add( (CONTAINER_ELEMENT) pInt );

	for( i = 0; i < container.Count(); i++ ){
		container.GetAt( (CONTAINER_ELEMENT&)pPhs, i );
		pPhs->SetSampleRate( pRow->RefreshRate );
	}
}


//************************************************************************
// CreateObjectsFromSTSPerfRow:
//
// PURPOSE:		Creates object from the STSPerformance record
//************************************************************************

void 
PHSDataManager::CreateObjectsFromSTSPerfRow( void *pRow_, Container &container ){

	STSPerfRecord			*pRow = (STSPerfRecord *)pRow_;
	PHSDataInt				*pInt;
	PHSDataThroughput		*pByte;
	U16						itemNumber;
	U32						i;
	PHSData					*pPhs;

	itemNumber = 1;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->AvgNumBSAReadsPerSec );
	pInt->SetName( CTS_SSAPI_PHS_STS_AVG_READS );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 2;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->MaxNumBSAReadsPerSec );
	pInt->SetName( CTS_SSAPI_PHS_STS_MAX_READS );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 3;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->MinNumBSAReadsPerSec );
	pInt->SetName( CTS_SSAPI_PHS_STS_MIN_READS );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 4;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->AvgNumBSAWritesPerSec );
	pInt->SetName( CTS_SSAPI_PHS_STS_AVG_WRITES );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 5;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->MaxNumBSAWritesPerSec );
	pInt->SetName( CTS_SSAPI_PHS_STS_MAX_WRITES );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 6;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->MinNumBSAWritesPerSec );
	pInt->SetName( CTS_SSAPI_PHS_STS_MIN_WRITES );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 7;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->AvgNumBSACmdsPerSec );
	pInt->SetName( CTS_SSAPI_PHS_STS_AVG_BSA_COMMANDS );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 8;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->MaxNumBSACmdsPerSec );
	pInt->SetName( CTS_SSAPI_PHS_STS_MAX_BSA_COMMANDS );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 9;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->MinNumBSACmdsPerSec );
	pInt->SetName( CTS_SSAPI_PHS_STS_MIN_BSA_COMMANDS );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 10;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->AvgNumSCSICmdsPerSec );
	pInt->SetName( CTS_SSAPI_PHS_STS_AVG_SCSI_COMMANDS );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 11;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->MaxNumSCSICmdsPerSec );
	pInt->SetName( CTS_SSAPI_PHS_STS_MAX_SCSI_COMMANDS );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 12;
	pInt = new PHSDataInt( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT, itemNumber ), PHSData::SNAPSHOT );
	pInt->SetValue( pRow->MinNumSCSICmdsPerSec );
	pInt->SetName( CTS_SSAPI_PHS_STS_MIN_SCSI_COMMANDS );
	container.Add( (CONTAINER_ELEMENT) pInt );

	itemNumber = 13;
	pByte	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pByte->SetValue( pRow->AvgNumBSABytesReadPerSec );
	pByte->SetName( CTS_SSAPI_PHS_STS_AVG_BYTES_READ );
	container.Add( (CONTAINER_ELEMENT) pByte );

	itemNumber = 14;
	pByte	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pByte->SetValue( pRow->MaxNumBSABytesReadPerSec );
	pByte->SetName( CTS_SSAPI_PHS_STS_MAX_BYTES_READ );
	container.Add( (CONTAINER_ELEMENT) pByte );

	itemNumber = 15;
	pByte	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pByte->SetValue( pRow->MinNumBSABytesReadPerSec );
	pByte->SetName( CTS_SSAPI_PHS_STS_MIN_BYTES_READ );
	container.Add( (CONTAINER_ELEMENT) pByte );

	itemNumber = 16;
	pByte	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pByte->SetValue( pRow->AvgNumBSABytesWrittenPerSec );
	pByte->SetName( CTS_SSAPI_PHS_STS_AVG_BYTES_WRITTEN );
	container.Add( (CONTAINER_ELEMENT) pByte );

	itemNumber = 17;
	pByte	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pByte->SetValue( pRow->MaxNumBSABytesWrittenPerSec );
	pByte->SetName( CTS_SSAPI_PHS_STS_MAX_BYTES_WRITTEN );
	container.Add( (CONTAINER_ELEMENT) pByte );

	itemNumber = 18;
	pByte	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pByte->SetValue( pRow->MinNumBSABytesWrittenPerSec );
	pByte->SetName( CTS_SSAPI_PHS_STS_MIN_BYTES_WRITTEN );
	container.Add( (CONTAINER_ELEMENT) pByte );

	itemNumber = 19;
	pByte	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pByte->SetValue( pRow->AvgNumBSABytesPerSec );
	pByte->SetName( CTS_SSAPI_PHS_STS_AVG_BYTES_TOTAL );
	container.Add( (CONTAINER_ELEMENT) pByte );

	itemNumber = 20;
	pByte	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pByte->SetValue( pRow->MaxNumBSABytesPerSec );
	pByte->SetName( CTS_SSAPI_PHS_STS_MAX_BYTES_TOTAL );
	container.Add( (CONTAINER_ELEMENT) pByte );

	itemNumber = 21;
	pByte	= new PHSDataThroughput( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT, itemNumber ), PHSData::SNAPSHOT );
	pByte->SetValue( pRow->MinNumBSABytesPerSec );
	pByte->SetName( CTS_SSAPI_PHS_STS_MIN_BYTES_TOTAL );
	container.Add( (CONTAINER_ELEMENT) pByte );

	for( i = 0; i < container.Count(); i++ ){
		container.GetAt( (CONTAINER_ELEMENT&)pPhs, i );
		pPhs->SetSampleRate( pRow->SampleRate );
	}
}


//************************************************************************
// CreateObjectsFromSTSStatusRow:
//
// PURPOSE:		Creates object from the STSStatus record
//************************************************************************

void 
PHSDataManager::CreateObjectsFromSTSStatusRow( void *pRow_, Container &container ){

	PHSDataInt64			*pInt64;
	PHSData					*pPhs;
	U16						itemNumber;
	U32						i;
	STSStatRecord			*pRow = (STSStatRecord *)pRow_;

	itemNumber = 1;
	pInt64 = new PHSDataInt64( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT64, itemNumber ), PHSData::SNAPSHOT );
	pInt64->SetValue( pRow->NumTimerTimeout );
	pInt64->SetName( CTS_SSAPI_PHS_STS_TIMER_TIMEOUTS );
	container.Add( (CONTAINER_ELEMENT) pInt64 );

	itemNumber = 2;
	pInt64 = new PHSDataInt64( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT64, itemNumber ), PHSData::SNAPSHOT );
	pInt64->SetValue( pRow->NumErrorRepliesReceived );
	pInt64->SetName( CTS_SSAPI_PHS_STS_ERROR_REPLIES_RECEIVED );
	container.Add( (CONTAINER_ELEMENT) pInt64 );

	itemNumber = 3;
	pInt64 = new PHSDataInt64( GetListenManager(), DesignatorId( pRow->rid, SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT64, itemNumber ), PHSData::SNAPSHOT );
	pInt64->SetValue( pRow->NumErrorRepliesSent );
	pInt64->SetName( CTS_SSAPI_PHS_STS_ERROR_REPLIES_SENT );
	container.Add( (CONTAINER_ELEMENT) pInt64 );

	for( i = 0; i < container.Count(); i++ ){
		container.GetAt( (CONTAINER_ELEMENT&)pPhs, i );
		pPhs->SetSampleRate( pRow->RefreshRate );
	}
}


//************************************************************************
// GetJumpCellByTable:
//
// PURPOSE:		Returns a jump cell for the shadow table specified
//************************************************************************

SSAPI_PHS_JUMP_TABLE_RECORD* 
PHSDataManager::GetJumpCellByTable( ShadowTable *pTable ){

	U32			i;

	for( i = 0; i < SSAPI_PHS_NUMBER_OF_TABLES_USED; i++ )
		if( (m_pTable + i)->pShadowTable == pTable )
			return (m_pTable + i);

	ASSERT(0);
	return NULL;
}
