//************************************************************************
// FILE:		HostManager.h
//
// PURPOSE:		Defines a class whose instance will be managing Host
//				objects in the O2K product.
//************************************************************************

#ifndef __HOST_MANAGER_H__
#define	__HOST_MANAGER_H__


#include "ObjectManager.h"
#include "Host.h"
#include "DesignatorIdVector.h"
#include "ShadowTable.h"
#include "HostDescriptorTable.h"
#include "SsapiLocalResponder.h"


class Container;
class StringResourceManager;
class DdmSSAPI;
struct SSAPI_HOST_MGR_JUMP_TABLE_RECORD;

#ifdef WIN32
#pragma pack(4)
#endif


#define	HM_HOST_TABLE					0x00000001

#define	HM_ALL_TABLES					(	HM_HOST_TABLE	)

#define	SSAPI_HM_NUMBER_OF_TABLES_USED		1

#define	HOST_MANAGER_NAME					"HostManager"


class HostManager : public ObjectManager {


	SSAPI_HOST_MGR_JUMP_TABLE_RECORD	*m_pTable;
	U32									m_builtTablesMask;	// inialization-time only
	bool								m_isIniting;		// inialization-time only
	U32									m_tablesToRebuild;	// inialization-time only
	RowId								m_tempRowId;
	U32									m_requestsOutstanding;
	SsapiLocalResponder					*m_pObjAddedResponder;
	SsapiLocalResponder					*m_pObjModifiedResponder;
	StringResourceManager				*m_pStringResourceManager;
	static HostManager					*m_pThis;
	bool								m_isModifyingConnOnHost;


//************************************************************************
// HostManager:
//
// PURPOSE:		The default constructor
//************************************************************************

HostManager( ListenManager *pListenManager, DdmServices *pParent, StringResourceManager *pSRM );


public:
	
