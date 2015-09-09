//************************************************************************
// FILE:		DeviceManager.h
//
// PURPOSE:		Defines class DeviceManager the will manage all device
//				object in the server side if the SSAPI layer.
//************************************************************************

#ifndef __DEVICE_MANAGER_H__
#define	__DEVICE_MANAGER_H__

#include "ObjectManager.h"
#include "EvcStatusRecord.h"
#include "IopStatusTable.h"
#include "CtTypes.h"
#include "DiskDescriptor.h"
#include "ShadowTable.h"
#include "SystemConfigSettingsTable.h"
#include "SsapiAssert.h"

class SsapiLocalResponder;
class CmdSender;
struct LoopDescriptorEntry;
class Chassis;
class Iop;
class DescriptorCollector;
struct SSAPI_DEVICE_MGR_JUMP_TABLE_RECORD;
struct StorageRollCallRecord;
struct PathDescriptor;

#ifdef WIN32
#pragma pack(4)
#endif


#define		SSAPI_DM_IOP_TABLE_BUILT			0x00000001
#define		SSAPI_DM_EVC_TABLE_BUILT			0x00000002
#define		SSAPI_DM_DISK_TABLE_BUILT			0x00000004
#define		SSAPI_DM_LOOP_TABLE_BUILT			0x00000008
#define		SSAPI_DM_SYS_CONFIG_TABLE_BUILT		0x00000010
#define		SSAPI_DM_SRC_TABLE_BUILT			0x00000020
#define		SSAPI_DM_PATH_TABLE_BUILT			0x00000040
#define		SSAPI_DM_ALL_TABLES_BUILT			(	SSAPI_DM_IOP_TABLE_BUILT | \
													SSAPI_DM_EVC_TABLE_BUILT | \
													SSAPI_DM_DISK_TABLE_BUILT| \
													SSAPI_DM_SYS_CONFIG_TABLE_BUILT | \
													SSAPI_DM_SRC_TABLE_BUILT |\
													SSAPI_DM_LOOP_TABLE_BUILT |\
													SSAPI_DM_PATH_TABLE_BUILT	)

#define		SSAPI_DM_NUMBER_OF_TABLES_USED		7
#define		DEVICE_MANAGER_NAME					"DeviceManager"

typedef		VOID (*TM_EXP_ROUTINE_TYPE)(UNSIGNED);

class DeviceManager : public ObjectManager {

	SSAPI_DEVICE_MGR_JUMP_TABLE_RECORD	*m_pTable;
	U32									m_builtTablesMask;
	SsapiLocalResponder					*m_pLocalResponder;	// used for listen on ADD_OBJECT events
	SsapiLocalResponder					*m_pObjectModifiedResponder;
	bool								m_isIniting;		// iniialization-time only
	U32									m_tablesToRebuild;	// iniialization-time only
	SystemConfigSettingsRecord			m_settingsRow;		// contains the row
	bool								m_isSettingsRowPresent; // init-time only
	RowId								m_tempRowId;		// internal only
	CmdSender							*m_pCmbQueue;
	DesignatorId						m_deviceWaitingForCmb; // do not delete
	DescriptorCollector					*m_pPathDescriptors;// shared with the 
															// StorageManager to save memory	
	static DeviceManager				*m_pThis;



//************************************************************************
// DeviceManager:
//
// PURPOSE:		Default constructor
//************************************************************************

DeviceManager( ListenManager *pListenManager, DdmServices *pParent );


public:
	
