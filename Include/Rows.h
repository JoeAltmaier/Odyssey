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
// This file is the declaration of Row objects interface to the Row(s)
// API of the Table Service.
// 
// $Log: /Gemini/Include/Rows.h $
// 
// 16    10/27/99 11:57a Ewedel
// Changed Initialize() members to take CPtsRecordBase* argument as row
// data specifier.  For non-varfield classes, this is added as an
// overload.  For varfield classes, this is now the *only* way of
// specifying row data (i.e., varfield table rows must be derived from
// CPtsRecordBase).
// 
// 15    10/06/99 3:27p Sgavarre
// add variable length fields
// 
// 14    8/13/99 4:58p Sgavarre
// Update ModifyRow, DeleteRows for multiRow operation
// 
// 13    8/10/99 7:49p Mpanas
// _DEBUG cleanup
// Remove _DEBUG since it should be defined
// in CtPrefix.h ONLY.
// 
// 12    7/27/99 4:13p Agusev
// #ifdef'ed out #define _DEBUG for WIN32
// 
// 11    7/21/99 7:02p Hdo
// Change the return type of Send() from STATUS to void
// Call the user callback routine when error
// Add error checking
// 
// 11/23/98	HD	Created.
// 12/14/98 HD  Modify the header file and implement TSInsertRow
// 01/13/99 HD  Implement TSModifyRow, TSReadRow, TSDeleteRow
//				Modify TSInsert converting message payload to SGL
// 02/20/99 JL	Pass sizeof(rowID) instead of sizeof(rowID*) in InsertRow().
// 03/22/99	HD  Update the PTS document and clean up
//
/*************************************************************************/
#ifndef __TSRows_h__
#define __TSRows_h__

#include "CtTypes.h"
#include "Ddm.h"
#include "Message.h"
#include "Messenger.h"
#include "TableMsgs.h"		// Contains the msg payload struct typedefs.

#ifndef _PtsRecordBase_h
# include  "PtsRecordBase.h"
#endif


typedef STATUS (DdmServices::*pTSCallback_t)(void *, STATUS);

/**************************************************************************/
// TSInsertRow Class Interface - Insert a new row into a table
//
// ************************ PUBLIC OPERATIONS ***************************
// STATUS Initialize()	--> Store the operation parameters in the object.
// Send()				--> Packs up the payload, allocates a message and
//							attaches the payload, and sends the msg to the
//							table service specifying our own callback handler
// 							(HandleModifyRowsReply) to unpack the reply.
// HandleInsertRowReply()-->This is the function specified as the callback in
// 							the Send method.  It deletes the message and calls
//							the user's callback. 
//
// ************************ PRIVATES INSTANCES **************************
//	m_pDdmServices	- Pointer to client's DDM.  Used to access Send().
//	m_rgbTableName	- Null terminated name of the table.
//	m_prgbRowData	- Pointer to the row data to insert.
//	m_cbRowData		- size of the row data in bytes.
//	m_pRowIDRet		- Pointer to returned RowID for newly inserted row.
//	m_pCallback		- Pointer to static callback function called upon reply.
//	m_pContext		- Context pointer that is passed to the Callback method
//
/**************************************************************************/
class TSInsertRow : public DdmServices
{
public:
	STATUS Initialize(	DdmServices		*pDdmServices,
						String64		rgbTableName,
						void			*prgbRowData,
						U32				cbRowData,
						rowID			*pRowIDRet,
						pTSCallback_t	pCallback,
						void			*pContext );

   //  the following form is for inserting a single record only
   //  (in either fixed or variable-field tables).  Note that per
   //   present PTS DDM definition, variable field data may not
   //   be inserted via TSInsertRow; use TSInsertVLRow instead).
	STATUS Initialize(	DdmServices		*pDdmServices,
						String64		rgbTableName,
						const CPtsRecordBase *pRowData,
						rowID			*pRowIDRet,
						pTSCallback_t	pCallback,
						void			*pContext )
   {
      return (Initialize (pDdmServices, rgbTableName,
                          (void *) pRowData, pRowData->FixedSize(),
                          pRowIDRet, pCallback, pContext));
   };

	// Send the TSInsertRow message to the Table Service.
	void Send();

