//************************************************************************
// FILE:		DeviceManager.cpp
//
// PURPOSE:		Defines class DeviceManager the will manage all device
//				object in the server side if the SSAPI layer.
//************************************************************************

#include "DeviceManager.h"
#include "ListenManager.h"
#include "ValueSet.h"
#include "SSAPITypes.h"
#include "ShadowTable.h"
#include "SList.h"
#include "ManagedObject.h"
#include "SSAPIServerVersion.h"
#include "SSAPILocalResponder.h"
#include "SSAPIEvents.h"	
#include "PHSDataManager.h"
#include "LoopDescriptor.h"
#include "ClassTypeMap.h"
#include "SlotMap.h"
#include "FCPortDatabaseTable.h"
#include "..\..\..\include\cmdqueues\DdmHotSwapCommands.h"
#include "CmdSender.h"
#include "Kernel.h"
#include "PathDescriptor.h"
#include "DescriptorCollector.h"
#include "StorageRollCallTable.h"
#include "UserManager.h"

// devices included
#include "IopTypes.h"
#include "BoardDevices.h"
#include "Fan.h"
#include "Battery.h"
#include "BusSegment.h"
#include "Chassis.h"
#include "ChassisPowerSupply.h"
#include "DiskPowerSupply.h"
#include "HDDDevice.h"
#include "FcPort.h"
#include "PciDeviceCollection.h"
#include "EvcDeviceCollection.h"
#include "FcDeviceCollection.h"
#include "DDH.h"
#include "SNac.h"
#include "FcPortInternal.h"
#include "CordSet.h"


#include "..\..\MSL\OsHeap.h"

#include "Trace_Index.h"
#ifdef TRACE_INDEX
#undef TRACE_INDEX
#endif
#define TRACE_INDEX TRACE_SSAPI_MANAGERS

#ifndef WIN32
#include "time.h"
#endif

#define	TIMER_UPDATE_RATE_FOR_CHASSIS_DEVICE	60	// seconds

extern "C" struct tm *gmtime_r(const time_t *, struct tm *res);
extern "C" STATUS SetTime( struct tm *t );

DeviceManager* DeviceManager::m_pThis	= NULL;
static ClassTypeMap		g_map;

//************************************************************************
// DeviceManager:
//
// PURPOSE:		Default constructor
//************************************************************************

DeviceManager::DeviceManager( ListenManager *pListenManager, DdmServices *pParent )
				:ObjectManager( pListenManager, DesignatorId( RowId(), SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER), pParent) {
	
	m_builtTablesMask	= 0;
	m_pCmbQueue	= new CmdSender(	HSW_CONTROL_QUEUE,
									HSW_CONTROL_COMMAND_SIZE,
									HSW_CONTROL_STATUS_SIZE,
									this);

	m_pTable = new SSAPI_DEVICE_MGR_JUMP_TABLE_RECORD[SSAPI_DM_NUMBER_OF_TABLES_USED];

	(m_pTable)->pTableName					= SYSTEM_CONFIG_SETTINGS_TABLE_NAME;
	(m_pTable)->tableMask					= SSAPI_DM_SYS_CONFIG_TABLE_BUILT;
	(m_pTable)->pRowInsertedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DeviceManager,SystemConfigTableRowInsertedCallback);
	(m_pTable)->pRowDeletedCallback			= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DeviceManager,SystemConfigTableRowDeletedCallback);
	(m_pTable)->pRowModifiedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DeviceManager,SystemConfigTableRowModifiedCallback);
	(m_pTable)->pCreateObjectsFromRow		= (DeviceManager::CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(DeviceManager,CreateObjectsFromSysConfigRow);
	(m_pTable)->rowSize						= sizeof(SystemConfigSettingsRecord);
	(m_pTable)->pFieldDef					= (fieldDef*)SystemConfigSettingsTableFieldDefs;
	(m_pTable)->fieldDefSize				= cbSystemConfigSettingsTableFieldDefs;

	(m_pTable +1 )->pTableName				= EVC_STATUS_TABLE;
	(m_pTable +1 )->tableMask				= SSAPI_DM_EVC_TABLE_BUILT;
	(m_pTable +1 )->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DeviceManager,EvcTableRowAddedCallback);
	(m_pTable +1 )->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DeviceManager,EvcTableRowDeletedCallback);
	(m_pTable +1 )->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DeviceManager,EvcTableRowModifiedCallback);
	(m_pTable +1 )->pCreateObjectsFromRow	= (DeviceManager::CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(DeviceManager,CreateObjectsFromEvcRow);
	(m_pTable +1 )->rowSize					= sizeof(EVCStatusRecord);
	(m_pTable +1 )->pFieldDef				= (fieldDef*)aEvcStatusTable_FieldDefs;
	(m_pTable +1 )->fieldDefSize			= cbEvcStatusTable_FieldDefs;

	(m_pTable + 2)->pTableName				= CT_IOPST_TABLE_NAME;
	(m_pTable + 2)->tableMask				= SSAPI_DM_IOP_TABLE_BUILT;
	(m_pTable + 2)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DeviceManager,IopTableRowAddedCallback);
	(m_pTable + 2)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DeviceManager,IopTableRowDeletedCallback);
	(m_pTable + 2)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DeviceManager,IopTableRowModifiedCallback);
	(m_pTable + 2)->pCreateObjectsFromRow	= (DeviceManager::CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(DeviceManager,CreateObjectsFromIopRow);
	(m_pTable + 2)->rowSize					= sizeof(IOPStatusRecord);
	(m_pTable + 2)->pFieldDef				= (fieldDef*)aIopStatusTable_FieldDefs;
	(m_pTable + 2)->fieldDefSize			= cbIopStatusTable_FieldDefs;


	(m_pTable + 3)->pTableName				= PATH_DESC_TABLE;
	(m_pTable + 3)->tableMask				= SSAPI_DM_PATH_TABLE_BUILT;
	(m_pTable + 3)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DeviceManager,PathTableRowInsertedCallback);
	(m_pTable + 3)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DeviceManager,PathTableRowDeletedCallback);
	(m_pTable + 3)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DeviceManager,PathTableRowModifiedCallback);
	(m_pTable + 3)->pCreateObjectsFromRow	= (DeviceManager::CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(DeviceManager,CreateObjectsFromPathRow);
	(m_pTable + 3)->rowSize					= sizeof(PathDescriptor);
	(m_pTable + 3)->pFieldDef				= (fieldDef*)PathDescriptorTable_FieldDefs;
	(m_pTable + 3)->fieldDefSize			= cbPathDescriptorTable_FieldDefs;

	(m_pTable + 4)->pTableName				= DISK_DESC_TABLE;
	(m_pTable + 4)->tableMask				= SSAPI_DM_DISK_TABLE_BUILT;
	(m_pTable + 4)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DeviceManager,DiskTableRowAddedCallback);
	(m_pTable + 4)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DeviceManager,DiskTableRowDeletedCallback);
	(m_pTable + 4)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DeviceManager,DiskTableRowModifiedCallback);
	(m_pTable + 4)->pCreateObjectsFromRow	= (DeviceManager::CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(DeviceManager,CreateObjectsFromDiskRow);
	(m_pTable + 4)->rowSize					= sizeof(DiskDescriptor);
	(m_pTable + 4)->pFieldDef				= (fieldDef*)DiskDescriptorTable_FieldDefs;
	(m_pTable + 4)->fieldDefSize			= cbDiskDescriptorTable_FieldDefs;

	(m_pTable + 5)->pTableName				= LOOP_DESCRIPTOR_TABLE;
	(m_pTable + 5)->tableMask				= SSAPI_DM_LOOP_TABLE_BUILT;
	(m_pTable + 5)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DeviceManager,LoopTableRowAddedCallback);
	(m_pTable + 5)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DeviceManager,LoopTableRowDeletedCallback);
	(m_pTable + 5)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DeviceManager,LoopTableRowModifiedCallback);
	(m_pTable + 5)->pCreateObjectsFromRow	= (DeviceManager::CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(DeviceManager,CreateObjectsFromLoopRow);
	(m_pTable + 5)->rowSize					= sizeof(LoopDescriptorEntry);
	(m_pTable + 5)->pFieldDef				= (fieldDef*)Loop_Descriptor_Table_FieldDefs;
	(m_pTable + 5)->fieldDefSize			= cbLoop_Descriptor_Table_FieldDefs;
	
	(m_pTable + 6)->pTableName				= STORAGE_ROLL_CALL_TABLE;
	(m_pTable + 6)->tableMask				= SSAPI_DM_SRC_TABLE_BUILT;
	(m_pTable + 6)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DeviceManager,SrcTableRowInsertedCallback);
	(m_pTable + 6)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DeviceManager,SrcTableRowDeletedCallback);
	(m_pTable + 6)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DeviceManager,SrcTableRowModifiedCallback);
	(m_pTable + 6)->pCreateObjectsFromRow	= (DeviceManager::CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(DeviceManager,UpdateDiskObjectsWithSRCData);
	(m_pTable + 6)->rowSize					= sizeof(StorageRollCallRecord);
	(m_pTable + 6)->pFieldDef				= (fieldDef*)StorageRollCallTable_FieldDefs;
	(m_pTable + 6)->fieldDefSize			= cbStorageRollCallTable_FieldDefs;
	
	for( int i = 0; i < SSAPI_DM_NUMBER_OF_TABLES_USED; i++ )
		(m_pTable + i)->pShadowTable	= new ShadowTable(	(m_pTable + i)->pTableName,
															this,
															(m_pTable + i)->pRowInsertedCallback,
															(m_pTable + i)->pRowDeletedCallback,
															(m_pTable + i)->pRowModifiedCallback,
															(m_pTable + i)->rowSize );
	SetIsReadyToServiceRequests( false );

	m_isIniting			= true;
	m_tablesToRebuild	= 0;
	m_pPathDescriptors	= new DescriptorCollector;

	// register for OBJECT_ADDED events
	m_pLocalResponder = new SsapiLocalResponder( this, (LOCAL_EVENT_CALLBACK)METHOD_ADDRESS(DeviceManager,ObjectAddedEventCallback) ); 
	GetListenManager()->AddListenerForObjectAddedEvent( SSAPI_LISTEN_OWNER_ID_ANY, m_pLocalResponder->GetSessionID(), CALLBACK_METHOD(m_pLocalResponder, 1) );
	
	m_pObjectModifiedResponder = new SsapiLocalResponder( this, (LOCAL_EVENT_CALLBACK)METHOD_ADDRESS(DeviceManager,ObjectModifiedEventCallback) ); 
	GetListenManager()->AddListenerForObjectModifiedEventForManagers( SSAPI_LISTEN_OWNER_ID_ANY, m_pObjectModifiedResponder->GetSessionID(), CALLBACK_METHOD(m_pObjectModifiedResponder, 1) );


	SSAPI_TRACE( TRACE_L2, "\nDeviceManager: Intializing...." );
	DefineAllTables();

	memset( &m_settingsRow, 0, sizeof( m_settingsRow ) );
	m_isSettingsRowPresent = false;

	m_pCmbQueue->csndrInitialize((pInitializeCallback_t)METHOD_ADDRESS(DeviceManager,InitCmbQueueCallback));
}


