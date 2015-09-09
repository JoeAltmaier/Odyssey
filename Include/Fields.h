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
// $Log: /Gemini/Odyssey/DdmPTS/Fields.h $
// 
// 12    8/16/99 9:32a Sgavarre
// include cbKeyFieldValue in ModifyBitsField interface
// 
// 11    8/13/99 4:56p Sgavarre
// add ModifyBitsInField;  change interface for ModifyField for multirow
// operation;
// 
// 10    8/10/99 7:49p Mpanas
// _DEBUG cleanup
// Remove _DEBUG since it should be defined
// in CtPrefix.h ONLY.
// 
// 9     7/27/99 4:17p Agusev
// #ifdef'ed out the #define _DEBUG for Win32
// 
// 8     7/23/99 2:40p Sgavarre
// add QuerySetRowID, TestSetOrClear field 
// 
// 7     7/21/99 7:02p Hdo
// Change the return type of Send() from STATUS to void
// Call the user callback routine when error
// Add error checking
// 
// 01/15/99	HD	Created.
// 03/22/99	HD  Update the PTS document and clean up
//
/*************************************************************************/
#ifndef __TSFields_h__
#define __TSFields_h__

#include "CtTypes.h"
#include "Ddm.h"
#include "Message.h"
#include "Messenger.h"
#include "TableMsgs.h"		// Contains the msg payload struct typedefs.

typedef STATUS (DdmServices::*pTSCallback_t)(void *, STATUS);

/**************************************************************************/
// TSModifyField Class Interface - Modify the contents of a field in a row in a table.
//
// ************************ PUBLIC OPERATIONS ***************************
// STATUS Initialize()	--> Store the operation parameters in the object.
// Send()				--> Packs up the payload, allocates a message and
//							attaches the payload, and sends the msg to the
//							table service specifying our own callback handler
// 							(HandleModifyRowsReply) to unpack the reply.
// HandleModifyFieldsReply()-This is the function specified as the callback in
// 							the Send method.  It deletes the message and calls
//							the user's callback. 
//
// ************************ PRIVATES INSTANCES **************************
//	m_pDdmServices		- Pointer to client's DDM.  Used to access Send().
//	m_rgbTableName		- Null terminated name of the table.
//	m_rgbKeyFieldName	- The key field name used to identify the row to modify
//	m_pKeyFieldValue	- The key field value used to search for the row to modify.
//	m_cbKeyFieldValue	- the size of the key field value in bytes.
//	m_rgbFieldName		- The name of the field to modify.
//	m_pbFieldValue		- The modified value for the field.
//	m_cbFieldValue		- The size of the modified field value in bytes.
//	m_cRowsToModify		- The number of rows to modify; if '0' modify all that match
//	m_pcRowsModifiedRet	- The number of rows that were modified
//	m_pRowIDRet			- Pointer to returned rowID(s) for the modified row.
//	m_cbMaxRowID		- size of buffer for rowIDs
//	m_pCallback			- Pointer to static callback function called upon reply.
//	m_pContext			- Context pointer that is passed to the Callback method
//
/**************************************************************************/
class TSModifyField : public DdmServices
{
public:
	STATUS Initialize(DdmServices		*pDdmServices,
					String64			rgbTableName,
					String64			rgbKeyFieldName,
					void				*pKeyFieldValue,
					U32					cbKeyFieldValue,
					String64			rgbFieldName,
					void				*pbFieldValue,
					U32					cbFieldValue,
					U32					cRowsToModify,
					U32					*pcRowsModifiedRet,
					rowID				*pRowIDRet,
					U32					cbMaxRowID,
					pTSCallback_t		pCallback,
					void				*pContext );

	// Send the message to the Table Service.
	void Send();

	// The object's actual reply handler.
	STATUS HandleModifyFieldsReply( MessageReply *pMsg );

private:
	DdmServices		*m_pDdmServices;
	String64		m_rgbTableName;
	String64		m_rgbKeyFieldName;
	void			*m_pKeyFieldValue;
	U32				m_cbKeyFieldValue;
	String64		m_rgbFieldName;
	void			*m_pbFieldValue;
	U32				m_cbFieldValue;
	U32				m_cRowsToModify;
	U32				*m_pcRowsModifiedRet;
	rowID			*m_pRowIDRet;
	U32				m_cbMaxRowID;
	pTSCallback_t	m_pCallback;
	void			*m_pContext;
} ;


/**************************************************************************/
// TSModifyBits Class Interface - Modify the contents of a field in a row in a table.
//
// ************************ PUBLIC OPERATIONS ***************************
// STATUS Initialize()	--> Store the operation parameters in the object.
// Send()				--> Packs up the payload, allocates a message and
//							attaches the payload, and sends the msg to the
//							table service specifying our own callback handler
// HandleModifyBitsReply()-This is the function specified as the callback in
// 							the Send method.  It deletes the message and calls
//							the user's callback. 
//
// ************************ PRIVATES INSTANCES **************************
// Parameters:
//	m_pDdmServices		- Pointer to client's DDM.  Used to access Send().
//	m_rgbTableName		- Null terminated name of the table.
//	m_opField			- operation to perform:  AND, OR, XOR
//					  		OpAndBits, OpOrBits, 	OpXorBits (defined in ptscommon.h)
//	m_rgbKeyFieldName	- The key field name used to identify the row to modify
//	m_pKeyFieldValue	- The key field value used to search for the row to modify.
//	m_rgbFieldName		- The name of the field to modify.
//	m_pbFieldMask		- The value to 'OR, AND, XOR' with the field.
//	m_cRowsToModify		- The number of rows to modify: '0' will modify all that match
//	m_prgFieldIdsRet	- Pointer to array of returned struct that identifies the modified rows:.
//							typedef struct {
//								rowID	rowModified;
//								U32		fieldRet;
//							} modField;
//	m_cbMax				- size of prgFieldIdsRet
	
