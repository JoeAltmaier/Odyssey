//******************************************************************************
// FILE:		StorageManager.h
//
// PURPOSE:		Defines the object manager reponsible for managing storage
//				elements available in the O2K
//******************************************************************************

#ifndef	__STORAGE_MANAGER_H__
#define __STORAGE_MANAGER_H__

#include "CmdSender.h"
#include "ObjectManager.h"
#include "RaidDefs.h"
#include "ArrayDescriptor.h"
#include "DiskDescriptor.h"
#include "StorageRollCallTable.h"
#include "RaidMemberTable.h"
#include "RaidSpareDescriptor.h"
#include "ShadowTable.h"
#include "DescriptorCollector.h"
#include "PartitionTable.h"

class StringResourceManager;
class StorageElementArray;
class StorageElement;
class StorageCollectionSparePool;
class StorageElementBase;
class StorageElementSsd;
class Container;
struct SSAPI_STORAGE_MGR_JUMP_TABLE_RECORD;

#ifdef WIN32
#pragma pack(4)
#endif


#define		SSAPI_SM_SRC_TABLE					0x00000001
#define		SSAPI_SM_DISK_TABLE					0x00000002
#define		SSAPI_SM_MEMBER_TABLE				0x00000004
#define		SSAPI_SM_SPARE_TABLE				0x00000008
#define		SSAPI_SM_ARRAY_TABLE				0x00000010
#define		SSAPI_SM_PATH_TABLE					0x00000020
#define		SSAPI_SM_DEVICE_TABLE				0x00000040
#define		SSAPI_SM_PART_TABLE					0x00000080
#define		SSAPI_SM_SSD_TABLE					0x00000100
#define		SSAPI_SM_ALL_TABLES_BUILT			(	SSAPI_SM_SRC_TABLE |\
													SSAPI_SM_DISK_TABLE |\
													SSAPI_SM_MEMBER_TABLE |\
													SSAPI_SM_SPARE_TABLE |\
													SSAPI_SM_ARRAY_TABLE |\
													SSAPI_SM_PATH_TABLE	|\
													SSAPI_SM_DEVICE_TABLE |\
													SSAPI_SM_PART_TABLE |\
													SSAPI_SM_SSD_TABLE	)

#define		SSAPI_SM_NUMBER_OF_TABLES_USED		9

#define		STORAGE_MANAGER_NAME				"StorageManager"

class StorageManager : public ObjectManager {

	SSAPI_STORAGE_MGR_JUMP_TABLE_RECORD		*m_pTable;
	U32										m_builtTablesMask;
	SsapiLocalResponder						*m_pObjectAddedResponder;	
	bool									m_isIniting;		// iniialization-time only
	U32										m_tablesToRebuild;	// iniialization-time only
	U32										m_outstandingRequests;
	StringResourceManager					*m_pStringResourceManager;
	bool									m_shouldRebuildAllTables; // init time only
	CmdSender								*m_pRaidCommandQ;
	CmdSender								*m_pPartitionCommandQ;
	DescriptorCollector						m_descriptorCollector;	// for the one referenced in SRC
	DescriptorCollector						m_memberDescriptorCollector;
	DescriptorCollector						m_spareDescriptorCollector;
	static	StorageManager					*m_pThis;
	DescriptorCollector						m_pendingSrcUpdates;	// RowIds of elements whose redundant updates are pending
	RowId									m_remainderNameRid;		// for create partition


//************************************************************************
// StorageManager:
//
// PURPOSE:		Default constructor
//************************************************************************

StorageManager( ListenManager			*pListenManager,	
				DdmServices				*pParent,
				StringResourceManager	*pSRManager );


public:

	friend class StorageElementArray;
	friend class StorageElementArray1;
	friend class StorageElementPartition;

