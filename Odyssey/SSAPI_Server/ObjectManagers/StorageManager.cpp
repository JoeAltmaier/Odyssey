//******************************************************************************
// FILE:		StorageManager.cpp
//
// PURPOSE:		Defines the object manager reponsible for managing storage
//				elements available in the O2K
//
// IMPLEMENTATION NOTES:
//				1> The idea is to have all descriptors available when an array\
//				is added. This way we need not do any additional reads and fear
//				that something gets modified while we're doing it. To accomplish
//				this, we need to collect all the descriptors there are 
//				in the system. We also need to update them as needed.
//******************************************************************************


#include "StorageManager.h"
#include "StorageElementDiskInternal.h"
#include "StorageElementDiskExternal.h"
#include "StorageElementArray0.h"
#include "StorageElementArray1HotCopyAuto.h"
#include "StorageElementArray1HotCopyManual.h"
#include "StorageCollectionSparePool.h"
#include "StorageElementPassThruTape.h"
#include "StorageElementPassThruSes.h"
#include "StorageElementPartition.h"
#include "StorageElementSsd.h"

#include "StringResourceManager.h"
#include "SList.h"
#include "SCSI.h"
#include "CoolVector.h"
#include "SsapiLocalResponder.h"
#include "SsdDescriptor.h"
#include "ListenManager.h"
#include "ClassTypeMap.h"
#include "PhsDataManager.h"
#include "SsapiEvents.h"
#include "SsdDescriptor.h"

#include "ArrayDescriptor.h"
#include "DiskDescriptor.h"
#include "StorageRollCallTable.h"
#include "RaidSpareDescriptor.h"
#include "RaidMemberTable.h"
#include "RaidUtilTable.h"
#include "PathDescriptor.h"
#include "DeviceDescriptor.h"

#include "RaidDefs.h"
#include "RmstrCmnds.h"
#include "RmstrEvents.h"
#include "RmstrErrors.h"
#include "Raid2SsapiErrorConvertor.h"

#include "PmstrCmnds.h"
#include "PmstrEvents.h"

#include "Trace_Index.h"
#ifdef TRACE_INDEX
#undef TRACE_INDEX
#endif
#define TRACE_INDEX TRACE_SSAPI_MANAGERS


StorageManager* StorageManager::m_pThis	= NULL;

//************************************************************************
// StorageManager:
//
// PURPOSE:		Default constructor
//************************************************************************

StorageManager::StorageManager( ListenManager			*pListenManager,	
								DdmServices				*pParent,
								StringResourceManager	*pSRManager )
:ObjectManager( pListenManager, DesignatorId( RowId(), SSAPI_MANAGER_CLASS_TYPE_STORAGE_MANAGER ), pParent){

	m_outstandingRequests = 0;
	m_pStringResourceManager = pSRManager;
	m_shouldRebuildAllTables = false;

	m_builtTablesMask	= 0;

	m_pTable = new SSAPI_STORAGE_MGR_JUMP_TABLE_RECORD[SSAPI_SM_NUMBER_OF_TABLES_USED];

	(m_pTable)->pTableName					= RAID_ARRAY_DESCRIPTOR_TABLE;
	(m_pTable)->tableMask					= SSAPI_SM_ARRAY_TABLE;
	(m_pTable)->pRowInsertedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,ArrayTableRowInserted);
	(m_pTable)->pRowDeletedCallback			= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,ArrayTableRowDeleted);
	(m_pTable)->pRowModifiedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,ArrayTableRowModified);
	(m_pTable)->pCreateObjectsFromRow		= (StorageManager::CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(StorageManager,CreateNoObjects);
	(m_pTable)->rowSize						= sizeof(RAID_ARRAY_DESCRIPTOR);
	(m_pTable)->pFieldDef					= (fieldDef*)ArrayDescriptorTable_FieldDefs;
	(m_pTable)->fieldDefSize				= sizeofArrayDescriptorTable_FieldDefs;

	(m_pTable + 1)->pTableName				= RAID_SPARE_DESCRIPTOR_TABLE;
	(m_pTable + 1)->tableMask				= SSAPI_SM_SPARE_TABLE;
	(m_pTable + 1)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,SpareTableRowInserted);
	(m_pTable + 1)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,SpareTableRowDeleted);
	(m_pTable + 1)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,SpareTableRowModified);
	(m_pTable + 1)->pCreateObjectsFromRow	= (StorageManager::CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(StorageManager,CreateNoObjects);
	(m_pTable + 1)->rowSize					= sizeof(RAID_SPARE_DESCRIPTOR);
	(m_pTable + 1)->pFieldDef				= (fieldDef*)SpareDescriptorTable_FieldDefs;
	(m_pTable + 1)->fieldDefSize			= sizeofSpareDescriptorTable_FieldDefs;

	(m_pTable + 2)->pTableName				= RAID_MEMBER_DESCRIPTOR_TABLE;
	(m_pTable + 2)->tableMask				= SSAPI_SM_MEMBER_TABLE;
	(m_pTable + 2)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,MemberTableRowInserted);
	(m_pTable + 2)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,MemberTableRowDeleted);
	(m_pTable + 2)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,MemberTableRowModified);
	(m_pTable + 2)->pCreateObjectsFromRow	= (StorageManager::CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(StorageManager,CreateNoObjects);
	(m_pTable + 2)->rowSize					= sizeof(RAID_ARRAY_MEMBER);
	(m_pTable + 2)->pFieldDef				= (fieldDef*)MemberDescriptorTable_FieldDefs;
	(m_pTable + 2)->fieldDefSize			= sizeofMemberDescriptorTable_FieldDefs;

	(m_pTable + 3)->pTableName				= DISK_DESC_TABLE;
	(m_pTable + 3)->tableMask				= SSAPI_SM_DISK_TABLE;
	(m_pTable + 3)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,DiskTableRowInserted);
	(m_pTable + 3)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,DiskTableRowDeleted);
	(m_pTable + 3)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,DiskTableRowModified);
	(m_pTable + 3)->pCreateObjectsFromRow	= (StorageManager::CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(StorageManager,CreateDiskObjects);
	(m_pTable + 3)->rowSize					= sizeof(DiskDescriptor);
	(m_pTable + 3)->pFieldDef				= (fieldDef*)DiskDescriptorTable_FieldDefs;
	(m_pTable + 3)->fieldDefSize			= cbDiskDescriptorTable_FieldDefs;


	(m_pTable + 4)->pTableName				= STORAGE_ROLL_CALL_TABLE;
	(m_pTable + 4)->tableMask				= SSAPI_SM_SRC_TABLE;
	(m_pTable + 4)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,SrcTableRowInserted);
	(m_pTable + 4)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,SrcTableRowDeleted);
	(m_pTable + 4)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,SrcTableRowModified);
	(m_pTable + 4)->pCreateObjectsFromRow	= (StorageManager::CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(StorageManager,CreateNoObjects);
	(m_pTable + 4)->rowSize					= sizeof(StorageRollCallRecord);
	(m_pTable + 4)->pFieldDef				= (fieldDef*)StorageRollCallTable_FieldDefs;
	(m_pTable + 4)->fieldDefSize			= cbStorageRollCallTable_FieldDefs;


	(m_pTable + 5)->pTableName				= PATH_DESC_TABLE;
	(m_pTable + 5)->tableMask				= SSAPI_SM_PATH_TABLE;
	(m_pTable + 5)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,PathTableRowInserted);
	(m_pTable + 5)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,PathTableRowDeleted);
	(m_pTable + 5)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,PathTableRowModified);
	(m_pTable + 5)->pCreateObjectsFromRow	= (StorageManager::CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(StorageManager,CreateNoObjects);
	(m_pTable + 5)->rowSize					= sizeof(PathDescriptor);
	(m_pTable + 5)->pFieldDef				= (fieldDef*)PathDescriptorTable_FieldDefs;
	(m_pTable + 5)->fieldDefSize			= cbPathDescriptorTable_FieldDefs;

	(m_pTable + 6)->pTableName				= DEVICE_DESC_TABLE;
	(m_pTable + 6)->tableMask				= SSAPI_SM_DEVICE_TABLE;
	(m_pTable + 6)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,DeviceTableRowInserted);
	(m_pTable + 6)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,DeviceTableRowDeleted);
	(m_pTable + 6)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,DeviceTableRowModified);
	(m_pTable + 6)->pCreateObjectsFromRow	= (StorageManager::CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(StorageManager,CreateNoObjects);
	(m_pTable + 6)->rowSize					= sizeof(DeviceDescriptor);
	(m_pTable + 6)->pFieldDef				= (fieldDef*)DeviceDescriptorTable_FieldDefs;
	(m_pTable + 6)->fieldDefSize			= cbDeviceDescriptorTable_FieldDefs;

	(m_pTable + 7)->pTableName				= PARTITION_DESCRIPTOR_TABLE;
	(m_pTable + 7)->tableMask				= SSAPI_SM_PART_TABLE;
	(m_pTable + 7)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,PartitionTableRowInserted);
	(m_pTable + 7)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,PartitionTableRowDeleted);
	(m_pTable + 7)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,PartitionTableRowModified);
	(m_pTable + 7)->pCreateObjectsFromRow	= (StorageManager::CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(StorageManager,CreateNoObjects);
	(m_pTable + 7)->rowSize					= sizeof(PARTITION_DESCRIPTOR);
	(m_pTable + 7)->pFieldDef				= (fieldDef*)PartitionDescriptorTable_FieldDefs;
	(m_pTable + 7)->fieldDefSize			= sizeofPartitionDescriptorTable_FieldDefs;

	(m_pTable + 8)->pTableName				= SSD_DESCRIPTOR_TABLE;
	(m_pTable + 8)->tableMask				= SSAPI_SM_SSD_TABLE;
	(m_pTable + 8)->pRowInsertedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,SsdTableRowInserted);
	(m_pTable + 8)->pRowDeletedCallback		= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,SsdTableRowDeleted);
	(m_pTable + 8)->pRowModifiedCallback	= (ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(StorageManager,SsdTableRowModified);
	(m_pTable + 8)->pCreateObjectsFromRow	= (StorageManager::CREATE_OBJECTS_FROM_ROW)METHOD_ADDRESS(StorageManager,CreateNoObjects);
	(m_pTable + 8)->rowSize					= sizeof(SSD_Descriptor);
	(m_pTable + 8)->pFieldDef				= (fieldDef*)SSD_descriptor_table_field_defs;
	(m_pTable + 8)->fieldDefSize			= cb_SSD_descriptor_table_field_defs;
	
	for( int i = 0; i < SSAPI_SM_NUMBER_OF_TABLES_USED; i++ )
		(m_pTable + i)->pShadowTable	= new ShadowTable(	(m_pTable + i)->pTableName,
															this,
															(m_pTable + i)->pRowInsertedCallback,
															(m_pTable + i)->pRowDeletedCallback,
															(m_pTable + i)->pRowModifiedCallback,
															(m_pTable + i)->rowSize );
	SetIsReadyToServiceRequests( false );

	m_isIniting			= true;
	m_tablesToRebuild	= 0;

	// register for OBJECT_ADDED events
	m_pObjectAddedResponder = new SsapiLocalResponder( this, (LOCAL_EVENT_CALLBACK)METHOD_ADDRESS(StorageManager,ObjectAddedEventCallback) ); 
	GetListenManager()->AddListenerForObjectAddedEvent( SSAPI_LISTEN_OWNER_ID_ANY, m_pObjectAddedResponder->GetSessionID(), CALLBACK_METHOD(m_pObjectAddedResponder, 1) );
	
	SSAPI_TRACE( TRACE_L2, "\nStorageManager: Intializing...." );

	// Initialize communication channels with the RAID Master
	m_pRaidCommandQ = new CmdSender(RMSTR_CMD_QUEUE_TABLE,
									sizeof(RMSTR_CMND_INFO),
									sizeof(RMSTR_EVENT_INFO),
									this);
	m_pRaidCommandQ->csndrInitialize((pInitializeCallback_t)METHOD_ADDRESS(StorageManager,RaidCmdQInitReply));
	AddOutstandingRequest();
	m_pRaidCommandQ->csndrRegisterForEvents((pEventCallback_t)METHOD_ADDRESS(StorageManager,RaidEventHandler));

	
	// Initialize communication channels with the partition master
	m_pPartitionCommandQ = new CmdSender(PARTITION_MSTR_CMD_QUEUE,
										sizeof(PMSTR_CMND_INFO),
										sizeof(PMSTR_EVENT_INFO),
										this);
	m_pPartitionCommandQ->csndrInitialize((pInitializeCallback_t)METHOD_ADDRESS(StorageManager,PartitionCmdQInitReply));
	m_pPartitionCommandQ->csndrRegisterForEvents((pEventCallback_t)METHOD_ADDRESS(StorageManager,PartitionEventHandler));


	DefineAllTables();
	AddOutstandingRequest();
}


//************************************************************************
// StorageManager:
//
// PURPOSE:		The destructor
//************************************************************************

