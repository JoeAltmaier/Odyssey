//************************************************************************
// FILE:		ConnectionManager.h
//
// PURPOSE:		Defines the ConnectionManager object which will be responsible
//				for connection and data path management in the O2K
//				products.
//************************************************************************

#ifndef __CONNECTION_MANAGER_H__
#define __CONNECTION_MANAGER_H__

//object managers
#include "ObjectManager.h"

// table defs
#include "HostConnectionDescriptorTable.h"
#include "FCPortDatabaseTable.h"


// other includes
#include "ShadowTable.h"

// forward refs
struct SSAPI_CONN_MGR_JUMP_TABLE_RECORD;
struct ADD_PATH_STRINGS_CELL;
class Container;
class StringResourceManager;
class UpstreamConnection;
class DdmSSAPI;
class DataPath;
class ConnectionBase;


#ifdef WIN32
#pragma pack(4)
#endif


// tables used by this mgr
#define	CM_PATH_TABLE						0x00000001
#define	CM_FC_DB_TABLE						0x00000002
#define	CM_ALL_TABLES						(	CM_PATH_TABLE |\
												CM_FC_DB_TABLE \
											)
#define	SSAPI_CM_NUMBER_OF_TABLES_USED		2



#define	CONNECTION_MANAGER_NAME					"ConnectionManager"


class ConnectionManager : public ObjectManager {

	static ConnectionManager			*m_pThis;
	SSAPI_CONN_MGR_JUMP_TABLE_RECORD	*m_pTable;
	U32									m_builtTablesMask;	// inialization-time only
	bool								m_isIniting;		// inialization-time only
	U32									m_tablesToRebuild;	// inialization-time only
	RowId								m_tempRowId;
	U32									m_requestsOutstanding;
	SsapiLocalResponder					*m_pObjAddedResponder;
	SsapiLocalResponder					*m_pObjModifiedResponder;
	StringResourceManager				*m_pStringResourceManager;
	U32									m_updatesForHostConnPending;	// to ingore redundant updates
	bool								m_isAnotherStringUpdatePending;
	ADD_PATH_STRINGS_CELL				*m_pAddPathStringsCell;

//************************************************************************
// ConnectionManager:
//
// PURPOSE:		Default constructor
//************************************************************************

ConnectionManager( ListenManager *pListenManager, DdmServices *pParent, StringResourceManager *pSRM );


public:

	friend	class DataPath;
	friend  class RedundantDataPath;	
	friend  class ClusteredDataPath;
	friend	class ConnectionBase;
	friend  class Host;

