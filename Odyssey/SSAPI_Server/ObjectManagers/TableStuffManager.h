//************************************************************************
// FILE:		TableStuffManager.h
//
// PURPOSE:		An object manager that is responsible for the Rambo PnP
//************************************************************************

#ifndef __TABLE_STUFF_MANAGER_H__
#define	__TABLE_STUFF_MANAGER_H__


#include "ObjectManager.h"
#include "PtsCommon.h"
class ShadowTable;
class TableMetaData;


typedef void (DdmServices::*TMGR_CALLBACK)(void*);

class TableStuffManager : public ObjectManager {

	ShadowTable					*m_pTableOfTables;
	bool						m_isIniting;
	bool						m_shouldRebuild;
	bool						m_wasTableUpdatedWhileAdding;
	ShadowTable					*m_pTableBeingAdded;
	void						*m_pTempBuffer;
	U32							m_pTempU32;
	static TableStuffManager	*m_pThis;

	friend class TableStuff;
	friend class TableStuffRow;

//************************************************************************
// 
//************************************************************************

TableStuffManager( ListenManager *pLM, DdmServices *pParent );

public:


//************************************************************************
// Ctor:
//
// PURPOSE:		Creates the manager
//************************************************************************

static ObjectManager* Ctor(	ListenManager			*pLManager, 
							DdmServices				*pParent, 
							StringResourceManager	*pSRManager ){

	return m_pThis? m_pThis : m_pThis = new TableStuffManager( pLManager, pParent );
}


//************************************************************************
// 
//************************************************************************

virtual ~TableStuffManager(); 


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
// GetName:
//
// PURPOSE:		Returns the name of the manager
//************************************************************************

virtual StringClass GetName() { return StringClass("Mr.Rambo-On-Demand"); }


//************************************************************************
//
//************************************************************************

void StuffContainerWithRowObjectsForTable( U16 tableId, Container &container );

protected:


private:

//************************************************************************
// ObjectDeletedCallbackHandler:
//
// PURPOSE:		This method may be provided by subclasses to be notified
//				by events from other object managers. 
//************************************************************************

virtual void ObjectDeletedCallbackHandler( SSAPIEvent *, bool ){}


//************************************************************************
// PTS callback for the PTS's "Table of tables"
//************************************************************************

STATUS TableAdded( void *pRows, U32 numberOfRows, ShadowTable* );
STATUS TableRemoved( void *pRows, U32 numberOfRows, ShadowTable* );
STATUS TableModified( void *pRows, U32 numberOfRows, ShadowTable* );


//************************************************************************
// PTS callback for the ALL tables. 
//
// NOTE:		Use the ptr to a ShadowTable object to determine which
//				table has been modified.
//************************************************************************

STATUS RowAdded( void *pRows, U32 numberOfRows, ShadowTable *pTable );
STATUS RowRemoved( void *pRows, U32 numberOfRows, ShadowTable *pTable );
STATUS RowModified( void *pRows, U32 numberOfRows, ShadowTable *pTable );


//************************************************************************
// 
//************************************************************************

STATUS TableOfTablesInitialized( void *pContext, STATUS rc );
STATUS TableOfTablesEnumerated( void *pContext, STATUS rc );

//************************************************************************
// AddNewTable:
//************************************************************************

void AddNewTable( StringClass tableName, U32 rowSize, TMGR_CALLBACK pCallback, void *pContext );
STATUS NewTableInitialized( void *pContext, STATUS rc );
STATUS NewTableEnumerated( void *pContext, STATUS rc );

//************************************************************************
//
//************************************************************************

void TableAddedWhileInitializing( void *pContext );


//************************************************************************
//
//************************************************************************

void AddTableObject( ShadowTable *pTable  );
void AddTableRowObjects( ShadowTable *pTable, void *pTableData );


//************************************************************************
//
//************************************************************************

TableMetaData* GetTableMetaData();


//************************************************************************
//
//************************************************************************

void DoneAddingNewTable( void *pContext );


//************************************************************************
//
//************************************************************************

void DeleteTableRow( TableStuffRow &obj, SsapiResponder *pResponder );


//************************************************************************
//
//************************************************************************

void ModifyTableRow( TableStuffRow &obj, SsapiResponder *pResponder );


//************************************************************************
//
//************************************************************************

void DeleteTableStuff( TableStuff &obj, SsapiResponder *pResponder );
STATUS DeleteTableStuffCallback( void *pContext, STATUS rc );

//************************************************************************
//************************************************************************

STATUS PtsDefaultCallback( void*, STATUS ){ return OK; }

//************************************************************************
//************************************************************************

void ModifyField( TableStuffRow &row, UnicodeString fieldName, ValueSet &newValue, SsapiResponder * );
STATUS ModifyFieldCallback( void *pContext, STATUS rc );
void* GetValue( fieldDef *pField, ValueSet &vs, int code, SsapiResponder *pResponder, U32 &newValueSize  );

};


struct ADD_NEW_TABLE_CELL{
	TMGR_CALLBACK	pCompletionCallback;
	void			*pContext;
	ShadowTable		*pTable;
	void			*pTempBuffer;
};


struct TABLE_STUFF_HEADER{
	rowID				rid;			
	U32					version;				
	U32					size;
};

#define	GET_ROW_SIZE( pRow ) ((TABLE_STUFF_HEADER *)pRow)->size

#endif // __TABLE_STUFF_MANAGER_H__