//************************************************************************
// DeviceManager:
//
// PURPOSE:		The destructor
//************************************************************************

DeviceManager::~DeviceManager(){


	delete m_pCmbQueue;
	delete m_pPathDescriptors;

	GetListenManager()->DeleteListenerForObjectAddedEvent( SSAPI_LISTEN_OWNER_ID_ANY, m_pLocalResponder->GetSessionID() );
	delete m_pLocalResponder;
	
	GetListenManager()->DeleteListenerForObjectModifiedEventForManagers( SSAPI_LISTEN_OWNER_ID_ANY, m_pObjectModifiedResponder->GetSessionID() );
	delete m_pObjectModifiedResponder;

	for( int i = 0; i < SSAPI_DM_NUMBER_OF_TABLES_USED; i++ )
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
//				returned false, only then should they try to handle 
//				a request.
//************************************************************************

bool 
DeviceManager::Dispatch( ValueSet *pRequestParms, U32 requestCode, SsapiResponder *pResponder){

	bool			rc = false;

	if( ObjectManager::Dispatch( pRequestParms, requestCode, pResponder ) )
		return true;

	switch( requestCode ){
		case SSAPI_DEVICE_MANAGER_SET_DEVICE_LOCKED:
			rc = SetDeviceLocked( pRequestParms, pResponder );
			break;

		case SSAPI_DEVICE_MANAGER_SET_DEVICE_IN_SERVICE:
			rc = SetDeviceInService( pRequestParms, pResponder );
			break;

		case SSAPI_DEVICE_MANAGER_SET_DEVICE_POWERED:
			rc = SetDevicePowered( pRequestParms, pResponder );
			break;

		default:
			ASSERT(0);
			break;
	}

	return rc;
}


//************************************************************************
// GetDeviceState:
//
// PURPOSE:		Retrieves a device's state.
//
// RETURN:		true:		device was found
//************************************************************************

bool 
DeviceManager::GetDeviceState( DesignatorId id, int &state, U32 &stateString ){

	Device	*pDevice = (Device *)GetManagedObject( &id );

	if( pDevice ){
		state		= pDevice->GetState();
		stateString	= pDevice->GetStateString();
		return true;
	}

	return false;
}


//************************************************************************
// SetDevicePowered:
//
// PURPOSE:		Attempts to power On/Off a powerable device
//				Will respond regardless of anything!
//
// RETURN:		true
//************************************************************************

bool 
DeviceManager::SetDevicePowered( ValueSet *pParms, SsapiResponder *pResponder ){

	DesignatorId				id;
	int							newPoweredState, oldPoweredState, rc = 0;
	Device						*pDevice;

	rc |= pParms->GetGenericValue( (char *)&id, sizeof(id), SSAPI_DEVICE_MANAGER_SET_DEVICE_POWERED_OBJECT_ID );
	rc |= pParms->GetInt( SSAPI_DEVICE_MANAGER_SET_DEVICE_POWERED_POWERED, &newPoweredState );
	if( !rc )
		return pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INVALID_PARAM_EXCEPTION );

	pDevice = (Device *)GetManagedObject( &id );
	if( !pDevice )
		return pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
	
	if( g_map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_POWERABLE_INTERFACE, pDevice->GetClassType(), true ) ){
		oldPoweredState = pDevice->IsPowered()? 1 : 0;

		if( oldPoweredState == newPoweredState )
			pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
		else
			return newPoweredState? pDevice->PowerOn( pResponder ) : pDevice->PowerOff( pResponder );
	}
	else
		return pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INVALID_PARAM_EXCEPTION );

	return true;
}


//************************************************************************
// SetDeviceInService:
//
// PURPOSE:		Attempts to switch IsInService flag of a servicable device
//				Will respond regardless of anything!
//
// RETURN:		true
//************************************************************************

bool 
DeviceManager::SetDeviceInService( ValueSet *pParms, SsapiResponder *pResponder ){

	DesignatorId				id;
	int							newServiceState, rc = 0;
	Device						*pDevice;

	rc |= pParms->GetGenericValue( (char *)&id, sizeof(id), SSAPI_DEVICE_MANAGER_SET_DEVICE_IN_SERVICE_OBJECT_ID );
	rc |= pParms->GetInt( SSAPI_DEVICE_MANAGER_SET_DEVICE_IN_SERVICE_IN_SERVICE, &newServiceState );
	if( !rc )
		return pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INVALID_PARAM_EXCEPTION );

	pDevice = (Device *)GetManagedObject( &id );
	if( !pDevice )
		return pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
	
	if( g_map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_SERVICEABLE_INTERFACE, pDevice->GetClassType(), true ) ){
		if( pDevice->GetServiceState() == newServiceState )
			pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
		else
			return pDevice->SetServiceState( pResponder, newServiceState );
	}
	else
		return pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INVALID_PARAM_EXCEPTION );

	return true;
}


//************************************************************************
// SetDeviceLocked:
//
// PURPOSE:		Attempts to lock/unlock a lockable device
//				Will respond regardless of anything!
//
// RETURN:		true
//************************************************************************

bool 
DeviceManager::SetDeviceLocked( ValueSet *pParms, SsapiResponder *pResponder ){

	DesignatorId				id;
	int							newLockState, oldLockState, rc = 0;
	Device						*pDevice;

	rc |= pParms->GetGenericValue( (char *)&id, sizeof(id), SSAPI_DEVICE_MANAGER_SET_DEVICE_LOCKED_OBJECT_ID );
	rc |= pParms->GetInt( SSAPI_DEVICE_MANAGER_SET_DEVICE_LOCKED_LOCKED, &newLockState );
	if( !rc )
		return pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INVALID_PARAM_EXCEPTION );

	pDevice = (Device *)GetManagedObject( &id );
	if( !pDevice )
		return pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
	
	if( g_map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_LOCKABLE_INTERFACE, pDevice->GetClassType(), true ) ){
		oldLockState = pDevice->IsLocked()? 1 : 0;

		if( oldLockState == newLockState )
			pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
		else
			return newLockState? pDevice->Lock( pResponder ) : pDevice->UnLock( pResponder );
	}
	else
		return pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INVALID_PARAM_EXCEPTION );

	return true;
}


//************************************************************************
// DefineAllTables:
//
// PURPOSE:		Issues requests to the PTS to define all tables if not yet
//				defined
//************************************************************************