	typedef	bool (StorageManager::*CREATE_OBJECTS_FROM_ROW) ( Container &container, void *pRows, U32 rowCount );
	typedef void (StorageManager::*CREATE_OBJECT_FROM_UI_VECTOR) (ValueSet&, SsapiResponder*, RowId );



//************************************************************************
// StorageManager:
//
// PURPOSE:		The destructor
//************************************************************************

~StorageManager();


//************************************************************************
// GetName:
//
// PURPOSE:		Returns the name of the manager
//************************************************************************

virtual StringClass GetName() { return STORAGE_MANAGER_NAME; }


//************************************************************************
// Ctor:
//
// PURPOSE:		Creates the manager
//************************************************************************

static ObjectManager* Ctor(	ListenManager			*pLManager, 
							DdmServices				*pParent, 
							StringResourceManager	*pSRManager ){

	return m_pThis? m_pThis : m_pThis = new StorageManager( pLManager, pParent, pSRManager );
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


private:

//************************************************************************
//  Outstanding requests management crap
//************************************************************************
void AddOutstandingRequest(){ m_outstandingRequests++; SetIsReadyToServiceRequests(false); }
void RemoveOutstandingRequest(){ if( !--m_outstandingRequests ) SetIsReadyToServiceRequests(true);}


//************************************************************************
// DefineAllTables:
//
// PURPOSE:		Issues requests to the PTS to define all tables if not yet
//				defined
//************************************************************************

STATUS DefineAllTables();


//************************************************************************
// DefineAllTablesReplyHandler:
//
// PURPOSE:		Handles replies to requests issued in DefineAllTables()
//************************************************************************

STATUS DefineAllTablesReplyHandler( void *pContext, STATUS status );


//************************************************************************
// InitializeAllTables:
//
// PURPOSE:		Sends requests to the PTS to init ShadowTable objects
//				for all the tables used.
//************************************************************************

STATUS InitializeAllTables();


//************************************************************************
// InitializeAllTablesReplyHandler:
//
// PURPOSE:		Handles replies to requests to the PTS sent from 
//				InitializeAllTables()
//************************************************************************

STATUS InitializeAllTablesReplyHandler( void *pContext, STATUS status );


//************************************************************************
// EnumerateAllTables:
//
// PUPROSE:		Issues requests to the PTS to enumerate all tables used
//************************************************************************

STATUS EnumerateAllTables();


//************************************************************************
// EnumerateAllTablesReplyHandler:
//
// PURPOSE:		Handles replies to requests sent by EnumerateAllTables().
//				When all table have been enumerated, sets "ready for 
//				service flag"
//************************************************************************

STATUS EnumerateAllTablesReplyHandler( void *pContext, STATUS rc );


//******************************************************************************
// AllTablesHaveBeenEnumerated
//
// PURPOSE:		Called when all tables have been successfully enumed and
//				we can start building objects
//******************************************************************************

void AllTablesHaveBeenEnumerated();


//******************************************************************************
// PTS listen callbacks for RAID tables
//******************************************************************************

STATUS MemberTableRowModified( void *pRows, U32 rowCount, ShadowTable * );
STATUS MemberTableRowInserted( void *pRows, U32 rowCount, ShadowTable * );
STATUS MemberTableRowDeleted( void *pRows, U32 rowCount, ShadowTable * );

STATUS SpareTableRowModified( void *pRows , U32 rowCount, ShadowTable* );
STATUS SpareTableRowInserted( void *pRows , U32 rowCount, ShadowTable* );
STATUS SpareTableRowDeleted( void *pRows , U32 rowCount, ShadowTable* );


//******************************************************************************
// PTS Callbacks for the Array Descriptor Table
//******************************************************************************

STATUS ArrayTableRowInserted( void *pRows, U32 numberOfRows, ShadowTable* );
STATUS ArrayTableRowDeleted( void *pRows, U32 numberOfRows, ShadowTable* );
STATUS ArrayTableRowModified( void *pRows, U32 numberOfRows, ShadowTable* );


//******************************************************************************
// PTS Callbacks for the Partition Descriptor Table
//******************************************************************************

STATUS PartitionTableRowInserted( void *pRows, U32 numberOfRows, ShadowTable* );
STATUS PartitionTableRowDeleted( void *pRows, U32 numberOfRows, ShadowTable* );
STATUS PartitionTableRowModified( void *pRows, U32 numberOfRows, ShadowTable* );


//******************************************************************************
// PTS callbacks for the Disk Descriptor
//******************************************************************************

STATUS DiskTableRowInserted( void *pRows, U32 numberOfRows, ShadowTable* );
STATUS DiskTableRowDeleted( void *pRows, U32 numberOfRows, ShadowTable* );
STATUS DiskTableRowModified( void *pRows, U32 numberOfRows, ShadowTable* );


//******************************************************************************
// PTS callbacks for the SRC Table
//******************************************************************************

STATUS SrcTableRowInserted( void *pRows, U32 numberOfRows, ShadowTable* );
STATUS SrcTableRowDeleted( void *pRows, U32 numberOfRows, ShadowTable* );
STATUS SrcTableRowModified( void *pRows, U32 numberOfRows, ShadowTable* );


//******************************************************************************
// PTS callbacks for PathDescriptor table
//******************************************************************************

STATUS PathTableRowInserted( void *p, U32 i, ShadowTable *t){ return PathTableRowModified(p,i,t); }
STATUS PathTableRowDeleted( void *,	U32 , ShadowTable*){ return OK; }
STATUS PathTableRowModified( void *pRows,	U32 numberOfRows,	ShadowTable*);


//******************************************************************************
// PTS callbacks for DeviceDescriptor (SES & Tape) table
//******************************************************************************

STATUS DeviceTableRowInserted( void *pRows,	U32 numberOfRows,	ShadowTable*);
STATUS DeviceTableRowDeleted( void *pRows,	U32 numberOfRows,	ShadowTable*);
STATUS DeviceTableRowModified( void *pRows,	U32 numberOfRows,	ShadowTable*);


//******************************************************************************
// PTS callbacks for the SSD descriptor Table
//******************************************************************************

STATUS SsdTableRowInserted( void *pRows, U32 numberOfRows,	ShadowTable*);
STATUS SsdTableRowDeleted( void *pRows,	U32 numberOfRows,	ShadowTable*);
STATUS SsdTableRowModified( void *pRows, U32 numberOfRows,	ShadowTable*);


//******************************************************************************
// CreateObjects:
//
// PURPOSE:		Method that create objects from tables
//
// NOTE:		If 'pRows' is NULL, method will use the tempTableData
//******************************************************************************

bool CreateDiskObjects( Container &container, void *pRows, U32 rowCount  );
bool CreateAndAddArrayObjects( void *pRows, U32 rowCount   );
bool CreateSparePoolObjects( Container &container, void *pRows, U32 rowCount  );
bool CreateNoObjects( Container &container,void *, U32 ) { return true; }
bool CreateDeviceObjects( Container &container, void *pRows, U32 rowCount );
bool CreateSsdObjects( Container &container, void *pRows, U32 rowCount );
bool CreateAndAddPartitionObjects( Container &container, void *pRows, U32 rowCount );

//************************************************************************
// GetShadowTable:
//
// PURPOSE:		Performs a look up of the table by the table mask
//************************************************************************

ShadowTable* GetShadowTable( U32 tableMask );


//************************************************************************
// GetTempTableData:
// 
// PURPOSE:		Retrieves pointer to temp table data
//************************************************************************

void* GetTempTableData( U32 tableMask );

//************************************************************************
// ObjectDeletedCallbackHandler:
//
// PURPOSE:		This method may be provided by subclasses to be notified
//				by events from other object managers. 
//************************************************************************

virtual void ObjectDeletedCallbackHandler( SSAPIEvent *pEvent , bool isLast );


//************************************************************************
// ObjectAddedEventCallback:
//
// PURPOSE:		This is a callback method for all OBJECT_ADDED events 
//				coming from the listen manager.
//************************************************************************

void ObjectAddedEventCallback( ValueSet *pVs, bool isLast, int eventObjectId );


//************************************************************************
// ClearAllTempTables:
//
// PURPOSE:		Deallocates memory taken by temp tables 
//************************************************************************

void ClearAllTempTables();


//************************************************************************
// RaidCmdQInitReply:
//
// PURPOSE:		Callback for the Raid command Q
//************************************************************************

void RaidCmdQInitReply( STATUS rc ){ RemoveOutstandingRequest(); }
void PartitionCmdQInitReply( STATUS rc ){  }


//************************************************************************
// RaidEventHandler:
//
// PURPOSE:		CAlled by the Raid command Q on command completetion and
//				to report events
//************************************************************************

void RaidEventHandler( STATUS comletionCode, void *pStatusData );
void PartitionEventHandler( STATUS comletionCode, void *pStatusData );


//************************************************************************
// SaveDescriptors:
//
// PURPOSE:		Saves descriptors contained in the table specified by the
//				'tableMask'
//************************************************************************

void SaveDescriptors( U32 tableMask, U32 descriptorSize, DescriptorCollector& );


//************************************************************************
// CreateArray:
//
// PURPOSE:			Send Create_Array opcode to the RAID Master
//************************************************************************

void CreateArray( ValueSet &requestParms, SsapiResponder *pResponder, RowId );


//************************************************************************
// DeleteArray:
//
// PURPOSE:			Deletes an array from the system
//************************************************************************

void DeleteArray( StorageElementArray *pArray, SsapiResponder *pResponder );


//************************************************************************
// RaidCommandCompletionCallback:
//
// PURPOSE:		Called back to for all commands submitted to the RAID Q.
//************************************************************************

void RaidCommandCompletionCallback(	STATUS			completionCode,
									void			*pResultData,
									void			*pCmdData,
									void			*pCmdContext );


//************************************************************************
// PartitionCommandCompletionCallback:
//
// PURPOSE:		Called back to for all commands submitted to the Partiton
//				Master.
//************************************************************************

void PartitionCommandCompletionCallback(STATUS			completionCode,
										void			*pResultData,
										void			*pCmdData,
										void			*pCmdContext );


//************************************************************************
// Spare operations:
//************************************************************************

void AddDedicatedSpare( DesignatorId arrayId, DesignatorId spareId, SsapiResponder *pResponder );
void DeleteDedicatedSpare( DesignatorId arrayId, DesignatorId spareId, SsapiResponder *pResponder );
void AddPoolSpare( DesignatorId arrayId, DesignatorId spareId, SsapiResponder *pResponder );
void DeletePoolSpare( DesignatorId arrayId, DesignatorId spareId, SsapiResponder *pResponder );


//************************************************************************
// GetArrayByArrayRowId:
//
// PURPOSE:		Looks for the array with the array rid specified
//************************************************************************

StorageElementArray* GetArrayByArrayRowId( RowId rid );


//************************************************************************
// GetSparePool:
//
// PURPOSE:		Looks up the spare pool object
//************************************************************************

StorageCollectionSparePool* GetSparePool();

//************************************************************************
// Member operations
//************************************************************************

void AddArrayMember( DesignatorId arrayId, DesignatorId memberId, SsapiResponder *pResponder );
void DeleteArrayMember( DesignatorId arrayId, DesignatorId memberId, SsapiResponder *pResponder );
void DownArrayMember( DesignatorId arrayId, DesignatorId memberId, SsapiResponder *pResponder );
void SetPreferredMember( DesignatorId arrayId, DesignatorId memberId, SsapiResponder *pResponder );
void SetSourceMember( DesignatorId arrayId, DesignatorId memberId, SsapiResponder *pResponder );

//************************************************************************
// Spare id look up
//************************************************************************

bool GetSRCIdBySpareId( RowId spareId, RowId &srcId );
bool GetSpareIdBySRCId( RowId srcId, RowId &spareId );


//************************************************************************
// Member id look up
//************************************************************************

bool GetSRCIdByMemberId( RowId memberId, RowId &srcId );
bool GetMemberIdBySRCId( RowId srcId, RowId &memberId );


//************************************************************************
// ChangeStorageElementName:
//
// PURPOSE:		Changes element's name
//************************************************************************

void ChangeStorageElementName( DesignatorId elementId, UnicodeString name, SsapiResponder *pResponder );
STATUS ChangeStorageElementNameCallback( void *pContext, STATUS rc );

//************************************************************************
// ReadElementName:
//
// PURPOSE:		Reads storage element's name and will update the element
//				when the name is read
//************************************************************************

void ReadElementName( StorageElementBase *pElement );
STATUS ReadElementNameCallback( void *pContext, STATUS rc );


//************************************************************************
// DummyCallback:
//************************************************************************

STATUS DummyCallback( void*, STATUS ) { return OK; }
STATUS InsertRemainderNameCallback( void *pContext, STATUS );



//************************************************************************
// AddHotCopy:
//
// PURPOSE:		Adds a hot copy array
//************************************************************************

void AddHotCopy( ValueSet &requestParms, SsapiResponder *pResponder, RowId ridName  );


//************************************************************************
// BreakHotCopy:
//
// PURPOSE:		Removes a Hot Copy element
//************************************************************************

void RemoveHotCopy( ValueSet &requestParms, SsapiResponder *pResponder );


//************************************************************************
// CreatePartition
//
// PURPOSE:		Creates a partition by sending a command to the 
//				Partition Master
//************************************************************************

void CreatePartition(	ValueSet &partition, SsapiResponder *pResponder, 
						RowId nameRid,		 RowId remainderNameRid );


//************************************************************************
// MergePartitions:
//
// PURPOSE:		Merges partitions
//
// RECEIVE:		pVsWithIds:		a value set that contains partition objects
//								ids to be merged
//************************************************************************

void MergePartitions( ValueSet *pVsWithIds, SsapiResponder *pResponder );


//************************************************************************
// InsertNameAndContinue:
//
// PURPOSE:		Inserts a name into the PTS and once the name is in,
//				call the pVector specified with the paramteres given
//************************************************************************

void InsertNameAndContinue(	ValueSet *pRequestParms, 
							SsapiResponder *pResponder, 
							CREATE_OBJECT_FROM_UI_VECTOR pVector, 
							bool shouldCheckForDuplicateName );
STATUS InsertNameAndContinueCallback( void *pContext, STATUS status );


//************************************************************************
// RecoverName:
//
// PURPOSE:		Checks if there was such an element and recovers the name
//************************************************************************

void RecoverName( StorageElement &element );


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

void ShuffleReferencesInArraysAndPartitions();


//************************************************************************
// CreatePartitionStart:
//
// PURPOSE:		Starts the create partition process
//************************************************************************

void CreatePartitionStart( ValueSet &objectValues, UnicodeString remainderName, SsapiResponder *pResponder );


//************************************************************************
// GetElementByDescriptorRowId:
//
// PURPOSE:		Looks up an element by the descriptor row id
//************************************************************************

StorageElementBase* GetElementByDescriptorRowId( RowId rid );


//************************************************************************
// BuildPhsDataIdVector:
//
// PURPOSE:		Builds the PHS id vector for the element
//************************************************************************

void BuildPhsDataIdVector( StorageElementBase*, bool alwaysRebuild = false );


//************************************************************************
// DoesBelongToThisElement:
//
// PURPOSE:		Checks if a given phs object id is claimed by the 
//				storage element specified.
//************************************************************************

bool DoesBelongToThisElement( StorageElementBase*, DesignatorId& rid );



};

struct SM_INSERT_NAME_CELL{
	ValueSet										*pParms;
	SsapiResponder									*pResponder;
	RowId											ridName;
	StorageManager::CREATE_OBJECT_FROM_UI_VECTOR	pVector;
	StorageManager									*pThis;
	bool											flag;
};

struct SSAPI_STORAGE_MGR_JUMP_TABLE_RECORD{
	char									*pTableName;
	ShadowTable								*pShadowTable;
	U32										tableMask;
	ShadowTable::SHADOW_LISTEN_CALLBACK		pRowInsertedCallback;
	ShadowTable::SHADOW_LISTEN_CALLBACK		pRowDeletedCallback;
	ShadowTable::SHADOW_LISTEN_CALLBACK		pRowModifiedCallback;
	StorageManager::CREATE_OBJECTS_FROM_ROW	pCreateObjectsFromRow;
	U32										rowSize;
	void									*pTempTable;
	fieldDef								*pFieldDef;
	U32										fieldDefSize;
};


struct READ_NAME_CELL{
	StorageManager		*pThis;
	UnicodeString		*pName;
	DesignatorId		elementId;
};


struct MODIFY_NAME_CELL{

	MODIFY_NAME_CELL( StorageManager *pT ) { pThis = pT; }

	StorageManager		*pThis;
	RowId				rid;
	DesignatorId		id;
	UnicodeString		name;
};

#endif // __STORAGE_MANAGER_H__