StorageManager::~StorageManager(){

	delete m_pRaidCommandQ;
	delete m_pPartitionCommandQ;

	GetListenManager()->DeleteListenerForObjectAddedEvent( SSAPI_LISTEN_OWNER_ID_ANY, m_pObjectAddedResponder->GetSessionID() );
	delete m_pObjectAddedResponder;
	
	for( int i = 0; i < SSAPI_SM_NUMBER_OF_TABLES_USED; i++ )
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
StorageManager::Dispatch( ValueSet *pRequestParms, U32 requestCode, SsapiResponder *pResponder){

	DesignatorId		arrayId, spareId, poolId, memberId, elementId;
	UnicodeString		name;
	ValueSet			*pVs;

	if( ObjectManager::Dispatch( pRequestParms, requestCode, pResponder ) )
		return true;

	switch( requestCode ){
		case SSAPI_STORAGE_MANAGER_ADD_ARRAY:
			InsertNameAndContinue( pRequestParms, pResponder, METHOD_ADDRESS(StorageManager, CreateArray), true );
			break;

		case SSAPI_STORAGE_MANAGER_ADD_HOT_COPY:
			InsertNameAndContinue( pRequestParms, pResponder, METHOD_ADDRESS(StorageManager, AddHotCopy), false );
			break;

		case SSAPI_STORAGE_MANAGER_BREAK_HOT_COPY:
			RemoveHotCopy( *pRequestParms, pResponder );
			break;

		case SSAPI_STORAGE_MANAGER_ADD_DEDICATED_SPARE:
			pRequestParms->GetGenericValue( (char *)&arrayId, sizeof(arrayId), SSAPI_STORAGE_MANAGER_ADD_DEDICATED_SPARE_ARRAY_ID );
			pRequestParms->GetGenericValue( (char *)&spareId, sizeof(spareId), SSAPI_STORAGE_MANAGER_ADD_DEDICATED_SPARE_NEW_SPARE_ID );
			AddDedicatedSpare( arrayId, spareId, pResponder );
			break;


		case SSAPI_STORAGE_MANAGER_ADD_POOL_SPARE:
			pRequestParms->GetGenericValue( (char *)&poolId, sizeof(poolId), SSAPI_STORAGE_MANAGER_ADD_POOL_SPARE_POOL_ID );
			pRequestParms->GetGenericValue( (char *)&spareId, sizeof(spareId), SSAPI_STORAGE_MANAGER_ADD_POOL_SPARE_NEW_SPARE_ID );
			AddPoolSpare( poolId, spareId, pResponder );
			break;

		case SSAPI_STORAGE_MANAGER_DELETE_DEDICATED_SPARE:
			pRequestParms->GetGenericValue( (char *)&arrayId, sizeof(arrayId), SSAPI_STORAGE_MANAGER_DELETE_DEDICATED_SPARE_ARRAY_ID );
			pRequestParms->GetGenericValue( (char *)&spareId, sizeof(spareId), SSAPI_STORAGE_MANAGER_DELETE_DEDICATED_SPARE_SPARE_ID );
			DeleteDedicatedSpare( arrayId, spareId, pResponder );
			break;

		case SSAPI_STORAGE_MANAGER_DELETE_POOL_SPARE:
			pRequestParms->GetGenericValue( (char *)&poolId, sizeof(poolId), SSAPI_STORAGE_MANAGER_DELETE_POOL_SPARE_POOL_ID );
			pRequestParms->GetGenericValue( (char *)&spareId, sizeof(spareId), SSAPI_STORAGE_MANAGER_DELETE_POOL_SPARE_SPARE_ID );
			DeletePoolSpare( poolId, spareId, pResponder );
			break;

		case SSAPI_STORAGE_MANAGER_ADD_MEMBER:
			pRequestParms->GetGenericValue( (char *)&arrayId, sizeof(arrayId), SSAPI_STORAGE_MANAGER_ADD_MEMBER_ARRAY_ID);
			pRequestParms->GetGenericValue( (char *)&memberId, sizeof(memberId), SSAPI_STORAGE_MANAGER_ADD_MEMBER_NEW_MEMBER_ID);
			AddArrayMember( arrayId, memberId, pResponder );
			break;

		case SSAPI_STORAGE_MANAGER_REMOVE_MEMBER:
			pRequestParms->GetGenericValue( (char *)&arrayId, sizeof(arrayId), SSAPI_STORAGE_MANAGER_REMOVE_MEMBER_ARRAY_ID);
			pRequestParms->GetGenericValue( (char *)&memberId, sizeof(memberId), SSAPI_STORAGE_MANAGER_REMOVE_MEMBER_MEMBER_ID);
			DeleteArrayMember( arrayId, memberId, pResponder );
			break;

		case SSAPI_STORAGE_MANAGER_CHANGE_STORAGE_NAME:
			pRequestParms->GetGenericValue( (char *)&elementId, sizeof(elementId), SSAPI_STORAGE_MANAGER_CHANGE_STORAGE_NAME_ELEMENT_ID );
			pRequestParms->GetString( SSAPI_STORAGE_MANAGER_CHANGE_STORAGE_NAME_NEW_NAME, &name );
			ChangeStorageElementName( elementId, name, pResponder );
			break;

		case SSAPI_STORAGE_MANAGER_SET_PREF_MEMBER:
			pRequestParms->GetGenericValue( (char *)&arrayId, sizeof(arrayId), SSAPI_STORAGE_MANAGER_SET_PREF_MEMBER_ARRAY_ID);
			pRequestParms->GetGenericValue( (char *)&memberId, sizeof(memberId), SSAPI_STORAGE_MANAGER_SET_PREF_MEMBER_MEMBER_ID);
			SetPreferredMember( arrayId, memberId, pResponder );
			break;

		case SSAPI_STORAGE_MANAGER_SET_SOURCE_MEMBER:
			pRequestParms->GetGenericValue( (char *)&arrayId, sizeof(arrayId), SSAPI_STORAGE_MANAGER_SET_SOURCE_MEMBER_ARRAY_ID);
			pRequestParms->GetGenericValue( (char *)&memberId, sizeof(memberId), SSAPI_STORAGE_MANAGER_SET_SOURCE_MEMBER_MEMBER_ID);
			SetSourceMember( arrayId, memberId, pResponder );
			break;

		case SSAPI_STORAGE_MANAGER_MERGE_PARTITIONS:
			pVs = (ValueSet *)pRequestParms->GetValue( SSAPI_STORAGE_MANAGER_MERGE_PARTITIONS_PARTITON_ID_VECTOR );
			ASSERT( pVs );
			MergePartitions( pVs, pResponder );
			break;

		case SSAPI_STORAGE_MANAGER_CREATE_PARTITION:
			pVs = (ValueSet *)pRequestParms->GetValue(SSAPI_STORAGE_MANAGER_CREATE_PARTITION_NEW_PARTITION_OBJECT);
			ASSERT( pVs );
			pRequestParms->GetString( SSAPI_STORAGE_MANAGER_CREATE_PARTITION_REMAINDER_NAME, &name );
			CreatePartitionStart( *pVs, name, pResponder );
			break;

		default:
			ASSERT(0);
			pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
	}

	return true;
}


//************************************************************************
// DefineAllTables:
//
// PURPOSE:		Issues requests to the PTS to define all tables if not yet
//				defined
//************************************************************************

STATUS 
StorageManager::DefineAllTables(){

	STATUS	status = OK;

	for( int i = 0; i < SSAPI_SM_NUMBER_OF_TABLES_USED; i++ )
		status	|= (m_pTable + i)->pShadowTable->DefineTable(	(m_pTable + i)->pFieldDef,
																(m_pTable + i)->fieldDefSize,
																32,
																(pTSCallback_t)METHOD_ADDRESS(StorageManager,DefineAllTablesReplyHandler),
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
StorageManager::DefineAllTablesReplyHandler( void *pContext, STATUS status ){

	U32			mask	= (U32)pContext;
	
	m_builtTablesMask	|= mask;
	if( (m_builtTablesMask & SSAPI_SM_ALL_TABLES_BUILT) == SSAPI_SM_ALL_TABLES_BUILT ){
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
StorageManager::InitializeAllTables(){

	STATUS status = OK;

	for( int i = 0; i < SSAPI_SM_NUMBER_OF_TABLES_USED; i++ )
		status |= (m_pTable + i)->pShadowTable->Initialize(	(pTSCallback_t)METHOD_ADDRESS(StorageManager,InitializeAllTablesReplyHandler), 
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
StorageManager::InitializeAllTablesReplyHandler( void *pContext, STATUS status ){

	U32			mask	= (U32)pContext;
	
	m_builtTablesMask	|= mask;
	if( (m_builtTablesMask & SSAPI_SM_ALL_TABLES_BUILT) == SSAPI_SM_ALL_TABLES_BUILT ){
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
StorageManager::EnumerateAllTables(){

	STATUS		status = OK;
	
	for( int i = 0; i < SSAPI_SM_NUMBER_OF_TABLES_USED; i++ )
		status |= (m_pTable + i)->pShadowTable->Enumerate(	&(m_pTable + i)->pTempTable,
															(pTSCallback_t)METHOD_ADDRESS(StorageManager,EnumerateAllTablesReplyHandler),
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
StorageManager::EnumerateAllTablesReplyHandler( void *pContext, STATUS rc ){

	U32				mask = (U32)pContext;
	CoolVector		container;

	for( int i = 0; i < SSAPI_SM_NUMBER_OF_TABLES_USED; i++ ){
		if( mask == (m_pTable + i)->tableMask ){
			if( m_tablesToRebuild & (m_pTable + i)->tableMask ){
				m_tablesToRebuild &= ~(m_pTable + i)->tableMask;
				delete (m_pTable + i)->pTempTable;
				return (m_pTable + i)->pShadowTable->Enumerate(	&(m_pTable + i)->pTempTable, (pTSCallback_t)METHOD_ADDRESS(StorageManager,EnumerateAllTablesReplyHandler), (void *)(m_pTable + i)->tableMask);
			}
			else{
				m_builtTablesMask |= mask;
				break;
			}
		}
	}
	
	if( (m_builtTablesMask & SSAPI_SM_ALL_TABLES_BUILT) == SSAPI_SM_ALL_TABLES_BUILT ){

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
StorageManager::AllTablesHaveBeenEnumerated(){
	
	Container			*pContainer = new CoolVector;		

	m_isIniting = false;

	SaveDescriptors( SSAPI_SM_SPARE_TABLE, sizeof(RAID_SPARE_DESCRIPTOR), m_spareDescriptorCollector );
	SaveDescriptors( SSAPI_SM_MEMBER_TABLE, sizeof(RAID_ARRAY_MEMBER), m_memberDescriptorCollector );
	SaveDescriptors( SSAPI_SM_DISK_TABLE, sizeof(DiskDescriptor), m_descriptorCollector );
	SaveDescriptors( SSAPI_SM_SSD_TABLE, sizeof(SSD_Descriptor), m_descriptorCollector );
	SaveDescriptors( SSAPI_SM_PART_TABLE, sizeof(PARTITION_DESCRIPTOR), m_descriptorCollector );

	CreateDiskObjects( *pContainer, NULL, 0 );
	AddObjectsIntoManagedObjectsVector( *pContainer );
	pContainer->RemoveAll();

	CreateSsdObjects( *pContainer, NULL, 0 );
	AddObjectsIntoManagedObjectsVector( *pContainer );
	pContainer->RemoveAll();
	
	CreateAndAddPartitionObjects( *pContainer, NULL, 0 );

	SaveDescriptors( SSAPI_SM_ARRAY_TABLE, sizeof(RAID_ARRAY_DESCRIPTOR), m_descriptorCollector );
	CreateAndAddArrayObjects( NULL, 0 );

	CreateSparePoolObjects( *pContainer, NULL, 0  );
	AddObjectsIntoManagedObjectsVector( *pContainer );

	pContainer->RemoveAll();
	CreateDeviceObjects( *pContainer, GetTempTableData(SSAPI_SM_DEVICE_TABLE) , GetShadowTable(SSAPI_SM_DEVICE_TABLE)->GetNumberOfRows() );
	AddObjectsIntoManagedObjectsVector( *pContainer );

	ShuffleReferencesInArraysAndPartitions();
	SSAPI_TRACE( TRACE_L2, "\nStorageManager: ...Done! Objects built: ", GetManagedObjectCount() );
	RemoveOutstandingRequest();

	ClearAllTempTables();
	delete pContainer;
}


//************************************************************************
// SaveDescriptors:
//
// PURPOSE:		Saves descriptors contained in the table specified by the
//				'tableMask'
//************************************************************************

void 
StorageManager::SaveDescriptors( U32 tableMask, U32 descriptorSize, DescriptorCollector& collector ){

	U32					i;
	char				*pDescriptor;

	pDescriptor = (char *)GetTempTableData( tableMask );

	for( i =0 ; i < GetShadowTable(tableMask)->GetNumberOfRows(); i++, pDescriptor += descriptorSize )
		collector.Add( pDescriptor, descriptorSize );
}


//******************************************************************************
// PTS listen callbacks for RAID tables
//******************************************************************************


STATUS 
StorageManager::MemberTableRowModified( void *pRows, U32 rowCount, ShadowTable *p ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_MEMBER_TABLE;
		return OK;
	}

	// update the descriptor
	MemberTableRowDeleted( pRows, rowCount, p );
	return MemberTableRowInserted( pRows, rowCount, p );
}


STATUS 
StorageManager::MemberTableRowInserted( void *pRows, U32 rowCount, ShadowTable * ){


	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_MEMBER_TABLE;
		return OK;
	}

	RAID_ARRAY_MEMBER	*p = (RAID_ARRAY_MEMBER *)pRows;
	U32					i;

	for( i = 0; i < rowCount; i++, p++ )
		m_memberDescriptorCollector.Add( p, sizeof(RAID_ARRAY_MEMBER) );

	return OK;
}


STATUS 
StorageManager::MemberTableRowDeleted( void *pRows, U32 rowCount, ShadowTable * ){


	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_MEMBER_TABLE;
		return OK;
	}

	RAID_ARRAY_MEMBER	*p = (RAID_ARRAY_MEMBER *)pRows;
	U32					i;

	for( i = 0; i < rowCount; i++, p++ )
		m_memberDescriptorCollector.Delete( p->thisRID );

	return OK;
}


//******************************************************************************
// PTS Callbacks for the Array Descriptor Table
//******************************************************************************

STATUS 
StorageManager::ArrayTableRowInserted( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_ARRAY_TABLE;
		return OK;
	}

	U32						i;
	RAID_ARRAY_DESCRIPTOR	*pDesc = (RAID_ARRAY_DESCRIPTOR *)pRows;

	for( i = 0; i < numberOfRows; i++, pDesc++ )
		m_descriptorCollector.Add( (void *)pDesc, sizeof(RAID_ARRAY_DESCRIPTOR) );

	return OK;
}


STATUS 
StorageManager::ArrayTableRowDeleted( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_ARRAY_TABLE;
		return OK;
	}

	U32						i;
	RAID_ARRAY_DESCRIPTOR	*pDesc = (RAID_ARRAY_DESCRIPTOR *)pRows;

	for( i = 0; i < numberOfRows; i++, pDesc++ )
		m_descriptorCollector.Delete( pDesc->thisRID );

	return OK;
}


STATUS 
StorageManager::ArrayTableRowModified( void *pRows, U32 numberOfRows, ShadowTable *p ){

	U32						i;
	RAID_ARRAY_DESCRIPTOR	*pDesc = (RAID_ARRAY_DESCRIPTOR *)pRows;
	StorageElementArray		*pArray;


	// update the descriptor stored
	ArrayTableRowDeleted( pRows, numberOfRows, p );
	ArrayTableRowInserted( pRows, numberOfRows, p );

	// update the descriptor data in the array object
	for( i = 0; i < numberOfRows; i++, pDesc++ ){
		pArray = GetArrayByArrayRowId( pDesc->thisRID );
		if( pArray ) 
			pArray->BuildYourselfFromPtsRow( pDesc, this );
	}

	return OK;
}


STATUS 
StorageManager::SpareTableRowModified( void *pRows , U32 rowCount, ShadowTable *p ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_SPARE_TABLE;
		return OK;
	}

	// update the descriptor
	SpareTableRowDeleted( pRows, rowCount, p);
	return SpareTableRowInserted( pRows, rowCount, p);
}


STATUS 
StorageManager::SpareTableRowInserted( void *pRows , U32 rowCount, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_SPARE_TABLE;
		return OK;
	}

	U32						i;
	RAID_SPARE_DESCRIPTOR	*p = (RAID_SPARE_DESCRIPTOR *)pRows;

	for( i = 0; i < rowCount; i++, p++ )
		m_spareDescriptorCollector.Add( p, sizeof(RAID_SPARE_DESCRIPTOR));

	return OK;
}


STATUS 
StorageManager::SpareTableRowDeleted( void *pRows , U32 rowCount, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_SPARE_TABLE;
		return OK;
	}

	U32						i;
	RAID_SPARE_DESCRIPTOR	*p = (RAID_SPARE_DESCRIPTOR *)pRows;

	for( i = 0; i < rowCount; i++, p++ )
		m_spareDescriptorCollector.Delete( p->thisRID );

	return OK;
}


//******************************************************************************
// PTS callbacks for the Disk Descriptor
//******************************************************************************

STATUS 
StorageManager::DiskTableRowInserted( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_DISK_TABLE;
		return OK;
	}

	SSAPI_TRACE( TRACE_L3, "\nStorageManager::DiskTableRowInserted" );
	
	DiskDescriptor	*pRow = (DiskDescriptor *)pRows;
	U32				i;

	// this should soon be used in the SRC table, so collect it
	for( i = 0; i < numberOfRows; i++, pRow++ )
		m_descriptorCollector.Add( (void *)pRow, sizeof(DiskDescriptor) );

	return OK;
}


STATUS 
StorageManager::DiskTableRowDeleted( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_DISK_TABLE;
		return OK;
	}

	DiskDescriptor	*pDescriptor =  (DiskDescriptor *)pRows;
	U32				i;
	
	SSAPI_TRACE( TRACE_L3, "\nStorageManager::DiskTableRowDeleted" );
	
	for( i = 0; i < numberOfRows; i++, pDescriptor++ )
		m_descriptorCollector.Delete( pDescriptor->rid );
	
	// we should get notification from SRC
	return OK;
}


STATUS 
StorageManager::DiskTableRowModified( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_DISK_TABLE;
		return OK;
	}

	SSAPI_TRACE( TRACE_L3, "\nStorageManager::DiskTableRowModified" );

	StorageElementDisk	*pDisk;
	U32					i;
	DiskDescriptor		*pRow = (DiskDescriptor *)pRows;

	for( i = 0; i < numberOfRows; i++, pRow++ ){
		// check that we have this disk, and then update it
		pDisk = (StorageElementDisk *)GetElementByDescriptorRowId( pRow->rid );
		if( pDisk ){
			pDisk->BuildYourselfFromPtsRow( pRow );
			GetListenManager()->PropagateObjectModifiedEventForManagers( GetDesignatorId(), pDisk );
			m_descriptorCollector.Delete( pRow->rid );
			m_descriptorCollector.Add( pRow, sizeof(DiskDescriptor) );
		}
	}
	
	return OK;
}


//******************************************************************************
// PTS callbacks for the SSD descriptor Table
//******************************************************************************

STATUS 
StorageManager::SsdTableRowInserted( void *pRows, U32 numberOfRows,	ShadowTable*){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_SSD_TABLE;
		return OK;
	}

	SSAPI_TRACE( TRACE_L3, "\nStorageManager::SsdTableRowInserted" );
	
	DiskDescriptor	*pRow = (DiskDescriptor *)pRows;
	U32				i;

	// this should soon be used in the SRC table, so collect it
	for( i = 0; i < numberOfRows; i++, pRow++ )
		m_descriptorCollector.Add( (void *)pRow, sizeof(DiskDescriptor) );

	return OK;
}


STATUS
StorageManager::SsdTableRowDeleted( void *pRows,	U32 numberOfRows,	ShadowTable*){


	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_SSD_TABLE;
		return OK;
	}

	DiskDescriptor	*pDescriptor =  (DiskDescriptor *)pRows;
	U32				i;
	
	SSAPI_TRACE( TRACE_L3, "\nStorageManager::SsdTableRowDeleted" );
	
	for( i = 0; i < numberOfRows; i++, pDescriptor++ )
		m_descriptorCollector.Delete( pDescriptor->rid );
	
	// we should get notification from SRC
	return OK;
}


