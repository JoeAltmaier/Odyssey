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
// $Log: /Gemini/Odyssey/DdmPTS/Fields.cpp $
// 
// 34    10/14/99 6:00p Dpatel
// FieldValue could not be used on the stack for
// ModifyField/Bits..reviewed by 
// eric wedel.
// 
// 33    10/14/99 3:16p Jlane
// Don't return erc if key value is null as a temp hack to support
// ALL_ROWS name.
// 
// 32    10/12/99 10:02a Jlane
// Remove unused route by slot code.  Use strlen()+1for name SGLs
// throughout.
// 
// 31    8/30/99 9:41a Sgavarre
// merge by hand...
// 
// 30    8/30/99 9:11a Sgavarre
// clear getsgl size
// 
// 28    8/16/99 9:32a Sgavarre
// include cbKeyFieldValue in ModifyBitsField interface
// 
// 27    8/13/99 4:55p Sgavarre
// add ModifyBitsInField;  change interface for ModifyField for multirow
// operation;
// 
// 26    7/28/99 6:16p Sgavarre
// fix rowIDret parameter in ModifyField
// 
// 25    7/28/99 10:22a Sgavarre
// fix parameter in ModifyFields rowIDRet
// 
// 24    7/23/99 2:40p Sgavarre
// add QuerySetRowID, TestSetOrClear field 
// 
// 23    7/22/99 3:10p Hdo
// 
// 22    7/21/99 7:02p Hdo
// Change the return type of Send() from STATUS to void
// Call the user callback routine when error
// Add error checking
// 
// 21    7/19/99 5:18p Hdo
// Change the return type of Send() to void
// Add error checking
// Call user callback if error happens in Send()
// 
// 20    6/22/99 3:53p Jhatwich
// updated for windows
// 
// 19    6/08/99 3:44p Sgavarre
// remove reply payloads
// 
// 18    6/08/99 12:13a Ewedel
// Fixed "key field name" SGL item in Send().  It and "key field value"
// were swapped.
// 
// 17    6/07/99 6:55p Hdo
// Add SetParentDdm and fix the second Sgl item in Send()
// 
// 16    6/08/99 8:04p Tnelson
// Fix all warnings...
// 
// 15    5/24/99 8:29p Hdo
// 
// 14    5/11/99 10:53a Jhatwich
// win32
// 
// 13    4/16/99 6:14p Jlane
// Delete the Msg in our reply handler consistently.
// 
// 12    4/07/99 5:18p Ewedel
// Changed to use new payload size argument to Message() constructor.
// 
// 11    4/03/99 6:32p Jlane
// Multiple changes to interface, messaage payloads and message handlers
// to avoid having to add reply payloads as this destroys SGL items.  We
// do this by allocating space for reply data in the msg payload.
// 
// 10    4/02/99 1:52p Jlane
// #if'd out workaround code to send messages to slots since interslot
// route by function wasn't working.
// 
// 9     3/27/99 1:49p Jlane
// Changed all calls to Send() to use the send to slot version if we're
// not on the HBC.  This works around the OS limitation of not corrrectly
// handling route by function code between boards.  This should be
// temporary but should also work indefinitely.
// 
// 8     3/24/99 7:47p Jtaylor
// OS_DETAIL_STATUS_SUCCESS
//
// Update Log: 
// 01/15/99	HD	Created.
// 03/22/99	HD  Update the PTS document and clean up
//
/*************************************************************************/
#ifndef PTS_TRACE
	#undef _DEBUG
#endif
#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_PTS
#include "Odyssey_Trace.h"

#include "Fields.h"
#include "CtUtils.h"