STATUS 
DeviceManager::DefineAllTables(){

	STATUS	status = OK;

	for( int i = 0; i < SSAPI_DM_NUMBER_OF_TABLES_USED; i++ )
		status	|= (m_pTable + i)->pShadowTable->DefineTable(	(m_pTable + i)->pFieldDef,
																(m_pTable + i)->fieldDefSize,
																32,
																(pTSCallback_t)METHOD_ADDRESS(DeviceManager,DefineAllTablesReplyHandler),
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
DeviceManager::DefineAllTablesReplyHandler( void *pContext, STATUS status ){

	U32			mask	= (U32)pContext;
	
	m_builtTablesMask	|= mask;
	if( (m_builtTablesMask & SSAPI_DM_ALL_TABLES_BUILT) == SSAPI_DM_ALL_TABLES_BUILT ){
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
DeviceManager::InitializeAllTables(){

	STATUS status = OK;

	for( int i = 0; i < SSAPI_DM_NUMBER_OF_TABLES_USED; i++ )
		status |= (m_pTable + i)->pShadowTable->Initialize(	(pTSCallback_t)METHOD_ADDRESS(DeviceManager,InitializeAllTablesReplyHandler), 
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
DeviceManager::InitializeAllTablesReplyHandler( void *pContext, STATUS status ){
	
	U32			mask	= (U32)pContext;
	
	m_builtTablesMask	|= mask;
	if( (m_builtTablesMask & SSAPI_DM_ALL_TABLES_BUILT) == SSAPI_DM_ALL_TABLES_BUILT ){
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
DeviceManager::EnumerateAllTables(){

	STATUS		status = OK;
	
	for( int i = 0; i < SSAPI_DM_NUMBER_OF_TABLES_USED; i++ )
		status |= (m_pTable + i)->pShadowTable->Enumerate(	&(m_pTable + i)->pTempTable,
															(pTSCallback_t)METHOD_ADDRESS(DeviceManager,EnumerateAllTablesReplyHandler),
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
DeviceManager::EnumerateAllTablesReplyHandler( void *pContext, STATUS rc ){

	U32				mask = (U32)pContext, index, objNumber;
	SList			container;
	DesignatorId	*pId;
	Device			*pDevice;

	for( int i = 0; i < SSAPI_DM_NUMBER_OF_TABLES_USED; i++ ){
		if( mask == (m_pTable + i)->tableMask ){
			if( m_tablesToRebuild & (m_pTable + i)->tableMask ){
				m_tablesToRebuild &= ~(m_pTable + i)->tableMask;
				delete (m_pTable + i)->pTempTable;
				return (m_pTable + i)->pShadowTable->Enumerate(	&(m_pTable + i)->pTempTable, (pTSCallback_t)METHOD_ADDRESS(DeviceManager,EnumerateAllTablesReplyHandler), (void *)(m_pTable + i)->tableMask);
			}
			m_builtTablesMask |= mask;
			if( rc == OK ){
				for( index = 0; index < (m_pTable + i)->pShadowTable->GetNumberOfRows(); index++ )
					(this->*(m_pTable + i)->pCreateObjectsFromRow)( ((char *)(m_pTable + i)->pTempTable) + index*(m_pTable + i)->rowSize, container );
				AddObjectsIntoManagedObjectsVector( container );
			}
			else{
				delete (m_pTable + i)->pTempTable;
				return OK;
			}
		}
	}

	if( ((m_builtTablesMask & SSAPI_DM_ALL_TABLES_BUILT) == SSAPI_DM_ALL_TABLES_BUILT) ){
		m_isIniting = false;
		// add PHS Data item ids at last
		((PHSDataManager *)GetObjectManager(SSAPI_MANAGER_CLASS_TYPE_PHS_DATA_MANAGER))->GetAllAvailableObjectIds( container );
		for( objNumber = 0; objNumber < GetManagedObjectCount(); objNumber++ ){
			pDevice = (Device *)GetManagedObject( objNumber );
			for( index = 0; index < container.Count(); index++ ){
				container.GetAt( (CONTAINER_ELEMENT &)pId, index );
				pDevice->HandlePhsObjectAddedEvent( *pId );	
			}
			Kernel::Reschedule();
		}
		// clean-up
		for( index = 0; index < container.Count(); index++ ){
			container.GetAt( (CONTAINER_ELEMENT &)pId, index );
			delete pId;
		}

		// build logical map
		CreateLogicalDeviceCollections();
		ReconcileDeviceChierachy( true );

		// set initial status according to the StatusReporterInterface
		for( objNumber = 0; objNumber < GetManagedObjectCount(); objNumber++ ){
			pDevice = (Device *)GetManagedObject( objNumber );
			pDevice->ApplyNecessaryStatusRollupRules();
		}

		// free temp tables
		for( index = 0; index < SSAPI_DM_NUMBER_OF_TABLES_USED; index++ )
			delete (m_pTable + index)->pTempTable;

		// set ready!
		SSAPI_TRACE( TRACE_L2, "\nDeviceManager: ...Done! Objects built: ", GetManagedObjectCount() );
		SetIsReadyToServiceRequests( true );

#ifndef WIN32
		//start the timer to update time in the Chassis object
		Send(	new RqOsTimerStart(TIMER_UPDATE_RATE_FOR_CHASSIS_DEVICE*1000000,TIMER_UPDATE_RATE_FOR_CHASSIS_DEVICE*1000000),
				NULL,
				REPLYCALLBACK(DeviceManager,UpdateChassis) );
#endif
	}

	return OK;
}


//************************************************************************
// CreareObjectsFromEvcRow:
//
// PURPOSE:		Creates managed objects off the row. Puts pointers
//				to them into the container
//************************************************************************

bool 
DeviceManager::CreateObjectsFromEvcRow( EVCStatusRecord *pRow, Container &container ){

	Device			*pDevice;
	U32				numberOfFans		= sizeof(pRow->FanSpeed) / sizeof(pRow->FanSpeed[0]),
					numberOfChassisPS	= sizeof(pRow->fInputOK) / sizeof(pRow->fInputOK[0]),
					numberOfDiskPS		= sizeof(pRow->fDCtoDCEnable) / sizeof(pRow->fDCtoDCEnable[0]),
					numberOfBatteries	= sizeof(pRow->fBatteryInstalled) / sizeof(pRow->fBatteryInstalled[0]),
					ps,
					objectNumberInEvcRow = 0;
	static bool		areBusSegmentsCreated = false;

	// create chassis object
	pDevice = new Chassis( GetListenManager(), objectNumberInEvcRow++ );
	pDevice->BuildYourselfFromPtsRow( pRow );
	if( m_isSettingsRowPresent )
		((Chassis *)pDevice)->BuildYourselfFromConfigSettingsRow( &m_settingsRow );

	if( GetChassisDevice() )
		delete pDevice;
	else
		container.Add( (CONTAINER_ELEMENT) pDevice );


	// create Fan objects
	for( U32 fan = 0; fan < numberOfFans; fan++ ){
		pDevice = new Fan( GetListenManager(), fan, objectNumberInEvcRow++ );
		pDevice->BuildYourselfFromPtsRow( pRow );
		RecoverChildIds( pDevice );
		RecoverParentIds( pDevice );
		container.Add( (CONTAINER_ELEMENT) pDevice );
	}

	// create chassis power supplies
	for( ps = 0; ps < numberOfChassisPS; ps++ ){
		pDevice = new ChassisPowerSupply( GetListenManager(), ps, objectNumberInEvcRow++ );
		pDevice->BuildYourselfFromPtsRow( pRow );
		RecoverChildIds( pDevice );
		RecoverParentIds( pDevice );
		container.Add( (CONTAINER_ELEMENT) pDevice );
	}

	// create disk power supplies
	for( ps = 0; ps < numberOfDiskPS; ps++ ){
		pDevice = new DiskPowerSupply( GetListenManager(), ps, objectNumberInEvcRow++ );
		pDevice->BuildYourselfFromPtsRow( pRow );
		RecoverChildIds( pDevice );
		RecoverParentIds( pDevice );
		container.Add( (CONTAINER_ELEMENT) pDevice );
	}

	// create battery objects
	for( ps = 0; ps < numberOfBatteries; ps++ ){
		pDevice = new Battery( GetListenManager(), ps, objectNumberInEvcRow++ );
		pDevice->BuildYourselfFromPtsRow( pRow );
		RecoverChildIds( pDevice );
		RecoverParentIds( pDevice );
		container.Add( (CONTAINER_ELEMENT) pDevice );
	}

	// create bus segment
	for( U32 bS = 0; !areBusSegmentsCreated && bS < SSAPI_NUMBER_OF_BUS_SEGEMENTS; bS++ ){
		pDevice = new BusSegment( GetListenManager(), bS );
		pDevice->BuildYourselfFromPtsRow( pRow );
		RecoverChildIds( pDevice );
		RecoverParentIds( pDevice );
		container.Add( (CONTAINER_ELEMENT) pDevice );
	}
	areBusSegmentsCreated = true;

	for( ps = 1; ps <= SSAPI_NUMBER_OF_CORD_SETS; ps++ ){
		pDevice = new CordSet( GetListenManager(), ps );
		pDevice->BuildYourselfFromPtsRow( pRow );
		RecoverChildIds( pDevice );
		RecoverParentIds( pDevice );
		container.Add( (CONTAINER_ELEMENT) pDevice );
	}

	return true;
}


//************************************************************************
// CreateObjectsFromDiskRow:
//
// PURPOSE:		Creates management objects off the row. Puts pointers
//				to them into the conatiner
//************************************************************************

bool 
DeviceManager::CreateObjectsFromDiskRow( DiskDescriptor *pRow, Container &container ){
	
	if( pRow->DiskType != TypeFCDisk )
		return true;

	if( ( pRow->CurrentStatus != DriveRemoved ) && (pRow->CurrentStatus != DriveNotPresent ) ){
		HDDDevice		*pDevice = new HDDDevice( GetListenManager() );
		U32				i;
		PathDescriptor	*pPath;
	
		pDevice->BuildYourselfFromPtsRow( pRow );
		RecoverChildIds( pDevice );
		RecoverParentIds( pDevice );

		// now, find all the paths to this puppy
		for( i = 0; m_pPathDescriptors->GetAt( i, (void *&)pPath ); i++ )
			if( RowId(pRow->rid) == pPath->ridDescriptor )
				pDevice->BuildYourselfFromPathRow( pPath );

		// add into the container
		container.Add( (CONTAINER_ELEMENT)pDevice );
	}

	return true;
}


//************************************************************************
// CreateObjectsFromIopRow:
//
// PURPOSE:		Creates management objects off the row. Puts pointers
//				to them into the conatiner
//************************************************************************

bool 
DeviceManager::CreateObjectsFromIopRow( IOPStatusRecord *pRow, Container &container ){
	
	Board			*pBoard = NULL;
	DDH				*pDdh;
	SlotMap			map;
	DesignatorId	id;
	ManagedObject	*pObj;
	rowID			rid;

	switch( pRow->eIOPCurrentState ){
		case IOPS_EMPTY:
		case IOPS_UNKNOWN:
			break;

		case IOPS_BLANK:
			// check if this is a board, not a DDH
			if( pRow->Slot > SlotMap::GetMaxSlotNumber() )
				break;

			if( map.GetSegmentNumberBySlotNumber(pRow->Slot) == -1 )
				pBoard = new HbcFillerBoard( GetListenManager() );
			else
				pBoard = new FillerBoard( GetListenManager() );
				
			RecoverChildIds( pBoard );
			RecoverParentIds( pBoard );
			pBoard->BuildYourselfFromPtsRow( pRow );
			container.Add( (CONTAINER_ELEMENT)pBoard );	
			break;

		default:
			switch( pRow->IOP_Type ){
				case IOPTY_HBC:
					pBoard = new Hbc( GetListenManager() );
					pBoard->BuildYourselfFromPtsRow( pRow );
					RecoverChildIds( pBoard );
					RecoverParentIds( pBoard );
					container.Add( (CONTAINER_ELEMENT)pBoard );
					break;

				case IOPTY_NAC:
					if( (pRow->Slot == IOP_RAC0) || (pRow->Slot == IOP_RAC1) )
						pBoard = new SNac( GetListenManager() );
					else
						pBoard = new Nac( GetListenManager() );
					pBoard->BuildYourselfFromPtsRow( pRow );
					RecoverChildIds( pBoard );
					RecoverParentIds( pBoard );
					container.Add( (CONTAINER_ELEMENT)pBoard );
					break;

				case IOPTY_SSD:
					pBoard = new Ssd( GetListenManager() );
					pBoard->BuildYourselfFromPtsRow( pRow );
					RecoverChildIds( pBoard );
					RecoverParentIds( pBoard );
					container.Add( (CONTAINER_ELEMENT)pBoard );
					break;

				case IOPTY_EVC:
					break;

				case IOPTY_DDH:
					pDdh = new DDH( GetListenManager() );
					pDdh->BuildYourselfFromPtsRow( pRow );
					RecoverChildIds( pDdh );
					RecoverParentIds( pDdh );
					container.Add( (CONTAINER_ELEMENT)pDdh );
					break;

				default:
					ASSERT(0);
					return false;
			}
	}

	container.GetAt( (CONTAINER_ELEMENT &)pBoard, 0 );
	if( pBoard )
		RecoverPhsIds( pBoard );
	
	
	memcpy( &rid, &pRow->rid, sizeof(rowID) );		// HACKGAI
	GetDesignatorIdByRowId( rid, id );
	// check if the type of board changed (IOP <-> filler, or if a board was removed 
	pObj = GetManagedObject( &id );
	if(  ( pObj && !pBoard ) 
		||
		(pObj && (pObj->GetClassType() != pBoard->GetClassType() )  ) ){

		ManagedObject *pObj =  GetManagedObject( &id );
		CoolVector v;
		v.Add( (CONTAINER_ELEMENT) pObj );
		DeleteObjectsFromTheSystem( v );

		if( pBoard ){
			Device *pDevice;
			for( U32 deviceNum = 0; deviceNum < GetManagedObjectCount(); deviceNum++ ){
				pDevice = (Device *)GetManagedObject( deviceNum );
				if( pDevice->IsYourDevice( *pBoard ) ){
					pDevice->AddChildId( pBoard );
					pBoard->AddParentId( pDevice );
				}
			}
		}
	}
	
	return true;
}



//************************************************************************
// ReconcileNewObjectsWithPhsDataObjects
//
// PURPOSE:		For every device object in the container, goes thru all
//				phs data items and supplies it to the device object so that
//				the latter could determine oif this phs item is its child
//************************************************************************

void 
DeviceManager::ReconcileNewObjectsWithPhsDataObjects( Container &newObjects ){

	Container		*pContainer = new CoolVector;
	CoolVector		phsIds;
	Device			*pDevice;
	DesignatorId	*pId, id;
	U32				devNum, phsNum;
	ManagedObject	*pOldObj;
	RowId			*pRid;
	PHSDataManager	*pM = (PHSDataManager *)GetObjectManager(SSAPI_MANAGER_CLASS_TYPE_PHS_DATA_MANAGER);
	bool			wasModified;

	for( devNum = 0; devNum < newObjects.Count(); devNum++ ){
		newObjects.GetAt( (CONTAINER_ELEMENT &)pDevice, devNum );
		id = pDevice->GetDesignatorId();
		wasModified = false;
		pOldObj = GetManagedObject( &id );
		if( !pOldObj || pDevice->HasYouPhsVectorChanged((Device *)pOldObj) ){	// new object
			pDevice->GetPhsDataRowId( phsIds );
			while( phsIds.Count() ){
				phsIds.GetAt( (CONTAINER_ELEMENT &)pRid, 0 );
				pM->GetObjectIdsByRowId( *pContainer, *pRid );
				for( phsNum = 0; phsNum < pContainer->Count(); phsNum++ ){
					pContainer->GetAt( (CONTAINER_ELEMENT &) pId, phsNum );
					pDevice->HandlePhsObjectAddedEvent( *pId, false );
					wasModified = true;
				}
				phsIds.RemoveAt( 0 );
				delete pRid;
				FreeMemoryForTheContainerWithIds( *pContainer );
			}
		}
		else{			// existed before, simply restore ids
			RecoverPhsIds( pDevice );
		}
		Kernel::Reschedule();
		if( wasModified )
			GetListenManager()->PropagateObjectModifiedEventForManagers( GetDesignatorId(), pDevice );
	}
	delete pContainer;
}



//************************************************************************
// EvcTableRowAddedCallback:
//
// PURPOSE:		Called when rows were added to the EVC table
//************************************************************************

STATUS 
DeviceManager::EvcTableRowAddedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){
	
	EVCStatusRecord		*pRow = (EVCStatusRecord *)pRows;
	U32					index;
	Container			*pContainer;

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_DM_EVC_TABLE_BUILT;
		return OK;
	}
	SSAPI_TRACE( TRACE_L3, "\nDeviceManager::EvcTableRowAddedCallback" );
	pContainer = new SList;

	for( index = 0; index < numberOfRows; index++, pRow++ )
		CreateObjectsFromEvcRow( pRow, *pContainer );
	
	ReconcileNewObjectsWithPhsDataObjects( *pContainer );
	AddObjectsIntoManagedObjectsVector( *pContainer );

	if( pRow--, pRow->KeyPosition == CT_KEYPOS_SECURITY ){
		UserManager *pUM = (UserManager *)GetObjectManager(SSAPI_MANAGER_CLASS_TYPE_USER_MANAGER);
		pUM->CheckAndAddWellknownAccount( StringClass("dell") );
	}

	BumpUpConfigId();

	delete pContainer;
	return OK;
}

//************************************************************************
// EvcTableRowDeletedCallback:
//
// PURPOSE:		Called when rows were deleted from the EVC table
//************************************************************************

STATUS 
DeviceManager::EvcTableRowDeletedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){

	Container			*pContainer; 
	EVCStatusRecord		*pRow = (EVCStatusRecord *)pRows;
	U32					index;
	ManagedObject		*pObj;

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_DM_EVC_TABLE_BUILT;
		return OK;
	}
	SSAPI_TRACE( TRACE_L3, "\nDeviceManager::EvcTableRowDeletedCallback" );
	pContainer = new SList;

	for( index = 0; index < numberOfRows; index++, pRow++ )
		CreateObjectsFromEvcRow( pRow, *pContainer );

	DeleteObjectsFromTheSystem( *pContainer );

	BumpUpConfigId();
	
	// clean-up
	while( pContainer->Count() ){
		pContainer->GetAt( (CONTAINER_ELEMENT &)pObj, 0 );
		pContainer->RemoveAt( 0 );
		delete pObj;
	}

	delete pContainer;
	return OK;
}


//************************************************************************
// EvcTableRowModifiedCallback:
//
// PURPOSE:		Called when rows were modified in the EVC table
//************************************************************************

STATUS 
DeviceManager::EvcTableRowModifiedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_DM_EVC_TABLE_BUILT;
		return OK;
	}
	SSAPI_TRACE( TRACE_L3, "\nDeviceManager::EvcTableRowModifiedCallback" );
	return EvcTableRowAddedCallback( pRows, numberOfRows, NULL );
}


//************************************************************************
// DiskTableRowAddedCallback:
//
// PURPOSE:		Called when rows are added to the DiskDescriptor
//************************************************************************

STATUS 
DeviceManager::DiskTableRowAddedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_DM_DISK_TABLE_BUILT;
		return OK;
	}

	DiskDescriptor		*pRow = (DiskDescriptor *)pRows;
	U32					index;
	Container			*pContainer;

	SSAPI_TRACE( TRACE_L3, "\nDeviceManager::DiskTableRowAddedCallback" );
	pContainer = new SList;

	for( index = 0; index < numberOfRows; index++, pRow++ )
		CreateObjectsFromDiskRow( pRow, *pContainer );
	
	AddObjectsIntoManagedObjectsVector( *pContainer );
	BumpUpConfigId();

	delete pContainer;
	return OK;
}


//************************************************************************
// DiskTableRowDeletedCallback:
//
// PURPOSE:		Called when rows are deleted from the DiskDescriptor
//************************************************************************

STATUS 
DeviceManager::DiskTableRowDeletedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_DM_DISK_TABLE_BUILT;
		return OK;
	}

	Container			*pContainer; 
	DiskDescriptor		*pRow = (DiskDescriptor *)pRows;
	U32					index;
	ManagedObject		*pObj;

	SSAPI_TRACE( TRACE_L3, "\nDeviceManager::DiskTableRowDeletedCallback" );
	pContainer = new SList;

	for( index = 0; index < numberOfRows; index++, pRow++ )
		CreateObjectsFromDiskRow( pRow, *pContainer );

	DeleteObjectsFromTheSystem( *pContainer );

	BumpUpConfigId();
	
	// clean-up
	while( pContainer->Count() ){
		pContainer->GetAt( (CONTAINER_ELEMENT &)pObj, 0 );
		pContainer->RemoveAt( 0 );
		delete pObj;
	}

	delete pContainer;
	return OK;
}