	// The object's actual reply handler.
	STATUS HandleInsertRowsReply(MessageReply *pMsg);

private:
	DdmServices		*m_pDdmServices;
	String64		m_rgbTableName;
	void			*m_prgbRowData;
	U32				m_cbRowData;
	rowID			*m_pRowIDRet;
	U32				m_cRowsInsertedRet;				// not currently used by interface
	pTSCallback_t	m_pCallback;
	void			*m_pContext;
} ;


/**************************************************************************/
// TSInsertVLRow Class Interface - Insert a new variable length row into a table
//
// ************************ PUBLIC OPERATIONS ***************************
// STATUS Initialize()	--> Store the operation parameters in the object.
// Send()				--> Packs up the payload, allocates a message and
//							attaches the payload, and sends the msg to the
//							table service specifying our own callback handler
// 							(HandleInsertVLRowReply) to unpack the reply.
// HandleInsertVLRowReply()-->This is the function specified as the callback in
// 							the Send method.  It deletes the message and calls
//							the user's callback. 
//
// ************************ PRIVATES INSTANCES **************************
//	m_pDdmServices	- Pointer to client's DDM.  Used to access Send().
//	m_rgbTableName	- Null terminated name of the table.
//	m_pRowData		- Pointer to the fixed row data to insert.
//	m_cbRowData		- size of the fixed row data in bytes.
//	m_pRowIDRet		- Pointer to returned RowID for newly inserted row.
//	m_pCallback		- Pointer to static callback function called upon reply.
//	m_pContext		- Context pointer that is passed to the Callback method
//
/**************************************************************************/
class TSInsertVLRow : public DdmServices
{
public:
	STATUS Initialize (	DdmServices		*pDdmServices,
						String64		rgbTableName,
						const CPtsRecordBase  *pRowData,
						rowID			*pRowIDRet,
						pTSCallback_t	pCallback,
						void			*pContext );

	// Send the TSInsertVLRow message to the Table Service.
	void Send();

	// The object's actual reply handler.
	STATUS HandleInsertVarLenRowReply(MessageReply *pMsg);

private:
	DdmServices		*m_pDdmServices;
	String64		m_rgbTableName;
	const CPtsRecordBase *m_pRowData;
	rowID			*m_pRowIDRet;
	pTSCallback_t	m_pCallback;
	void			*m_pContext;
} ;


/**************************************************************************/
// TSModifyRow Class Interface - Modify the contents of a row in a table.
//
// ************************ PUBLIC OPERATIONS ***************************
// STATUS Initialize()	--> Store the operation parameters in the object.
// Send()				--> Packs up the payload, allocates a message and
//							attaches the payload, and sends the msg to the
//							table service specifying our own callback handler
// 							(HandleModifyRowsReply) to unpack the reply.
// HandleModifyRowReply()-->This is the function specified as the callback in
// 							the Send method.  It deletes the message and calls
//							the user's callback. 
//
// ************************ PRIVATES INSTANCES **************************
//	m_pDdmServices		- Pointer to client's DDM.  Used to access Send().
//	m_rgbTableName		- Null terminated name of the table.
//	m_rgbKeyFieldName	- The key field name used to identify the row to modify
//	m_pKeyFieldValue	- The key field value used to search for the row to modify.
//	m_cbKeyFieldValue	- the size of the key field value in bytes.
//	m_prgbRowData		- Pointer to the row data to insert.
//	m_cbRowData			- size of the row data in bytes.
//	m_cRowsToModify		- count of rows to modify;  '0' means ALL that match
//	m_pcRowsModifiedRet	- pointer to the U32 that will have the # of rows modified.
//	m_pRowIDRet			- Pointer to returned RowID for newly modified row.
//	m_cbMaxRowID		- size of rowID buffer

//	m_pCallback			- Pointer to static callback function called upon reply.
//	m_pContext			- Context pointer that is passed to the Callback method
//
/**************************************************************************/
class TSModifyRow : public DdmServices
{
public:
	STATUS Initialize(	DdmServices		*pDdmServices,
						String64		rgbTableName,
						String64		rgbKeyFieldName,
						void			*pKeyFieldValue,
						U32				cbKeyFieldValue,
						void			*prgbRowData,
						U32				cbRowData,
						U32				cRowsToModify,
						U32				*pcRowsModifiedRet,
						rowID			*pRowIDRet,
						U32				cbMaxRowID,
						pTSCallback_t	pCallback,
						void			*pContext );