/**************************************************************************/
// TSModifyField Class Interface - Modify the contents of a field in a row in a table.
//
/**************************************************************************/
// STATUS Initialize()	--> Store the operation parameters in the object.
//
// Parameters:
//	pDdmServices	- Pointer to client's DDM.  Used to access Send().
//	rgbTableName	- Null terminated name of the table.
//	rgbKeyFieldName	- The key field name used to identify the row to modify
//	pKeyFieldValue	- The key field value used to search for the row to modify.
//	cbKeyFieldValue	- the size of the key field value in bytes.
//	rgbFieldName	- The name of the field to modify.
//	pbFieldValue	- The modified value for the field.
//	cbFieldValue	- The size of the modified field value in bytes.
//	cRowsToModify	- count of rows to modify;  '0' means ALL that match
//	pcRowsModifiedRet - pointer to the U32 that will have the # of rows modified.
//	pRowIDRet		- Pointer to returned rowID(s) for the modified rows.
//	cbMaxRowIDs		- size of buffer for rowIDs
//	pCallback		- Pointer to static callback function called upon reply.
//	pContext		- Context pointer that is passed to the Callback method
//
/**************************************************************************/
STATUS TSModifyField::Initialize(	DdmServices*	pDdmServices,
								String64		rgbTableName,
								String64		rgbKeyFieldName,
								void			*pKeyFieldValue,
								U32				cbKeyFieldValue,
								String64		rgbFieldName,
								void			*pbFieldValue,
								U32				cbFieldValue,
								U32				cRowsToModify,
								U32				*pcRowsModifiedRet,
								rowID			*pRowIDRet,
								U32				cbMaxRowID,
								pTSCallback_t	pCallback,
								void*			pContext )
{
	TRACE_ENTRY(TSModifyField::Initialize);

	// Check for NULL in required parameters.

	if ((pDdmServices == NULL)   || (rgbTableName == NULL) ||  (rgbKeyFieldName == NULL) ||
		(pKeyFieldValue == NULL) || (cbKeyFieldValue == 0) || (rgbFieldName == NULL)	 ||
		(pbFieldValue == NULL )  || (cbFieldValue == 0))

		return ercBadParameter;

	// Initialize our base class.

	SetParentDdm(pDdmServices);
	m_pDdmServices 		= pDdmServices;

	strcpy(m_rgbTableName, rgbTableName);
  	strcpy(m_rgbKeyFieldName, rgbKeyFieldName);
	m_pKeyFieldValue 	= new char[cbKeyFieldValue];
	memcpy(m_pKeyFieldValue,pKeyFieldValue,cbKeyFieldValue);
 	m_cbKeyFieldValue 	= cbKeyFieldValue;
	strcpy(m_rgbFieldName, rgbFieldName);
	
	
	m_pbFieldValue 		= new char[cbFieldValue];
	memcpy(m_pbFieldValue,pbFieldValue, cbFieldValue);
	m_cbFieldValue 		= cbFieldValue;
	
	m_cRowsToModify		= cRowsToModify;
	m_pcRowsModifiedRet	= pcRowsModifiedRet;
	m_cbMaxRowID		= cbMaxRowID;

	m_pRowIDRet 		= pRowIDRet;
	m_pCallback 		= pCallback;
	m_pContext 			= pContext;
	return OS_DETAIL_STATUS_SUCCESS;
}	// TSModifyField::Initialize