//************************************************************************
// DiskTableRowModifiedCallback:
//
// PURPOSE:		Called when rows are modified in the DiskDescriptor
//************************************************************************

STATUS 
DeviceManager::DiskTableRowModifiedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_DM_DISK_TABLE_BUILT;
		return OK;
	}

	SSAPI_TRACE( TRACE_L3, "\nDeviceManager::DiskTableRowModifiedCallback" );
	return DiskTableRowAddedCallback( pRows, numberOfRows, NULL );
}


//************************************************************************
// IopTableRowAddedCallback:
//
// PURPOSE:		CAlled when rows were added to the IOP table
//************************************************************************

STATUS 
DeviceManager::IopTableRowAddedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_DM_IOP_TABLE_BUILT;
		return OK;
	}
	IOPStatusRecord		*pRow = (IOPStatusRecord *)pRows;
	U32					index;
	Container			*pContainer;

	SSAPI_TRACE( TRACE_L3, "\nDeviceManager::IOPTableRowAddedCallback" );
	pContainer = new SList;

	for( index = 0; index < numberOfRows; index++, pRow++ )
		CreateObjectsFromIopRow( pRow, *pContainer );
	
	ReconcileNewObjectsWithPhsDataObjects( *pContainer );
	AddObjectsIntoManagedObjectsVector( *pContainer );

	BumpUpConfigId();

	delete pContainer;
	return OK;
}