	typedef	(ConnectionManager::*CREATE_OBJECTS_FROM_ROW)( void *pRow, Container &container, bool a = false );


//************************************************************************
// ~ConnectionManager:
//
// PURPOSE:		The destructor
//************************************************************************

~ConnectionManager();


//************************************************************************
// GetName:
//
// PURPOSE:		Returns the name of the manager
//************************************************************************

virtual StringClass GetName() { return StringClass(CONNECTION_MANAGER_NAME); }


//************************************************************************
// Ctor:
//
// PURPOSE:		Creates the manager
//************************************************************************

static ObjectManager* Ctor(	ListenManager			*pLManager, 
							DdmServices				*pParent, 
							StringResourceManager	*pSRManager ){

	return m_pThis? m_pThis : m_pThis = new ConnectionManager( pLManager, pParent, pSRManager );
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
//************************************************************************

protected:

//************************************************************************
// ObjectDeletedCallbackHandler:
//
// PURPOSE:		This method may be provided by subclasses to be notified
//				by events from other object managers. 
//************************************************************************

virtual void ObjectDeletedCallbackHandler( SSAPIEvent *pEvent , bool isLast );


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
// AddDataPath:
//
// PURPOSE:		Attempts to add a new data path object
//************************************************************************

bool AddDataPath( ValueSet *pValueSet, SsapiResponder *pResponder );



//************************************************************************
// ModifyPathStrings:
//************************************************************************

bool ModifyPathStrings( ValueSet &requestParms, SsapiResponder* );
STATUS ModifyPathStringsCallback( void *pContext, STATUS rc );


//************************************************************************
// DeleteDataPath:
//
// PURPOSE:		Attempts to delete a data path object
//************************************************************************

bool DeleteDataPath( DesignatorId &id, SsapiResponder *pResponder );



//************************************************************************
// AddObject:
//
// PURPOSE:		Adds an object to the system
//************************************************************************

virtual bool AddObject( ValueSet &objectValues, SsapiResponder *pResponder );


//************************************************************************
// ObjectAddedDeletedModifiedReplyCallback:
//
// PURPOSE:		Responsible for handling the callback and replying to UI
//************************************************************************

STATUS ObjectAddedDeletedModifiedReplyCallback( void *pContext, STATUS rc );



//************************************************************************
// Table callbacks for the connection table
//************************************************************************

STATUS PathTableRowInserted( void *pContext, U32 numberOfRows, ShadowTable *);
STATUS PathTableRowDeleted( void *pContext, U32 numberOfRows, ShadowTable* );
STATUS PathTableRowModified( void *pContext, U32 numberOfRows, ShadowTable *);


//************************************************************************
// Table callbacks for the FC DataBase table table
//************************************************************************

STATUS FcDbTableRowInserted( void *pContext, U32 numberOfRows, ShadowTable *);
STATUS FcDbTableRowDeleted( void *pContext, U32 numberOfRows, ShadowTable* );
STATUS FcDbTableRowModified( void *pContext, U32 numberOfRows, ShadowTable *);


//************************************************************************
// CreateObjects methods
//
// PURPOSE:		Create management objects off the row. Puts pointers
//				to them into the conatiner.
//************************************************************************

bool CreateNoObjects( void *, Container &, bool a = false )	{ return true; }
bool CreateObjectsFromPathRow( HostConnectionDescriptorRecord *pRow, Container &container, bool autoAdd = false );
bool CreateObjectsFromFcDbRow( FCPortDatabaseRecord *pRow, Container &container, bool autoAdd = false );


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
// ReconcileObjectStates:
//
// PURPOSE:		Reconciles states of DataPaths
//************************************************************************

void ReconcileObjectStates( Container *pObjectsToReconcile = NULL );


//************************************************************************
// GetDataPathByConnId:
//
// PURPOSE:		Performs a look up for a data path that is bound
//				to the Connection with the designator id specified
//************************************************************************

DataPath* GetDataPathByConnId( DesignatorId connId );


//************************************************************************
// ConnectionReadNameCallback:
//
// PURPOSE:		Callback for ReadString
//************************************************************************

STATUS ConnectionReadNameCallback( void *pContext, STATUS rc );


//************************************************************************
// ModifyConnectionName:
//
// PURPOSE:		Modifies the name of the Connection object
//************************************************************************

void ModifyConnectionName( ConnectionBase *pConn, UnicodeString newName, SsapiResponder *pResponder );


//************************************************************************
// AddDeleteConnectionNameCallback:
//
// PURPOSE:		Called when connection name is deleted or added
//				if pContext == NULL --> delete
//				otherwise --> add
//************************************************************************

STATUS AddDeleteConnectionNameCallback( void *pContext, STATUS rc );


//************************************************************************
// ConnNameFieldModifiedCallback:
//
// PURPOSE:		Called when 'ridName' field on the FC DB descriptor is 
//				modified
//************************************************************************

STATUS ConnNameFieldModifiedCallback( void *pContext, STATUS rc );


//************************************************************************
// ReadPathStringCallback:
//
// PURPOSE:		Called when a string for a path is read
//************************************************************************

STATUS ReadPathStringCallback( void *pContext, STATUS rc );


//************************************************************************
// DoesThisPathExist:
//************************************************************************

bool DoesThisPathExist( ValueSet &requestParms, SsapiResponder *pResponder );


//************************************************************************
// 
//************************************************************************

STATUS DoNothing( void*, STATUS ){ return OK; }
void Dummy( ValueSet *, bool , int  ){}


//************************************************************************
// PurgeConnIdsFromPathRow:
//
// PURPOSE:		Purges connection ids from a host connection descriptor
//************************************************************************

void PurgeConnIdsFromPathRow( HostConnectionDescriptorRecord *pRow, Container &positions );
STATUS PurgeConnIdsFromPathRowCallback( void *pContext, STATUS rc );


//************************************************************************
// AddConnIdToDataPath:
//
// PURPOSE:		Adds an Id of an Connection to a data path
//************************************************************************

void AddConnIdToDataPath( DataPath *pPath, UpstreamConnection *, SsapiResponder *pResponder, U32 flags );


//************************************************************************
// RemoveConnIdFromDataPath:
//
// PURPOSE:		Removes Id of an connection from a data path
//************************************************************************

void RemoveConnIdFromDataPath( DataPath *pPath, DesignatorId connId, SsapiResponder *pResponder );


//************************************************************************
// DeleteConnection:
//
// PURPOSE:		Deletes the connection object from the PTS table and the
//				SSAPI sub-system
//************************************************************************

void DeleteConnection( ConnectionBase *pConn, SsapiResponder *pResponder );
STATUS DeleteConnectionCallback( void *pContext, STATUS rc );


//************************************************************************
// GetDdmSSAPI
//
// PURPOSE:		A handy routine
//************************************************************************

DdmSSAPI* GetDdmSSAPI() { return (DdmSSAPI *)pParentDdmSvs; }


//************************************************************************
// GetAffectedManagedObjects:
//
// PURPOSE:		Performs a look up of the affected objects by any change
//				to the MO with 'id' specified. Puts pointers to affected
//				objects into the container.
//
// RETURN:		number of objects affected
//************************************************************************

U32 GetAffectedManagedObjects( DesignatorId id, Container &affectedObjects );


//************************************************************************
// SetConnectionIds:
//
// PURPOSE:		Associates a vector of connection with a data path.
//				This method is provided to handle multiple updates
//				as an atomic operation instead of using a series of
//				add/remove methods
//************************************************************************

void SetConnectionIds( DataPath *pPath, ValueSet *pVsIds, SsapiResponder *pResponder );


//************************************************************************
// AddRemoveConnIdCallback:
//************************************************************************

STATUS AddRemoveConnIdCallback( void *pContext, STATUS rc );

};


struct SSAPI_CONN_MGR_JUMP_TABLE_RECORD{
	char										*pTableName;
	ShadowTable									*pShadowTable;
	U32											tableMask;
	ShadowTable::SHADOW_LISTEN_CALLBACK			pRowInsertedCallback;
	ShadowTable::SHADOW_LISTEN_CALLBACK			pRowDeletedCallback;
	ShadowTable::SHADOW_LISTEN_CALLBACK			pRowModifiedCallback;
	ConnectionManager::CREATE_OBJECTS_FROM_ROW	pCreateObjectsFromRow;
	U32											rowSize;
	void										*pTempTable;
	fieldDef									*pFieldDef;
	U32											fieldDefSize;
};


#define STRING_OP_CONTEXT_PATH_NAME			1
#define STRING_OP_CONTEXT_PATH_DESCRIPTION	2
struct READ_STRING_CELL{
	UnicodeString						*pName;
	DesignatorId						id;
	U32									context;
	ConnectionManager					*pThis;

	READ_STRING_CELL(ConnectionManager *p){ pThis = p; }
};

struct WRITE_STRING_CELL{
	DesignatorId						id;
	U32									context;
	RowId								*pRid;
	ConnectionManager					*pThis;
	UnicodeString						string;
	WRITE_STRING_CELL(ConnectionManager *p) {pThis=p;}
};


struct ADD_DELETE_PATH_NAME_CELL{
	RowId								rid;
	DesignatorId						connId;	
	ConnectionManager					*pThis;

	ADD_DELETE_PATH_NAME_CELL( ConnectionManager *p ) {pThis =p;}
};

struct ADD_PATH_STRINGS_CELL{
	UnicodeString						name;
	UnicodeString						description;
	DesignatorId						id;
};

#endif // __CONNECTION_MANAGER_H__