	friend	class Chassis;
	friend  class Board;
	friend  class HDDDevice;
	friend	class Iop;
	typedef	bool (DeviceManager::*CREATE_OBJECTS_FROM_ROW) (void *pRow, Container &container );


//************************************************************************
// DeviceManager:
//
// PURPOSE:		The destructor
//************************************************************************

~DeviceManager();


//************************************************************************
// GetName:
//
// PURPOSE:		Returns the name of the manager
//************************************************************************

virtual StringClass GetName() { return StringClass(DEVICE_MANAGER_NAME); }


//************************************************************************
// Ctor:
//
// PURPOSE:		Creates the manager
//************************************************************************

static ObjectManager* Ctor(	ListenManager			*pLManager, 
							DdmServices				*pParent, 
							StringResourceManager	*pSRManager ){

	return m_pThis? m_pThis : m_pThis = new DeviceManager( pLManager, pParent );
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

virtual bool Dispatch( ValueSet *pRequestParms, U32 requestCode, SsapiResponder *pResponder);


//************************************************************************
// IsThereSuchDevice:
//
// PURPOSE:		Lets external clients check if a given device is present
//				in the system.
//************************************************************************

bool IsThereSuchDevice( DesignatorId id ) { return GetManagedObject(&id)? true : false; }


//************************************************************************
// GetDeviceState:
//
// PURPOSE:		Retrieves a device's state.
//
// RETURN:		true:		device was found
//************************************************************************

bool GetDeviceState( DesignatorId id, int &state, U32 &stateString );


//************************************************************************
// GetChassisDeviceId:
//
// PURPOSE:		Returns id of the chassis device
//************************************************************************

DesignatorId GetChassisDeviceId();


//************************************************************************
// GetIopBySlot:
//
// PURPOSE:		Returns id of the IOP device in the slot specified. The
//				id will be Clear if no such iop exists
//************************************************************************

DesignatorId GetIopBySlot( int slotNumber );


//************************************************************************
// GetPortByInstanceNumber:
//
// PURPOSE:		Returns id of the FcLoop device with the FC instance
//				number as specified. the returned id will be clear if
//				no such loop exists.
//************************************************************************

DesignatorId GetPortByInstanceNumber( U32 instanceNumber );


//************************************************************************
// AreThesePortsOnPartneredNacs:
//
// PURPOSE:		Determines if the two ports specified belong to NACs that
//				are failover partners.
//************************************************************************

bool AreThesePortsOnPartneredNacs( const DesignatorId &port1, const DesignatorId &port2 );


//************************************************************************
// GetPathDescriptors:
//
// PURPOSE:		Returns the descriptor collector with path descriptors
//				This method is provided for the StorageManager to save
//				the memory (we may have a bunch of paths)	
//************************************************************************

const DescriptorCollector& GetPathDescriptors() { return *m_pPathDescriptors; }



private:

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


//************************************************************************
// EvcTableRowAddedCallback:
//
// PURPOSE:		Called when rows were added to the EVC table
//************************************************************************

STATUS EvcTableRowAddedCallback( void *pRows, U32 numberOfRows, ShadowTable* );


//************************************************************************
// EvcTableRowDeletedCallback:
//
// PURPOSE:		Called when rows were deleted from the EVC table
//************************************************************************

STATUS EvcTableRowDeletedCallback( void *pRows, U32 numberOfRows, ShadowTable*);


//************************************************************************
// EvcTableRowModifiedCallback:
//
// PURPOSE:		Called when rows were modified in the EVC table
//************************************************************************

STATUS EvcTableRowModifiedCallback( void *pRows, U32 numberOfRows, ShadowTable* );


//************************************************************************
// LoopTableRowAddedCallback:
//
// PURPOSE:		Called when rows were added to the LoopDescriptor table
//************************************************************************

STATUS LoopTableRowAddedCallback( void *pRows, U32 numberOfRows, ShadowTable* );


//************************************************************************
// LoopTableRowDeletedCallback:
//
// PURPOSE:		Called when rows were deleted from the LoopDescriptor table
//************************************************************************

STATUS LoopTableRowDeletedCallback( void *pRows, U32 numberOfRows, ShadowTable* );


//************************************************************************
// LoopTableRowModifiedCallback:
//
// PURPOSE:		Called when rows were modified in the LoopDescriptor table
//************************************************************************

STATUS LoopTableRowModifiedCallback( void *pRows, U32 numberOfRows, ShadowTable* );


//************************************************************************
// IopTableRowAddedCallback:
//
// PURPOSE:		CAlled when rows were added to the IOP table
//************************************************************************

STATUS IopTableRowAddedCallback( void *pRows, U32 numberOfRows, ShadowTable* );


//************************************************************************
// IopTableRowDeletedCallback:
//
// PURPOSE:		Called when rows were deleted from the IOP table
//************************************************************************

STATUS IopTableRowDeletedCallback( void *pRows, U32 numberOfRows, ShadowTable* );


//************************************************************************
// IopTableRowModifiedCallback:
//
// PURPOSE:		Called when rows are modified in the IOP table
//************************************************************************

STATUS IopTableRowModifiedCallback( void *pRows, U32 numberOfRows, ShadowTable* );


//************************************************************************
// DiskTableRowAddedCallback:
//
// PURPOSE:		Called when rows are added to the DiskDescriptor
//************************************************************************

STATUS DiskTableRowAddedCallback( void *pRows, U32 numberOfRows, ShadowTable* );


//************************************************************************
// DiskTableRowDeletedCallback:
//
// PURPOSE:		Called when rows are deleted from the DiskDescriptor
//************************************************************************

STATUS DiskTableRowDeletedCallback( void *pRows, U32 numberOfRows, ShadowTable* );


//************************************************************************
// DiskTableRowModifiedCallback:
//
// PURPOSE:		Called when rows are modified in the DiskDescriptor
//************************************************************************

STATUS DiskTableRowModifiedCallback( void *pRows, U32 numberOfRows, ShadowTable* );


//************************************************************************
// Methods used as callback for the SystemConfigSettingsTable
//
//
//************************************************************************

STATUS SystemConfigTableRowModifiedCallback( void *pRows, U32 numberOfRows, ShadowTable*);
STATUS SystemConfigTableRowInsertedCallback( void *pRows, U32 numberOfRows, ShadowTable*);
STATUS SystemConfigTableRowDeletedCallback( void *pRows, U32 numberOfRows, ShadowTable*);


//************************************************************************
// PTS callbacks for the SRC table
//************************************************************************

STATUS SrcTableRowInsertedCallback(  void *pRows, U32 numberOfRows, ShadowTable* );
STATUS SrcTableRowDeletedCallback(  void *pRows, U32 numberOfRows, ShadowTable* );
STATUS SrcTableRowModifiedCallback(  void *pRows, U32 numberOfRows, ShadowTable* );


//************************************************************************
// PTS callbacks for the PathDescriptor table
//************************************************************************

STATUS PathTableRowInsertedCallback(  void *pRows, U32 numberOfRows, ShadowTable* );
STATUS PathTableRowDeletedCallback(  void *pRows, U32 numberOfRows, ShadowTable* );
STATUS PathTableRowModifiedCallback(  void *pRows, U32 numberOfRows, ShadowTable* );


//************************************************************************
// CreateObjectsFromSysConfigRow:
//
// PURPOSE:		Stores the row to be accessed later
//************************************************************************

bool CreateObjectsFromSysConfigRow( SystemConfigSettingsRecord *pRow, Container &container );


//************************************************************************
// CreareObjectsFromEvcRow:
//
// PURPOSE:		Creates managed objects off the row. Puts pointers
//				to them into the container
//************************************************************************

bool CreateObjectsFromEvcRow( EVCStatusRecord *pRow, Container &container );


//************************************************************************
// CreateObjectsFromLoopRow:
//
// PURPOSE:		Creates managed objects off the row. Puts pointers
//				to them into the container
//************************************************************************

bool CreateObjectsFromLoopRow( LoopDescriptorEntry *pRow, Container &container );


//************************************************************************
// CreateObjectsFromIopRow:
//
// PURPOSE:		Creates management objects off the row. Puts pointers
//				to them into the conatiner
//************************************************************************

bool CreateObjectsFromIopRow( IOPStatusRecord *pRow, Container &container );


//************************************************************************
// CreateObjectsFromDiskRow:
//
// PURPOSE:		Creates management objects off the row. Puts pointers
//				to them into the conatiner
//************************************************************************

bool CreateObjectsFromDiskRow( DiskDescriptor *pRow, Container &container );


//************************************************************************
// CreateObjectsFromPathRow:
//
// PURPOSE:		Called when the path decriptor table is enumerated for the
//				first time. Simply copies entries into the path 
//				descriptor collector
//************************************************************************

bool CreateObjectsFromPathRow( PathDescriptor *pRow, Container &container );


bool CreateNoObjects( LoopDescriptorEntry*, Container& ){ return true; }


//************************************************************************
// UpdateDiskObjectsWithSRCData:
//
// PURPOSE:		Called when we first read the SRC table to update
//				disk objects with PHS row ids.
//************************************************************************

bool UpdateDiskObjectsWithSRCData( StorageRollCallRecord *pRow, Container& );


//************************************************************************
// ReconcileNewObjectsWithPhsDataObjects
//
// PURPOSE:		For every device object in the container, goes thru all
//				phs data items and supplies it to the device object so that
//				the latter could determine oif this phs item is its child
//************************************************************************

void ReconcileNewObjectsWithPhsDataObjects( Container &newObjects );


//************************************************************************
// ReconcileDeviceChierachy:
//
// PURPOSE:		Reconciles 'children' and 'parents' for all managed objects
//				Posts appropriate events (CHILD_ADDED, OBJECT_MODIFED )
//				whenever necessary.
//************************************************************************

void ReconcileDeviceChierachy( bool isFirstTime = false );


//************************************************************************
// CreateLogicalDeviceCollections:
//
// PURPOSE:		Creates device collection objects and adds them into the
//				system.
//************************************************************************

void CreateLogicalDeviceCollections(  );

//************************************************************************
// ObjectDeletedCallbackHandler:
//
// PURPOSE:		This method may be provided by subclasses to be notified
//				by events from other object managers. 
//************************************************************************

virtual void ObjectDeletedCallbackHandler( SSAPIEvent *pEvent , bool isLast ) {}


//************************************************************************
// ObjectAddedEventCallback:
//
// PURPOSE:		This is a callback method for all OBJECT_ADDED events 
//				coming from the listen manager.
//************************************************************************

void ObjectAddedEventCallback( ValueSet *pVs, bool isLast, int eventObjectId );


//************************************************************************
// ObjectModifiedEventCallback:
//
// PURPOSE:		This is a callback method for all OBJECT_MODIFIED events 
//				coming from the listen manager.
//************************************************************************

void ObjectModifiedEventCallback( ValueSet *pVs, bool isLast, int eventObjectId );


//************************************************************************
// SetDevicePowered:
//
// PURPOSE:		Attempts to power On/Off a powerable device
//				Will respond regardless of anything!
//
// RETURN:		true
//************************************************************************

bool SetDevicePowered( ValueSet *pParms, SsapiResponder *pResponder );


//************************************************************************
// SetDeviceInService:
//
// PURPOSE:		Attempts to switch IsInService flag of a servicable device
//				Will respond regardless of anything!
//
// RETURN:		true
//************************************************************************

bool SetDeviceInService( ValueSet *pParms, SsapiResponder *pResponder );


//************************************************************************
// SetDeviceLocked:
//
// PURPOSE:		Attempts to lock/unlock a lockable device
//				Will respond regardless of anything!
//
// RETURN:		true
//************************************************************************

bool SetDeviceLocked( ValueSet *pParms, SsapiResponder *pResponder );


//************************************************************************
// GetChassisDevice:
//
// PURPOSE:		Looks up and returns the ptr to the chassis device or
//				NULL if no such device is found
//************************************************************************

Chassis* GetChassisDevice();


//************************************************************************
// ModifyChassisDevice:
//
// PURPOSE:		Attempts to modify the chassis device's data
//************************************************************************

bool ModifyChassisDevice( ValueSet &objectValues, SsapiResponder *pResponder );


//************************************************************************
// ModifyChassisDeviceReplyCallback:
//
// PURPOSE:		Handles the PTS reply and responds to the caller
//************************************************************************

STATUS ModifyChassisDeviceReplyCallback( void *pContext, STATUS rc );


//************************************************************************
// GetShadowTable:
//
// PURPOSE:		Performs a look up of the table by the table mask
//************************************************************************

ShadowTable* GetShadowTable( U32 tableMask );


//************************************************************************
// InitCmbQueueCallback
//************************************************************************

void InitCmbQueueCallback( STATUS rc ) { ASSERT(!rc); };


//************************************************************************
// ChangeBoardLockState:
//
// PURPOSE:		Locks/unlocks a board
//************************************************************************

void ChangeBoardLockState( TySlot slot, bool shouldTurnOn, DesignatorId, SsapiResponder* );
void ChangeBoardLockStateCallback(	STATUS			completionCode,
									void			*pResultData,
									void			*pCmdData,
									void			*pCmdContext );

//************************************************************************
// ChangeHDDLockState:
//
// PURPOSE:		Locks/unlocks an HDD device
//************************************************************************

void ChangeHDDLockState( U32 bayNumber, bool shouldTurnOn, DesignatorId, SsapiResponder* );
void ChangeHDDLockStateCallback(	STATUS			completionCode,
									void			*pResultData,
									void			*pCmdData,
									void			*pCmdContext );


//************************************************************************
// ChangeIopServiceState:
//
// PURPOSE:		Brings an Iop in/out of service
//************************************************************************

void ChangeIopServiceState( Iop *pIop, bool bringInService, SsapiResponder* );
void ChangeIopServiceStateCallback(	STATUS			completionCode,
									void			*pResultData,
									void			*pCmdData,
									void			*pCmdContext );


//************************************************************************
// ChangeIopPowerState:
//
// PURPOSE:		Changes power state of an IOP 
//************************************************************************

void ChangeIopPowerState( Iop *pIop, bool powerDown, SsapiResponder* );
void ChangeIopPowerStateCallback(	STATUS			completionCode,
									void			*pResultData,
									void			*pCmdData,
									void			*pCmdContext );


//************************************************************************
// PowerDownChassis:
//
// PURPOSE:		Attempts to power down the box.
//************************************************************************

void PowerDownChassis( SsapiResponder *pResponder );
void PowerDownChassisCallback( STATUS			completionCode,
								void			*pResultData,
								void			*pCmdData,
								void			*pCmdContext );

//************************************************************************
// GetTempTableData:
// 
// PURPOSE:		Retrieves pointer to temp table data
//************************************************************************

void* GetTempTableData( U32 tableMask );


//************************************************************************
// FindHDDAndSetItsPhsIds
//************************************************************************

void FindHDDAndSetItsPhsIds( RowId ridDisk, RowId ridStatus, RowId ridPerf );


//************************************************************************
// UpdateChassis:
//
// PURPOSE:		A routine called by the system timer to update the time 
//				value inside the Chassis object.
//************************************************************************

STATUS UpdateChassis( Message *pMsg );





};




struct SSAPI_DEVICE_MGR_JUMP_TABLE_RECORD{
	const char								*pTableName;
	ShadowTable								*pShadowTable;
	U32										tableMask;
	ShadowTable::SHADOW_LISTEN_CALLBACK		pRowInsertedCallback;
	ShadowTable::SHADOW_LISTEN_CALLBACK		pRowDeletedCallback;
	ShadowTable::SHADOW_LISTEN_CALLBACK		pRowModifiedCallback;
	DeviceManager::CREATE_OBJECTS_FROM_ROW	pCreateObjectsFromRow;
	U32										rowSize;
	void									*pTempTable;
	fieldDef								*pFieldDef;
	U32										fieldDefSize;
};

#endif	// __DEVICE_MANGER_H__