//************************************************************************
// IopTableRowDeletedCallback:
//
// PURPOSE:		Called when rows were deleted from the IOP table
//************************************************************************

STATUS 
DeviceManager::IopTableRowDeletedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_DM_IOP_TABLE_BUILT;
		return OK;
	}

	Container			*pContainer; 
	IOPStatusRecord		*pRow = (IOPStatusRecord *)pRows;
	U32					index;
	ManagedObject		*pObj;

	SSAPI_TRACE( TRACE_L3, "\nDeviceManager::IOPTableRowDeletedCallback" );
	pContainer = new SList;

	for( index = 0; index < numberOfRows; index++, pRow++ )
		CreateObjectsFromIopRow( pRow, *pContainer );

	DeleteObjectsFromTheSystem( *pContainer );

	BumpUpConfigId();
	
	// clean-up
	while( pContainer->Count() ){
		pContainer->GetAt( (CONTAINER_ELEMENT &)pObj, 0 );
		pContainer->RemoveAt( 0 );
		delete pObj;
	}

	delete pContainer;
	return OK;
}


//************************************************************************
// IopTableRowModifiedCallback:
//
// PURPOSE:		Called 
//************************************************************************

STATUS 
DeviceManager::IopTableRowModifiedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_DM_IOP_TABLE_BUILT;
		return OK;
	}

	SSAPI_TRACE( TRACE_L3, "\nDeviceManager::IOPTableRowModifiedCallback" );
	return IopTableRowAddedCallback( pRows, numberOfRows, NULL );
}


//************************************************************************
// ObjectAddedEventCallback:
//
// PURPOSE:		This is a callback method for all OBJECT_ADDED events 
//				coming from the listen manager.
//************************************************************************

void 
DeviceManager::ObjectAddedEventCallback( ValueSet *pVs, bool isLast, int eventObjectId ){

	SSAPIEvent				*pEvent = new SSAPIEventObjectAdded( NULL );
	ValueSet				*pChild = new ValueSet;
	Device					*pObj;
	bool					isPhs = false;
	int						classType;

	*pEvent = *(ValueSet *)pVs->GetValue( eventObjectId );
	*pChild = *(ValueSet *)pEvent->GetValue( SSAPI_EVENT_FID_MANAGED_OBJECT );
	
	pChild->GetInt( SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &classType );
	isPhs |= g_map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_PHS_DATA, classType, true );
		
	for( U32 index = 0; isPhs && (index < GetManagedObjectCount()); index++ ){
		pObj = (Device *)GetManagedObject( index );
		pObj->HandleObjectAddedEvent( pChild );
	}
	
	delete pChild;
	delete pEvent;

	ReconcileDeviceChierachy();
}


//************************************************************************
// ObjectModifiedEventCallback:
//
// PURPOSE:		This is a callback method for all OBJECT_MODIFIED events 
//				coming from the listen manager.
//************************************************************************

void 
DeviceManager::ObjectModifiedEventCallback( ValueSet *pVs, bool isLast, int eventObjectId ){

	SSAPIEvent				*pEvent = new SSAPIEventObjectAdded( NULL );
	ValueSet				*pObj = new ValueSet;
	Device					*pDevice;
	int						classType;
	U32						i;
	DesignatorId			id;

	*pEvent = *(ValueSet *)pVs->GetValue( eventObjectId );
	*pObj	= *(ValueSet *)pEvent->GetValue( SSAPI_EVENT_FID_MANAGED_OBJECT );
	pObj->GetInt( SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &classType );

	if( g_map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_DEVICE, classType, true ) ){
		pObj->GetGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID );
		if( (pDevice = (Device *)GetManagedObject( &id ) ) !=NULL ){
		
			// go thru children
			for( i = 0; i < pDevice->GetChildCount(); i++ ){
				id  =pDevice->GetChildIdAt( i );
				((Device *)GetManagedObject( &id ))->ApplyNecessaryStatusRollupRules();
			}
	
			// go thru parents
			for( i = 0; i < pDevice->GetParentCount(); i++ ){
				id  =pDevice->GetParentIdAt( i );
				((Device *)GetManagedObject( &id ))->ApplyNecessaryStatusRollupRules();
			}
		}
	}
	
	delete pObj;
	delete pEvent;

	// TBDGAI: why am i doing it here? comment out!
	//ReconcileDeviceChierachy();
}

//************************************************************************
// LoopTableRowAddedCallback:
//
// PURPOSE:		Called when rows were added to the LoopDescriptor table
//************************************************************************

STATUS 
DeviceManager::LoopTableRowAddedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_DM_LOOP_TABLE_BUILT;
		return OK;
	}

	LoopDescriptorEntry		*pRow = (LoopDescriptorEntry *)pRows;
	U32						index;
	Container				*pContainer;

	SSAPI_TRACE( TRACE_L3, "\nDeviceManager::LoopTableRowAddedCallback" );
	pContainer = new SList;

	for( index = 0; index < numberOfRows; index++, pRow++ )
		CreateObjectsFromLoopRow( pRow, *pContainer );
	
	ReconcileNewObjectsWithPhsDataObjects( *pContainer );
	AddObjectsIntoManagedObjectsVector( *pContainer );


	BumpUpConfigId();

	delete pContainer;

	return OK;
}


//************************************************************************
// LoopTableRowDeletedCallback:
//
// PURPOSE:		Called when rows were deleted from the LoopDescriptor table
//************************************************************************