STATUS 
StorageManager::SsdTableRowModified( void *pRows, U32 numberOfRows,	ShadowTable*){


	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_SSD_TABLE;
		return OK;
	}

	SSAPI_TRACE( TRACE_L3, "\nStorageManager::SsdTableRowModified" );

	StorageElementSsd	*pSsd;
	U32					i;
	DiskDescriptor		*pRow = (DiskDescriptor *)pRows;

	for( i = 0; i < numberOfRows; i++, pRow++ ){
		// check that we have this board, and then update it
		pSsd = (StorageElementSsd *)GetElementByDescriptorRowId( pRow->rid );
		if( pSsd ){
			pSsd->BuildYourselfFromPtsRow( pRow );
			GetListenManager()->PropagateObjectModifiedEventForManagers( GetDesignatorId(), pSsd );
			m_descriptorCollector.Delete( pRow->rid );
			m_descriptorCollector.Add( pRow, sizeof(DiskDescriptor) );
		}
	}
	
	return OK;
}


//******************************************************************************
// PTS Callbacks for the Partition Descriptor Table
//******************************************************************************

STATUS 
StorageManager::PartitionTableRowInserted( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_PART_TABLE;
		return OK;
	}
	
	PARTITION_DESCRIPTOR	*pDescr = (PARTITION_DESCRIPTOR *)pRows;
	U32						index;

	for( index = 0; index < numberOfRows; index++, pDescr++ )
		m_descriptorCollector.Add( pDescr, sizeof(PARTITION_DESCRIPTOR) );

	return OK;
}


STATUS 
StorageManager::PartitionTableRowDeleted( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_PART_TABLE;
		return OK;
	}

	PARTITION_DESCRIPTOR	*pDescr = (PARTITION_DESCRIPTOR *)pRows;
	U32						index;

	for( index = 0; index < numberOfRows; index++, pDescr++ )
		m_descriptorCollector.Delete( pDescr->rid );

	return OK;
}


STATUS 
StorageManager::PartitionTableRowModified( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_PART_TABLE;
		return OK;
	}

	PARTITION_DESCRIPTOR	*pDescr = (PARTITION_DESCRIPTOR *)pRows;
	U32						index;

	for( index = 0; index < numberOfRows; index++, pDescr++ ){
		m_descriptorCollector.Delete( pDescr->rid );
		m_descriptorCollector.Add( pDescr, sizeof(PARTITION_DESCRIPTOR) );
	}

	return OK;
}


//******************************************************************************
// PTS callbacks for the SRC Table
//******************************************************************************

STATUS 
StorageManager::SrcTableRowInserted( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_SRC_TABLE;
		return OK;
	}

	SSAPI_TRACE( TRACE_L3, "\nStorageManager::SrcTableRowInserted" );

	StorageRollCallRecord	*pRecord = (StorageRollCallRecord *)pRows;
	U32						i;
	CoolVector				container;
	StorageElementBase		*pElement;
	DesignatorId			id;

	// we only need Disk, SSD, and PassThru stuff:
	for( i = 0; i < numberOfRows; i++, pRecord++ ){
		if( (pRecord->storageclass == SRCTypeFCDisk) || (pRecord->storageclass == SRCTypeExternalFCDisk) )
			CreateDiskObjects( container, pRecord, 1 );
		else if( pRecord->storageclass == SRCTypeSSD)
			CreateSsdObjects( container, pRecord, 1 );
		else
			continue;

		container.GetAt( (CONTAINER_ELEMENT &)pElement, 0 );
		if( pElement )
			id = pElement->GetDesignatorId();
		AddObjectsIntoManagedObjectsVector( container );
		if( GetSparePool() && pElement ){
			pElement = (StorageElementBase *)GetManagedObject( &id );
			if( GetSparePool()->IsYourChild( pElement->GetDesignatorId() ) )
				pElement->AddParentId( GetSparePool() );
		}
	}


	return OK;
}


STATUS 
StorageManager::SrcTableRowDeleted( void *pRows, U32 numberOfRows, ShadowTable* ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_SRC_TABLE;
		return OK;
	}

	SSAPI_TRACE( TRACE_L3, "\nStorageManager::SrcTableRowDeleted" );

	StorageRollCallRecord	*pRecord = (StorageRollCallRecord *)pRows;
	U32						i;
	CoolVector				container;
	StorageElementBase		*pElement;

	// we only need Disk, SSD, and PassThru stuff:
	for( i = 0; i < numberOfRows; i++, pRecord++ ){
		if( (pRecord->storageclass == SRCTypeFCDisk) || (pRecord->storageclass == SRCTypeExternalFCDisk) )
			CreateDiskObjects( container, pRecord, 1 );
		else if( pRecord->storageclass == SRCTypeSSD)
			CreateSsdObjects( container, pRecord, 1 );
		else
			continue;
	}

	DeleteObjectsFromTheSystem( container );

	while( container.Count() ){
		container.GetAt( (CONTAINER_ELEMENT &)pElement, 0 );
		container.RemoveAt( 0 );
		delete pElement;
	}

	return OK;
}


STATUS 
StorageManager::SrcTableRowModified( void *pRows, U32 numberOfRows, ShadowTable *p ){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_SRC_TABLE;
		return OK;
	}

	DesignatorId			id;
	StorageElementBase		*pElement;
	U32						i;
	StorageRollCallRecord	*pRow = (StorageRollCallRecord *)pRows;
	ManagedObject			*pParent;
	void					*pTemp;

	if( numberOfRows == 1 && m_pendingSrcUpdates.Get( (RowId &)pRow->rid, pTemp ) ){
		m_pendingSrcUpdates.Delete( pRow->rid );
		SSAPI_TRACE( TRACE_L2, "\nStorageManager: ignoring redundant update --> SRC" );
		return OK;
	}

	SSAPI_TRACE( TRACE_L3, "\nStorageManager::SrcTableRowModified" );

	for( i = 0; i < numberOfRows; i++, pRow++ ){
		GetDesignatorIdByRowId( pRow->rid, id );
		pElement = (StorageElement *)GetManagedObject( &id );
		if( pElement && (pElement->GetRidName() != pRow->ridName) ){
			pElement->SetRidName( pRow->ridName );
			ReadElementName( pElement );
		}

		if( pElement ){
			if( ( pElement->GetRidStatus() != pRow->ridStatusRecord ) ||
				( pElement->GetRidPerformance() != pRow->ridPerformanceRecord ) ){

				pElement->PurgeAllPhsIds();
				pElement->BuildYourselfFromSRCRow( pRow );
				BuildPhsDataIdVector( pElement, true );
			}
			else
				pElement->BuildYourselfFromSRCRow( pRow );
			GetListenManager()->PropagateObjectModifiedEventForManagers( GetDesignatorId(), pElement );
		}

		// make sure that if it's available, it is neither a spare no or member
		if( pElement && !pRow->fUsed ){
			if( pElement->GetIsMember() ){
				pElement->UnsetAsMember();
				pParent = pElement->GetParent( 0 );
				ASSERT(pParent);
				pElement->DeleteParentId( pParent->GetDesignatorId() );
				pParent->DeleteChildId( pElement );
			}
			else if( pElement->GetIsSpare() || pElement->GetIsPoolSpare() ){
				pElement->UnsetAsSpare();
				pParent = pElement->GetParent( 0 );
				ASSERT(pParent);
				pElement->DeleteParentId( pParent->GetDesignatorId() );
				pParent->DeleteChildId( pElement );
			}
		}

		// make sure that if it used, we know about it
	}

	return OK;
}

//******************************************************************************
// CreateDiskObjects:
//
// PURPOSE:		Method that create objects from tables
//******************************************************************************

