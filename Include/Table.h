/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This file is the declaration of an object interface to the Table 
// API of the Table Service.
// 
// $Log: /Gemini/Include/Table.h $
// 
// 25    8/30/99 12:16p Jlane
// Make fielddef param const to avoid warning.
// 
// 24    8/28/99 5:39p Sgavarre
// add dynamic reply buffers for returned data;  no change to client
// interface
// 
// 23    8/10/99 7:49p Mpanas
// _DEBUG cleanup
// Remove _DEBUG since it should be defined
// in CtPrefix.h ONLY.
// 
// 22    7/27/99 4:11p Agusev
// #ifdef'ed out the _DEBUG define for WIN32
// 
// 21    7/21/99 7:02p Hdo
// Change the return type of Send() from STATUS to void
// Call the user callback routine when error
// Add error checking
// 
// 20    7/14/99 2:17p Sgavarre
// Add parameter to GetTableDef that specifies the number of field per row
// 
// 19    7/12/99 2:28p Sgavarre
// Add params for DeleteTable
// 
// 18    7/01/99 7:04p Hdo
// Add TSDeleteTable class
// 
// 17    6/22/99 3:55p Jhatwich
// updated for windows
// 
// 16    5/24/99 8:28p Hdo
// Add TSGetTableDef::GetReturnedTableDef()
// 
// 15    5/12/99 5:13p Hdo
// Add TSDefineTable::GetTableID()
// Intialize pointers to NULL after delete for pMsg
// 
// 14    5/11/99 10:55a Jhatwich
// win32
// 
// 13    4/06/99 11:47a Jlane
// Added const to Define Table params to avoid warnings.
// 
// 12    4/03/99 6:32p Jlane
// Multiple changes to interface, messaage payloads and message handlers
// to avoid having to add reply payloads as this destroys SGL items.  We
// do this by allocating space for reply data in the msg payload.
// 
// 11    3/22/99 1:37p Hdo
// Table: Merge w/ EW version and add comments
// 
// 03/22/99	HD  Update the PTS document and clean up
//
// 14    3/19/99 7:07p Ewedel
// Fixed a weirdness in TSEnumTable::HandleEnumTableReply() where the
// count of bytes returned by PTS was not being reported to the caller.
// There is a separate assign (still there) to m_cbDataRetMax, which might
// be wrong.
// Also added vss Log: keyword.
//
// 02/14/99 JFL	Don't delete &pMsg (should be pMsg) in HandleEnumTableReply.
// 02/01/99	JFL	Merged with Sherri's TableMsgs.h Changed the field named
//				pGetTableDefReplyPayload->cFieldDefsRet to pGetTableDefReplyPayload->cbFieldDefsRet
//				and also changed the TSGetTableDef field name for the same value accordingly. 
// 01/21/99	JFL	In TSEnumTable::Initialize: starting row num shouldn't be a reference.
// 01/18/99	JFL	Misc. cleanup & modified to conform to the new TableMsgs.h format.
// 12/28/98 HD	Added the pContext value to all classes
//				Changed parms allowing to pass NULL pointer
// 12/11/98 HD  Converted pointers variables to reference variables
// 11/23/98	HD	Modified TSDefineTable, TSGetTableDef, & TSEnumTable becoming
//				objects with callback facility.
// 11/17/98	JFL	Created.
//
/*************************************************************************/
#ifndef __Table_h__
#define __Table_h__

#include "CtTypes.h"
#include "Ddm.h"
#include "Message.h"
#include "Messenger.h"
#include "TableMsgs.h"		// Contains the msg payload struct typedefs.

typedef STATUS (DdmServices::*pTSCallback_t)(void *, STATUS);

#ifdef WIN32
#define TSCALLBACK(clas,method)	(pTSCallback_t) method
#elif defined(__ghs__)  // Green Hills
#define TSCALLBACK(clas,method)	(pTSCallback_t) &clas::method
#else	// MetroWerks
#define TSCALLBACK(clas,method)	(pTSCallback_t)&method
#endif