/**********************************************************************/
// Send() - Packs up the payload, allocates a message and attaches the
// payload, and sends the msg to the table service specifying our own
// callback handler (HandleModifyFieldsReply) to unpack the reply.
//
/**********************************************************************/
void TSModifyField::Send()
{
	TRACE_ENTRY(TSModifyField::Send);

	STATUS					status;
	ModifyFieldsPayload		myModifyFieldsPayload;
	
	// Allocate a message to send to the table service.
	// [Be sure to leave payload space for replies.]
	Message *pMsg = new Message(TS_FUNCTION_MODIFY_FIELD,
                               sizeof(ModifyFieldsPayload));
	
	myModifyFieldsPayload.cRowsToModify = m_cRowsToModify;
	
	// Add the Table Name as SGL of the message
	pMsg->AddSgl(	MODIFY_FIELDS_MSG_TABLENAME_SGI,
					m_rgbTableName,
					strlen(m_rgbTableName)+1,
					SGL_SEND );

	// Add the Key Field Name as SGL of the message
	pMsg->AddSgl(	MODIFY_FIELDS_MSG_KEY_NAMES_SGI,
					m_rgbKeyFieldName,
					strlen(m_rgbKeyFieldName)+1,
					SGL_SEND );
	
	// Add the data of Key Field to the SGL
	pMsg->AddSgl(	MODIFY_FIELDS_MSG_KEY_VALUES_SGI,
					m_pKeyFieldValue,
					m_cbKeyFieldValue,
					SGL_SEND );

	// Add the modified field value as SGL of the message
	pMsg->AddSgl(	MODIFY_FIELDS_MSG_FIELD_NAMES_SGI,
					m_rgbFieldName,
					strlen(m_rgbFieldName)+1,
					SGL_SEND );

	// Add the data of Key Field to the SGL
	pMsg->AddSgl(	MODIFY_FIELDS_MSG_FIELD_VALUES_SGI,
					m_pbFieldValue,
					m_cbFieldValue,
					SGL_SEND );

	// Add the return buffer to the message as SGL
	pMsg->AddSgl(	MODIFY_FIELDS_REPLY_ROWIDS_SGI,
					NULL,					// m_pRowIDRet,
					NULL,					// m_cbMaxRowID,
					SGL_DYNAMIC_REPLY );

	// Call the Table Service to modify the field.
	status = DdmServices::Send(pMsg, NULL, REPLYCALLBACK(TSModifyField, HandleModifyFieldsReply));

	// If the send failed delete the message and object before returning
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		if( m_pDdmServices && m_pCallback)
			(m_pDdmServices->*m_pCallback)( m_pContext, status );
		TRACEF(TRACE_PTS, ("TSModifyField::Send status=%d\n", status));
		CheckFreeAndClear(m_pKeyFieldValue);
		CheckFreeAndClear(m_pbFieldValue);
		delete pMsg;
		delete this;
	}
}	// TSModifyField::Send()

/**************************************************************************/
// STATUS HandleModifyFieldsReply() - This is the function specified as the 
// callback in the Send method.  It deletes the message and calls the
// user's callback. 
//
// Parameters:
//	pMsg	- The replied message
/**************************************************************************/
STATUS TSModifyField::HandleModifyFieldsReply( MessageReply* pMsg )
{
	TRACE_ENTRY(TSModifyField::HandleModifyFieldsReply);

	ModifyFieldsPayload	*pModifyFieldsReplyPayload;
	STATUS				status = pMsg->DetailedStatusCode;
	void				*pRowIDRet;
	U32					cbMaxRowID = 0;

	// Get a pointer to the reply payload and cast it to our reply structure.
	pModifyFieldsReplyPayload = (ModifyFieldsPayload*)pMsg->GetPPayload();
	if (m_pcRowsModifiedRet != NULL)
		*m_pcRowsModifiedRet = pModifyFieldsReplyPayload->cRowsModifiedRet;

	if (m_pRowIDRet != NULL)
	{
	 	pMsg->GetSgl( MODIFY_FIELDS_REPLY_ROWIDS_SGI,
					  &pRowIDRet,					
					  &cbMaxRowID );				// m_cbMaxRowID,

 		if (cbMaxRowID > m_cbMaxRowID)				// only copy as many as will fit 
			cbMaxRowID = m_cbMaxRowID;

		memcpy (m_pRowIDRet, pRowIDRet, cbMaxRowID);	// copy into the user's buffer 
	}

	// Call the user's specified callback.
	if( m_pDdmServices && m_pCallback)
		status = (m_pDdmServices->*m_pCallback)( m_pContext, status );

	// Delete our message delete ourselves
	CheckFreeAndClear(m_pKeyFieldValue);
	CheckFreeAndClear(m_pbFieldValue);	
	delete pMsg;
	delete this;

	return status;
}	// TSModifyField::HandleModifyFieldsReply


/**************************************************************************/
// TSModifyBits Class Interface - Modify the bits of a 32 bit field with
//		an OR, AND, or XOR operation.
//
/**************************************************************************/
// STATUS Initialize()	--> Store the operation parameters in the object.
//
// Parameters:
//	pDdmServices	- Pointer to client's DDM.  Used to access Send().
//	rgbTableName	- Null terminated name of the table.
//	opField			- operation to perform:  AND, OR, XOR
//					  OpAndBits, OpOrBits, 	OpXorBits (defined in ptscommon.h)