bool 
StorageManager::CreateDiskObjects( Container &container, void *pRows, U32 rowCount  ){

	U32						i, pathNum;
	DiskDescriptor			*pDescriptor;
	StorageRollCallRecord	*pRow;
	StorageElementDisk		*pDisk;
	PathDescriptor			*pPath;
	DeviceManager			*pDM = (DeviceManager *)GetObjectManager(SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER);
	DescriptorCollector		&pathDescriptors = (DescriptorCollector	&)pDM->GetPathDescriptors();

	if( !pRows ){ // get the temp table data
		pRows = GetTempTableData( SSAPI_SM_SRC_TABLE );
		rowCount = GetShadowTable( SSAPI_SM_SRC_TABLE )->GetNumberOfRows();
	}

	for( i = 0, pRow = (StorageRollCallRecord *)pRows; i < rowCount; i++, pRow++ ){

		if( pRow->storageclass == SRCTypeExternalFCDisk )
			pDisk = new StorageElementDiskExternal( GetListenManager(), this );
		else if( pRow->storageclass == SRCTypeFCDisk )
			pDisk = new StorageElementDiskInternal( GetListenManager(), this );
		else
			continue;


		pDisk->BuildYourselfFromSRCRow( pRow );
		
		// get descriptor from descriptor collector
		if( !m_descriptorCollector.Get( (RowId &)pRow->ridDescriptorRecord, (void *&)pDescriptor ) )
				ASSERT(0);

		pDisk->BuildYourselfFromPtsRow( pDescriptor );

		// find the paths for this disk
		for( pathNum = 0; pathDescriptors.GetAt( pathNum, (void *&)pPath ); pathNum++ )
			if( RowId( pDescriptor->rid ) == pPath->ridDescriptor )
				pDisk->BuildYourselfFromPathRow( pPath );

		// continue ...
		RecoverName( *pDisk );
		ReadElementName( pDisk );
		RecoverParentIds( pDisk );
		RecoverChildIds( pDisk );
		BuildPhsDataIdVector( pDisk );
		container.Add( (CONTAINER_ELEMENT)pDisk );
	}

	return true;
}


bool 
StorageManager::CreateSsdObjects( Container &container, void *pRows, U32 rowCount ){

	U32						i;
	DiskDescriptor			*pDescriptor;
	StorageRollCallRecord	*pRow;
	StorageElementSsd		*pSsd;

	if( !pRows ){ // get the temp table data
		pRows = GetTempTableData( SSAPI_SM_SRC_TABLE );
		rowCount = GetShadowTable( SSAPI_SM_SRC_TABLE )->GetNumberOfRows();
	}

	for( i = 0, pRow = (StorageRollCallRecord *)pRows; i < rowCount; i++, pRow++ ){

		if( pRow->storageclass == SRCTypeSSD )
			pSsd = new StorageElementSsd( GetListenManager(), this );
		else
			continue;

		pSsd->BuildYourselfFromSRCRow( pRow );
		
		// get descriptor from descriptor collector
		if( !m_descriptorCollector.Get( (RowId &)pRow->ridDescriptorRecord, (void *&)pDescriptor ) )
				ASSERT(0);

		pSsd->BuildYourselfFromPtsRow( pDescriptor );
		RecoverName( *pSsd );
		ReadElementName( pSsd );
		RecoverParentIds( pSsd );
		RecoverChildIds( pSsd );
		BuildPhsDataIdVector( pSsd );
		container.Add( (CONTAINER_ELEMENT)pSsd );
	}
	return true;
}


bool StorageManager::CreateAndAddArrayObjects( void *pRows, U32 rowCount  ){

	U32						i;
	RAID_ARRAY_DESCRIPTOR	*pDescriptor;
	StorageRollCallRecord	*pRow;
	StorageElementArray		*pArray;
	StorageElement			*pStorageElement;
	U32 					j;
	CoolVector				container;
	

	if( !pRows ){ // get the temp table data
		pRows = GetTempTableData( SSAPI_SM_SRC_TABLE );
		rowCount = GetShadowTable( SSAPI_SM_SRC_TABLE )->GetNumberOfRows();
	}

	for( i = 0, pRow = (StorageRollCallRecord *)pRows; i < rowCount; i++, pRow++ ){
		if( pRow->storageclass != SRCTypeArray )
			continue;
	
		if( !m_descriptorCollector.Get( (RowId &)pRow->ridDescriptorRecord, (void *&)pDescriptor ) ){
			ASSERT(0);
			continue;
		}

		if( pDescriptor->raidLevel == RAID0 )
			pArray = new StorageElementArray0( GetListenManager(), this );
		else if( pDescriptor->raidLevel == RAID1 ){
			if( pDescriptor->createPolicy.StartHotCopyWithManualBreak )
				pArray = new StorageElementArray1HotCopyManual( GetListenManager(), this );
			else if( pDescriptor->createPolicy.StartHotCopyWithAutoBreak )
				pArray = new StorageElementArray1HotCopyAuto( GetListenManager(), this );
			else
				pArray = new StorageElementArray1( GetListenManager(), this );
		}
		else
			ASSERT(0);

		pArray->BuildYourselfFromSRCRow( pRow );
		pArray->BuildYourselfFromPtsRow( pDescriptor, this );
		container.Add( (CONTAINER_ELEMENT)pArray );

		// now, set flags and states on members and spares
		for( j = 0; j < GetManagedObjectCount(); j++ ){
			RAID_ARRAY_MEMBER	*pMember;
			RowId				rid;

			pStorageElement = (StorageElement *)GetManagedObject( j );
			
			if( pArray->IsYourDedicatedSpare( *pStorageElement, this ) ){
				pStorageElement->SetAsDedicatedSpare();
				pStorageElement->AddParentId( pArray );
				pArray->AddChildId( pStorageElement, false );
				continue;
			}
			if( pArray->IsYourElement( *pStorageElement, this ) ){ // member, find its descriptor
				GetMemberIdBySRCId( pStorageElement->GetDesignatorId().GetRowId(), rid );
				m_memberDescriptorCollector.Get( rid, (void *&)pMember );
				pStorageElement->SetMemberStatus( pMember->memberHealth, false );
				pStorageElement->AddParentId( pArray );
				pArray->AddChildId( pStorageElement, false );
			}
		}
		ReadElementName( pArray );
		BuildPhsDataIdVector( pArray );
		AddObjectsIntoManagedObjectsVector( container );
		container.RemoveAll();
	}

	return true;
}
 

bool 
StorageManager::CreateAndAddPartitionObjects( Container &container, void *pRows, U32 rowCount ){

	U32						i;
	PARTITION_DESCRIPTOR	*pDescriptor;
	StorageRollCallRecord	*pRow;
	StorageElementPartition	*pPartition;

	if( !pRows ){ // get the temp table data
		pRows = GetTempTableData( SSAPI_SM_SRC_TABLE );
		rowCount = GetShadowTable( SSAPI_SM_SRC_TABLE )->GetNumberOfRows();
	}

	for( i = 0, pRow = (StorageRollCallRecord *)pRows; i < rowCount; i++, pRow++ ){
		if( pRow->storageclass != SRCTypePartition )
			continue;

		if( !m_descriptorCollector.Get( (RowId &)pRow->ridDescriptorRecord, (void *&)pDescriptor ) ){
			ASSERT(0);
			continue;
		}

		pPartition = new StorageElementPartition( GetListenManager(), this );
		pPartition->BuildYourselfFromSRCRow( pRow );
		pPartition->BuildYourselfFromPtsRow( pDescriptor );
		ReadElementName( pPartition );
		//BuildPhsDataIdVector( pPartition );
		container.Add( (CONTAINER_ELEMENT) pPartition );
	}

	if( container.Count() )
		AddObjectsIntoManagedObjectsVector( container );

	return true;
}


bool 
StorageManager::CreateSparePoolObjects( Container &container, void *pRows, U32 rowCount  ){

	StorageCollectionSparePool	*pPool = new StorageCollectionSparePool( GetListenManager(), DesignatorId(), this );
	StorageElement				*pElement;
	U32							i;
	DesignatorId				id;
	RAID_SPARE_DESCRIPTOR		*pDescriptor;

	for( i = 0; m_spareDescriptorCollector.GetAt( i, (void *&)pDescriptor); i++ ){
		if( pDescriptor->spareType != RAID_DEDICATED_SPARE ){
			GetDesignatorIdByRowId( pDescriptor->SRCTRID, id );
			pElement = (StorageElement *)GetManagedObject( &id );
			ASSERT( pElement );
			pElement->SetAsPoolSpare();
			pElement->AddParentId( pPool );
			pPool->AddChildId( pElement );
		}
	}

	container.Add( (CONTAINER_ELEMENT) pPool );
	return true;
}

//************************************************************************
// GetShadowTable:
//
// PURPOSE:		Performs a look up of the table by the table mask
//************************************************************************