	friend	class Host;					
	typedef	(HostManager::*CREATE_OBJECTS_FROM_ROW)( void *pRow, Container &container, bool a = false );

//************************************************************************
// ~HostManager:
//
// PURPOSE:		The destructor
//************************************************************************

~HostManager();


//************************************************************************
// GetName:
//
// PURPOSE:		Returns the name of the manager
//************************************************************************

virtual StringClass GetName() { return StringClass(HOST_MANAGER_NAME); }


//************************************************************************
// Ctor:
//
// PURPOSE:		Creates the manager
//************************************************************************

static ObjectManager* Ctor(	ListenManager			*pLManager, 
							DdmServices				*pParent, 
							StringResourceManager	*pSRManager ){

	return m_pThis? m_pThis : m_pThis = new HostManager( pLManager, pParent, pSRManager );
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
// IsAvailableConnection:
//
// PURPOSE:		Checks if this connection is already owned by a host.
//************************************************************************

bool IsAvailableConnection( const DesignatorId &id );


//************************************************************************
// DoConnectionsBelongToSameHost:
//
// PURPOSE:		Checks if all the ids specified in the container are 
//				assigned to the same host
//
// NOTE:		The container contains ptrs to DesignatorId objects.
//				This method DOES NOT do any memory clean up.
//************************************************************************

bool DoConnectionsBelongToSameHost( Container &connectionIds );


//************************************************************************
// GetHostIdByConnectionId:
//
// PURPOSE:		Looks up a host to which the connection is assigned.
//
// OUTPUT:		on success, hostId contains the id of the host object
//
// RETURN:		success:	true
//				failure:	false
//************************************************************************

bool GetHostIdByConnectionId( const DesignatorId &connId, DesignatorId &hostId );


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
// AddHost:
//
// PURPOSE:		Attempts to add a new Host object
//************************************************************************

bool AddHost( ValueSet &objectValues, SsapiResponder *pResponder );


//************************************************************************
// DeleteHost:
//
// PURPOSE:		Deletes the host
//************************************************************************

bool DeleteHost( DesignatorId id, SsapiResponder *pResponder );


//************************************************************************
// ModifyHost: 
//
// PURPOSE:		Modifies host data. The value set may contain an incomplete
//				object, but the ID must be there!!!
//************************************************************************

bool ModifyHost( ValueSet &objectValues, SsapiResponder *pResponder );


//************************************************************************
// AddObject:
//
// PURPOSE:		Adds an object to the system
//
// NOTE:		Override default implementation by calling to AddHost()
//************************************************************************

virtual bool AddObject( ValueSet &objectValues, SsapiResponder *pResponder );


//************************************************************************
// ObjectAddedDeletedModifiedReplyCallback:
//
// PURPOSE:		Responsible for handling the callback and replying to UI
//************************************************************************

STATUS ObjectAddedDeletedModifiedReplyCallback( void *pContext, STATUS rc );


//************************************************************************
// Table callbacks for the HostDescriptorTable
//************************************************************************

STATUS HostTableRowInserted( void *pContext, U32 numberOfRows, ShadowTable *);
STATUS HostTableRowDeleted( void *pContext, U32 numberOfRows, ShadowTable* );
STATUS HostTableRowModified( void *pContext, U32 numberOfRows, ShadowTable *);


//************************************************************************
// CreateObjects methods
//
// PURPOSE:		Create management objects off the row. Puts pointers
//				to them into the conatiner.
//************************************************************************

bool CreateNoObjects( void *, Container &, bool a = false )	{ return true; }
bool CreateObjectsFromHostRow( HostDescriptorRecord *pRow, Container &container, bool autoAdd = false );


//************************************************************************
// GetShadowTable:
//
// PURPOSE:		Performs a look up of the table by the table mask
//************************************************************************

ShadowTable* GetShadowTable( U32 tableMask );


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
//************************************************************************

void AddOutstandingReq(){ m_requestsOutstanding++; SetIsReadyToServiceRequests( false ); }
void RemoveOutstandingReq();


//************************************************************************
// 
//************************************************************************

STATUS DoNothing( void*, STATUS ){ return OK; }


//************************************************************************
// PurgeConnIdFromHostRow:
//
// PURPOSE:		Purges ids from a host descriptor
//************************************************************************

void PurgeConnIdFromHostRow( HostDescriptorRecord *pRow, Container &positions );
STATUS PurgeConnIdFromHostRowCallback( void *pContext, STATUS rc );


//************************************************************************
// AddConnIdToHost:
//
// PURPOSE:		Adds an Id of an Connection to host
//************************************************************************

void AddConnIdToHost( Host *pHost, DesignatorId &connId, SsapiResponder *pResponder );
STATUS AddRemoveConnIdCallback( void *pContext, STATUS rc );


//************************************************************************
// RemoveConnIdFromHost:
//
// PURPOSE:		REmoves an Id of an Connection from host
//************************************************************************

void RemoveConnIdFromHost( Host *pHost, DesignatorId, SsapiResponder *pResponder );


//************************************************************************
// PURPOSE:		For local ssapi responder callbacks. (so we can utilize
//				methods called by external clients internally)
//************************************************************************

void Dummy( ValueSet *, bool , int  ){}


//************************************************************************
// IsValidConnection:
//
// PURPOSE:		Checks with the ConnectionManager if the given ID
//				is endeed an ID of some Connection object
//************************************************************************

bool IsValidConnection( const DesignatorId &id );


//************************************************************************
// GetConnIdByRowId:
//
// PURPOSE:		Queries the ConnectionManager for the id of a Connection
//				object given the RowId.
//
// RETURN:		true:	such object exists and is indeed a Connection object
//				false:	the above condition did not hold for the RowId given
//************************************************************************

bool GetConnIdByRowId( const RowId &rid, DesignatorId &id );


//************************************************************************
// GetHostByConnection:
//
// PURPOSE:		Performs a look up for a host that has the connection
//				specified.
//
// RETURN:		success: ptr to host object
//				failure: NULL (no such host)
//************************************************************************

Host* GetHostByConnection( const DesignatorId &connId );


//************************************************************************
// SetConnectionIds:
//
// PURPOSE:		An atomic operation to set a vector of connection ids at
//				one time instead of using add/remove methods
//************************************************************************

void SetConnectionIds( Host *pHost, ValueSet *pVsIds, SsapiResponder *pResponder );



DdmSSAPI* GetDdmSSAPI() { return (DdmSSAPI *)pParentDdmSvs; }
};


struct SSAPI_HOST_MGR_JUMP_TABLE_RECORD{
	char									*pTableName;
	ShadowTable								*pShadowTable;
	U32										tableMask;
	ShadowTable::SHADOW_LISTEN_CALLBACK		pRowInsertedCallback;
	ShadowTable::SHADOW_LISTEN_CALLBACK		pRowDeletedCallback;
	ShadowTable::SHADOW_LISTEN_CALLBACK		pRowModifiedCallback;
	HostManager::CREATE_OBJECTS_FROM_ROW	pCreateObjectsFromRow;
	U32										rowSize;
	void									*pTempTable;
	fieldDef								*pFieldDef;
	U32										fieldDefSize;
};





#endif // __HOST_MANAGER_H__