/**************************************************************************/
// TSDefineTable Class Interface - Create a new table.
//
// ************************ PRIVATE INSTANCES ***************************
//	m_pDdmServices	- Pointer to client's DDM.  Used to access Send().
//	m_rgbTableName	- Null terminated name of the table to create.
//	m_prgFieldDefs	- Pointer to FieldDef array that defines the fields
//	m_cbrgFieldDefs	- size of the above m_prgFieldDefs array in bytes.
//	m_cEntriesRsv	- Number of table entries to create initially.
//	m_PersistFlags	- pointer to returned flag indicating if table is persistant.
//	m_pCallback		- Pointer to static callback function called upon reply.
//	m_pContext		- Context pointer that is passed to the Callback method
// ************************ PUBLIC OPERATIONS ***************************
// STATUS Initialize()	--> Store the operation parameters in the object.
// STATUS Send()		--> Packs up the payload, allocates a message and
//							attaches the payload, and sends the msg to the
//							table service specifying our own callback handler
// 							(HandleModifyRowsReply) to unpack the reply.
// HandleDefineTableReply() This is the function specified as the callback in
// 							the Send method.  It deletes the message and calls
//							the user's callback.
// GetTableID()			--> Return the rowID of the newly define table
/**************************************************************************/
class TSDefineTable : public DdmServices
{
public:
	// Constructor
	TSDefineTable() { m_rowIDRet.Table = m_rowIDRet.HiPart = m_rowIDRet.LoPart = 0; }

	// Initialize all parameters necessary to get the Table Def'n.
	STATUS Initialize(	DdmServices*	pDdmServices,
						String64		rgbTableName,
						const fieldDef*	prgFieldDefs,
						U32				cbrgFieldDefs,
						U32				cEntriesRsv,
						U32				PersistFlags,
						pTSCallback_t	pCallback,
						void*			pContext
					  );

	// Send the DefineTable message to the Table Service.
	void Send();

	// The object's actual reply handler.
	STATUS HandleDefineTableReply( Message	*pMsg );
	rowID	GetTableID();

private:
	DdmServices		*m_pDdmServices;
	String64		 m_rgbTableName;
	const fieldDef	*m_prgFieldDefs;
	U32				 m_cbrgFieldDefs;
	U32				 m_cEntriesRsv;
	U32				 m_PersistFlags;
	pTSCallback_t	 m_pCallback;
	void			*m_pContext;

	rowID			 m_rowIDRet;
}; // TSDefineTable



/**************************************************************************/
// GetTableDef - Get the definition data of an existing table
//
// ************************ PRIVATE INSTANCES ***************************
//	m_pDdmServices		- Pointer to client's DDM.  Used to access Send().
//	m_rgbTableName		- Null terminated name of the table.
//	m_prgFieldDefsRet	- The returned fieldDef array that defines the fields
//  m_cbrgFieldDefsRetMax - Size of prgFieldDefsRet array.
//	m_pcbFieldDefsRet	- pointer to returned number of bytes in Field Def. array.
//	m_pcbRowRet			- pointer to returned number of bytes per row.
//	m_pcRowsRet			- pointer to returned count of rows in table
//	m_pcFieldsRet		- pointer to returned count of fields in a row
//	m_pPersistFlagstRet	- pointer to returned flag indicating if table is persistant.
//	m_pCallback			- Pointer to static callback function called upon reply.
//	m_pContext			- Context pointer that is passed to the Callback method
//
// ************************ PUBLIC OPERATIONS ***************************
// STATUS Initialize()	--> Store the operation parameters in the object.
// STATUS Send()		--> Packs up the payload, allocates a message and
//							attaches the payload, and sends the msg to the
//							table service specifying our own callback handler
// 							(HandleModifyRowsReply) to unpack the reply.
// HandleGetTableDefReply() This is the function specified as the callback in
// 							the Send method.  It deletes the message and calls
//							the user's callback. 
//
/**************************************************************************/
class TSGetTableDef : public DdmServices
{
public:
	// Constructor
	TSGetTableDef() { m_TableDef.cFields = m_TableDef.cbRow = m_TableDef.cRows = 0; }

	// Initialize all parameters necessary to get the Table Def'n.
	STATUS Initialize(	DdmServices		*pDdmServices,
						String64		 rgbTableName,
						fieldDef		*prgFieldDefsRet,
						U32				 cbrgFieldDefsRet,
						U32				*pcFieldDefsRet,
						U32				*pcbRowRet,
						U32				*pcRowsRet,
						U32				*pcFieldsRet,
						U32				*pPersistFlagsRet,
						pTSCallback_t	 pCallback,
						void			*pContext
					 );

	// Send the GetTableDef message to the Table Service.
	void Send();