//	rgbKeyFieldName	- The key field name used to identify the row to modify
//	pKeyFieldValue	- The key field value used to search for the row to modify.
//	cbKeyFieldValue - The size of the key field value
//	rgbFieldName	- The name of the field to modify.
//	pbFieldMask		- The value to 'OR, AND, XOR' with the field.
//	cRowsToModify	- The number of rows to modify;  '0' means all that match
//	prgFieldIdsRet	- Pointer to array of returned struct that identifies the modified rows:.
//						typedef struct {
//							rowID	rowModified;
//							U32		fieldRet;
//						} modField;
//	cbMax			- size of prgFieldIdsRet
	
//	pCallback		- Pointer to static callback function called upon reply.
//	pContext		- Context pointer that is passed to the Callback method
//
/**************************************************************************/
STATUS TSModifyBits::Initialize( DdmServices*	pDdmServices,
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
								void*			pContext )
{
	TRACE_ENTRY(TSModifyField::Initialize);

	// Initialize our base class.
	if ((pDdmServices == NULL) ||
		(rgbTableName == NULL) ||
		(rgbKeyFieldName == NULL) ||
		(rgbFieldName == NULL) ||
		(pbFieldMask == NULL))
		return ercBadParameter; 

	SetParentDdm(pDdmServices);
	m_pDdmServices 		= pDdmServices;

	strcpy(m_rgbTableName, rgbTableName);
 	
 	m_opFlag = opFlag;
 	strcpy(m_rgbKeyFieldName, rgbKeyFieldName);
 	
 	m_pKeyFieldValue = new char[cbKeyFieldValue];
	memcpy(m_pKeyFieldValue,pKeyFieldValue,cbKeyFieldValue);
	
	
	
 	strcpy(m_rgbFieldName, rgbFieldName);
	m_cbKeyFieldValue 	= cbKeyFieldValue;

	m_pbFieldValue 		= pbFieldMask;
	m_prgFieldIdsRet	= prgFieldIdsRet;
	m_cbrgFieldIds		= cbMax;
	m_cRowsToModify		= cRowsToModify;
	m_pcRowsModRet		= pcRowsModRet;
	m_pCallback 		= pCallback;
	m_pContext 			= pContext;

	return OS_DETAIL_STATUS_SUCCESS;
}	// TSModifyBits::Initialize


/**********************************************************************/
// Send() - Packs up the payload, allocates a message and attaches the
// payload, and sends the msg to the table service specifying our own
// callback handler (HandleModifyBitsReply) to unpack the reply.
//
/**********************************************************************/

void TSModifyBits::Send()
{
	TRACE_ENTRY(TSModifyBits::Send);

	STATUS						status;
	ModifyBitsPayload			myModifyBitsPayload;

	// Allocate a message to send to the table service.
	// [Be sure to leave payload space for replies.]
	Message *pMsg = new Message(TS_FUNCTION_MODIFY_BITS,
                               sizeof(ModifyBitsPayload));
	
	myModifyBitsPayload.opFlag = m_opFlag;
	myModifyBitsPayload.cRowsToModify = m_cRowsToModify;
	
 	pMsg->AddPayload( &myModifyBitsPayload, sizeof(myModifyBitsPayload));

	// Add the Table Name as SGL of the message
	pMsg->AddSgl(	MODIFY_BITS_MSG_TABLENAME_SGI,
					m_rgbTableName,
					strlen(m_rgbTableName)+1,
					SGL_SEND );

	// Add the Key Field Name as SGL of the message
	pMsg->AddSgl(	MODIFY_BITS_MSG_KEY_NAME_SGI,
					m_rgbKeyFieldName,
					strlen(m_rgbKeyFieldName)+1,
					SGL_SEND );
	
	// Add the data of Key Field to the SGL
	pMsg->AddSgl(	MODIFY_BITS_MSG_KEY_VALUE_SGI,
					m_pKeyFieldValue,
					m_cbKeyFieldValue,
					SGL_SEND );

	// Add the modified field value as SGL of the message
	pMsg->AddSgl(	MODIFY_BITS_MSG_FIELD_NAME_SGI,
					m_rgbFieldName,
					strlen(m_rgbFieldName)+1,
					SGL_SEND );

	// Add the data of Key Field to the SGL
	pMsg->AddSgl(	MODIFY_BITS_MSG_FIELD_MASK_SGI,
					m_pbFieldValue,
					sizeof (U32),
					SGL_SEND );

	// Add the return buffer to the message as SGL
	pMsg->AddSgl(	MODIFY_BITS_REPLY_ROWIDS_SGI,
					NULL,					// m_prgFieldIdsRet
					NULL,					// m_cbrgFieldIds 
					SGL_DYNAMIC_REPLY );


	// Call the Table Service to modify the field.
	status = DdmServices::Send(pMsg, NULL, REPLYCALLBACK(TSModifyBits, HandleModifyBitsReply));

	// If the send failed delete the message and object before returning
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		if( m_pDdmServices && m_pCallback)
			(m_pDdmServices->*m_pCallback)( m_pContext, status );
		TRACEF(TRACE_PTS, ("TSModifyBits::Send status=%d\n", status));
		CheckFreeAndClear(m_pKeyFieldValue);
		delete pMsg;
		delete this;
	}
}	// TSModifyBits::Send()