    //  if you wish to modify variable-length field data in a row modify,
    //  you must use this form of Initialize():
	STATUS Initialize(	DdmServices		*pDdmServices,
						String64		rgbTableName,
						String64		rgbKeyFieldName,
						void			*pKeyFieldValue,
						U32				cbKeyFieldValue,
						const CPtsRecordBase *pRowData,
						rowID			*pRowIDRet,
						U32				cbMaxRowID,
						pTSCallback_t	pCallback,
						void			*pContext )
    {
       STATUS  sRet;

       sRet = Initialize (pDdmServices, rgbTableName,
                          rgbKeyFieldName, pKeyFieldValue, cbKeyFieldValue,
                          (void *) pRowData, pRowData->TotalRecSize(),
                          1, NULL,     // only one varfield row can be modified
                          pRowIDRet, cbMaxRowID,
                          pCallback, pContext);
       m_fHasPtsRecBase = TRUE;
       return (sRet);
    };

	// Send the TSModifyRow message to the Table Service.
	void Send();

	// The object's actual reply handler.
	STATUS HandleModifyRowsReply(MessageReply *pMsg);

private:
	DdmServices		*m_pDdmServices;
	String64		m_rgbTableName;
	String64		m_rgbKeyFieldName;
	void			*m_pKeyFieldValue;
	U32				m_cbKeyFieldValue;
	void			*m_prgbRowData;
	U32				m_cbRowData;
	U32				m_cRowsToModify;
	U32				*m_pcRowsModifiedRet;
	rowID			*m_pRowIDRet;
	U32				m_cbMaxRowID;
	pTSCallback_t	m_pCallback;
	void			*m_pContext;
    BOOL            m_fHasPtsRecBase;
};

/**************************************************************************/
// TSReadRow Class Interface - Read a specified row from the specified table
//
// ************************ PUBLIC OPERATIONS ***************************
// STATUS Initialize()	--> Store the operation parameters in the object.
// Send()				--> Packs up the payload, allocates a message and
//							attaches the payload, and sends the msg to the
//							table service specifying our own callback handler
// 							to unpack the reply.
// HandleReadRowReply()	--> This is the function specified as the callback in
// 							the Send method.  It deletes the message and calls
//							the user's callback. 
//
// ************************ PRIVATES INSTANCES **************************
//	m_pDdmServices		- Pointer to client's DDM.  Used to access Send().
//	m_rgbTableName		- Null terminated name of the table.
//	m_rgbKeyFieldName	- The key field name used to identify the row to modify
//	m_pKeyFieldValue	- The key field value used to search for the row to modify.
//	m_cbKeyFieldValue	- the size of the key field value in bytes.
//	m_prgbRowDataRet	- Pointer to the caller-supplied buffer to return
//                        row data in.
//	m_cbRowDataRetMax	- size of the *m_prgbRowDataRet row data buffer, in bytes.
//	m_pcRowsReadRet		- pointer to count of rows read.
//	m_pCallback			- Pointer to static callback function called upon reply.
//	m_pContext			- Context pointer that is passed to the Callback method
//
/**************************************************************************/
class TSReadRow : public DdmServices
{
public:
	STATUS Initialize(	DdmServices		*pDdmServices,
						String64		rgbTableName,
						String64		rgbKeyFieldName,
						void			*pKeyFieldValue,
						U32				cbKeyFieldValue,
						void			*prgbRowDataRet,
						U32				cbRowDataRetMax,
						U32				*pcRowsReadRet,
						pTSCallback_t	pCallback,
						void			*pContext );

	// Send the TSReadRow message to the Table Service.
	void Send();

	// The object's actual reply handler.
	STATUS HandleReadRowsReply(MessageReply *pMsg);

private:
	DdmServices		*m_pDdmServices;
	String64		m_rgbTableName;
	String64		m_rgbKeyFieldName;
	void			*m_pKeyFieldValue;
	U32				m_cbKeyFieldValue;
	void			*m_prgbRowDataRet;
	U32				m_cbRowDataRetMax;
	U32				*m_pcRowsReadRet;
	pTSCallback_t	m_pCallback;
	void			*m_pContext;
};