ShadowTable* 
StorageManager::GetShadowTable( U32 tableMask ){

	for( U32 i = 0; i < SSAPI_SM_NUMBER_OF_TABLES_USED; i++ )
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
StorageManager::GetTempTableData( U32 tableMask ){

	for( U32 i = 0; i < SSAPI_SM_NUMBER_OF_TABLES_USED; i++ )
		if( m_pTable[i].tableMask == tableMask )
			return m_pTable[i].pTempTable;


	ASSERT(0);
	return NULL;
}

//************************************************************************
// ClearAllTempTables:
//
// PURPOSE:		Deallocates memory taken by temp tables 
//************************************************************************

void 
StorageManager::ClearAllTempTables(){

	for( U32 i = 0; i < SSAPI_SM_NUMBER_OF_TABLES_USED; i++ ){
		delete m_pTable[i].pTempTable;
		m_pTable[i].pTempTable = NULL;
	}
}


//************************************************************************
// ObjectDeletedCallbackHandler:
//
// PURPOSE:		This method may be provided by subclasses to be notified
//				by events from other object managers. 
//************************************************************************

void 
StorageManager::ObjectDeletedCallbackHandler( SSAPIEvent *pEvent , bool isLast ){

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
// ObjectAddedEventCallback:
//
// PURPOSE:		This is a callback method for all OBJECT_ADDED events 
//				coming from the listen manager.
//************************************************************************

void 
StorageManager::ObjectAddedEventCallback( ValueSet *pVs, bool isLast, int eventObjectId ){

	SSAPIEvent				*pEvent = new SSAPIEventObjectAdded( NULL );
	ValueSet				*pChild = new ValueSet;
	ClassTypeMap			map;
	bool					isPhsObject;
	int						classType;
	ManagedObject			*pObj;
	DesignatorId			id;

	*pEvent = *(ValueSet *)pVs->GetValue( eventObjectId );
	*pChild = *(ValueSet *)pEvent->GetValue( SSAPI_EVENT_FID_MANAGED_OBJECT );
	pChild->GetInt( SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &classType );

	isPhsObject = map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_PHS_DATA, classType, true );
	if( isPhsObject ){
		pChild->GetGenericValue( (char *)&id, sizeof(id), SSAPI_OBJECT_FID_ID );
		pObj = GetObjectWithPhsId( id );
		if( pObj )
			pObj->DeletePhsDataItem( id );
	}
	
	delete pChild;
	delete pEvent;
}


//************************************************************************
// RaidEventHandler:
//
// PURPOSE:		CAlled by the Raid command Q on command completetion and
//				to report events
//************************************************************************

void 
StorageManager::RaidEventHandler( STATUS eventCode, void *pStatusData ){

	if( m_isIniting ){
		m_shouldRebuildAllTables = true;
		return;
	}

	RMSTR_EVT_ARRAY_ADDED_STATUS		*pEvtArrayAdded;
	RMSTR_EVT_ARRAY_DELETED_STATUS		*pEvtArrayDeleted;
	RMSTR_EVT_SPARE_ADDED_STATUS		*pEvtSpareAdded;
	RMSTR_EVT_SPARE_DELETED_STATUS		*pEvtSpareDeleted;
	RMSTR_EVT_MEMBER_ADDED_STATUS		*pEvtMemberAdded;
	RMSTR_EVT_MEMBER_REMOVED_STATUS		*pEvtMemberRemoved;
	RMSTR_EVT_MEMBER_DOWN_STATUS		*pEvtMemberDown;
	RMSTR_EVT_ARRAY_CRITICAL_STATUS		*pEvtArrayCritical;
	RMSTR_EVT_SOURCE_MEMBER_CHANGED_STATUS	*pEvtMemberChanged;
	RMSTR_EVT_ARRAY_FAULT_TOLERANT_STATUS	*pEvtArrayOK;
	RMSTR_EVT_SPARE_ACTIVATED_STATUS	*pSpareActivated;
	StorageElement						*pElement;
	DesignatorId						id;
	StorageElementArray					*pArray;
	CoolVector							container;
	RAID_ARRAY_DESCRIPTOR				*pArrDescr;
	U32									i;

	switch( eventCode ){

		case RMSTR_EVT_ARRAY_ADDED:
			pEvtArrayAdded = (RMSTR_EVT_ARRAY_ADDED_STATUS *)pStatusData;
			CreateAndAddArrayObjects( &pEvtArrayAdded->SRCData, 1 );
			GetDesignatorIdByRowId( pEvtArrayAdded->SRCData.rid, id );
			pElement = (StorageElement *)GetManagedObject( &id );
			break;

		case RMSTR_EVT_ARRAY_DELETED:
			pEvtArrayDeleted = (RMSTR_EVT_ARRAY_DELETED_STATUS *)pStatusData;
			GetDesignatorIdByRowId( pEvtArrayDeleted->SRCTRowId, id );
			pArray = (StorageElementArray *)GetManagedObject( &id );
			if( !pArray )
				break;
	
		    for( i = 0; i < pArray->GetChildCount(); i++ ){
				id = pArray->GetChildIdAt( i );
				pElement = (StorageElement *)GetManagedObject( &id );
				pElement->UnsetAsMember();
				pElement->UnsetAsSpare();

				// make sure that if it was a hot copy, one ex-member stays
				// used, as it is exported
				if(	pEvtArrayDeleted->arrayData.createPolicy.StartHotCopyWithAutoBreak
					||
					pEvtArrayDeleted->arrayData.createPolicy.StartHotCopyWithManualBreak ){

					if( i == pEvtArrayDeleted->arrayData.hotCopyExportMemberIndex )
				      pElement->SetIsUsed( true, true );
				}
			}

			container.Add( (CONTAINER_ELEMENT)pArray);
			DeleteObjectsFromTheSystem( container );
			break;

		case RMSTR_EVT_SPARE_ACTIVATED:
			pSpareActivated = (RMSTR_EVT_SPARE_ACTIVATED_STATUS *)pStatusData;
			GetDesignatorIdByRowId( pSpareActivated->spareData.SRCTRID, id );
			pElement = (StorageElement *)GetManagedObject( &id );
			if( GetSparePool()->IsYourElement( *pElement ) ){
				GetSparePool()->DeleteChildId( pElement, true );
				GetSparePool()->ReconcileCapacity();
				pElement->DeleteParentId( GetSparePool()->GetDesignatorId(), false );
			}
			pElement->UnsetAsSpare();
			pElement->SetMemberStatus( RAID_STATUS_DOWN, false );
			pArray = GetArrayByArrayRowId( pSpareActivated->spareData.arrayRID );
			pElement->AddParentId( pArray );
			pArray->AddChildId( pElement );
			break;

		case RMSTR_EVT_SPARE_DELETED:
			pEvtSpareDeleted = (RMSTR_EVT_SPARE_DELETED_STATUS *)pStatusData;
			GetDesignatorIdByRowId( pEvtSpareDeleted->spareData.SRCTRID, id );
			pElement = (StorageElement *)GetManagedObject( &id );
			
			if( pElement && (pEvtSpareDeleted->spareData.spareType == RAID_DEDICATED_SPARE) ){
				pArray = GetArrayByArrayRowId( pEvtSpareDeleted->spareData.arrayRID );
				pArray->DeleteChildId( pElement );
				// parents are unset in the SRC row modified handler
			}
			else{	// Pool spare
				GetSparePool()->DeleteChildId( pElement );
				GetSparePool()->ReconcileCapacity();
			}

			break;

		case RMSTR_EVT_SPARE_ADDED:
			pEvtSpareAdded = (RMSTR_EVT_SPARE_ADDED_STATUS *)pStatusData;
			GetDesignatorIdByRowId( pEvtSpareAdded->spareData.SRCTRID, id );
			pElement = (StorageElement *)GetManagedObject( &id );
			
			if( pEvtSpareAdded->spareData.spareType == RAID_DEDICATED_SPARE ){
				pArray = GetArrayByArrayRowId( pEvtSpareAdded->spareData.arrayRID );
				pElement->SetAsDedicatedSpare();
				pElement->AddParentId( pArray );
				pArray->AddChildId( pElement );
			}
			else{	// Pool spare
				pElement->SetAsPoolSpare();
				GetSparePool()->AddChildId( pElement );
				pElement->AddParentId( GetSparePool() );
				GetSparePool()->ReconcileCapacity();
			}
			break;

		case RMSTR_EVT_MEMBER_ADDED:
			pEvtMemberAdded = (RMSTR_EVT_MEMBER_ADDED_STATUS *)pStatusData;
			GetDesignatorIdByRowId( pEvtMemberAdded->memberData.memberRID, id );
			pElement = (StorageElement *)GetManagedObject( &id );
			m_descriptorCollector.Get( (RowId &)pEvtMemberAdded->memberData.arrayRID , (void *&)pArrDescr );
			GetDesignatorIdByRowId( pArrDescr->SRCTRID, id );
			pArray = (StorageElementArray *)GetManagedObject( &id );

			pElement->SetMemberStatus(pEvtMemberAdded->memberData.memberHealth);
			pElement->AddParentId( pArray );
			pArray->AddChildId( pElement );
			break;

		case RMSTR_EVT_MEMBER_REMOVED:
			pEvtMemberRemoved = (RMSTR_EVT_MEMBER_REMOVED_STATUS *)pStatusData;
			GetDesignatorIdByRowId( pEvtMemberRemoved->memberData.memberRID, id );
			pElement = (StorageElement *)GetManagedObject( &id );
			m_descriptorCollector.Get( (RowId &)pEvtMemberRemoved->memberData.arrayRID , (void *&)pArrDescr );
			GetDesignatorIdByRowId( pArrDescr->SRCTRID, id );
			pArray = (StorageElementArray *)GetManagedObject( &id );
			if( pElement ){
				pElement->UnsetAsMember();
				pArray->DeleteChildId( pElement );
				pElement->DeleteParentId( pArray->GetDesignatorId() );	
			}
			pArray->SetYourState();
			break;

		case RMSTR_EVT_MEMBER_DOWN:
			pEvtMemberDown = (RMSTR_EVT_MEMBER_DOWN_STATUS *)pStatusData;
			GetDesignatorIdByRowId( pEvtMemberDown->memberData.memberRID, id );
			pElement = (StorageElement *)GetManagedObject( &id );
			m_descriptorCollector.Get( (RowId &)pEvtMemberDown->memberData.arrayRID , (void *&)pArrDescr );
			GetDesignatorIdByRowId( pArrDescr->SRCTRID, id );
			pArray = (StorageElementArray *)GetManagedObject( &id );
			if( pElement ){
				pElement->SetMemberStatus(pEvtMemberDown->memberData.memberHealth );
			}
			pArray->SetYourState();
			break;

		case RMSTR_EVT_ARRAY_CRITICAL:
		case RMSTR_EVT_ARRAY_OFFLINE:
			pEvtArrayCritical = (RMSTR_EVT_ARRAY_CRITICAL_STATUS *)pStatusData;
			GetDesignatorIdByRowId( pEvtArrayCritical->arrayData.SRCTRID, id );
			pArray = (StorageElementArray *)GetManagedObject( &id );
			pArray->SetYourState();
			break;

		case RMSTR_EVT_SOURCE_MEMBER_CHANGED:
		case RMSTR_EVT_PREFERRED_MEMBER_CHANGED:
			pEvtMemberChanged = (RMSTR_EVT_SOURCE_MEMBER_CHANGED_STATUS *)pStatusData;
			GetDesignatorIdByRowId( pEvtMemberChanged->arrayData.SRCTRID, id );
			pArray = (StorageElementArray *)GetManagedObject( &id );
			// the object should have been updated when the descriptor was changed.
			// thus, now simply post an event
			pArray->FireEventObjectModifed();
			break;

		case RMSTR_EVT_ARRAY_FAULT_TOLERANT:
			pEvtArrayOK = (RMSTR_EVT_ARRAY_FAULT_TOLERANT_STATUS *)pStatusData;
			GetDesignatorIdByRowId( pEvtArrayOK->arrayData.SRCTRID, id );
			pArray = (StorageElementArray *)GetManagedObject( &id );
			// go thru children and set their states
			for( i = 0; i < pArray->GetChildCount(); i++ ){
				id = pArray->GetChildIdAt( i );
				pElement = (StorageElement *)GetManagedObject( &id );
				pElement->SetMemberStatus( pEvtArrayOK->memberHealth[i] ); 
			}
			pArray->SetYourState();
			pArray->FireEventObjectModifed();
			break;

		default:
			//ASSERT(0);
			break;
	}
}



void 
StorageManager::PartitionEventHandler( STATUS comletionCode, void *pStatusData ){

	if( m_isIniting ){
		m_shouldRebuildAllTables = true;
		return;
	}

	PMSTR_EVENT_INFO					*pEventInfo = (PMSTR_EVENT_INFO *)pStatusData;
	PMSTR_EVT_PARTITION_CREATED_STATUS	*pEventCreated = (PMSTR_EVT_PARTITION_CREATED_STATUS *)&pEventInfo->partitionCreatedStatus;
	PMSTR_EVT_PARTITION_DELETED_STATUS	*pEventDeleted = (PMSTR_EVT_PARTITION_DELETED_STATUS *)&pEventInfo->partitionDeletedStatus;
	PMSTR_EVT_PARTITION_MODIFIED_STATUS	*pEventModified = (PMSTR_EVT_PARTITION_MODIFIED_STATUS *)&pEventInfo->partitionModifiedStatus;
	StorageElementPartition				*pPartition;
	CoolVector							container;
	DesignatorId						id;
	StorageElementBase					*pElement;

	switch( comletionCode ){

		case PMSTR_EVT_PARTITION_CREATED:
			pPartition = new StorageElementPartition( GetListenManager(), this );
			pPartition->BuildYourselfFromSRCRow( &pEventCreated->SRCData );
			pPartition->BuildYourselfFromPtsRow( &pEventCreated->partitionData );
			pPartition->SetName( (void *)&pEventCreated->partitionName, false );
			pPartition->SetRidName( pEventCreated->partitionNameRowId );
			id = pPartition->GetOwnerId();
			pElement = (StorageElementBase *)GetManagedObject( &id );
			if( pElement )
				pElement->SetIsUsed( true, true );
			container.Add( (CONTAINER_ELEMENT) pPartition );
			AddObjectsIntoManagedObjectsVector( container );
			ShuffleReferencesInArraysAndPartitions();
			break;

		case PMSTR_EVT_PARTITION_DELETED:
			GetDesignatorIdByRowId( pEventDeleted->partitionData.SRCTRID, id );
			pPartition = (StorageElementPartition *)GetManagedObject( &id );
			ASSERT( pPartition );
			if( pPartition ){
				id = pPartition->GetOwnerId();
				pElement = (StorageElementBase *)GetManagedObject( &id );
				if( pElement )
					pElement->SetIsUsed( false, true );
				container.Add( (CONTAINER_ELEMENT)pPartition );
				DeleteObjectsFromTheSystem( container );
			}
			break;

		case PMSTR_EVT_PARTITION_MODIFIED:
			GetDesignatorIdByRowId( pEventModified->partitionData.SRCTRID, id );
			pPartition = (StorageElementPartition *)GetManagedObject( &id );
			ASSERT( pPartition );
			if( pPartition ){
				pPartition->BuildYourselfFromPtsRow( &pEventModified->partitionData );
				pPartition->BuildYourselfFromSRCRow( &pEventModified->SRCData );
				GetListenManager()->PropagateObjectModifiedEventForManagers( GetDesignatorId(), pPartition );
			}
			break;

		default:
			ASSERT(0);
			return;
	}
}

//************************************************************************
// CreateArray:
//
// PURPOSE:			Send Create_Array opcode to the RAID Master
//************************************************************************

void 
StorageManager::CreateArray( ValueSet &requestParms, SsapiResponder *pResponder, RowId ridName ){

	RMSTR_CMND_INFO					command;
	RMSTR_CREATE_ARRAY_DEFINITION	*pCommandInfo = &command.cmdParams.createArrayDefinition;
	STATUS							rc;
	int								classType = 0xFFFFFFFF;
	StorageElementArray				*pArray;
	ValueSet						*pArrayValues;
	U32								index;

	pArrayValues = (ValueSet *)requestParms.GetValue(SSAPI_STORAGE_MANAGER_ADD_ARRAY_ARRAY_OBJECT);
	pArrayValues->GetInt( SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &classType );

	if( !requestParms.GetValue(SSAPI_STORAGE_MANAGER_ADD_ARRAY_MEMBER_ID_VECTOR) ){
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER,CTS_SSAPI_INVALID_PARAM_EXCEPTION);
		return;
	}


	switch( classType ){
		case SSAPI_OBJECT_CLASS_TYPE_ARRAY_0_STORAGE_ELEMENT:
			pArray = new StorageElementArray0( GetListenManager(), this );
			break;

		case SSAPI_OBJECT_CLASS_TYPE_ARRAY_1_STORAGE_ELEMENT:
			pArray = new StorageElementArray1( GetListenManager(), this );
			break;

		default:
			ASSERT(0);
			pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER,CTS_SSAPI_INVALID_PARAM_EXCEPTION);
			return;
	}

	command.opcode		= RMSTR_CMND_CREATE_ARRAY;

	pArray->WriteYourselfIntoCreateArrayDefinition( requestParms, pCommandInfo );
	pCommandInfo->arrayNameRowId = ridName.GetRowID();

	for( index = 0; index < pCommandInfo->numberMembers + pCommandInfo->numberSpares; index++ )
		m_pendingSrcUpdates.Add( &pCommandInfo->arrayMembers[index], sizeof(pCommandInfo->arrayMembers[index]) );

	rc = m_pRaidCommandQ->csndrExecute(	&command,
										(pCmdCompletionCallback_t)METHOD_ADDRESS(StorageManager,RaidCommandCompletionCallback),
										pResponder );

	if( rc != OK )
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);
	else
		AddOutstandingRequest();

	delete pArray;
}


//************************************************************************
// RaidCommandCompletionCallback:
//
// PURPOSE:		Called back to for all commands submitted to the RAID Q.
//************************************************************************