STATUS 
DeviceManager::LoopTableRowDeletedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_DM_LOOP_TABLE_BUILT;
		return OK;
	}

	Container				*pContainer; 
	LoopDescriptorEntry		*pRow = (LoopDescriptorEntry *)pRows;
	U32						index;
	ManagedObject			*pObj;

	SSAPI_TRACE( TRACE_L3, "\nDeviceManager::LoopTableRowDeletedCallback" );
	pContainer = new SList;

	for( index = 0; index < numberOfRows; index++, pRow++ )
		CreateObjectsFromLoopRow( pRow, *pContainer );

	DeleteObjectsFromTheSystem( *pContainer );

	BumpUpConfigId();
	
	// clean-up
	while( pContainer->Count() ){
		pContainer->GetAt( (CONTAINER_ELEMENT &)pObj, 0 );
		pContainer->RemoveAt( 0 );
		delete pObj;
	}

	delete pContainer;
	return OK;
}


//************************************************************************
// LoopTableRowModifiedCallback:
//
// PURPOSE:		Called when rows were modified in the LoopDescriptor table
//************************************************************************

STATUS 
DeviceManager::LoopTableRowModifiedCallback( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_DM_LOOP_TABLE_BUILT;
		return OK;
	}

	SSAPI_TRACE( TRACE_L3, "\nDeviceManager::LoopTableRowModifiedCallback" );

	return LoopTableRowAddedCallback( pRows, numberOfRows, NULL );
}


//************************************************************************
// CreateObjectsFromLoopRow:
//
// PURPOSE:		Creates managed objects off the row. Puts pointers
//				to them into the container
//************************************************************************

bool 
DeviceManager::CreateObjectsFromLoopRow( LoopDescriptorEntry *pRow, Container &container ){

	FcPort		*pDevice;
	
	if( (pRow->ChipNumber == 2) && ( (pRow->slot == IOP_RAC0) || (pRow->slot == IOP_RAC1) ) )
		pDevice = new FcPortInternal( GetListenManager() );
	else
		pDevice = new FcPort( GetListenManager() );
	
	pDevice->BuildYourselfFromPtsRow( pRow );
	RecoverChildIds( pDevice );
	RecoverParentIds( pDevice );
	container.Add( (CONTAINER_ELEMENT)pDevice );
	return true;
}


//************************************************************************
// Methods used as callback for the SystemConfigSettingsTable
//
//
//************************************************************************

STATUS 
DeviceManager::SystemConfigTableRowModifiedCallback( void *pRows, U32 numberOfRows, ShadowTable*){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_DM_SYS_CONFIG_TABLE_BUILT;
		return OK;
	}

	memcpy( &m_settingsRow, pRows, sizeof( m_settingsRow ) );

	Chassis *pChassis = GetChassisDevice();
	if( !pChassis ){
		ASSERT(0);
		return OK;
	}
	m_isSettingsRowPresent = true;
	pChassis->BuildYourselfFromConfigSettingsRow( (SystemConfigSettingsRecord *)pRows );
	RecoverChildIds( pChassis );
	RecoverParentIds( pChassis );
	GetListenManager()->PropagateObjectModifiedEventForManagers( GetDesignatorId(), pChassis );
	BumpUpConfigId();

	return OK;
}


STATUS 
DeviceManager::SystemConfigTableRowInsertedCallback( void *pRows, U32 numberOfRows, ShadowTable *pT){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_DM_SYS_CONFIG_TABLE_BUILT;
		return OK;
	}

	m_isSettingsRowPresent = true;

	return SystemConfigTableRowModifiedCallback( pRows, numberOfRows, pT );

}


STATUS 
DeviceManager::SystemConfigTableRowDeletedCallback( void *pRows, U32 numberOfRows, ShadowTable*){


	ASSERT(0);

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_DM_SYS_CONFIG_TABLE_BUILT;
		return OK;
	}

	memset( &m_settingsRow, 0, sizeof( m_settingsRow ) );
	m_isSettingsRowPresent = false;

	Chassis *pChassis = GetChassisDevice();
	if( !pChassis )
		return OK;
	// TBDGAI : OK, but what am I to do? who would delete this row? 
	return OK;
}


//************************************************************************
// CreateObjectsFromSysConfigRow:
//
// PURPOSE:		Stores the row to be accessed later
//************************************************************************

bool 
DeviceManager::CreateObjectsFromSysConfigRow( SystemConfigSettingsRecord *pRow, Container &container ){

	// this data will be used when we are building the chassis device
	memcpy( &m_settingsRow, pRow, sizeof( m_settingsRow ) );
	m_isSettingsRowPresent = true;

	return true;
}



//************************************************************************
// CreateLogicalDeviceCollections:
//
// PURPOSE:		Creates device collection objects and adds them into the
//				system.
//************************************************************************

void 
DeviceManager::CreateLogicalDeviceCollections(){
	
	DeviceCollection	*pCollection;
	Container			*pContainer = new SList;
	
	// EVC collection
	pCollection = new EvcDeviceCollection( GetListenManager() );
	pContainer->Add( (CONTAINER_ELEMENT)pCollection );

	// FC Collection
	pCollection = new FcDeviceCollection( GetListenManager() );
	pContainer->Add( (CONTAINER_ELEMENT)pCollection );

	// PCI Collection
	pCollection = new PciDeviceCollection( GetListenManager() );
	pContainer->Add( (CONTAINER_ELEMENT)pCollection );

	AddObjectsIntoManagedObjectsVector( *pContainer );
	delete pContainer;
}

//************************************************************************
// ReconcileDeviceChierachy:
//
// PURPOSE:		Reconciles 'children' and 'parents' for all managed objects
//				Posts appropriate events (CHILD_ADDED, OBJECT_MODIFED )
//				whenever necessary.
//************************************************************************

void 
DeviceManager::ReconcileDeviceChierachy( bool isFirstTime ){

	
	Device				*pDeviceBeingReconciled, *pDevice;
	U32					deviceBeingReconciledNum, deviceNum;
	
	for( deviceBeingReconciledNum =0; deviceBeingReconciledNum < GetManagedObjectCount(); deviceBeingReconciledNum++ ){
		pDeviceBeingReconciled = (Device *)GetManagedObject( deviceBeingReconciledNum );
		if( pDeviceBeingReconciled->GetStateString() ==0 )
			deviceNum = 0;
		pDeviceBeingReconciled->ComposeYourOverallState();

		for( deviceNum = 0; deviceNum < GetManagedObjectCount(); deviceNum++ ){
			if( deviceNum == deviceBeingReconciledNum )
				continue;

			pDevice = (Device *)GetManagedObject( deviceNum );
			if( pDevice->IsYourDevice( *pDeviceBeingReconciled ) ){
				pDevice->AddChildId( pDeviceBeingReconciled );
				pDeviceBeingReconciled->AddParentId( pDevice );
			}
		}
		Kernel::Reschedule();
	}

	SSAPI_TRACE( TRACE_L3, "\nDeviceManager: Successfully reconciled device hierachy..." );
}


//************************************************************************
// GetChassisDevice:
//
// PURPOSE:		Looks up and returns the ptr to the chassis device or
//				NULL if no such device is found
//************************************************************************

Chassis* 
DeviceManager::GetChassisDevice(){

	ManagedObject		*pObj;

	for( U32 i = 0; i < GetManagedObjectCount(); i++ ){
		pObj = GetManagedObject( i );
		if( pObj->GetClassType() == SSAPI_OBJECT_CLASS_TYPE_CHASSIS )
			return (Chassis *)pObj;
	}

	return NULL;
}


//************************************************************************
// ModifyChassisDevice:
//
// PURPOSE:		Attempts to modify the chassis device's data
//************************************************************************

bool 
DeviceManager::ModifyChassisDevice( ValueSet &objectValues, SsapiResponder *pResponder ){

	Chassis						*pNewChassis = new Chassis( GetListenManager(), 0 ),
								*pOldChassis = GetChassisDevice();
	SystemConfigSettingsRecord	row;
	ShadowTable					*pTable;
	ValueSet					*pTrapAddresses;
	LocalizedDateTime			tempTime;

	// first, check if there are two many trap ip addresses specified
	pTrapAddresses = (ValueSet *)objectValues.GetValue(SSAPI_CHASSIS_FID_TRAP_IP_ADDRESS_VECTOR);
	if( pTrapAddresses && (pTrapAddresses->GetCount() > pOldChassis->GetMaxTrapAddressCount()) ){
		delete pNewChassis;
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_EXCEPTION_TOO_MANY_TRAP_ADDRESSES );
		return true;
	}

	// check if the gmt_time is present. If so -- set the system time
	if( objectValues.GetInt64( SSAPI_CHASSIS_FID_GMT_TIME_BASE, &tempTime ) ){
		struct tm	tempTm;
		time_t		tempTime_t;

		tempTime_t = tempTime / 1000;
		gmtime_r( &tempTime_t, &tempTm );
		SetTime( &tempTm );
	}
	
	pOldChassis->BuildYourValueSet();
	*((ValueSet *)pNewChassis) = *pOldChassis;
	pOldChassis->Clear();

	pNewChassis->OverrideExistingValues( &objectValues );
	pNewChassis->BuildYourselfFromYourValueSet();
	pNewChassis->WriteYourselfIntoConfigSettingsRow( &row );

	pTable = GetShadowTable( SSAPI_DM_SYS_CONFIG_TABLE_BUILT );

	if( m_isSettingsRowPresent )
		pTable->ModifyRow( pOldChassis->GetConfigSettingsRowId(), &row, (pTSCallback_t)METHOD_ADDRESS(DeviceManager,ModifyChassisDeviceReplyCallback) , pResponder );
	else
		pTable->InsertRow( &row, &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DeviceManager,ModifyChassisDeviceReplyCallback) , pResponder );

	delete pNewChassis;
	return true;
}