/**************************************************************************/
// STATUS HandleModifyBitsReply() - This is the function specified as the 
// callback in the Send method.  It deletes the message and calls the
// user's callback. 
//
// Parameters:
//	pMsg	- The replied message
/**************************************************************************/
STATUS TSModifyBits::HandleModifyBitsReply( MessageReply* pMsg )
{
	TRACE_ENTRY(TSModifyBits::HandleModifyBitsReply);

 	ModifyBitsPayload			*pModifyBitsPayload;
	STATUS						status = pMsg->DetailedStatusCode;
	void						*prgFieldIdsRet;
	U32							cbrgFieldIds = 0; 

	// Get a pointer to the payload
	pModifyBitsPayload = (ModifyBitsPayload*)pMsg->GetPPayload();
	 
	if (m_pcRowsModRet != NULL)
		*m_pcRowsModRet = pModifyBitsPayload->cRowsModifiedRet;

	if (m_prgFieldIdsRet != NULL)
	{
	 	pMsg->GetSgl( MODIFY_BITS_REPLY_ROWIDS_SGI,
					  &prgFieldIdsRet,
					  &cbrgFieldIds );		

 		if (cbrgFieldIds > m_cbrgFieldIds)		// only copy as many as will fit 
			cbrgFieldIds = m_cbrgFieldIds;

		memcpy (m_prgFieldIdsRet, prgFieldIdsRet, cbrgFieldIds);	// copy into the user's buffer 
	}

	// Call the user's specified callback.
	if( m_pDdmServices && m_pCallback)
		status = (m_pDdmServices->*m_pCallback)( m_pContext, status );

	// Delete our message delete ourselves
	CheckFreeAndClear(m_pKeyFieldValue);	
	delete pMsg;
	delete this;

	return status;
}	// TSModifyField::HandleModifyBitsReply


/**************************************************************************/
// TSQuerySetRID Class Interface - Query or Set RID field.
//
/**************************************************************************/
// STATUS Initialize()	--> Store the operation parameters in the object.
//
// Parameters:
//	pDdmServices	- Pointer to client's DDM.  Used to access Send().
//	rgbTableName	- Null terminated name of the table.
//  opRowId			- Operation to be performed
//	pRowID			- Pointer to rowID .
//	pCallback		- Pointer to static callback function called upon reply.
//	pContext		- Pointer to the user reply callback routine
//
/**************************************************************************/
STATUS TSQuerySetRID::Initialize ( DdmServices	*pDdmServices,
								String64		rgbTableName,
								fieldOpType		opRowId,
								rowID			*pRowID,
								pTSCallback_t	pCallback,
								void			*pContext )
{
	// Check the parameters
	if ((pDdmServices == NULL) || (rgbTableName == NULL) || (opRowId == NULL) ||(pRowID == NULL))
		return ercBadParameter; 

	// Initialize our base class.
	SetParentDdm(pDdmServices);
	
	m_pDdmServices 		= pDdmServices;
	strcpy(m_rgbTableName, rgbTableName);
	m_opOnRid			= opRowId;
	m_pRowID	 		= pRowID;
	m_pCallback 		= pCallback;
	m_pContext 			= pContext;
	return OS_DETAIL_STATUS_SUCCESS;
}	// TSQuerySetRID::Initialize

