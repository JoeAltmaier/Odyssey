//************************************************************************
// FILE:		StringResourceManager.h
//
// PURPOSE:		Defines the object that can intellegently manage string 
//				resources. The object will store strings so that they
//				use up the least amount of space possible.
//************************************************************************

#ifndef __STRING_RESOURCE_MANAGER_H__
#define	__STRING_RESOURCE_MANAGER_H__

#include "DdmOsServices.h"
#include "ShadowTable.h"
#include "StringClass.h"
#include "UnicodeString.h"

struct PTS_TRANSACTION_READ_CONTEXT;

#ifdef WIN32
#pragma pack(4)
#endif

	enum SRM_TABLES{		// must be in sync with TableNames !!!!
					TABLE_16	= 0,
					TABLE_32,
					TABLE_64,
					TABLE_128,
					TABLE_256,

					NUMBER_OF_TABLES };


struct SRM_TABLE_JUMP_TABLE{
	SRM_TABLES		cellId;
	char*			name;
	fieldDef*		tableDef;
	U32				tableDefSize;
	U32				bytesPerRow;
	U32				maxResourceSize;		// in bytes
	U32				tableVersion;
};



typedef	ShadowTable*	P_ShadowTable;

class StringResourceManager : public DdmServices {

	
	P_ShadowTable		*m_pTables;
	bool				m_isInited;
	pTSCallback_t		m_pInitCompleteCallback;



public:

//************************************************************************
// StringResourceManager:
//
// PURPOSE:		Default constructor
//
// RECEIVE:		pParent:				DDM to use for message sending/receiving
//				pObjectReadyCallback:	Method to be called when the manager
//										is ready.
//************************************************************************

StringResourceManager(	DdmServices		*pParent,
						pTSCallback_t	pObjectReadyCallback );


//************************************************************************
// ~StringResourceManager:
//
// PURPOSE:		The destructor
//************************************************************************

~StringResourceManager();


//************************************************************************
// WriteString: (multiple overloads)
//
// PURPOSE:		Inserts a string into the PTS. The table used depends
//				on the string size
//	
// RECEIVE:		string:					the resource in different types
//				pCompletetionCallback	the method called when the operation
//										has completed.
//				*pRid					ptr to location where the row id
//										of the string will be put on completion
//				pContext:				any context
//			
// RETURN:		true:					SEND() was successful
//************************************************************************

bool WriteString(	UnicodeString	string,	
					rowID			*pRid,
					pTSCallback_t	pCompletetionCallback,
					void			*pContext );


bool WriteString(	StringClass		string,	
					rowID			*pRid,
					pTSCallback_t	pCompletetionCallback,
					void			*pContext );


//************************************************************************
// ReadString:	(multiple overloads)
//
// PURPOSE:		Reads a string from a PTS table and stores it in the format
//				requested
//
// RECEIVE:		string:					string resource
//				pCompletionCallback:	method called on completion
//				rid:					row id of the strig resource
//				pContext:				any context				
//************************************************************************

bool ReadString(	UnicodeString	*pString,
					RowId			rid,
					pTSCallback_t	pCompletionCallback,
					void			*pContext );


bool ReadString(	StringClass		*pString,
					RowId			rid,
					pTSCallback_t	pCompletionCallback,
					void			*pContext );


//************************************************************************
// DeleteString:
//
// PURPOSE:		Deletes the string resource identified by the 'rid'
//
// RETURN:		true:		PTS Send() succeeded
//************************************************************************

bool DeleteString(	RowId			rid,
					pTSCallback_t	pCompletionCallback,
					void			*pContext );

private:

//************************************************************************
// DummyCallback:
//
// PURPOSE:		Dummy callback for the ShadowTable objects
//************************************************************************

STATUS DummyCallback( void*, STATUS ){ return OK;}
STATUS DummyCallbackU32( void*, U32, ShadowTable* ){ return OK;}



//************************************************************************
// InitializeTableCallback:
//
// PURPOSE:		Called when a shadow table object is inited
//************************************************************************

STATUS InitializeTableCallback( void *pContext, STATUS status );


//************************************************************************
// WriteDataToTable:
//
// PURPOSE:		Performs the actual write to a table. Selects the right
//				table based on the data size.
//************************************************************************

bool WriteDataToTable(	void			*pData, 
						U32				dataSize,
						rowID			*pRid, 
						pTSCallback_t	pCompletionCallback,
						void			*pContext );


//************************************************************************
// WriteDataToTableReplyCallback:
//
// PURPOSE:		Handles PTS reply and responds to the user
//************************************************************************

STATUS WriteDataToTableReplyCallback( void *pContext, STATUS status );


//************************************************************************
// ReadDataFromTable:
//
// PURPOSE:		Does an actual read from a PTS table. The table to use
//				is determined by the row id of the string resource to read.
//************************************************************************

bool ReadDataFromTable(	PTS_TRANSACTION_READ_CONTEXT *pContext,	RowId rid );


//************************************************************************
// ReadDataFromTableReplyCallback:
//
// PURPOSE:		Handles PTS reply and responds to the user
//************************************************************************

STATUS ReadDataFromTableReplyCallback( void *pContext, STATUS status );


//************************************************************************
// DeleteStringReplyCallback:
//
// PURPOSE:		Handles PTS reply and responds to the user
//************************************************************************

STATUS DeleteStringReplyCallback( void *pContext, STATUS rc );

};


struct PTS_TRANSACTION_WRITE_CONTEXT{
	void			*pContext;
	pTSCallback_t	pCompletionCallback;
};


enum SRM_DATA_TYPE{
	SRM_UNICODE_STRING	= 0,
	SRM_ASCII_STRING,

	SRM_NUMBER_OF_DATA_TYPES
};

struct PTS_TRANSACTION_READ_CONTEXT{
	void			*pContext;
	pTSCallback_t	pCompletionCallback;
	void			*pStringObj;
	SRM_DATA_TYPE	dataType;
	void			*pRowData;
};


struct PTS_TRANSACTION_DELETE_CONTEXT{
	void			*pContext;
	pTSCallback_t	pCompletionCallback;
};

#endif // __STRING_RESOURCE_MANAGER_H__