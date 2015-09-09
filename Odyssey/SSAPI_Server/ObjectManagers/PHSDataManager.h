//************************************************************************
// FILE:		PHSDataManager.h
//
// PURPOSE:		Defines class PHSDataManager that will be responsible
//				for all PHS data items in the SSAPI layer. 
//************************************************************************

#ifndef __PHS_DATA_MANAGER_H__
#define	__PHS_DATA_MANAGER_H__

#include "ObjectManager.h"
#include "EVCStatusRecord.h"
#include "IOPStatusTable.h"
#include "DiskPerformanceTable.h"
#include "DiskStatusTable.h"
#include "SsapiLocalResponder.h"
#include "ShadowTable.h"

struct SSAPI_PHS_JUMP_TABLE_RECORD;
#ifdef WIN32
#pragma pack(4)
#endif


#define	SSAPI_PHS_EVC_TABLE					0x00000001
#define SSAPI_PHS_IOP_TABLE					0x00000002
#define SSAPI_PHS_DISK_STATUS_TABLE			0x00000004
#define SSAPI_PHS_DISK_PERF_TABLE			0x00000008
#define	SSAPI_PHS_ARRAY_STATUS_TABLE		0x00000010
#define	SSAPI_PHS_ARRAY_PERF_TABLE			0x00000020
#define	SSAPI_PHS_SSD_STATUS_TABLE			0x00000040
#define	SSAPI_PHS_SSD_PERF_TABLE			0x00000080
#define	SSAPI_PHS_STS_STATUS_TABLE			0x00000100
#define	SSAPI_PHS_STS_PERF_TABLE			0x00000200

#define	SSAPI_PHS_ALL_TABLES				( SSAPI_PHS_EVC_TABLE \
											| SSAPI_PHS_IOP_TABLE \
											| SSAPI_PHS_DISK_STATUS_TABLE \
											| SSAPI_PHS_DISK_PERF_TABLE \
											| SSAPI_PHS_ARRAY_STATUS_TABLE \
											| SSAPI_PHS_ARRAY_PERF_TABLE \
											| SSAPI_PHS_SSD_STATUS_TABLE \
											| SSAPI_PHS_SSD_PERF_TABLE \
											| SSAPI_PHS_STS_STATUS_TABLE \
											| SSAPI_PHS_STS_PERF_TABLE )

#define	SSAPI_PHS_NUMBER_OF_TABLES_USED		10	// update on a change!!!

#define	PHS_DATA_MANAGER_NAME				"PhsDataManager"


class PHSDataManager : public ObjectManager{

	U32							m_tableMask;		// mask of built tables
	U32							m_tablesToRebuild;	// mask of tables that were altered while being built
	bool						m_isIniting;