/**********************************************************************/
// Send() - Packs up the payload, allocates a message and attaches the
// payload, and sends the msg to the table service specifying our own
// callback handler to unpack the reply.
//
/**********************************************************************/
void	TSQuerySetRID::Send()
{
	STATUS						status;
	QuerySetRIDPayload			myQuerySetPayload;
	
	// Allocate a message to send to the table service.
	Message *pMsg = new Message(TS_FUNCTION_QUERY_SET_RID,
                               sizeof(QuerySetRIDPayload));

	myQuerySetPayload.opRID = m_opOnRid;
	if (m_opOnRid == OpSetRID) 
		myQuerySetPayload.rowId = *m_pRowID;
			
	// Add payload structure to the message.

	pMsg->AddPayload( &myQuerySetPayload, sizeof(myQuerySetPayload));
	
	// Add the Table Name as SGL of the message
	pMsg->AddSgl( QUERY_SET_RID_MSG_TABLENAME_SGI,
					m_rgbTableName,
					strlen(m_rgbTableName)+1,
					SGL_SEND );

	// Call the Table Service 
	status = DdmServices::Send(pMsg, NULL, REPLYCALLBACK(TSQuerySetRID, HandleQuerySetRIDReply));

	// If the send failed delete the message before returning
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		if	(m_pCallback != NULL)
			(m_pDdmServices->*m_pCallback)( m_pContext, status );

		TRACEF(TRACE_PTS, ("TSQuerySetRID::Send status=%d\n", status));
		delete pMsg;
		delete this;
	}
}	// TSQuerySetRID::Send()


/**************************************************************************/
// STATUS HandleQuerySetRID() - This is the function specified as the 
// callback in the Send method.  It deletes the message and calls the
// user's callback. 
//
// Parameters:
//	pMsg	- The replied message
/**************************************************************************/

STATUS TSQuerySetRID::HandleQuerySetRIDReply( MessageReply* pMsg )
{

	TRACE_ENTRY(TSModifyField::HandleModifyFieldsReply);

	QuerySetRIDPayload			*pQuerySetRIDPayload;
	STATUS						status = pMsg->DetailedStatusCode;

	// Get a pointer to the payload
	pQuerySetRIDPayload = (QuerySetRIDPayload*)pMsg->GetPPayload();
	 
	// Retrieve the rowID from the reply.
	if ((m_opOnRid == OpQueryRID) &&  (m_pRowID != NULL))
		*m_pRowID = pQuerySetRIDPayload->rowId;
	
	
	// Call the user's specified callback.
	if( m_pCallback != NULL)
		status = (m_pDdmServices->*m_pCallback)( m_pContext, status );

	// Delete our message delete ourselves
	delete pMsg;
	delete this;
	return status;

}	// TSQuerySetRID::HandleQuerySetRID


/**************************************************************************/
// TSTestSetField Class Interface - Test the contents of a field and set.
//
/**************************************************************************/
// STATUS Initialize()	--> Store the operation parameters in the object.
//
// Parameters:
//	pDdmServices	- Pointer to client's DDM.  Used to access Send().
//  opFlag			- Operation flag:  'test and set', or 'clear'
//	rgbTableName	- Null terminated name of the table.
//	rgbKeyFieldName	- The key field name used to identify the row to modify
//	pKeyFieldValue	- The key field value used to search for the row to modify.
//	cbKeyFieldValue	- the size of the key field value in bytes.
//	rgbFieldName	- The name of the field to modify.
//	pTestFlagRet	- Pointer to test flag returned 
//	pCallback		- Pointer to static callback function called upon reply.
//	pContext		- Pointer to the user reply callback routine
//
/**************************************************************************/
STATUS TSTestSetOrClearField::Initialize(	DdmServices	*pDdmServices,
								String64		rgbTableName,
								fieldOpType		opFlag,
								String64		rgbKeyFieldName,
								void			*pKeyFieldValue,
								U32				cbKeyFieldValue,
								String64		rgbFieldName,
								BOOL			*pTestFlagRet,
								pTSCallback_t	pCallback,
								void 			*pContext )
{
	// Check the parameters
	if ((pDdmServices == NULL) || (rgbTableName == NULL) || (rgbKeyFieldName == NULL) ||
		(pKeyFieldValue == NULL) || (cbKeyFieldValue == 0) || (rgbFieldName == NULL))
	
		return ercBadParameter; 

	// Initialize our base class.
	SetParentDdm(pDdmServices);
	
	m_pDdmServices 		= pDdmServices;
	strcpy(m_rgbTableName, rgbTableName);
	m_opFlag = opFlag;
	strcpy(m_rgbKeyFieldName, rgbKeyFieldName);
	m_pKeyFieldValue 	= pKeyFieldValue;
	m_cbKeyFieldValue 	= cbKeyFieldValue;
	strcpy(m_rgbFieldName, rgbFieldName);
	m_pTestFlagRet 		= pTestFlagRet;
	m_pCallback 		= pCallback;
	m_pContext 			= pContext;
	return OS_DETAIL_STATUS_SUCCESS;
}	// TSTestSetClearField::Initialize