void 
StorageManager::RaidCommandCompletionCallback(	STATUS			completionCode,
												void			*pResultData,
												void			*pCmdData,
												void			*pCmdContext ){

	RMSTR_CMND_INFO					*pCommand = (RMSTR_CMND_INFO *)pCmdData;
	RMSTR_CREATE_ARRAY_DEFINITION	*pCommandInfo = &pCommand->cmdParams.createArrayDefinition;
	SsapiResponder					*pResponder;
	Raid2SsapiErrorConvertor		errorConvertor;
	U32								exceptionString = errorConvertor.GetSsapiException( completionCode );

	switch( pCommand->opcode ){

		case RMSTR_CMND_CREATE_ARRAY:
		case RMSTR_CMND_DELETE_ARRAY:
		case RMSTR_CMND_CREATE_SPARE:
		case RMSTR_CMND_DELETE_SPARE:
		case RMSTR_CMND_ADD_MEMBER:
		case RMSTR_CMND_REMOVE_MEMBER:
		case RMSTR_CMND_DOWN_A_MEMBER:
		case RMSTR_CMND_CHANGE_SOURCE_MEMBER:
		case RMSTR_CMND_CHANGE_PREFERRED_MEMBER:
			pResponder = (SsapiResponder *)pCmdContext;

			if( m_outstandingRequests ){ // check if this was submitted before the failover
				if( completionCode != RMSTR_SUCCESS )
					pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, exceptionString );
				else
					pResponder->RespondToRequest( SSAPI_RC_SUCCESS );

				RemoveOutstandingRequest();
			}
			break;

		default:
			ASSERT(0);
			break;
	}

}


//************************************************************************
// DeleteArray:
//
// PURPOSE:			Deletes an array from the system
//************************************************************************

void 
StorageManager::DeleteArray( StorageElementArray *pArray, SsapiResponder *pResponder ){

	RMSTR_CMND_INFO			command;
	RMSTR_DELETE_ARRAY_INFO	*pCmdInfo = &command.cmdParams.deleteArrayInfo;
	STATUS					rc;

	memset( &command, 0, sizeof(command) );
	command.opcode = RMSTR_CMND_DELETE_ARRAY;
	pCmdInfo->arrayRowId = pArray->GetArrayRid();

	rc = m_pRaidCommandQ->csndrExecute(	&command,
										(pCmdCompletionCallback_t)METHOD_ADDRESS(StorageManager,RaidCommandCompletionCallback),
										pResponder );

	if( rc != OK )
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);
	else
		AddOutstandingRequest();
}


//************************************************************************
// Spare operations:
//************************************************************************

void 
StorageManager::AddDedicatedSpare( DesignatorId arrayId, DesignatorId spareId, SsapiResponder *pResponder ){

	RMSTR_CMND_INFO			command;
	RMSTR_CREATE_SPARE_INFO	*pCmdInfo = &command.cmdParams.createSpareInfo;
	STATUS					rc;
	StorageElementArray		*pArray = (StorageElementArray *)GetManagedObject( &arrayId );
	

	if( !pArray ){
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALIDPARM_ARRAY_ID_INVALID);
		return;
	}

	memset( &command, 0, sizeof(command) );
	command.opcode = RMSTR_CMND_CREATE_SPARE;
	pCmdInfo->spareType =  RAID_DEDICATED_SPARE;
	pCmdInfo->spareId	=  spareId.GetRowId().GetRowID();
	pCmdInfo->arrayRowId=  pArray->GetArrayRid().GetRowID();		

	rc = m_pRaidCommandQ->csndrExecute(	&command,
										(pCmdCompletionCallback_t)METHOD_ADDRESS(StorageManager,RaidCommandCompletionCallback),
										pResponder );

	if( rc != OK )
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);
	else{
		AddOutstandingRequest();
		m_pendingSrcUpdates.Add( &pCmdInfo->spareId, sizeof(pCmdInfo->spareId) );
	}
}


void 
StorageManager::DeleteDedicatedSpare( DesignatorId arrayId, DesignatorId spareId, SsapiResponder *pResponder ){

	RMSTR_CMND_INFO			command;
	RMSTR_DELETE_SPARE_INFO	*pCmdInfo = &command.cmdParams.deleteSpareInfo;
	STATUS					rc;
	RowId					rid;

	memset( &command, 0, sizeof(command) );
	command.opcode = RMSTR_CMND_DELETE_SPARE;
	GetSpareIdBySRCId( spareId.GetRowId(), rid );
	pCmdInfo->spareId = rid.GetRowID();

	rc = m_pRaidCommandQ->csndrExecute(	&command,
										(pCmdCompletionCallback_t)METHOD_ADDRESS(StorageManager,RaidCommandCompletionCallback),
										pResponder );

	if( rc != OK )
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);
	else
		AddOutstandingRequest();
}


void 
StorageManager::AddPoolSpare( DesignatorId arrayId, DesignatorId spareId, SsapiResponder *pResponder ){

	RMSTR_CMND_INFO			command;
	RMSTR_CREATE_SPARE_INFO	*pCmdInfo = &command.cmdParams.createSpareInfo;
	STATUS					rc;

	memset( &command, 0, sizeof(command) );
	command.opcode = RMSTR_CMND_CREATE_SPARE;
	pCmdInfo->spareType =  RAID_GENERAL_POOL_SPARE;
	pCmdInfo->spareId	=  spareId.GetRowId().GetRowID();

	rc = m_pRaidCommandQ->csndrExecute(	&command,
										(pCmdCompletionCallback_t)METHOD_ADDRESS(StorageManager,RaidCommandCompletionCallback),
										pResponder );

	if( rc != OK )
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);
	else{
		m_pendingSrcUpdates.Add( &pCmdInfo->spareId, sizeof(pCmdInfo->spareId) );
		AddOutstandingRequest();
	}
}


void 
StorageManager::DeletePoolSpare( DesignatorId arrayId, DesignatorId spareId, SsapiResponder *pResponder ){

	DeleteDedicatedSpare( arrayId, spareId, pResponder );
}


//************************************************************************
// GetArrayByArrayRowId:
//
// PURPOSE:		Looks for the array with the array rid specified
//************************************************************************

StorageElementArray* 
StorageManager::GetArrayByArrayRowId( RowId rid ){
	
	U32				i;
	ManagedObject	*pObj;
	ClassTypeMap	map;

	for( i = 0; i < GetManagedObjectCount(); i++ ){
		pObj = GetManagedObject( i );
		if( map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_ARRAY_STORAGE_ELEMENT, pObj->GetClassType(), true ) )
			if( ((StorageElementArray *)pObj)->GetArrayRid() == rid ) 
				return (StorageElementArray *)pObj;
	}
	
//	ASSERT(0);
	return NULL;
}


//************************************************************************
// GetSparePool:
//
// PURPOSE:		Looks up the spare pool object
//************************************************************************

StorageCollectionSparePool* 
StorageManager::GetSparePool(){

	ManagedObject		*pObj;
	U32					i;

	for( i = 0; i < GetManagedObjectCount(); i++ ){
		pObj = GetManagedObject( i );
		if( pObj->GetClassType() == SSAPI_OBJECT_CLASS_TYPE_STORAGE_COLL_SPARE_POOL)
			return (StorageCollectionSparePool *)pObj;
	}

	ASSERT(0);
	return NULL;
}


//************************************************************************
// Member operations
//************************************************************************

void 
StorageManager::AddArrayMember( DesignatorId arrayId, DesignatorId memberId, SsapiResponder *pResponder ){

	StorageElement			*pMember = (StorageElement *)GetManagedObject( &memberId );
	StorageElementArray		*pArray	 = (StorageElementArray *)GetManagedObject( &arrayId );
	RMSTR_CMND_INFO			command;
	RMSTR_ADD_MEMBER_INFO	*pCmdInfo = &command.cmdParams.addMemberInfo;
	STATUS					rc;

	if( !pArray || !pMember ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
		return;
	}

	memset( &command, 0, sizeof(command) );
	command.opcode = RMSTR_CMND_ADD_MEMBER;
	pCmdInfo->arrayRowId = pArray->GetArrayRid().GetRowID();
	pCmdInfo->newMemberRowId = pMember->GetDesignatorId().GetRowId().GetRowID();

	rc = m_pRaidCommandQ->csndrExecute(	&command,
										(pCmdCompletionCallback_t)METHOD_ADDRESS(StorageManager,RaidCommandCompletionCallback),
										pResponder );

	if( rc != OK )
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);
	else
		AddOutstandingRequest();
}


void 
StorageManager::DeleteArrayMember( DesignatorId arrayId, DesignatorId memberId, SsapiResponder *pResponder ){

	StorageElement				*pMember = (StorageElement *)GetManagedObject( &memberId );
	StorageElementArray			*pArray	 = (StorageElementArray *)GetManagedObject( &arrayId );
	RMSTR_CMND_INFO				command;
	RMSTR_REMOVE_MEMBER_INFO	*pCmdInfo = &command.cmdParams.removeMemberInfo;
	STATUS						rc;
	RowId						rid;

	if( !pArray || !pMember ){
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
		return;
	}

	memset( &command, 0, sizeof(command) );
	command.opcode = RMSTR_CMND_REMOVE_MEMBER;
	pCmdInfo->arrayRowId = pArray->GetArrayRid().GetRowID();
	GetMemberIdBySRCId( memberId.GetRowId(), rid );
	pCmdInfo->memberRowId = rid.GetRowID();

	rc = m_pRaidCommandQ->csndrExecute(	&command,
										(pCmdCompletionCallback_t)METHOD_ADDRESS(StorageManager,RaidCommandCompletionCallback),
										pResponder );

	if( rc != OK )
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);
	else
		AddOutstandingRequest();
}


void 
StorageManager::DownArrayMember( DesignatorId arrayId, DesignatorId memberId, SsapiResponder *pResponder ){

	StorageElement				*pMember = (StorageElement *)GetManagedObject( &memberId );
	StorageElementArray			*pArray	 = (StorageElementArray *)GetManagedObject( &arrayId );
	RMSTR_CMND_INFO				command;
	RMSTR_DOWN_A_MEMBER_INFO	*pCmdInfo = &command.cmdParams.downAMemberInfo;
	STATUS						rc;
	RowId						rid;

	if( !pArray || !pMember ){
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
		return;
	}

	memset( &command, 0, sizeof(command) );
	command.opcode = RMSTR_CMND_DOWN_A_MEMBER;
	pCmdInfo->arrayRowId = pArray->GetArrayRid().GetRowID();
	GetMemberIdBySRCId( memberId.GetRowId(), rid );
	pCmdInfo->memberRowId = rid.GetRowID();

	rc = m_pRaidCommandQ->csndrExecute(	&command,
										(pCmdCompletionCallback_t)METHOD_ADDRESS(StorageManager,RaidCommandCompletionCallback),
										pResponder );

	if( rc != OK )
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);
	else
		AddOutstandingRequest();
}


void 
StorageManager::SetPreferredMember( DesignatorId arrayId, DesignatorId memberId, SsapiResponder *pResponder ){

	StorageElement						*pMember = (StorageElement *)GetManagedObject( &memberId );
	StorageElementArray					*pArray	 = (StorageElementArray *)GetManagedObject( &arrayId );
	RMSTR_CMND_INFO						command;
	RMSTR_CHANGE_PREFERRED_MEMBER_INFO	*pCmdInfo = &command.cmdParams.changePreferredMemberInfo;
	STATUS								rc;
	RowId								rid;

	if( !pArray || !pMember ){
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
		return;
	}

	memset( &command, 0, sizeof(command) );
	command.opcode = RMSTR_CMND_CHANGE_PREFERRED_MEMBER;
	pCmdInfo->arrayRowId = pArray->GetArrayRid().GetRowID();
	GetMemberIdBySRCId( memberId.GetRowId(), rid );
	pCmdInfo->newMemberRowId = rid.GetRowID();

	rc = m_pRaidCommandQ->csndrExecute(	&command,
										(pCmdCompletionCallback_t)METHOD_ADDRESS(StorageManager,RaidCommandCompletionCallback),
										pResponder );

	if( rc != OK )
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);
	else
		AddOutstandingRequest();
}


void 
StorageManager::SetSourceMember( DesignatorId arrayId, DesignatorId memberId, SsapiResponder *pResponder ){

	StorageElement					*pMember = (StorageElement *)GetManagedObject( &memberId );
	StorageElementArray				*pArray	 = (StorageElementArray *)GetManagedObject( &arrayId );
	RMSTR_CMND_INFO					command;
	RMSTR_CHANGE_SOURCE_MEMBER_INFO	*pCmdInfo = &command.cmdParams.changeSourceMemberInfo;
	STATUS							rc;
	RowId							rid;

	if( !pArray || !pMember ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
		return;
	}

	memset( &command, 0, sizeof(command) );
	command.opcode = RMSTR_CMND_CHANGE_SOURCE_MEMBER;
	pCmdInfo->arrayRowId = pArray->GetArrayRid().GetRowID();
	GetMemberIdBySRCId( memberId.GetRowId(), rid );
	pCmdInfo->newMemberRowId = rid.GetRowID();

	rc = m_pRaidCommandQ->csndrExecute(	&command,
										(pCmdCompletionCallback_t)METHOD_ADDRESS(StorageManager,RaidCommandCompletionCallback),
										pResponder );

	if( rc != OK )
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);
	else
		AddOutstandingRequest();
}


//************************************************************************
// Spare id look up
//************************************************************************

bool 
StorageManager::GetSRCIdBySpareId( RowId spareId, RowId &srcId ){

	RAID_SPARE_DESCRIPTOR		*p;
	U32							i;

	for( i = 0; m_spareDescriptorCollector.GetAt( i, (void *&)p ); i++ )
		if( spareId == p->thisRID ){
			srcId = p->SRCTRID;
			return true;
		}

	ASSERT(0);
	return false;
}


bool 
StorageManager::GetSpareIdBySRCId( RowId srcId, RowId &spareId ){

	RAID_SPARE_DESCRIPTOR		*p;
	U32							i;

	for( i = 0; m_spareDescriptorCollector.GetAt( i, (void* &)p ); i++ )
		if( srcId == p->SRCTRID ){
			spareId = p->thisRID;
			return true;
		}


	return false;
}


//************************************************************************
// Member id look up
//************************************************************************

bool 
StorageManager::GetSRCIdByMemberId( RowId memberId, RowId &srcId ){

	RAID_ARRAY_MEMBER		*p;
	U32						i;

	for( i = 0; m_memberDescriptorCollector.GetAt( i, (void *&)p ); i++ )
		if( memberId == p->thisRID ){
			srcId = p->memberRID;
			return true;
		}

	ASSERT(0);
	return false;
}