/**************************************************************************/
// TSReadVLRow Class Interface - Read a specified var len row from the specified table
//
// ************************ PUBLIC OPERATIONS ***************************
// STATUS Initialize()	--> Store the operation parameters in the object.
// Send()				--> Packs up the payload, allocates a message and
//							attaches the payload, and sends the msg to the
//							table service specifying our own callback handler
// 							(HandleReadVarLenRowsReply) to unpack the reply.
// HandleReadVarLenRowReply This is the function specified as the callback in
// 							the Send method.  It deletes the message and calls
//							the user's callback. 
//
// ************************ PRIVATES INSTANCES **************************
//	m_pDdmServices		- Pointer to client's DDM.  Used to access Send().
//	m_rgbTableName		- Null terminated name of the table.
//	m_rgbKeyFieldName	- The key field name used to identify the row(s) to modify
//	m_pKeyFieldValue	- The key field value used to search for the row(s) to modify.
//	m_cbKeyFieldValue	- the size of the key field value in bytes.
//	m_ppRowDataRet		- Ptr to user pointer where the address of the row data is written
//	m_pcbRowDataRet		- pointer where the size of the row data is returned
//	m_pcRowsReadRet		- pointer to count of rows read.
//	m_pCallback			- Pointer to static callback function called upon reply.
//	m_pContext			- Context pointer that is passed to the Callback method
//
/**************************************************************************/
class TSReadVLRow : public DdmServices
{
public:
	STATUS Initialize(	DdmServices		*pDdmServices,
						String64		rgbTableName,
						String64		rgbKeyFieldName,
						void			*pKeyFieldValue,
						U32				cbKeyFieldValue,
						CPtsRecordBase  **ppRowDataRet,
						U32				*pcbRowDataRet,
						U32				*pcRowsReadRet,
						pTSCallback_t	pCallback,
						void			*pContext );

	// Send the TSReadVLRow message to the Table Service.
	void Send();

	// The object's actual reply handler.
	STATUS HandleReadVarLenRowReply(MessageReply *pMsg);

private:
	DdmServices		*m_pDdmServices;
	String64		m_rgbTableName;
	String64		m_rgbKeyFieldName;
	void			*m_pKeyFieldValue;
	U32				m_cbKeyFieldValue;
	CPtsRecordBase	**m_ppRowDataRet;
	U32				*m_pcbRowDataRet;
	U32				*m_pcRowsReadRet;
	pTSCallback_t	m_pCallback;
	void			*m_pContext;
};


/**************************************************************************/
// TSDeleteRow Class Interface - Delete a specified row from the specified table
//
// ************************ PUBLIC OPERATIONS ***************************
// STATUS Initialize()	--> Store the operation parameters in the object.
// Send()				--> Packs up the payload, allocates a message and
//							attaches the payload, and sends the msg to the
//							table service specifying our own callback handler
// 							(HandleModifyRowsReply) to unpack the reply.
// HandleDeleteRowReply()-->This is the function specified as the callback in
// 							the Send method.  It deletes the message and calls
//							the user's callback. 
//
// ************************ PRIVATES INSTANCES **************************
//	m_pDdmServices		- Pointer to client's DDM.  Used to access Send().
//	m_rgbTableName		- Null terminated name of the table.
//	m_rgbKeyFieldName	- The key field name used to identify the row to modify
//	m_pKeyFieldValue	- The key field value used to search for the row to modify.
//	m_cbKeyFieldValue	- the size of the key field value in bytes.
//	m_cRowsToDelete		- Number of rows to delete;  0 for all matching rows
//	m_pcRowsDelRet		- Pointer to returned number of rows deleted
//	m_pCallback			- Pointer to static callback function called upon reply.
//	m_pContext			- Context pointer that is passed to the Callback method
//
/**************************************************************************/
class TSDeleteRow : public DdmServices
{
public:
	STATUS Initialize(	DdmServices		*pDdmServices,
						String64		rgbTableName,
						String64		rgbKeyFieldName,
						void			*pKeyFieldValue,
						U32				cbKeyFieldValue,
						U32				cRowsToDelete,
						U32				*pcRowsDelRet,
						pTSCallback_t	pCallback,
						void			*pContext );

	// Send the TSDeleteRow message to the Table Service.
	void Send();

	// The object's actual reply handler.
	STATUS HandleDeleteRowsReply(MessageReply *pMsg);

private:
	DdmServices		*m_pDdmServices;
	String64		m_rgbTableName;
	String64		m_rgbKeyFieldName;
	void			*m_pKeyFieldValue;
	U32				m_cbKeyFieldValue;
	U32				m_cRowsToDelete;
	U32				*m_pcRowsDelRet;
	pTSCallback_t	m_pCallback;
	void			*m_pContext;
};

#endif // __TSRows_h__