	SSAPI_PHS_JUMP_TABLE_RECORD *m_pTable;			// the table of tables used and different parms to process all of them in the same fashion
	static PHSDataManager		*m_pThis;


//************************************************************************
// PHSDataManager:
//
// PURPOSE:		Default constructor
//************************************************************************

PHSDataManager( ListenManager *pListenManager, DdmServices *pParent );


public:


//************************************************************************
// ~PHSDataManager:
//
// PURPOSE:		The destructor
//************************************************************************

~PHSDataManager();


//************************************************************************
// GetName:
//
// PURPOSE:		Returns the name of the manager
//************************************************************************

virtual StringClass GetName() { return StringClass(PHS_DATA_MANAGER_NAME); }


//************************************************************************
// Ctor:
//
// PURPOSE:		Creates the manager
//************************************************************************

static ObjectManager* Ctor(	ListenManager			*pLManager, 
							DdmServices				*pParent, 
							StringResourceManager	*pSRManager ){

	return m_pThis? m_pThis : m_pThis = new PHSDataManager( pLManager, pParent );
}


//************************************************************************
// Dispatch:
//
// PURPOSE:		Responsible for dispatching a request. Must either let
//				the base class handle it (if that one can) or handle it
//				itself or .... fail it!
//************************************************************************

bool Dispatch( ValueSet *pRequestParms, U32 requestCode, SsapiResponder *pResponder);



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

void GetAllAvailableObjectIds( Container &bag );


//************************************************************************
// GetObjectIdsByRowId:
//
// PURPOSE:		Returns a vector of object ids that were built from the
//				row with the row id specified
//
// NOTE:		The caller must go thru all objects in the container 
//				(ptr to DesignatorId) and delete() 'em!
//************************************************************************

void GetObjectIdsByRowId( Container &bag, RowId &rid );


//************************************************************************
//************************************************************************
//************************************************************************
//************************************************************************



protected:

//************************************************************************
// ObjectDeletedCallbackHandler:
//
// PURPOSE:		This method may be provided by subclasses to be notified
//				by events from other object managers. 
//************************************************************************

virtual void ObjectDeletedCallbackHandler( SSAPIEvent *pEvent , bool isLast );


//************************************************************************
//************************************************************************
//************************************************************************
//************************************************************************


private:

//************************************************************************
// DefineAllTables:
//
// PURPOSE:		Defines all PTS table the manager is going to use. This is
//				done to be able to listen on subsequent changes
//************************************************************************

STATUS DefineAllTables();


//************************************************************************
// DefineAllTablesReplyHandler:
//
// PURPOSE:		Handles callback from the PTS for DefineAllTables() method
//************************************************************************

STATUS DefineAllTablesReplyHandler( void *pContext, STATUS rc );


//************************************************************************
// InitializeAllTables:
//
// PURPOSE:		Issues ShadowTable::Initialize() to all tables used
//************************************************************************

STATUS InitializeAllTables();


//************************************************************************
// TableInitializeReplyHandler:
//
// PURPOSE:		Handles replies from the PTS, issues commands to enumerate
//				all tables used by the manager
//************************************************************************

STATUS TableInitializeReplyHandler( void *pContext, STATUS rc );


//************************************************************************
// EnumerateAllTables:
//
// PURPOSE:		Issues requests to the PTS (thru ShadowTables) to enumerate
//				all tables used by the manager.
//************************************************************************

STATUS EnumerateAllTables();


//************************************************************************
// EnumerateAllTablesReplyHandler:
//
// PURPOSE:		Responsible for handling PTS replies to the enumerate 
//				requests. The method must re-issue enumerate command if the
//				table has been modified to this time already. When all
//				tables have been enumerated, the method will set the
//				'readyToServiceRequests' flag and the manager is out of
//				coma.
//************************************************************************

STATUS EnumerateAllTablesReplyHandler( void *pContext, STATUS rc );


//************************************************************************
// The following methods are PTS listener callbacks for the EVCStatus table
//
// RECEIVE:		pRows:			ptr to array with rows affected
//				numberOfRows:	count of the entries in the array
//************************************************************************

STATUS EvcTableRowInsertedCallback( void *pRows, U32 numberOfRows, ShadowTable* );
STATUS EvcTableRowDeletedCallback( void *pRows, U32 numberOfRows, ShadowTable* );
STATUS EvcTableRowModifiedCallback( void *pRows, U32 numberOfRows, ShadowTable* ); 


//************************************************************************
// The following methods are PTS listener callbacks for the IOPStatus table
//
// RECEIVE:		pRows:			ptr to array with rows affected
//				numberOfRows:	count of the entries in the array
//************************************************************************

STATUS IopTableRowInsertedCallback( void *pRows, U32 numberOfRows, ShadowTable* );
STATUS IopTableRowDeletedCallback( void *pRows, U32 numberOfRows, ShadowTable* );
STATUS IopTableRowModifiedCallback( void *pRows, U32 numberOfRows , ShadowTable*); 


//************************************************************************
// The following methods are PTS listener callbacks for the Disk Status
// table
//
// RECEIVE:		pRows:			ptr to array with rows affected
//				numberOfRows:	count of the entries in the array
//************************************************************************

STATUS DiskStatusTableRowInsertedCallback( void *pRows, U32 numberOfRows, ShadowTable* );
STATUS DiskStatusTableRowDeletedCallback( void *pRows, U32 numberOfRows, ShadowTable* );
STATUS DiskStatusTableRowModifiedCallback( void *pRows, U32 numberOfRows, ShadowTable* ); 


//************************************************************************
// The following methods are PTS listener callbacks for the Disk Performance
// table
//
// RECEIVE:		pRows:			ptr to array with rows affected
//				numberOfRows:	count of the entries in the array
//************************************************************************

STATUS DiskPerfTableRowInsertedCallback( void *pRows, U32 numberOfRows, ShadowTable* );
STATUS DiskPerfTableRowDeletedCallback( void *pRows, U32 numberOfRows, ShadowTable* );
STATUS DiskPerfTableRowModifiedCallback( void *pRows, U32 numberOfRows, ShadowTable* ); 


//************************************************************************
// Pts callback for the Array, SSD, STS (Status && Performance) tables
//************************************************************************
STATUS TableRowInsertedCallback( void *pRows, U32 numberOfRows, ShadowTable *p );
STATUS TableRowDeletedCallback( void *pRows, U32 numberOfRows, ShadowTable *p );
STATUS TableRowModifiedCallback( void *pRows, U32 numberOfRows, ShadowTable *p );


//************************************************************************
// CreateObjectsFromSsdPerfRow:
//
// PURPOSE:		Creates object from the SSDPerformance record
//************************************************************************

void CreateObjectsFromSsdPerfRow( void *pRow_, Container &container );


//************************************************************************
// CreateObjectsFromSsdStatusRow:
//
// PURPOSE:		Creates object from the SSDStatus record
//************************************************************************

void CreateObjectsFromSsdStatusRow( void *pRow_, Container &container );


//************************************************************************
// CreateObjectsFromArrayPerfRow:
//
// PURPOSE:		Creates object from the ArrayPerformance record
//************************************************************************

void CreateObjectsFromArrayPerfRow( void *pRow_, Container &container );


//************************************************************************
// CreateObjectsFromArrayPerfRow:
//
// PURPOSE:		Creates object from the ArrayStatus record
//************************************************************************

void CreateObjectsFromArrayStatusRow( void *pRow_, Container &container );


//************************************************************************
// CreateObjectsFromSTSPerfRow:
//
// PURPOSE:		Creates object from the STSPerformance record
//************************************************************************

void CreateObjectsFromSTSPerfRow( void *pRow_, Container &container );


//************************************************************************
// CreateObjectsFromSTSStatusRow:
//
// PURPOSE:		Creates object from the STSStatus record
//************************************************************************

void CreateObjectsFromSTSStatusRow( void *pRow_, Container &container );


//************************************************************************
// CreateObjectsFromDiskStatusRow:
//
// PURPOSE:		Creates all PHS data objects from an DiskStatusRecord.
//				Puts new objects into the container
//************************************************************************

void CreateObjectsFromDiskStatusRow( DiskStatusRecord *pRow, Container &container );


//************************************************************************
// CreateObjectsFromDiskPerfRow:
//
// PURPOSE:		Creates all PHS data objects from an DiskPerformanceRecord.
//				Puts new objects into the container
//************************************************************************

void CreateObjectsFromDiskPerfRow( DiskPerformanceRecord *pRow, Container &container );


//************************************************************************
// CreateObjectsFromIopRow:
//
// PURPOSE:		Creates all PHS data objects from an IOPStatusRecord.
//				Puts new objects into the container
//************************************************************************

void CreateObjectsFromIopRow( IOPStatusRecord *pRow, Container &container );


//************************************************************************
// CreateObjectsFromEvcRow:
//
// PURPOSE:		Creates all PHS data objects from an EVCStatusRecord.
//				Puts new objects into the container
//************************************************************************

void CreateObjectsFromEvcRow( EVCStatusRecord *pRow, Container &container );


//************************************************************************
// ObjectAddedEventCallback:
//
// PURPOSE:		This is a callback method for all OBJECT_ADDED events 
//				coming from the listen manager.
//************************************************************************

void ObjectAddedEventCallback( ValueSet *pVs, bool isLast, int eventObjectId );


//************************************************************************
// GetJumpCellByTable:
//
// PURPOSE:		Returns a jump cell for the shadow table specified
//************************************************************************

SSAPI_PHS_JUMP_TABLE_RECORD* GetJumpCellByTable( ShadowTable *pTable );


//************************************************************************
//************************************************************************
//************************************************************************
//************************************************************************
//************************************************************************

};


typedef	bool (PHSDataManager::*CREATE_OBJECTS_FROM_ROW) (void *pRow, Container &container );

struct SSAPI_PHS_JUMP_TABLE_RECORD{
	const char								*pTableName;
	ShadowTable								*pShadowTable;
	U32										tableMask;
	ShadowTable::SHADOW_LISTEN_CALLBACK		pRowInsertedCallback;
	ShadowTable::SHADOW_LISTEN_CALLBACK		pRowDeletedCallback;
	ShadowTable::SHADOW_LISTEN_CALLBACK		pRowModifiedCallback;
	CREATE_OBJECTS_FROM_ROW					pCreateObjectsFromRow;
	U32										rowSize;
	void									*pTempTable;
	fieldDef								*pFieldDef;
	U32										fieldDefSize;
};

#endif // __PHS_DATA_MANAGER_H__