//************************************************************************
// ModifyChassisDeviceReplyCallback:
//
// PURPOSE:		Handles the PTS reply and responds to the caller
//************************************************************************

STATUS 
DeviceManager::ModifyChassisDeviceReplyCallback( void *pContext, STATUS rc ){

	SsapiResponder		*pResponder	= (SsapiResponder *)pContext;

	if( rc == OK )
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	else
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);

	return OK;
}


//************************************************************************
// GetShadowTable:
//
// PURPOSE:		Performs a look up of the table by the table mask
//************************************************************************

ShadowTable* 
DeviceManager::GetShadowTable( U32 tableMask ){

	for( U32 i = 0; i < SSAPI_DM_NUMBER_OF_TABLES_USED; i++ )
		if( m_pTable[i].tableMask == tableMask )
			return m_pTable[i].pShadowTable;


	ASSERT(0);
	return NULL;
}


//************************************************************************
// ChangeBoardLockState:
//
// PURPOSE:		Locks/unlocks a board
//************************************************************************

void 
DeviceManager::ChangeBoardLockState( TySlot slot, bool shouldTurnOn, DesignatorId boardId, SsapiResponder *pResponder ){
#if 0
	CHotSwapIntf		cmd;

	memset( &cmd, 0, sizeof(cmd) );

	cmd.eCommand	= shouldTurnOn? CHotSwapIntf::k_eRestoreIopSlot : CHotSwapIntf::k_eReleaseIopSlot;
	cmd.eSlot		= slot;
	m_deviceWaitingForCmb = boardId;
	SetIsReadyToServiceRequests( false );

	m_pCmbQueue->csndrExecute(	&cmd,
								(pCmdCompletionCallback_t)METHOD_ADDRESS(DeviceManager,ChangeBoardLockStateCallback),
								pResponder );
#endif
}


void 
DeviceManager::ChangeBoardLockStateCallback(STATUS			completionCode,
											void			*pResultData,
											void			*pCmdData,
											void			*pCmdContext ){

	SsapiResponder		*pResponder = (SsapiResponder *)pCmdContext;
	Board				*pBoard;

	if( completionCode == OK ){
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
		pBoard = (Board *)GetManagedObject( &m_deviceWaitingForCmb );
		if( pBoard ){
			pBoard->SetIsLocked( !pBoard->IsLocked() );
			pBoard->FireEventObjectModifed();
		}
	}
	else{
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INTERNAL, completionCode );
	}
	SetIsReadyToServiceRequests( true );
}

//************************************************************************
// ChangeHDDLockState:
//
// PURPOSE:		Locks/unlocks an HDD device
//************************************************************************

void 
DeviceManager::ChangeHDDLockState( U32 bayNumber, bool shouldLock, DesignatorId hddId, SsapiResponder *pResponder ){

	CHotSwapIntf		cmd;

	memset( &cmd, 0, sizeof(cmd) );

	cmd.eCommand	= shouldLock? CHotSwapIntf::k_eRestoreDriveBay : CHotSwapIntf::k_eReleaseDriveBay;
	cmd.ridDiskDescr = hddId.GetRowId();
	m_deviceWaitingForCmb = hddId;
	SetIsReadyToServiceRequests( false );

	m_pCmbQueue->csndrExecute(	&cmd,
								(pCmdCompletionCallback_t)METHOD_ADDRESS(DeviceManager,ChangeBoardLockStateCallback),
								pResponder );
}


void 
DeviceManager::ChangeHDDLockStateCallback(	STATUS			completionCode,
											void			*pResultData,
											void			*pCmdData,
											void			*pCmdContext ){

	SsapiResponder		*pResponder = (SsapiResponder *)pCmdContext;
	HDDDevice			*pHDD;

	if( completionCode == OK ){
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
		pHDD = (HDDDevice *)GetManagedObject( &m_deviceWaitingForCmb );
		if( pHDD ){
			pHDD->SetIsLocked( !pHDD->IsLocked() );
			pHDD->FireEventObjectModifed();
		}
	}
	else{
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);
	}
	SetIsReadyToServiceRequests( true );

}



//************************************************************************
// ChangeIopServiceState:
//
// PURPOSE:		Brings an Iop in/out of service
//************************************************************************

void 
DeviceManager::ChangeIopServiceState( Iop *pIop, bool bringInService, SsapiResponder *pResponder ){
#if 0
	CHotSwapIntf		cmd;

	ASSERT( bringInService == false);

	cmd.eCommand	= CHotSwapIntf::k_eFailoverIop;
	cmd.eSlot		= (TySlot)pIop->GetSlotNumber();

	m_deviceWaitingForCmb = pIop->GetDesignatorId();
	SetIsReadyToServiceRequests( false );

	m_pCmbQueue->csndrExecute(	&cmd,
								(pCmdCompletionCallback_t)METHOD_ADDRESS(DeviceManager,ChangeIopServiceStateCallback),
								pResponder );
#endif
}


void 
DeviceManager::ChangeIopServiceStateCallback(	STATUS			completionCode,
												void			*pResultData,
												void			*pCmdData,
												void			*pCmdContext ){

	SsapiResponder			*pResponder = (SsapiResponder *)pCmdContext;

	if( completionCode == OK )
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	else
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);

	SetIsReadyToServiceRequests( true );
}

//************************************************************************
// GetChassisDeviceId:
//
// PURPOSE:		Returns id of the chassis device
//************************************************************************

DesignatorId 
DeviceManager::GetChassisDeviceId(){

	return GetChassisDevice()->GetDesignatorId();
}


//************************************************************************
// GetIopBySlot:
//
// PURPOSE:		Returns id of the IOP device in the slot specified. The
//				id will be Clear if no such iop exists
//************************************************************************

DesignatorId 
DeviceManager::GetIopBySlot( int slotNumber ){

	Device			*pDevice;
	U32				i;

	for( i = 0; i < GetManagedObjectCount(); i++ ){
		pDevice = (Device *)GetManagedObject( i );
		if( g_map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_IOP, pDevice->GetClassType(), true ) )
			if( ((Board *)pDevice)->GetSlotNumber() == slotNumber )
				return pDevice->GetDesignatorId();
	}


	return DesignatorId();
}


//************************************************************************
// ChangeIopPowerState:
//
// PURPOSE:		Changes power state of an IOP 
//************************************************************************

void 
DeviceManager::ChangeIopPowerState( Iop *pIop, bool powerDown, SsapiResponder* pResponder ){
#if 0
	CHotSwapIntf		cmd;

	cmd.eCommand	= powerDown? CHotSwapIntf::k_ePowerDownIop : CHotSwapIntf::k_ePowerDownIop; 
	cmd.eSlot		= (TySlot)pIop->GetSlotNumber();

	m_deviceWaitingForCmb = pIop->GetDesignatorId();
	SetIsReadyToServiceRequests( false );

	m_pCmbQueue->csndrExecute(	&cmd,
								(pCmdCompletionCallback_t)METHOD_ADDRESS(DeviceManager,ChangeIopPowerStateCallback),
								pResponder );
#endif
}


void 
DeviceManager::ChangeIopPowerStateCallback(	STATUS			completionCode,
											void			*pResultData,
											void			*pCmdData,
											void			*pCmdContext ){

	SsapiResponder			*pResponder = (SsapiResponder *)pCmdContext;

	if( completionCode == OK )
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	else
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);

	SetIsReadyToServiceRequests( true );
}


//************************************************************************
// PowerDownChassis:
//
// PURPOSE:		Attempts to power down the box.
//************************************************************************

void 
DeviceManager::PowerDownChassis( SsapiResponder *pResponder ){
#if 0
	CHotSwapIntf		cmd;

	cmd.eCommand	= CHotSwapIntf::k_eShutdownSystem; 
	SetIsReadyToServiceRequests( false );

	m_pCmbQueue->csndrExecute(	&cmd,
								(pCmdCompletionCallback_t)METHOD_ADDRESS(DeviceManager,PowerDownChassisCallback),
								pResponder );

#endif
}


void 
DeviceManager::PowerDownChassisCallback( STATUS			completionCode,
										void			*pResultData,
										void			*pCmdData,
										void			*pCmdContext ){

	SsapiResponder			*pResponder = (SsapiResponder *)pCmdContext;

	if( completionCode == OK )
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	else
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);

	SetIsReadyToServiceRequests( true );
}


//************************************************************************
// GetPortByInstanceNumber:
//
// PURPOSE:		Returns id of the FcLoop device with the FC instance
//				number as specified. the returned id will be clear if
//				no such loop exists.
//************************************************************************