/**********************************************************************/
// Send() - Packs up the payload, allocates a message and attaches the
// payload, and sends the msg to the table service specifying our own
// callback handler (HandleTestSetFieldsReply) to unpack the reply.
//
/**********************************************************************/
void TSTestSetOrClearField::Send()
{
	STATUS						status;
	TestAndSetFieldPayload		myTestSetPayload;

	// Allocate a message to send to the table service.
	Message *pMsg = new Message(TS_FUNCTION_TEST_SET_FIELD,
                               sizeof(TestAndSetFieldPayload));
	
	// Add payload structure to the message.

	myTestSetPayload.opSetOrClear = m_opFlag;
	pMsg->AddPayload( &myTestSetPayload, sizeof(myTestSetPayload));
	
	// Add the Table Name as SGL of the message
	pMsg->AddSgl(	TEST_SET_FIELD_MSG_TABLENAME_SGI,
					m_rgbTableName,
					strlen(m_rgbTableName)+1,
					SGL_SEND );

	// Add the Key Field Name as SGL of the message
	pMsg->AddSgl(	TEST_SET_FIELD_MSG_KEY_NAME_SGI,
					m_rgbKeyFieldName,
					strlen(m_rgbKeyFieldName)+1,
					SGL_SEND );
	
	// Add the data of Key Field to the SGL
	pMsg->AddSgl(	TEST_SET_FIELD_MSG_KEY_VALUE_SGI,
					m_pKeyFieldValue,
					m_cbKeyFieldValue,
					SGL_SEND );

	// Add the modified field value as SGL of the message
	pMsg->AddSgl(	TEST_SET_FIELD_MSG_FIELD_NAME_SGI,
					m_rgbFieldName,
					strlen(m_rgbFieldName)+1,
					SGL_SEND );
   
	// Call the Table Service to modify the field.
	status = DdmServices::Send(pMsg, NULL,  REPLYCALLBACK(TSTestSetOrClearField, HandleTestSetFieldReply));

	// If the send failed delete the message before returning to the user's callback
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		if	(m_pCallback != NULL)
			(m_pDdmServices->*m_pCallback)( m_pContext, status );

		delete pMsg;
		delete this;
	}

}	// TSTestSetField::Send()

/**************************************************************************/
// STATUS HandleTestSetFieldReply() - This is the function specified as the 
// callback in the Send method.  It deletes the message and calls the
// user's callback. 
//
// Parameters:
//	pMsg	- The replied message
/**************************************************************************/
STATUS TSTestSetOrClearField::HandleTestSetFieldReply( MessageReply* pMsg )
{
	TestAndSetFieldPayload		*pTestSetFieldReplyPayload;
	STATUS						status = pMsg->DetailedStatusCode;

	// Get a pointer to the reply payload

	pTestSetFieldReplyPayload = (TestAndSetFieldPayload*)pMsg->GetPPayload();
	 
	// Retrieve the test flag

	if (m_pTestFlagRet)
		*m_pTestFlagRet = pTestSetFieldReplyPayload->fTestRet;
	
	// Call the user's specified callback.
	if( m_pCallback != NULL)
		status = (m_pDdmServices->*m_pCallback)( m_pContext, status );

	// Delete our message delete ourselves
	delete pMsg;
	delete this;

	return status;
}	// TSTestSetField::HandleDeleteRowReply


// end of file