bool 
StorageManager::GetMemberIdBySRCId( RowId srcId, RowId &memberId ){

	RAID_ARRAY_MEMBER		*p;
	U32						i;

	for( i = 0; m_memberDescriptorCollector.GetAt( i, (void* &)p ); i++ )
		if( srcId == p->memberRID ){
			memberId = p->thisRID;
			return true;
		}


	return false;
}

//************************************************************************
// ChangeStorageElementName:
//
// PURPOSE:		Changes element's name
//************************************************************************

void 
StorageManager::ChangeStorageElementName( DesignatorId elementId, UnicodeString name, SsapiResponder *pResponder ){

	bool				boolRc;
	StorageElement		*pElement = (StorageElement *)GetManagedObject( &elementId );
	RowId				rid;
	MODIFY_NAME_CELL	*pCell;
	ShadowTable			*pTable;
		
	if( !pElement || !name.GetLength() ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION);
		return;
	}

#ifdef CHECK_FOR_DUPLICATE_STORAGE_NAME
	StorageElement		*pObj;
	U32					i;

	// check for duplicate name
	for( i = 0; i < GetManagedObjectCount(); i++ ){
		pObj = (StorageElement *)GetManagedObject( i );
		if( pObj->GetName() == name ){
			pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_EXCEPTION_NAME_ALREADY_EXISTS );
			return;
		}
	}
#endif

	// first off, delete the old name if needed
	if( RowId( pElement->GetRidName() ).IsClear() == false ){
		m_pStringResourceManager->DeleteString(	pElement->GetRidName(),
												(pTSCallback_t)METHOD_ADDRESS(StorageManager,DummyCallback),
												NULL );

		pTable = GetShadowTable( pElement->GetCoreTableMask() );
		pTable->ModifyField(pElement->GetDesignatorId().GetRowId(),
							fdSRC_NAME_RID,
							&rid,
							sizeof(rid),
							(pTSCallback_t)METHOD_ADDRESS(StorageManager,DummyCallback),
							NULL );

		pElement->SetRidName( RowId() );
	}
		
	// now, write the new one
	pCell = new MODIFY_NAME_CELL( this );
	pCell->id = pElement->GetDesignatorId();
	pCell->name = name;
	boolRc = m_pStringResourceManager->WriteString(	name,
													&pCell->rid,
													(pTSCallback_t)METHOD_ADDRESS(StorageManager,ChangeStorageElementNameCallback),
													pCell );

	if( boolRc ){
		AddOutstandingRequest();
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	}
	else
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );

}


STATUS 
StorageManager::ChangeStorageElementNameCallback( void *pContext, STATUS rc ){

	MODIFY_NAME_CELL	*pCell = (MODIFY_NAME_CELL *)pContext;
	StorageElement		*pElement;
	ShadowTable			*pTable;

	if( rc == OK ){
		pElement = (StorageElement *)pCell->pThis->GetManagedObject( &pCell->id );
		ASSERT(pElement);
		if( pElement ){
			pTable = pCell->pThis->GetShadowTable( pElement->GetCoreTableMask() );
			pTable->ModifyField(pElement->GetDesignatorId().GetRowId(),
								fdSRC_NAME_RID,
								&pCell->rid,
								sizeof(pCell->rid),
								(pTSCallback_t)METHOD_ADDRESS(StorageManager,DummyCallback),
								NULL );
		}
	}
	
	pCell->pThis->RemoveOutstandingRequest();
	delete pCell;
	return OK;
}

//************************************************************************
// ReadElementName:
//
// PURPOSE:		Reads storage element's name and will update the element
//				when the name is read
//************************************************************************

void 
StorageManager::ReadElementName( StorageElementBase *pElement ){

	READ_NAME_CELL		*pCell;
	bool				rc;

	if( RowId( pElement->GetRidName() ).IsClear() )
		return;
	
	pCell = new READ_NAME_CELL(  );
	pCell->pName = new UnicodeString;
	pCell->pThis = this;
	pCell->elementId = pElement->GetDesignatorId();

	rc = m_pStringResourceManager->ReadString(	pCell->pName,
												pElement->GetRidName(),
												(pTSCallback_t)METHOD_ADDRESS(StorageManager,ReadElementNameCallback),
												pCell );
	if( rc )
		this->AddOutstandingRequest();
	else{
		delete pCell->pName;
		delete pCell;
	}
}



STATUS 
StorageManager::ReadElementNameCallback( void *pContext, STATUS rc ){

	READ_NAME_CELL		*pCell = (READ_NAME_CELL *)pContext;
	StorageElementBase	*pElement;

	if( rc == OK ){
		pElement = (StorageElementBase *)pCell->pThis->GetManagedObject( &pCell->elementId );
		ASSERT(pElement);
		if( pElement )
			pElement->SetName( *pCell->pName );
	}


	pCell->pThis->RemoveOutstandingRequest();

	delete pCell->pName;
	delete pCell;
	return OK;
}


//************************************************************************
// AddHotCopy:
//
// PURPOSE:		Adds a hot copy array
//************************************************************************

void 
StorageManager::AddHotCopy( ValueSet &requestParms, SsapiResponder *pResponder, RowId ridName  ){

	RMSTR_CMND_INFO					command;
	RMSTR_CREATE_ARRAY_DEFINITION	*pCommandInfo = &command.cmdParams.createArrayDefinition;
	STATUS							rc;
	int								classType = 0xFFFFFFFF;
	StorageElementArray				*pArray;
	ValueSet						*pArrayValues;
	DesignatorId					exportId;
	U32								priority;
	ManagedObject					*pExportMember;

	pArrayValues = (ValueSet *)requestParms.GetValue(SSAPI_STORAGE_MANAGER_ADD_HOT_COPY_ARRAY_OBJECT);
	pArrayValues->GetInt( SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &classType );

	if( !requestParms.GetValue(SSAPI_STORAGE_MANAGER_ADD_HOT_COPY_MEMBER_ID_VECTOR) 
		||
		!requestParms.GetU32(SSAPI_STORAGE_MANAGER_ADD_HOT_COPY_PRIORITY , &priority ) ){

		pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER,CTS_SSAPI_INVALID_PARAM_EXCEPTION);
		return;
	}

	requestParms.GetGenericValue( (char *)&exportId, sizeof(exportId), SSAPI_STORAGE_MANAGER_ADD_HOT_COPY_EXPORT_MEMBER_ID);
	pExportMember = GetManagedObject( &exportId );

	switch( classType ){
		case SSAPI_OBJECT_CLASS_TYPE_ARRAY_HOT_COPY_AUTO:
			pArray = new StorageElementArray1HotCopyAuto( GetListenManager(), this );
			break;

		case SSAPI_OBJECT_CLASS_TYPE_ARRAY_HOT_COPY_MANUAL:
			pArray = new StorageElementArray1HotCopyManual( GetListenManager(), this );
			break;

		default:
			ASSERT(0);
			pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER,CTS_SSAPI_INVALID_PARAM_EXCEPTION);
			return;
	}


	command.opcode		= RMSTR_CMND_CREATE_ARRAY;

	pArray->WriteYourselfIntoCreateArrayDefinition( requestParms, pCommandInfo, priority, pExportMember );
	pCommandInfo->arrayNameRowId = ridName.GetRowID();

	for( U32 index = 0; index < pCommandInfo->numberMembers + pCommandInfo->numberSpares; index++ )
		m_pendingSrcUpdates.Add( &pCommandInfo->arrayMembers[index], sizeof(pCommandInfo->arrayMembers[index]) );


	rc = m_pRaidCommandQ->csndrExecute(	&command,
										(pCmdCompletionCallback_t)METHOD_ADDRESS(StorageManager,RaidCommandCompletionCallback),
										pResponder );

	if( rc != OK )
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);
	else
		AddOutstandingRequest();

	delete pArray;
}


//************************************************************************
// BreakHotCopy:
//
// PURPOSE:		Removes a Hot Copy element
//************************************************************************

void 
StorageManager::RemoveHotCopy( ValueSet &requestParms, SsapiResponder *pResponder ){

	StorageElementArray				*pArray;
	StorageElement					*pExportMember;
	DesignatorId					id;
	RMSTR_CMND_INFO					command;
	RMSTR_DELETE_ARRAY_INFO			*pCmdInfo = &command.cmdParams.deleteArrayInfo;
	STATUS							rc;
	RowId							rid;

	requestParms.GetGenericValue( (char *)&id, sizeof(id), SSAPI_STORAGE_MANAGER_BREAK_HOT_COPY_ARRAY_ID );
	pArray = (StorageElementArray *)GetManagedObject( &id );
	requestParms.GetGenericValue( (char *)&id, sizeof(id), SSAPI_STORAGE_MANAGER_BREAK_HOT_COPY_EXPORT_MEMBER_ID );
	pExportMember = (StorageElement *)GetManagedObject( &id );

	if( !pArray || !pExportMember ){
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION);
		return;
	}

	memset( &command, 0, sizeof(command) );
	command.opcode = RMSTR_CMND_DELETE_ARRAY;
	pCmdInfo->arrayRowId = pArray->GetArrayRid();
	pCmdInfo->policy.BreakHotCopyMirror = 1;
	if( !GetMemberIdBySRCId( pExportMember->GetDesignatorId().GetRowId(), rid ) ){
		ASSERT(0);
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION);
		return;
	}
	pCmdInfo->hotCopyExportMemberRowId = rid;

	rc = m_pRaidCommandQ->csndrExecute(	&command,
										(pCmdCompletionCallback_t)METHOD_ADDRESS(StorageManager,RaidCommandCompletionCallback),
										pResponder );

	if( rc != OK )
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);
	else
		AddOutstandingRequest();
}


//************************************************************************
// InsertNameAndContinue:
//
// PURPOSE:		Inserts a name into the PTS and once the name is in,
//				call the pVector specified with the paramteres given
//************************************************************************

void 
StorageManager::InsertNameAndContinue(	ValueSet						*pRequestParms, 
										SsapiResponder					*pResponder, 
										CREATE_OBJECT_FROM_UI_VECTOR	pVector,
										bool							shouldCheckForDuplicateName ){

	ValueSet			*pArrayValues;
	UnicodeString		name;
	SM_INSERT_NAME_CELL	*pCell;
	bool				needToInsert = true, rc;

	pArrayValues = (ValueSet *)pRequestParms->GetValue(SSAPI_STORAGE_MANAGER_ADD_ARRAY_ARRAY_OBJECT);
	pArrayValues->GetString( SSAPI_STORAGE_ELEMENT_B_FID_NAME, &name );
	
	if( name.GetLength() == 0 )
		needToInsert = false;

#ifdef CHECK_FOR_DUPLICATE_STORAGE_NAME
	StorageElement		*pObj;
	U32					i;

	// check for duplicate name
	for( i = 0; shouldCheckForDuplicateName && (i < GetManagedObjectCount()); i++ ){
		pObj = (StorageElement *)GetManagedObject( i );
		if( pObj->GetName() == name ){
			needToInsert = false;
			break;
		}
	}

	if( !needToInsert ){
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_NAME_IS_BLANK);
		return;
	}
#endif
	
	pCell = new SM_INSERT_NAME_CELL;
	pCell->pParms		= pRequestParms;
	pCell->pResponder	= pResponder;
	pCell->pVector		= pVector;
	pCell->pThis		= this;
	if( needToInsert )
		rc = m_pStringResourceManager->WriteString(	name,
													&pCell->ridName,
													(pTSCallback_t)METHOD_ADDRESS(StorageManager, InsertNameAndContinueCallback),
													pCell );
	else{
		InsertNameAndContinueCallback( pCell, OK );
		rc = true;
	}

	if( rc )
		AddOutstandingRequest();
	else
		pResponder->RespondToRequest(SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);
}


STATUS 
StorageManager::InsertNameAndContinueCallback( void *pContext, STATUS status ){

	SM_INSERT_NAME_CELL	*pCell = (SM_INSERT_NAME_CELL *)pContext;

	((pCell->pThis)->*(pCell->pVector))( *pCell->pParms, pCell->pResponder, pCell->ridName );

	pCell->pThis->RemoveOutstandingRequest();
	delete pCell;
	return OK;
}


//************************************************************************
// RecoverName:
//
// PURPOSE:		Checks if there was such an element and recovers the name
//************************************************************************

void 
StorageManager::RecoverName( StorageElement &element ){

	DesignatorId		id = element.GetDesignatorId();
	StorageElement		*pE = (StorageElement *)GetManagedObject( &id );

	if( pE )
		element.SetName( pE->GetName(), false );
}


//************************************************************************
// CreatePartition
//
// PURPOSE:		Creates a partition by sending a command to the 
//				Partition Master
//************************************************************************

void 
StorageManager::CreatePartition( ValueSet &partition_, SsapiResponder *pResponder, RowId ridName, RowId remainderNameRid ){

	PMSTR_CMND_INFO				*pCommand = new PMSTR_CMND_INFO();
	PMSTR_CREATE_PARTITION_INFO	*pCmdInfo = new PMSTR_CREATE_PARTITION_INFO();
	StorageElementPartition		*pPartition = new StorageElementPartition( GetListenManager(), this );
	
	*pPartition = partition_;
	pPartition->BuildYourselfFromYourValueSet();

	memset( pCommand, 0, sizeof(PMSTR_CMND_INFO) );
	
	pCommand->opcode			= PMSTR_CMND_CREATE_PARTITION;
	pCmdInfo->srcToPartition	= pPartition->GetOwnerId().GetRowId().GetRowID();
	pCmdInfo->partitionNameRowId= ridName.GetRowID();
	pCmdInfo->partitionSize		= pPartition->GetCapacity() / BLOCK_SIZE;
	pCmdInfo->remainderPartitionNameRowId = remainderNameRid.GetRowID();

	memcpy( &pCommand->cmdParams.createPartitionInfo, pCmdInfo, sizeof(PMSTR_CREATE_PARTITION_INFO) );
	m_pPartitionCommandQ->csndrExecute(	pCommand,
										(pCmdCompletionCallback_t)METHOD_ADDRESS(StorageManager,PartitionCommandCompletionCallback),
										pResponder );
	AddOutstandingRequest();
	
	delete pPartition;
	delete pCmdInfo;
	delete pCommand;
}