DesignatorId 
DeviceManager::GetPortByInstanceNumber( U32 instanceNumber ){

	U32				i;
	ManagedObject	*pObj;
	FcPort			*pPort;

	for( i = 0; i < GetManagedObjectCount(); i++ ){
		pObj = GetManagedObject( i );
		if( g_map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_PORT, pObj->GetClassType(), true ) ){
			pPort = (FcPort *)pObj;
			if( pPort->GetPortNumber() == instanceNumber )
				return pPort->GetDesignatorId();
		}
	}

	return DesignatorId();
}


//************************************************************************
// PTS callbacks for the SRC table
//************************************************************************

STATUS 
DeviceManager::SrcTableRowInsertedCallback(  void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_DM_SRC_TABLE_BUILT;
		return OK;
	}

	StorageRollCallRecord	*pRow = (StorageRollCallRecord *)pRows;
	U32						i;

	for( i = 0; i < numberOfRows; i++, pRow++ )
		FindHDDAndSetItsPhsIds(	pRow->ridDescriptorRecord,
								pRow->ridStatusRecord,
								pRow->ridPerformanceRecord );
	return OK;
}


STATUS 
DeviceManager::SrcTableRowDeletedCallback(  void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_DM_SRC_TABLE_BUILT;
		return OK;
	}

	StorageRollCallRecord	*pRow = (StorageRollCallRecord *)pRows;
	U32						i;

	for( i = 0; i < numberOfRows; i++, pRow++ )
		FindHDDAndSetItsPhsIds(	pRow->ridDescriptorRecord,
								RowId(),
								RowId() );
	return OK;
}


STATUS 
DeviceManager::SrcTableRowModifiedCallback(  void *pRows, U32 numberOfRows, ShadowTable* ){


	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_DM_SRC_TABLE_BUILT;
		return OK;
	}

	StorageRollCallRecord	*pRow = (StorageRollCallRecord *)pRows;
	U32						i;

	for( i = 0; i < numberOfRows; i++, pRow++ )
		FindHDDAndSetItsPhsIds(	pRow->ridDescriptorRecord,
								pRow->ridStatusRecord,
								pRow->ridPerformanceRecord );
	return OK;
}


//************************************************************************
// FindHDDAndSetItsPhsIds
//************************************************************************

void 
DeviceManager::FindHDDAndSetItsPhsIds( RowId ridDisk, RowId ridStatus, RowId ridPerf ){
	
	DesignatorId	id, *pId;
	HDDDevice		*pDisk;
	CoolVector		phsIds, *pContainer = NULL;
	RowId			*pRid;
	PHSDataManager	*pM = (PHSDataManager *)GetObjectManager(SSAPI_MANAGER_CLASS_TYPE_PHS_DATA_MANAGER);
	

	if ( GetDesignatorIdByRowId( ridDisk, id ) ){
		pDisk = (HDDDevice *)GetManagedObject( &id );
		if( pDisk->SetPhsRowIds( ridStatus, ridPerf ) ){
			pContainer = new CoolVector;
			pDisk->PurgeAllPhsIds();
			pDisk->GetPhsDataRowId( phsIds );
			while( phsIds.Count() ){
				phsIds.GetAt( (CONTAINER_ELEMENT &)pRid, 0 );
				pM->GetObjectIdsByRowId( *pContainer, *pRid );
				for( U32 phsNum = 0; phsNum < pContainer->Count(); phsNum++ ){
					pContainer->GetAt( (CONTAINER_ELEMENT &) pId, phsNum );
					pDisk->AddPhsDataItem( *pId, false );
				}
				phsIds.RemoveAt( 0 );
				delete pRid;
				FreeMemoryForTheContainerWithIds( *pContainer );
			}
			GetListenManager()->PropagateObjectModifiedEventForManagers( GetDesignatorId(), pDisk);
		}
	}

	delete pContainer;
}


//************************************************************************
// UpdateDiskObjectsWithSRCData:
//
// PURPOSE:		Called when we first read the SRC table to update
//				disk objects with PHS row ids.
//************************************************************************

bool 
DeviceManager::UpdateDiskObjectsWithSRCData( StorageRollCallRecord *pRow, Container& ){
	
	FindHDDAndSetItsPhsIds(	pRow->ridDescriptorRecord,
							pRow->ridStatusRecord,
							pRow->ridPerformanceRecord );
	return true;
}


//************************************************************************
// GetTempTableData:
// 
// PURPOSE:		Retrieves pointer to temp table data
//************************************************************************

void* 
DeviceManager::GetTempTableData( U32 tableMask ){

	for( U32 i = 0; i < SSAPI_DM_NUMBER_OF_TABLES_USED; i++ )
		if( m_pTable[i].tableMask == tableMask )
			return m_pTable[i].pTempTable;


	ASSERT(0);
	return NULL;
}


//************************************************************************
// CreateObjectsFromPathRow:
//
// PURPOSE:		Called when the path decriptor table is enumerated for the
//				first time. Simply copies entries into the path 
//				descriptor collector
//************************************************************************

bool 
DeviceManager::CreateObjectsFromPathRow( PathDescriptor *pRow, Container &container ){

	m_pPathDescriptors->Add( pRow, sizeof( PathDescriptor ) );
	return true;
}


//************************************************************************
// PTS callbacks for the PathDescriptor table
//************************************************************************

STATUS 
DeviceManager::PathTableRowInsertedCallback(  void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_DM_PATH_TABLE_BUILT;
		return OK;
	}

	PathDescriptor			*pDescriptor = (PathDescriptor *)pRows;
	U32						i;

	for( i = 0; i < numberOfRows; i++, pDescriptor++ )
		m_pPathDescriptors->Add( pDescriptor, sizeof( PathDescriptor ) );

	return OK;
}


STATUS 
DeviceManager::PathTableRowDeletedCallback(  void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_DM_PATH_TABLE_BUILT;
		return OK;
	}

	PathDescriptor			*pDescriptor = (PathDescriptor *)pRows;
	U32						i;

	for( i = 0; i < numberOfRows; i++, pDescriptor++ )
		m_pPathDescriptors->Delete( pDescriptor->rid );

	return OK;
}


STATUS 
DeviceManager::PathTableRowModifiedCallback(  void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_DM_PATH_TABLE_BUILT;
		return OK;
	}

	PathDescriptor			*pDescriptor = (PathDescriptor *)pRows;
	U32						i;
	HDDDevice				*pDisk;
	DesignatorId			id;

	for( i = 0; i < numberOfRows; i++, pDescriptor++ ){
		// first update the descriptor in the collector
		m_pPathDescriptors->Delete( pDescriptor->rid );
		m_pPathDescriptors->Add( pDescriptor, sizeof( PathDescriptor ) );

		// now, update the disk if this is an active path
		GetDesignatorIdByRowId( pDescriptor->ridActiveDesc, id );
		if( ( pDisk = (HDDDevice *)GetManagedObject( &id ) ) != NULL ){
			pDisk->BuildYourselfFromPathRow( pDescriptor );
			GetListenManager()->PropagateObjectModifiedEventForManagers( GetDesignatorId(), pDisk );
		}
	}

	return OK;
}


//************************************************************************
// UpdateChassis:
//
// PURPOSE:		A routine called by the system timer to update the time 
//				value inside the Chassis object.
//************************************************************************

STATUS 
DeviceManager::UpdateChassis( Message *pMsg ){

	ManagedObject	*pChassis = GetChassisDevice();

#ifdef WIN32
	static time_t	lastCalled;
	time_t			temp;

	time( &temp );
	if( temp - lastCalled >= TIMER_UPDATE_RATE_FOR_CHASSIS_DEVICE ){
		lastCalled = temp;
		if( pChassis )
			GetListenManager()->PropagateObjectModifiedEventForManagers( GetDesignatorId(), pChassis );
	}
#else
	if( pChassis )
		GetListenManager()->PropagateObjectModifiedEventForManagers( GetDesignatorId(), pChassis );
#endif

	delete pMsg;
	return OK;
}


//************************************************************************
// AreThesePortsOnPartneredNacs:
//
// PURPOSE:		Determines if the two ports specified belong to NACs that
//				are failover partners.
//************************************************************************

bool 
DeviceManager::AreThesePortsOnPartneredNacs(const DesignatorId &port1, 
											const DesignatorId &port2 ){

	
	ClassTypeMap		map;
	FcPort				*pPort1, *pPort2;
	Iop					*pIop1, *pIop2;
	DesignatorId		id, id1, id2;

	// check the ids on validity
	if( !map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_PORT, port1.GetClassId(), true )
		||
		!map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_PORT, port2.GetClassId(), true ) )

		return false;

	// check if the objects are present
	id1 = port1; id2 = port2;
	pPort1 = (FcPort *) GetManagedObject( &id1 );
	pPort2 = (FcPort *) GetManagedObject( &id2 );
	if( !pPort1 || !pPort2 )
		return false;

	// get the Iops these ports are on and check if they are present
	id = GetIopBySlot( pPort1->GetSlotNumber() );
	pIop1 = (Iop *)GetManagedObject( &id );
	id = GetIopBySlot( pPort2->GetSlotNumber() );
	pIop2 = (Iop *)GetManagedObject( &id );
	if( !pIop1 || !pIop2 ){
		ASSERT(0);
		return false;
	}

	return (pIop1->GetRedundantSlotNumber() == pIop2->GetSlotNumber() );
}
