//************************************************************************
// FILE:		ShadowTable.h
//
// PURPOSE:		Defines a very cute object that wraps up a PTS table 
//				and reduces the number of PTS listeners we
//				will need and makes PTS interface easier. Essentially,
//				three listeners are registered for all tables:
//					-- Add any row
//					-- Delete any row
//					-- Modify any row
//
// NOTES:		1. Once the table object is instantiated, it must be either 
//				   Initialize()'d or Create()'d so that it has all the infor 
//				   about the table it will be working closely with.
//************************************************************************

#ifndef __SHADOW_TABLE_H__
#define __SHADOW_TABLE_H__

#ifdef WIN32
#pragma pack(4)
#endif

#include "StringClass.h"
#include "Container.h"
#include "DdmOsServices.h"
#include "PtsCommon.h"
#include "RowId.h"
#include "ReadTable.h"
#include "Listen.h"

#ifdef WIN32
#include <stdio.h>
#define Tracef	printf
#endif


class Message;





class ShadowTable : DdmServices {

public:
	typedef			STATUS (DdmServices::*SHADOW_LISTEN_CALLBACK)( void *pRowDataAffected, U32 numberOfRowsAffected, ShadowTable *pThis );


private:
	
	bool					m_isInited;				// shows if this object's been inited
	bool					m_areListenersInited;	// shows if the listeners have been registered
	StringClass				m_tableName;			// name of the table the object is associated
	U32						m_listener;				// id of listeners
	SHADOW_LISTEN_CALLBACK	m_insertRowCallback;	// method to call when insert row happens
	SHADOW_LISTEN_CALLBACK	m_deleteRowCallback;	// method to call when delete row happens
	SHADOW_LISTEN_CALLBACK	m_modifyRowCallback;	// method to call when modify row happens
	DdmServices				*m_pParentDdm;			// the DDM on which this object resides
	U32						m_sizeOfTableDataRet;	// size of table data returned with a listener reply
	void*					m_pTableDataRet;		// returned table data.
	U32*					m_pListenTypeRet;

	U32						m_numberOfRows;			// number of rows in the table
	U32						m_numberOfCols;			// number of cols in the table
	U32						m_bytesPerRow;
	U32						m_sizeOfModifiedRecord;
	void					*m_pModifiedRecord;		// for listeners to stuff
	RowId					m_tableRowId;			// row ID for this table
	TSDefineTable			*m_pDefineTableObject;
	fieldDef				*m_pFieldDef;

	rowID					m_tempRowId;
	TSListen				*m_pTableListenerObj;	// to be able to stop listenning
	

	enum STATE{								// used for context
		INIT_LISTENERS	= 1,
		DEFINE_TABLE,
		INSERT_ROW,
		DELETE_ROW,
		MODIFY_ROW,
		READ_ROW,
		READ_TABLE,
		READ_TABLE_DEF
	};
	
	
public:

	
	