//	m_pCallback			- Pointer to static callback function called upon reply.
//	m_pContext			- Context pointer that is passed to the Callback method
//

/**************************************************************************/
class TSModifyBits : public DdmServices
{
public:
STATUS Initialize	(	DdmServices*	pDdmServices,
						String64		rgbTableName,
						fieldOpType		opFlag,
						String64		rgbKeyFieldName,
						void			*pKeyFieldValue,
						U32				cbKeyFieldValue,
						String64		rgbFieldName,
						U32				*pbFieldMask,
						U32				cRowsToModify,
						U32				*pcRowsModRet,
						modField		*prgFieldIdsRet,
						U32				cbMax,
						pTSCallback_t	pCallback,
						void*			pContext );

	// Send the message to the Table Service.
	void Send();

	// The object's actual reply handler.
	STATUS HandleModifyBitsReply( MessageReply *pMsg );

private:
	DdmServices		*m_pDdmServices;
	String64		m_rgbTableName;
	fieldOpType		m_opFlag;
	String64		m_rgbKeyFieldName;
	void			*m_pKeyFieldValue;
	U32				m_cbKeyFieldValue;
	String64		m_rgbFieldName;
	void			*m_pbFieldValue;
	modField		*m_prgFieldIdsRet;
	U32				m_cbrgFieldIds;
	U32				m_cRowsToModify;
	U32				*m_pcRowsModRet;
	pTSCallback_t	m_pCallback;
	void			*m_pContext;
} ;

 /**************************************************************************/
// TSQuerySetRID Class Interface - Query or Set the rowID field.
//
// ************************ PUBLIC OPERATIONS ***************************
// STATUS Initialize()	--> Store the operation parameters in the object.
// Send()				--> Packs up the payload, allocates a message and
//							attaches the payload, and sends the msg to the
//							table service specifying our own callback handler
// HandleQuerySetRIDReply()-This is the function specified as the callback in
// 							the Send method.  It deletes the message and calls
//							the user's callback. 
//
// ************************ PRIVATES INSTANCES **************************
//	m_pDdmServices		- Pointer to client's DDM.  Used to access Send().
//	m_rgbTableName		- Null terminated name of the table.
//  m_opOnRid			- Operation to be performed
//	m_pRowID			- Pointer to returned rowID.
//	m_pCallback			- Pointer to static callback function called upon reply.
//	m_pContext			- Pointer to the user reply callback routine
//
/**************************************************************************/
class TSQuerySetRID : public DdmServices
{
public:
	STATUS Initialize(DdmServices		*pDdmServices,
					String64			rgbTableName,
					fieldOpType			opRowId,
					rowID				*pRowID,
					pTSCallback_t		pCallback,
					void				*pContext );

	// Send the message to the Table Service.
	void Send();

	// The object's actual reply handler.
	STATUS HandleQuerySetRIDReply( MessageReply *pMsg );

private:
	DdmServices		*m_pDdmServices;
	String64		m_rgbTableName;
	fieldOpType		m_opOnRid;
	rowID			*m_pRowID;
	pTSCallback_t	m_pCallback;
	void			*m_pContext;
} ;


/**************************************************************************/
// TSTestSetField Class Interface - TestSet the contents of a field in a row in a table.
//
// ************************ PUBLIC OPERATIONS ***************************
// STATUS Initialize()	--> Store the operation parameters in the object.
// Send()				--> Packs up the payload, allocates a message and
//							attaches the payload, and sends the msg to the
//							table service specifying our own callback handler
// 							(HandleTestSetFieldReply) to unpack the reply.
// HandleTestSetFieldReply()-This is the function specified as the callback in
// 							the Send method.  It deletes the message and calls
//							the user's callback. 
//
// ************************ PRIVATES INSTANCES **************************
//	m_pDdmServices		- Pointer to client's DDM.  Used to access Send().
//	m_rgbTableName		- Null terminated name of the table.
//	m_rgbKeyFieldName	- The key field name used to identify the row
//	m_pKeyFieldValue	- The key field value used to search for the row.
//	m_cbKeyFieldValue	- the size of the key field value in bytes.
//	m_rgbFieldName		- The name of the field to test.
//	m_pTestFlagRet		- Pointer to returned test flag.
//	m_pCallback			- Pointer to static callback function called upon reply.
//	m_pContext			- Pointer to the user reply callback routine
//
/**************************************************************************/
class TSTestSetOrClearField : public DdmServices
{
public:
	STATUS Initialize(DdmServices		*pDdmServices,
					String64			rgbTableName,
					fieldOpType			opFlag,
					String64			rgbKeyFieldName,
					void				*pKeyFieldValue,
					U32					cbKeyFieldValue,
					String64			rgbFieldName,
					BOOL				*pTestFlagRet,
					pTSCallback_t		pCallback,
					void				*pContext );

	// Send the message to the Table Service.
	void Send();

	// The object's actual reply handler.
	STATUS HandleTestSetFieldReply( MessageReply *pMsg );

private:
	DdmServices		*m_pDdmServices;
	String64		m_rgbTableName;
	fieldOpType		m_opFlag;
	String64		m_rgbKeyFieldName;
	void			*m_pKeyFieldValue;
	U32				m_cbKeyFieldValue;
	String64		m_rgbFieldName;
	BOOL			*m_pTestFlagRet;
	pTSCallback_t	m_pCallback;
	void			*m_pContext;
} ;


#endif // TSField