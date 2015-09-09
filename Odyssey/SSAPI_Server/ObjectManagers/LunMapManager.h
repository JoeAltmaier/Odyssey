//************************************************************************
// FILE:		LunMapManager.h
//
// PURPOSE:		Defines class LunMapManager whose instance will control
//				LUN mapping/unmapping as well as exporting/unexporting
//************************************************************************

#ifndef __LUN_MAP_MANAGER_H__
#define	__LUN_MAP_MANAGER_H__

#include "ObjectManager.h"
#include "ShadowTable.h"
#include "ExportTable.h"
#include "ExportTableUserInfoTable.h"
#include "StorageRollCallTable.h"
#include "SsapiLocalResponder.h"
#include "DescriptorCollector.h"


class StringResourceManager;
class CmdSender;
struct SSAPI_LUN_MAP_MGR_JUMP_TABLE_RECORD;

#ifdef WIN32
#pragma pack(4)
#endif


#define		SSAPI_LM_EXPORT_USER_INFO_TABLE		0x00000001
#define		SSAPI_LM_SRC_TABLE					0x00000002
#define		SSAPI_LM_ID_INFO_TABLE				0x00000004
#define		SSAPI_LM_EXPORT_TABLE				0x00000008 // this table must be processed last as it requires that all other table be built!

#define		SSAPI_LM_ALL_TABLES_BUILT			(	SSAPI_LM_EXPORT_TABLE | \
													SSAPI_LM_SRC_TABLE |\
													SSAPI_LM_ID_INFO_TABLE |\
													SSAPI_LM_EXPORT_USER_INFO_TABLE )

#define		SSAPI_LM_NUMBER_OF_TABLES_USED		4

#define		LUN_MAP_MANAGER_NAME				"LunMapManager"



class LunMapManager : public ObjectManager{

	StringResourceManager				*m_pStringResourceManager;
	SSAPI_LUN_MAP_MGR_JUMP_TABLE_RECORD	*m_pTable;
	U32									m_builtTablesMask;
	U32									m_tablesToRebuild;	// iniialization-time only
	bool								m_isIniting;
	SsapiLocalResponder					*m_pLocalResponder;	// used for listen on ADD_OBJECT events
	SsapiLocalResponder					*m_pObjectModifiedResponder;
	U32									m_requestsOutstanding;
	DescriptorCollector					m_descriptorCollector;
	DescriptorCollector					m_idInfoCollector;
	CmdSender							*m_pCmdSender;
	RowId								m_tempRowId;
	static	LunMapManager				*m_pThis;

	friend class LunMapEntry;

//************************************************************************
// LunMapManager:
//
// PURPOSE:		Default constructor
//************************************************************************

LunMapManager(	ListenManager			*pListenManager,	
				DdmServices				*pParent,
				StringResourceManager	*pSRManager );


public:

	enum LM_STRING_TYPE { LM_NAME = 1, LM_DESCRIPTION	};
	typedef	bool (LunMapManager::*CREATE_OBJECTS_FROM_ROW) (void *pRow, Container &container );



//************************************************************************
// ~LunMapManager:
//
// PURPOSE:		The destructor
//************************************************************************

~LunMapManager();


//************************************************************************
// GetName:
//
// PURPOSE:		Returns the name of the manager
//************************************************************************

virtual StringClass GetName() { return StringClass(LUN_MAP_MANAGER_NAME); }


//************************************************************************
// Ctor:
//
// PURPOSE:		Creates the manager
//************************************************************************

static ObjectManager* Ctor(	ListenManager			*pLManager, 
							DdmServices				*pParent, 
							StringResourceManager	*pSRManager ){

	return m_pThis? m_pThis : m_pThis = new LunMapManager( pLManager, pParent, pSRManager );
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


//************************************************************************
// ObjectDeletedCallbackHandler:
//
// PURPOSE:		This method may be provided by subclasses to be notified
//				by events from other object managers. 
//************************************************************************

virtual void ObjectDeletedCallbackHandler( SSAPIEvent *pEvent , bool isLast );


//************************************************************************
// AddObject:
//
// PURPOSE:		Adds an object to the system
//
// NOTE:		Must be overridden by object managers that can add objects
//************************************************************************

virtual bool AddObject( ValueSet &objectValues, SsapiResponder *pResponder  );


//************************************************************************
// IsThisPathInUse:
//
// PURPOSE:		Checks if any of the lun entries are exportes on the data 
//				path specified .
//************************************************************************

bool IsThisPathInUse( const DesignatorId &pathId );


STATUS AddObjectInsertStringsCallback( void *pContext, STATUS status );
STATUS AddObjectInsertUIRowCallback( void *pContext, STATUS status );
STATUS AddObjectTestCallback( void *pContext, STATUS status );
void   AddObjectCallback(	STATUS			completionCode,
							void			*pResultData,
							void			*pCmdData,
							void			*pCmdContext );

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
// PTS callbacks for ExportTableUserInfo Table
//************************************************************************

STATUS UserInfoRowInserted( void*, U32, ShadowTable* );
STATUS UserInfoRowDeleted( void*, U32, ShadowTable* );
STATUS UserInfoRowModified( void*, U32, ShadowTable* );


//************************************************************************
// PTS callbacks for Storage Roll Call Table
//************************************************************************

STATUS SrcTableRowInserted( void *pRows_, U32 numberOfRows, ShadowTable* );
STATUS SrcTableRowDeleted( void *pRows_, U32 numberOfRows, ShadowTable* );
STATUS SrcTableRowModified( void *pRows_, U32 numberOfRows, ShadowTable* );


//************************************************************************
// PTS callbacks for Export Table
//************************************************************************

STATUS ExportTableRowInserted( void *pRows_, U32 numberOfRows, ShadowTable* );
STATUS ExportTableRowDeleted( void *pRows_, U32 numberOfRows, ShadowTable* );
STATUS ExportTableRowModified( void *pRows_, U32 numberOfRows, ShadowTable* );


//************************************************************************
// PTS callbacks for IdInfo Table
//************************************************************************

STATUS IdInfoTableRowInserted( void *pRows_, U32 numberOfRows, ShadowTable* );
STATUS IdInfoTableRowDeleted( void *pRows_, U32 numberOfRows, ShadowTable* );
STATUS IdInfoTableRowModified( void *pRows_, U32 numberOfRows, ShadowTable* );


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
//
// PURPOSE:		Create objects from PTS rows
//************************************************************************

bool CreateObjectsFromUserInfoRow( ExportTableUserInfoRecord *pRow, Container &container ){ return true; }
bool CreateObjectsFromExportTableRow(	ExportTableEntry	*pRow, 
										Container			&container );

bool CreateNoObjects( void*, Container&) { return true; }


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
// Outstanding request business...
//************************************************************************

void AddOutstandingRequest(){ m_requestsOutstanding++; SetIsReadyToServiceRequests(false); }
void RemoveOutstandingRequest();


//************************************************************************
// ModifyObject:
//
// PURPOSE:		Responsible for modifying some info of a lun map entry
//				(like name, description)
//************************************************************************

bool ModifyObject( ValueSet &objectValues, SsapiResponder *pResponder );


//************************************************************************
// ModifyObjectStringsCallback:
//
// PURPOSE:		Called by the string resource manager to inform the object
//				that string resources have been updated
//************************************************************************

STATUS ModifyObjectStringsCallback( void *pContext, STATUS rc );


//************************************************************************
// ModifyUserInfoFieldCallback:
//
// PURPOSE:		Called when we modify rowId of the UserInfo record
//				for a given Export Table row
//************************************************************************

STATUS ModifyUserInfoFieldCallback( void *pContext, STATUS rc );


//************************************************************************
// ModifyLunMap:
//
// PURPOSE:		Modifies LunMap info. 
//************************************************************************

bool ModifyLunMap( ValueSet &objectValues, SsapiResponder *pResponder );


//************************************************************************
// DeleteLunMap:
// 
// PURPOSE:		Deletes LunMap entry
//************************************************************************

bool DeleteLunMap( DesignatorId id, SsapiResponder *pResponder );
void DeleteLunMapCallback(	STATUS			completionCode,
							void			*pResultData,
							void			*pCmdData,
							void			*pCmdContext );

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

bool CheckForDuplicateExportInfo( LunMapEntry *pLun, SsapiResponder *pResponder );


//************************************************************************
// ModifyLunMapReplyCallback:
//
// PURPOSE:		Called by the lun map entry to inform the manager 
//				the strings have been stored
//************************************************************************

STATUS ModifyLunMapReplyCallback( void *pContext, STATUS rc );


//************************************************************************
// ReadEntryString:
//
// PURPOSE:		Reads in the string associated with the entry
//************************************************************************

void ReadEntryString( RowId rid, LM_STRING_TYPE type, LunMapEntry &entry  );
STATUS ReadEntryStringCallback( void *pContext, STATUS status );


//************************************************************************
// UpdateEntryName:
//
// PURPOSE:		Updates entry's name
//************************************************************************

bool UpdateEntryName( LunMapEntry &entry, UnicodeString &newName, SsapiResponder* );
STATUS UpdateEntryNameCallback( void *pContext, STATUS status );


//************************************************************************
// UpdateEntryDescription;
//
// PURPOSE:		Updates entry's description
//************************************************************************

bool UpdateEntryDescription( LunMapEntry &entry, UnicodeString &description, SsapiResponder* );
STATUS UpdateEntryDescriptionCallabck( void *pContext, STATUS status );


//************************************************************************
// DummyCallback:
//
// PURPOSE:		Used as a callback for calls which we need no callback from :-)
//************************************************************************

STATUS DummyCallback( void*, STATUS ) { return OK; }


//************************************************************************
// InitVCMCommandQueueCallback:
//
// PURPOSE:		Called by the VCM command queue object to report the end
//				of command q. initialization.
//************************************************************************

void InitVCMCommandQueueCallback( STATUS rc );


//************************************************************************
// ExportLun:
//
// PURPOSE:		Attempts to export/unexport the lun entry specified. 
//				Always responds.
//************************************************************************

void ExportLun( DesignatorId &lunId, bool shouldExport, SsapiResponder *pResponder );
void ExportLunCallback( STATUS			completionCode,
						void			*pResultData,
						void			*pCmdData,
						void			*pCmdContext );


//************************************************************************
// VCMEventHandler:
//
// PURPOSE:		Called by the VCM to inform of async' events
//************************************************************************

void VCMEventHandler( STATUS eventCode, void *pEventData );


//************************************************************************
// GetEntryByRidHandle:
//
// PURPOSE:		Performs a look up of entries by the VCM rid handle
//************************************************************************

LunMapEntry* GetEntryByRidHandle( RowId ridHandle );


//************************************************************************
// BuildPhsDataIdVector:
//
// PURPOSE:		Builds the PHS id vector for the element
//************************************************************************

void BuildPhsDataIdVector( LunMapEntry*, bool alwaysRebuild = false );


//************************************************************************
// DoesBelongToThisElement:
//
// PURPOSE:		Checks if a given phs object id is claimed by the 
//				storage element specified.
//************************************************************************

bool DoesBelongToThisElement( LunMapEntry*, DesignatorId& rid );


//************************************************************************
// GetLunByVd:
//
// PURPOSE:		Searches existing LUNs for the one whose Next VD is 
//				specified.
//
// RETURN:		success:	ptr the object
//				failure:	NULL
//************************************************************************

LunMapEntry* GetLunByVd( VDN vd );


//************************************************************************
//************************************************************************
//************************************************************************
//************************************************************************
};

struct SSAPI_LUN_MAP_MGR_JUMP_TABLE_RECORD{
	char									*pTableName;
	ShadowTable								*pShadowTable;
	U32										tableMask;
	ShadowTable::SHADOW_LISTEN_CALLBACK		pRowInsertedCallback;
	ShadowTable::SHADOW_LISTEN_CALLBACK		pRowDeletedCallback;
	ShadowTable::SHADOW_LISTEN_CALLBACK		pRowModifiedCallback;
	LunMapManager::CREATE_OBJECTS_FROM_ROW	pCreateObjectsFromRow;
	U32										rowSize;
	void									*pTempTable;
	fieldDef								*pFieldDef;
	U32										fieldDefSize;
};


enum LM_STRING_TYPE { LM_NAME = 1, LM_DESCRIPTION	};

struct LM_READ_STRING_CELL{

	LunMapManager					*pThis;
	UnicodeString					string;
	DesignatorId					entryId;
	LunMapManager::LM_STRING_TYPE	stringType;
};


struct LM_WRITE_STRING_CELL{
	LunMapManager					*pThis;
	UnicodeString					string;
	RowId							ridString;
	DesignatorId					entryId;
	SsapiResponder					*pResponder;
};


struct LM_INSERT_STRING_CELL{
	LunMapManager					*pThis;
	RowId							ridName;
	RowId							ridDescription;
	ValueSet						requestValues;
	SsapiResponder					*pResponder;
	U32								outstandingRequests;
	DesignatorId					entryId;
	bool							hasErrorOccured;	// while outstanding requests complete
};


struct LM_INSERT_USER_INFO_ROW_CELL {
	SsapiResponder					*pResponder;
	ValueSet						objectValues;
	RowId							ridUserInfo;
	RowId							ridName;
	RowId							ridDescription;
};

#endif // __LUN_MAP_MANAGER_H__