	// The object's actual reply handler.
	STATUS HandleGetTableDefReply( Message  *pMsg );
	tableDef& GetReturnedTableDef();

private:
	DdmServices		*m_pDdmServices;
	String64		 m_rgbTableName;
	fieldDef		*m_prgFieldDefsRet;
	U32				 m_cbrgFieldDefsMax;
	U32				*m_pcbFieldDefsRet;
	U32				*m_pcbRowRet;
	U32				*m_pcRowsRet;
	U32				*m_pcFieldsRet;
	U32				*m_pPersistFlagsRet;
	pTSCallback_t	 m_pCallback;
	void			*m_pContext;
	
	// This is the structure returned by the table service.
	tableDef		m_TableDef;
	
}; // GetTableDef



/**************************************************************************/
// TSEnumTable - Read Sequential records from a table.
//
// ************************ PRIVATE INSTANCES ***************************
//	m_pDdmServices	- Pointer to client's DDM.  Used to access Send().
//  m_prgbTableName	- Null terminated name of the table.
//  m_uStartRowNum	- Starting row number (0 based).
//  m_pbRowDataRet	- Pointer to the returned data buffer.
//  m_cbDataRetMax	- Max size of returned RowDataRet buffer.
//					- limited by 8K transfer buffer size.
//  m_pcbRowDataRet	- Pointer to number of bytes returned.
//	m_pCallback		- Pointer to static callback function called upon reply.
//	m_pContext		- Context pointer that is passed to the Callback method
//
// ************************ PUBLIC OPERATIONS ***************************
// STATUS Initialize()	--> Store the operation parameters in the object.
// STATUS Send()		--> Packs up the payload, allocates a message and
//							attaches the payload, and sends the msg to the
//							table service specifying our own callback handler
// 							(HandleModifyRowsReply) to unpack the reply.
// HandleEnumTableReply()-->This is the function specified as the callback in
// 							the Send method.  It deletes the message and calls
//							the user's callback. 
//
/**************************************************************************/
class TSEnumTable : public DdmServices
{
public:
		// Initialize all parameters necessary to get the Table Def'n.
		STATUS Initialize(	DdmServices		*pDdmServices,
							String64		rgbTableName,
							U32				uStartRowNum,
							void			*pbRowDataRet,
							U32				cbDataRetMax,
							U32				*pcbRowDataRet,
							pTSCallback_t	pCallback,
							void			*pContext
						 );

		// Send the GetTableDef message to the Table Service.
		void Send();

		// The object's actual reply handler.
		STATUS HandleEnumTableReply( Message	*pMsg );

private:
	DdmServices		*m_pDdmServices;
	String64		m_rgbTableName;
	U32				m_uStartRowNum;
	void			*m_pbRowDataRet;
	U32				m_cbDataRetMax;
	U32				*m_pcbRowDataRet;
	pTSCallback_t	m_pCallback;
	void			*m_pContext;
}; // TSEnumTable


/**************************************************************************/
// TSDeleteTable - Delete a table.
//
// ************************ PRIVATE INSTANCES ***************************
//	m_pDdmServices		- Pointer to client's DDM.  Used to access Send().
//  m_rgbTableName		- Null terminated name of the table.
//  m_prgbTableName		- pointer to the null terminated name of the table.
//						- This pointer is used in Send to detect TableName == NULL
//  m_tableId			- optional rowID that represents table, instead of tablename.
//	m_pCallback			- Pointer to static callback function called upon reply.
//	m_pContext			- Context pointer that is passed to the Callback method
//
// ************************ PUBLIC OPERATIONS ***************************
// STATUS Initialize()	--> Store the operation parameters in the object.
// STATUS Send()		--> Packs up the payload, allocates a message and
//							attaches the payload, and sends the msg to the
//							table service specifying our own callback handler
// 							(HandleModifyRowsReply) to unpack the reply.
// HandleDeleteTableReply()-->This is the function specified as the callback in
// 							the Send method.  It deletes the message and calls
//							the user's callback. 
//
/**************************************************************************/
class TSDeleteTable : public DdmServices
{
public:
		// Initialize all parameters necessary to get the Table Def'n.
		STATUS Initialize(	DdmServices		*pDdmServices,
							String64		rgbTableName,
							rowID			tableId,
							pTSCallback_t	pCallback,
							void			*pContext
						 );

		// Send the TableDelete message to the Table Service.
		void Send();

		// The object's actual reply handler.
		STATUS HandleDeleteTableReply( Message	*pMsg );

private:
	DdmServices		*m_pDdmServices;
	String64		*m_prgbTableName;
	String64		m_rgbTableName;
	rowID			m_tableId;
	pTSCallback_t	m_pCallback;
	void			*m_pContext;
}; // TSDeleteTable

#endif // __Table_h__