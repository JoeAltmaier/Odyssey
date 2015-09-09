//******************************************************************************
// FILE:		ProcessManager.h
//
// PURPOSE:		Defines a manager-type object that will me managing processes
//				running in the O2K box
//******************************************************************************

#ifndef __PROCESS_MANAGER_H__
#define	__PROCESS_MANAGER_H__

#include "ObjectManager.h"
#include "RaidUtilTable.h"
#include "ArrayDescriptor.h"
#include "DescriptorCollector.h"
#include "ShadowTable.h"
#include "CmdSender.h"
#include "Raid2SsapiErrorConvertor.h"

class Container;
class ProcessRaidUtility;
struct SSAPI_PROCESS_MGR_JUMP_TABLE_RECORD;

#ifdef WIN32
#pragma pack(4)
#endif


#define		SSAPI_PM_ARRAY_TABLE				0x00000001
#define		SSAPI_PM_UTIL_TABLE					0x00000002
#define		SSAPI_PM_ALL_TABLES_BUILT			(	SSAPI_PM_ARRAY_TABLE |\
													SSAPI_PM_UTIL_TABLE )

#define		SSAPI_PM_NUMBER_OF_TABLES_USED		2

#define		PROCESS_MANAGER_NAME				"ProcessManager"

class ProcessManager : public ObjectManager {

	bool									m_shouldRebuildAllTables; // init time only
	CmdSender								*m_pRaidCommandQ;
	DescriptorCollector						m_descriptorCollector;
	SSAPI_PROCESS_MGR_JUMP_TABLE_RECORD		*m_pTable;
	U32										m_builtTablesMask;
	bool									m_isIniting;		// iniialization-time only
	U32										m_tablesToRebuild;	// iniialization-time only
	U32										m_outstandingRequests;
	Raid2SsapiErrorConvertor				m_errorConvertor;
	static ProcessManager					*m_pThis;

	friend class ProcessRaidUtility;


//******************************************************************************
// ProcessManager:
//
// PURPOSE:		Default constructor
//******************************************************************************

ProcessManager( ListenManager	*pListenManager, DdmServices *pParent );


public:


//******************************************************************************
// ~ProcessManager:
//
// PURPOSE:		The destructor
//******************************************************************************

~ProcessManager();


//************************************************************************
// GetName:
//
// PURPOSE:		Returns the name of the manager
//************************************************************************

virtual StringClass GetName() { return PROCESS_MANAGER_NAME; }


//************************************************************************
// Ctor:
//
// PURPOSE:		Creates the manager
//************************************************************************

static ObjectManager* Ctor(	ListenManager			*pLManager, 
							DdmServices				*pParent, 
							StringResourceManager	*pSRManager ){

	return m_pThis? m_pThis : m_pThis = new ProcessManager( pLManager, pParent );
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
// AddObject:
//
// PURPOSE:		Adds an object to the system
//
// NOTE:		Must be overridden by object managers that can add objects
//************************************************************************

virtual bool AddObject( ValueSet &objectValues, SsapiResponder *pResponder );


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
// PTS Callbacks for the ArrayDescriptor
//******************************************************************************

STATUS ArrayTableRowInsertedCallback( void *pRows, U32 numberOfRows, ShadowTable*);
STATUS ArrayTableRowModifiedCallback( void *pRows, U32 numberOfRows, ShadowTable*);
STATUS ArrayTableRowDeletedCallback( void *pRows, U32 numberOfRows, ShadowTable*);


//******************************************************************************
// PTS callbacks (one for all!)
//******************************************************************************

STATUS UtilTableModified( void*, U32, ShadowTable* );


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


//************************************************************************
// RaidEventHandler:
//
// PURPOSE:		CAlled by the Raid command Q on command completetion and
//				to report events
//************************************************************************

void RaidEventHandler( STATUS comletionCode, void *pStatusData );


//******************************************************************************
// CreateRaidUtils:
//
// PURPOSE:		Creates RAID util objects and stores them in the 'container'
//
// NOTE:		if pDescr == NULL, data will be fetched from temp table data,
//				else, the descriptor must be in collector
//******************************************************************************

void CreateRaidUtils( Container &container, RAID_ARRAY_UTILITY *pDescr, U32 count );


//******************************************************************************
// StartRaidUtility:
//
// PURPOSE:		Starts a Raid utility
//******************************************************************************

bool StartRaidUtility( ValueSet &utilValues, SsapiResponder *pResponder );


//******************************************************************************
// RaidCommandCompletionReply:
//
// PURPOSE:		Gets all command completetion replies from the 
//				Raid Master
//******************************************************************************

void RaidCommandCompletionReply(	STATUS				completionCode,
									void				*pStatusData,
									void				*pCmdData,
									void				*pCmdContext );


//******************************************************************************
// GetRaidUtility:
//
// PURPOSE:		Looks up a raid util by rid in the raid util descriptor
//******************************************************************************

ProcessRaidUtility* GetRaidUtility( RowId rid );


//******************************************************************************
//******************************************************************************
//******************************************************************************
//******************************************************************************
//******************************************************************************
//******************************************************************************


};


struct SSAPI_PROCESS_MGR_JUMP_TABLE_RECORD{
	char									*pTableName;
	ShadowTable								*pShadowTable;
	U32										tableMask;
	ShadowTable::SHADOW_LISTEN_CALLBACK		pRowInsertedCallback;
	ShadowTable::SHADOW_LISTEN_CALLBACK		pRowDeletedCallback;
	ShadowTable::SHADOW_LISTEN_CALLBACK		pRowModifiedCallback;
	U32										rowSize;
	void									*pTempTable;
	fieldDef								*pFieldDef;
	U32										fieldDefSize;
};


#endif // __PROCESS_MANAGER_H__