	typedef struct{
		STATE			state;
		void			*pCallersContext;
		pTSCallback_t	pCallback;
		U32				*pNumOfCols;		// for GetTableDef
		U32				*pBytesPerRow;		// for GetTableDef
		U32				*pNumOfRows;		// for GetTableDef
		void			*pRow;				// for InsertRow, ModifyRow
		rowID			*pRowId;			// for DeleteRow, ModifyRow, ModifyField
		void			*pTableData;		// for Enumerate
		void			*pNewValue;			// for ModifyFiled		
		U32				*pFieldDefSize;
		void			*pFieldDef;
	} CONTEXT;	

//************************************************************************
// ShadowTable:
// 
// PURPOSE:		Default destructor
//
// RECEIVE:		tableName:			name of the table
//				pParentDdm:			ptr to the DDM that owns this object
//				insertRowCallback:	method to call when a row is inserted
//				deleteRowCallback:	method to call when a row is deleted
//				modifyRowCallback:	method to call when a row is modified
//************************************************************************

ShadowTable( StringClass			tableName, 
			 DdmServices 			*pParentDdm, 
			 SHADOW_LISTEN_CALLBACK	insertRowCallback, 
			 SHADOW_LISTEN_CALLBACK	deleteRowCallback,
			 SHADOW_LISTEN_CALLBACK	modifyRowCallback,
			 U32					bytesPerRow);


//************************************************************************
// ~ShadowTable
//
// PURPOSE:		Destructor
//************************************************************************

~ShadowTable();


//************************************************************************
// Initialize:
//
// PURPOSE:		Initializes  the object. MUST be called (or Create() )
//				A call to the callback will signal completion. 
//				Check the "rc" for OK or !OK as !OK will indicate that 
//				no operation on the table will be conducted
//************************************************************************

STATUS Initialize( pTSCallback_t pCallback, void *pContext );


//************************************************************************
// DefineTable:
//
// PURPOSE:		Issues a request to define a table
//
// RECEIVE:		pDef:				field defintion of the table
//				sizeOfDef:			size of the defintion
//				entriesToReserve:	number of entries to allocate in the table
//				pCallback:			routine to call back to
//				pContext:			for caller's context
//************************************************************************

STATUS DefineTable( fieldDef *pDef, U32 sizeOfDef, U32 entriesToReserve,
					pTSCallback_t pCallback, void *pContext );


//************************************************************************
// InsertRow:
//
// PURPOSE:		Inserts a row into the table
//
// RECEIVE:		pRow:		ptr to the buffer representing a row
//				pRowId:		ptr of the cell where rowId will be put
//
//************************************************************************

STATUS InsertRow( void *pRow, rowID *pRowId, pTSCallback_t pCallback, void *pContext );


//************************************************************************
// DeleteRow:
//
// PURPOSE:		Deletes row from the table. 
//
// RECEIVE:		rowId:	row id of the row to be deleted
//************************************************************************

STATUS DeleteRow( rowID rowId, pTSCallback_t pCallback, void *pContext);


//************************************************************************
// Enumerate:
//
// PURPOSE:		Reads the table into buffer specified
//
// RECEIVE:		pTableData:	address of the pointer that will point to
//							the memory with table data.
//
// NOTE:		the caller must delete 'ppTableData' !!!
//************************************************************************

STATUS Enumerate( void **ppTableData, pTSCallback_t pCallback, void *pContext );


//************************************************************************
// ModifyRow:
//
// PURPOSE:		Modifes a row.
//
// RECEIVE:		rowId:		the row id of the row to modify
//				pRow:		ptr to memory that contains new row data 
//************************************************************************

STATUS ModifyRow( rowID rowId, void* pRow,  pTSCallback_t pCallback, void *pContext );


//************************************************************************
// ReadRow:
//
// PURPOSE:		Read a row
//
// RECEIVE:		rowId:		the id of the row to read
//				ppRow:		address of the pointer to a read row. allocated 
//							by the callee
//	
// NOTE:		ppRow must be delete()d after the callback returns.	
//************************************************************************

STATUS ReadRow( rowID rowId, void **ppRow, pTSCallback_t pCallback, void *pContext );


//************************************************************************
// ModifyField:
//
// PURPOSE:		Modifies a field in a row
//************************************************************************

STATUS ModifyField(	RowId			rid, 
					char			*pFieldName, 
					void			*pNewValue,
					U32				newValueSize,
					pTSCallback_t	pCallback, 
					void			*pContext );


//************************************************************************
// DeleteTable:
//
// PURPOSE:		Deletes the table from the PTS
//************************************************************************

void DeleteTable( pTSCallback_t pCallback, void *pContext );


//************************************************************************
// GetNumberOfRows:
//
// PURPOSE:		An accessor
//
// RETURN:		# of rows in the table
//************************************************************************

U32 GetNumberOfRows() const { return m_numberOfRows; }


//************************************************************************
// GetNumberOfCols:
//
// PURPOSE:		An accessor
//
// RETURN:		# of columns in the table
//************************************************************************

U32 GetNumberOfCols() const { return m_numberOfCols; }


//************************************************************************
// Accessors:
//
//************************************************************************

StringClass GetName() const			{ return m_tableName; }
RowId		GetTableRowId() const	{ return m_tableRowId; }
fieldDef*	GetFieldDefArray() const{ return m_pFieldDef; }
U32			GetBytesPerRow() const	{ return m_bytesPerRow; }

private:


//************************************************************************
// InitializeReplyHandler:
//
// PURPOSE:		Responsible for handling reply from Initialize()
//************************************************************************

STATUS InitializeReplyHandler( void *pContext, STATUS rc );


//************************************************************************
// ShadowTable:
//
// PURPOSE:		Copy constructor. Declared private to disallow cloning
//************************************************************************

ShadowTable( const ShadowTable& ){}


//************************************************************************
// operator=:
//
// PURPOSE:		Overload to disallow cloning
//************************************************************************

const ShadowTable& operator= ( const ShadowTable& original ){ return original; }


//************************************************************************
// Trace:
//
// PURPOSE:		Overloaded methods for debug purposes. Levels go 1 - 3
//				and are defined below
//************************************************************************



#define	ST_CRITICAL	1
#define	ST_WARNING	2
#define	ST_INFO		3

void Trace( StringClass s, U8 level = ST_INFO ){
	char*	p = s.CString(),*n = m_tableName.CString();
	
	//if( level != ST_INFO )
	//	Tracef( "\n%s:\t%s", n, p );
	delete p;
	delete n;
}

void Trace( U32 u32, U8 level = ST_INFO ){
	StringClass s = u32;
	Trace( s );
}

void Trace( bool b, U8 level = ST_INFO ){
	if( b )
		Trace( StringClass( "true" ) );
	else
		Trace( StringClass("false" ) );
}


//************************************************************************
// PopulateAddListenerObject:
//
// PURPOSE:		Fills up a PTS add listener object by calling its
//				Initialize()
//
// RECEIVE:		pListener:		ptr to the object to populate
//				listenerType:	type as defined in the PTS API
//				pListenerId:	ptr to the cell where listener id should 
//								be put
//************************************************************************

STATUS PopulateAddListenerObject(	TSListen	*pListener,		U32 listenerType, 
									U32			*pListenerId,	void *pContext,
									pTSCallback_t pCallback = (pTSCallback_t)0	); 
	
	
//************************************************************************
// DefineTableReplyHandler:
//
// PURPOSE:		Responsible for the handling if reply to define a table
//************************************************************************

STATUS DefineTableReplyHandler( void *pContext, STATUS rc );


//************************************************************************
// InsertRowReplyHandler:
//
// PURPOSE:		Responsible for handling replies to InsertRow operation
//************************************************************************

STATUS InsertRowReplyHandler( void *pContext, STATUS rc );


//************************************************************************
// DeleteRowReplyHandler:
//
// PURPOSE:		Responsible for handling reply to DeleteRow cmd
//************************************************************************

STATUS DeleteRowReplyHandler( void *pContext, STATUS rc );


//************************************************************************
// EnumerateReplyHandler:
//
// PURPOSE:		REsponsible for handling the reply to Enumerate()
//************************************************************************

STATUS EnumerateReplyHandler( void *pContext, STATUS rc );


//************************************************************************
// ModifyRowReplyHandler:
//
// PURPOSE:		Responsible for handling replies to ModifyRow()
//************************************************************************

STATUS ModifyRowReplyHandler( void *pContext, STATUS rc );


//************************************************************************
// ReadRowReplyHandler:
//
// PURPOSE:		Responsible for handling replies to ReadRow()
//************************************************************************

STATUS ReadRowReplyHandler( void *pContext, STATUS rc );


//************************************************************************
// ModifyFieldReplyHandler:
//
// PURPOSE:		REsponsible for handlinmg the PTS reply and responding 
//				to user
//************************************************************************

STATUS ModifyFieldReplyHandler( void *pContext, STATUS rc  );


//************************************************************************
// DeleteTableReplyHandler:
//
// PURPOSE:		Responsible for handling a callback to the DeleteTable()
//************************************************************************

STATUS DeleteTableReplyHandler( void *pContext, STATUS rc );


//************************************************************************
// LogFailure:
//
// PURPOSE:		Logs a failure in an operation as an internal event
//************************************************************************

void LogFailure( StringClass opName, STATUS rc );

};


#endif // __SHADOW_TABLE_H__