//************************************************************************
// MergePartitions:
//
// PURPOSE:		Merges partitions
//
// RECEIVE:		pVsWithIds:		a value set that contains partition objects
//								ids to be merged
//
// NOTE:		At this time we only support merging of two partitions
//************************************************************************

void 
StorageManager::MergePartitions( ValueSet *pVsWithIds, SsapiResponder *pResponder ){

	PMSTR_CMND_INFO				command;
	PMSTR_MERGE_PARTITION_INFO	*pCmdInfo = &command.cmdParams.mergePartitionInfo;
	DesignatorId				id;

	if( pVsWithIds->GetCount() < 2 ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
		return;
	}

	memset( &command, 0, sizeof(command) );
	command.opcode	= PMSTR_CMND_MERGE_PARTITIONS;
	
	pVsWithIds->GetGenericValue( (char *)&id, sizeof(id), 0 );
	pCmdInfo->srcPartitionRowId1 = id.GetRowId().GetRowID();

	pVsWithIds->GetGenericValue( (char *)&id, sizeof(id), 1 );
	pCmdInfo->srcPartitionRowId2 = id.GetRowId().GetRowID();

	m_pPartitionCommandQ->csndrExecute(	&command,
										(pCmdCompletionCallback_t)METHOD_ADDRESS(StorageManager,PartitionCommandCompletionCallback),
										pResponder );
	AddOutstandingRequest();
}


//************************************************************************
// PartitionCommandCompletionCallback:
//
// PURPOSE:		Called back to for all commands submitted to the Partiton
//				Master.
//************************************************************************

void 
StorageManager::PartitionCommandCompletionCallback(	STATUS	completionCode,
													void	*pResultData,
													void	*pCmdData,
													void	*pCmdContext ){

	PMSTR_CMND_INFO					*pCommand = (PMSTR_CMND_INFO *)pCmdData;
	SsapiResponder					*pResponder;
	Raid2SsapiErrorConvertor		errorConvertor;
	U32								exceptionString = errorConvertor.GetSsapiPartitionException( completionCode );

	switch( pCommand->opcode ){

		case PMSTR_CMND_MERGE_PARTITIONS:
		case PMSTR_CMND_CREATE_PARTITION:
			pResponder = (SsapiResponder *)pCmdContext;

			if( m_outstandingRequests ){ // check if this was submitted before the failover
				if( completionCode != RMSTR_SUCCESS )
					pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, exceptionString );
				else
					pResponder->RespondToRequest( SSAPI_RC_SUCCESS );

				RemoveOutstandingRequest();
			}
			break;

		default:
			ASSERT(0);
			break;
	}
}


//************************************************************************
// ShuffleReferencesInArraysAndPartitions:
//
// PURPOSE:		Array and partition objects may have references to each
//				other. Thus it is possible that at the time we are building
//				those objects, some of the  references will be null. So
//				we need to patch that after all objects have been built
//				This method goes thru all such objects and re-stuffs them.
//				Then it checks if there has been a change and posts an event
//				if necessary.
//************************************************************************

void 
StorageManager::ShuffleReferencesInArraysAndPartitions(){

	StorageElementBase		*pElement;
	U32						index;
	int						classType;
	ClassTypeMap			map;
	void					*pDescr;
	StorageElementPartition	*pPartition;
	StorageElementArray		*pArray;
	ValueSet				*pVs = new ValueSet();

	for( index = 0; index < GetManagedObjectCount(); index++ ){
		pElement = (StorageElementBase *)GetManagedObject( index );
		classType = pElement->GetClassType();
		if( classType == SSAPI_OBJECT_CLASS_TYPE_PARTITION_STORAGE_ELEMENT ){
			pPartition = (StorageElementPartition *)pElement;
			pPartition->BuildYourValueSet();
			*pVs = *pPartition;
			if( !m_descriptorCollector.Get( pPartition->GetPartitionRid(), pDescr ) )
				ASSERT(0);
			pPartition->BuildYourselfFromPtsRow( (PARTITION_DESCRIPTOR *)pDescr );
		}
		else if ( map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_ARRAY_STORAGE_ELEMENT, classType, true ) ){
			pArray = (StorageElementArray *)pElement;
			pArray->BuildYourValueSet();
			*pVs = *pArray;
			if( !m_descriptorCollector.Get( pArray->GetArrayRid(), pDescr ) )
				ASSERT(0);
			pArray->BuildYourselfFromPtsRow( (RAID_ARRAY_DESCRIPTOR *)pDescr, this );
		}
		else
			pVs->Clear();
		
		pElement->BuildYourValueSet();

		if( pVs->GetCount() && !(*pVs == (ValueSet &)*pElement) )
			GetListenManager()->PropagateObjectModifiedEventForManagers( GetDesignatorId(), pElement );

		pElement->Clear();
	}

	delete pVs;
}


//************************************************************************
// CreatePartitionStart:
//
// PURPOSE:		Starts the create partition process
//************************************************************************

void 
StorageManager::CreatePartitionStart( ValueSet &objectValues, UnicodeString remainderName, SsapiResponder *pResponder ){

	SM_INSERT_NAME_CELL	*pCell;
	bool				rc, isNameSupplied, isRemanderNameSupplied = false;
	UnicodeString		name;

	isNameSupplied = objectValues.GetString( SSAPI_STORAGE_ELEMENT_B_FID_NAME , &name )? true : false;
	m_remainderNameRid = RowId();

	if( remainderName.GetLength() ){
		isRemanderNameSupplied = true;
		pCell = new SM_INSERT_NAME_CELL;
	
		pCell->pParms		= &objectValues;
		pCell->pResponder	= pResponder;
		pCell->pThis		= this;
		pCell->flag			= !isNameSupplied;
		rc = m_pStringResourceManager->WriteString(	remainderName,
													&m_remainderNameRid,
													(pTSCallback_t)METHOD_ADDRESS(StorageManager, InsertRemainderNameCallback),
													pCell );
		if( rc )
			AddOutstandingRequest();
		else
			pResponder->RespondToRequest(SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);
	}
	
	if( isNameSupplied ){
		pCell = new SM_INSERT_NAME_CELL;

		pCell->pParms		= &objectValues;
		pCell->pResponder	= pResponder;
		pCell->pThis		= this;
		pCell->flag			= true;
		rc = m_pStringResourceManager->WriteString(	name,
													&pCell->ridName,
													(pTSCallback_t)METHOD_ADDRESS(StorageManager, InsertRemainderNameCallback),
													pCell );
		if( rc )
			AddOutstandingRequest();
		else
			pResponder->RespondToRequest(SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED);

	}
	
	if( !isRemanderNameSupplied && !isNameSupplied )
		CreatePartition( objectValues, pResponder, RowId(), RowId() );
}


STATUS 
StorageManager::InsertRemainderNameCallback( void *pContext, STATUS ){

	SM_INSERT_NAME_CELL	*pCell = (SM_INSERT_NAME_CELL *)pContext;
	
	if( pCell->flag) 
		pCell->pThis->CreatePartition( *pCell->pParms, pCell->pResponder, pCell->ridName, pCell->pThis->m_remainderNameRid );
	
	pCell->pThis->RemoveOutstandingRequest();
	delete pCell;

	return OK;
}


//************************************************************************
// GetElementByDescriptorRowId:
//
// PURPOSE:		Looks up an element by the descriptor row id
//************************************************************************

StorageElementBase* 
StorageManager::GetElementByDescriptorRowId( RowId rid ){

	StorageElementBase	*pElement;
	U32					i;

	for( i = 0; i < GetManagedObjectCount(); i++ ){
		pElement = (StorageElementBase *)GetManagedObject( i );
		if( pElement->GetRidDescriptor() == rid )
			return pElement;
	}

	return NULL;
}


//************************************************************************
// BuildPhsDataIdVector:
//
// PURPOSE:		Builds the PHS id vector for the element
//************************************************************************

void 
StorageManager::BuildPhsDataIdVector( StorageElementBase *pElement, bool alwaysRebuild ){

	ManagedObject		*pOldObject;
	DesignatorId		id = pElement->GetDesignatorId(), *pId;
	PHSDataManager		*pMgr = (PHSDataManager *)GetObjectManager( SSAPI_MANAGER_CLASS_TYPE_PHS_DATA_MANAGER );
	CoolVector			container, rowidContainer;
	RowId				*pRowId;

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
StorageManager::DoesBelongToThisElement( StorageElementBase *pElement, DesignatorId& id ){

	CoolVector			container;
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


//******************************************************************************
// CreateDeviceObjects:
//
// PURPOSE:		Called to create objects from a DeviceDescriptor rows.
//
// RECEIVE:		pRows:		a ptr to the array of DeviceDescriptor records
//				rowCount:	size of the above array
//******************************************************************************

bool 
StorageManager::CreateDeviceObjects( Container &container, void *pRows, U32 rowCount ){

	DeviceDescriptor		*pDescr = (DeviceDescriptor *)pRows;
	U32						index, pathNum;
	StorageElementPassThru	*pElement = NULL;
	PathDescriptor			*pPath;
	DeviceManager			*pDM = (DeviceManager *)GetObjectManager(SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER);
	DescriptorCollector		&pathDescriptors = (DescriptorCollector	&)pDM->GetPathDescriptors();


	for( index = 0; index < rowCount; index++, pDescr++ ){
		switch( pDescr->Type ){
			case SCSI_DEVICE_TYPE_ENCLOSURE_SERVICES:
				pElement = new StorageElementPassThruSes( GetListenManager(), this );
				break;

			case SCSI_DEVICE_TYPE_SEQUENTIAL:
				pElement = new StorageElementPassThruTape( GetListenManager(), this );
				break;

			default:
				ASSERT(0);
				continue;
		}
		
		pElement->BuildYourselfFromPtsRow( pDescr );

		// find the paths for this disk
		for( pathNum = 0; pathDescriptors.GetAt( pathNum, (void *&)pPath ); pathNum++ )
			if( RowId( pDescr->rid ) == pPath->ridDescriptor )
				pElement->BuildYourselfFromPathRow( pPath );

		ReadElementName( pElement );
		RecoverParentIds( pElement );
		RecoverChildIds( pElement );

		container.Add( (CONTAINER_ELEMENT)pElement );
	}

	return true;
}


//******************************************************************************
// PTS callbacks for PathDescriptor table
//******************************************************************************


STATUS 
StorageManager::PathTableRowModified( void *pRows,	U32 numberOfRows,	ShadowTable*){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_PATH_TABLE;
		return OK;
	}

	PathDescriptor			*pRow = (PathDescriptor *)pRows;
	StorageElementBase		*pElement;
	U32						i, element;
	DesignatorId			id;
	ClassTypeMap			map;

	for( i = 0; i < numberOfRows; i++, pRow++ ){
		// find the element and update it
		for( element = 0; element < GetManagedObjectCount(); element++ ){
			pElement = (StorageElementBase *)GetManagedObject( element );
			if( pElement->GetRidDescriptor() == pRow->ridDescriptor ){
				if( map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_DISK_STORAGE_ELEMENT, pElement->GetDesignatorId().GetClassId(), true ) ){
					((StorageElementDisk *)pElement)->BuildYourselfFromPathRow( pRow );
					GetListenManager()->PropagateObjectModifiedEventForManagers( GetDesignatorId(), pElement );	
				}
				else if( map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_STORAGE_ELEMENT_PASS_THRU, pElement->GetDesignatorId().GetClassId(), true  ) ){
					((StorageElementPassThru *)pElement)->BuildYourselfFromPathRow( pRow );
					GetListenManager()->PropagateObjectModifiedEventForManagers( GetDesignatorId(), pElement );	
				}
			}
		}
	}

	return OK;
}


//******************************************************************************
// PTS callbacks for DeviceDescriptor (SES & Tape) table
//******************************************************************************

STATUS 
StorageManager::DeviceTableRowInserted( void *pRows,	U32 numberOfRows,	ShadowTable*){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_DEVICE_TABLE;
		return OK;
	}

	CoolVector		container;

	CreateDeviceObjects( container, pRows, numberOfRows );
	AddObjectsIntoManagedObjectsVector( container );

	return OK;
}


STATUS 
StorageManager::DeviceTableRowDeleted( void *pRows,	U32 numberOfRows,	ShadowTable*){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_DEVICE_TABLE;
		return OK;
	}

	CoolVector			container;
	U32					i;
	DeviceDescriptor	*pRow = (DeviceDescriptor *)pRows;
	DesignatorId		id;
	ManagedObject		*pObj;

	for( i = 0; i < numberOfRows; i++, pRow++ ){
		GetDesignatorIdByRowId( pRow->rid, id );
		pObj = GetManagedObject( &id );
		container.Add( (CONTAINER_ELEMENT)pObj );
	}
	
	DeleteObjectsFromTheSystem( container );

	return OK;
}


STATUS 
StorageManager::DeviceTableRowModified( void *pRows,	U32 numberOfRows,	ShadowTable *p){

	if( m_isIniting ){
		m_tablesToRebuild |= SSAPI_SM_DEVICE_TABLE;
		return OK;
	}

	return DeviceTableRowInserted( pRows, numberOfRows, p );
}
