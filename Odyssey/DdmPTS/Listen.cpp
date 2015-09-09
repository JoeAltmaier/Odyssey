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
// This file is the definition of an object interface to the Listen 
// API of the Table Service.
// 
// $Log: /Gemini/Odyssey/DdmPTS/Listen.cpp $
// 
// 33    10/28/99 9:22a Sgavarre
// Fixes to TSTimedListen.
// 
// 32    10/27/99 11:02a Sgavarre
// Don't free returned listen type pointer - it points to data in the
// payload.
// 
// 31    10/14/99 3:17p Jlane
// Timed listen fixes and debugging.
// 
// 30    10/12/99 10:02a Jlane
// Remove unused route by slot code.  Use strlen()+1for name SGLs
// throughout.
// 
// 29    10/11/99 6:45p Dpatel
// strlen should be strlen+1
// 
// 28    9/06/99 7:07p Jlane
// Added TSTimedListen.
// 
// 27    8/13/99 5:46p Sgavarre
// Change listen rowRet SGL to be dynamic;  moved listentypeREt to
// payload;  use initial flag in payload instead of keeping flag in
// object.
// 
// 26    7/26/99 8:20p Agusev
// Fixed a type in Send()
// 
// 25    7/26/99 7:09p Hdo
// Undo error checking on prgbRowKeyFieldValue, cbRowKeyFieldValue,
// prgbFieldValue, cbFieldValue
// 
// 24    7/22/99 3:11p Hdo
// 
// 23    6/15/99 7:11p Sgavarre
// return the listen type in the payload;  listen type returned is now the
// same flags used to define listen, ie listenOnModifyOneRowAnyField
// 
// 22    6/08/99 3:47p Sgavarre
// remove reply payloads;  fix addsglparam for cbrowkeyfield;  
// make changes to listen reply to reflect the listenerId being in payload
// and not in an sgl.
// 
// 21    6/05/99 12:25a Ewedel
// Allow listen to work without requiring caller to supply a
// pListenerIDRet formal to Initialize().  Also cleaned up some callback
// cast warnings.
// 
// 20    5/11/99 10:53a Jhatwich
// win32
// 
// 19    5/07/99 3:02p Jlane
// In HandleListenreply, initialize m_cbTableDataRet to 0 before calling
// GetSgfl.  In the case where no table data is being returned non-zero
// values in m_cbTableDataRet cause the Sgl buffer to be alloced in
// GetSgl.  Doh!
// 
// 18    4/21/99 9:23a Jlane
// Added new seperate parameters for key field name and value pb/cb
// distinct from modified field name and pb/cb.
// 
// 17    4/16/99 6:14p Jlane
// Modifications made debugging Listen and enhancing for Dynamic Reply SGL
// items.
// 
// 16    4/07/99 5:17p Ewedel
// Changed to use new payload size argument to Message() constructor.
// 
// 15    4/03/99 6:32p Jlane
// Multiple changes to interface, messaage payloads and message handlers
// to avoid having to add reply payloads as this destroys SGL items.  We
// do this by allocating space for reply data in the msg payload.
// 
// 14    4/02/99 1:52p Jlane
// #if'd out workaround code to send messages to slots since interslot
// route by function wasn't working.
// 
// 13    3/27/99 1:43p Sgavarre
// change listen enums to U32
// 
// 12    3/21/99 11:30a Jlane
// Added parameters for returning the table, and taking a field value.
// Modified  code to deal with new Transport Multiple Reply buffer
// allocation.
//
// Update Log: 
// 11/17/98	JFL	Created.
// 1/18/99	JFL	Compiled (except for calls to send pending new DdmServices class)
//				and checked in.
// 02/27/99 JFL	Added support for returned records.
// 02/28/99 JFL Changed reference params to pointers.
// 03/27/99 sg	Change references to listentype and replytype
//
/*************************************************************************/
#include <cstring>

#include "Listen.h"
#include "Ddm.h"
#include "CtUtils.h"
#include "CtEvent.h"

/**************************************************************************/
// TSListen - Ask for replies when table data is modified.
//
// Parameters:
//	pDdmServices	- Pointer to Caller's Ddm.  Used for reply dispatch.
//	ListenType 		- The type of operation for which you want to listen.
//					  NOTE: See ListenTypeEnum declaration in TableMsgs.h
//	prgbTableName	- The name of the table on which you want to listen. 
//	rgbFieldName	- The name of the field on which you want to listen.
//					  NOTE: rgbFieldName is only used for some ListenTypes. 
//  ReplyMode		- 0 = reply continuously 1 = reply once only.
//  pListenerIDRet	- Pointer to returned Listener ID number.
//					  NOTE: the ListenerID is used to in the call stop listening.
//	pCallback		- Caller's function to be executed upon listen reply.
/**************************************************************************/
STATUS TSListen::Initialize(	
						DdmServices*		pDdmServices,
						U32					ListenType,
						String64			prgbTableName,
						String64			prgbRowKeyFieldName,
						void*				prgbRowKeyFieldValue,
						U32					cbRowKeyFieldValue,
						String64			prgbFieldName,
						void*				prgbFieldValue,
						U32					cbFieldValue,
						U32					ReplyMode,
						void**				ppTableDataRet,
						U32*				pcbTableDataRet,
						U32*				pListenerIDRet,
						U32**				ppListenTypeRet,
						void**				ppModifiedRecordRet,
						U32*				pcbModifiedRecordRet,
						pTSCallback_t		pCallback,
						void*				pContext
					 )
{
	// Initialize our base class.
	if( pDdmServices == NULL )
		return ercBadParameter;
	SetParentDdm(pDdmServices);

	// Stash parameters in member variables.
	m_pDdmServices = pDdmServices;
	m_ListenType = ListenType;

	if (prgbTableName != NULL)
		strcpy(m_rgbTableName, prgbTableName);
	else
		m_rgbTableName[0] = (char)NULL;

	if (prgbRowKeyFieldName != NULL)
		strcpy(m_rgbRowKeyFieldName, prgbRowKeyFieldName);
	else
		m_rgbRowKeyFieldName[0] = (char)NULL;

	m_prgbRowKeyFieldValue = prgbRowKeyFieldValue;
   	m_cbRowKeyFieldValue = cbRowKeyFieldValue;

	if (prgbFieldName != NULL)
		strcpy(m_rgbFieldName, prgbFieldName);
	else
		m_rgbFieldName[0] = (char)NULL;
  	m_prgbFieldValue = prgbFieldValue;
 	m_cbFieldValue = cbFieldValue;

	m_ReplyMode = ReplyMode;
	m_ppTableDataRet = ppTableDataRet;
	m_pcbTableDataRet = pcbTableDataRet;
	m_pListenerIDRet = pListenerIDRet;
	m_ppListenTypeRet = ppListenTypeRet;
	m_ppModifiedRecordRet = ppModifiedRecordRet;
	m_pcbModifiedRecordRet = pcbModifiedRecordRet;
//	m_cbModifiedRecordRet = *m_pcbModifiedRecordRet;	 // what ???
	m_pCallback = pCallback;
	m_pContext = pContext;

 	return OS_DETAIL_STATUS_SUCCESS;
}



/**********************************************************************/
// Send - Packs up the payload, allocates a message and attaches the
// payload, and sends the msg to the table service specifying our own
// callback handler (DispatchListenReply) to unpack the reply.
/**********************************************************************/
void TSListen::Send()
{
	ListenPayload	myListenPayload;
	STATUS			status;

	// Allocate a new private (listen) message to send to the table service.
	Message *pMsg=new Message(TS_FUNCTION_LISTEN,
                             sizeof(ListenPayload));

	// Copy the values into message payload and SGL items
	myListenPayload.listenType = m_ListenType;	// Type of operation for which you want to listen.
	if (m_ppTableDataRet)						// If the user asked for the table...
		// Or in the ReplyFirstWithTable Flag into the user's spec=d flags.
		m_ReplyMode |=  ReplyFirstWithTable;
	myListenPayload.replyMode = m_ReplyMode;	// 0=>Reply multiple 1 = Reply once only.

	// Add the payload structure to the message.
	pMsg->AddPayload( &myListenPayload, sizeof(myListenPayload));

	// Add the Table name as an SGL item
	pMsg->AddSgl(	LISTEN_MSG_TABLENAME_SGI,
					m_rgbTableName,
					strlen(m_rgbTableName)+1,
					SGL_SEND);

	// Add the Row Key Field name as an SGL item
	pMsg->AddSgl(	LISTEN_MSG_ROWKEY_FIELDNAME_SGI,
					m_rgbRowKeyFieldName,
					strlen(m_rgbRowKeyFieldName)+1,
					SGL_SEND);

	// Add the Row Key Field value as an SGL item
	pMsg->AddSgl(	LISTEN_MSG_ROWKEY_FIELDVALUE_SGI,
					m_prgbRowKeyFieldValue,
					m_cbRowKeyFieldValue,
					SGL_SEND);

	// Add the Field name as an SGL item
	pMsg->AddSgl(	LISTEN_MSG_FIELDNAME_SGI,
					m_rgbFieldName,
					strlen(m_rgbFieldName)+1,
					SGL_SEND);

	// Add the Field value as an SGL item
	pMsg->AddSgl(	LISTEN_MSG_FIELDVALUE_SGI,
					m_prgbFieldValue,
					m_cbFieldValue,
					SGL_SEND);

	// Only add an SGL item for the returned Table data if the user wants it.
	if (m_ppTableDataRet)
	{
		// Add the return Table data SGL item
		// NULL pb indicates data buffer to be alloc'd by transport.
		// NUll cb will tell transport to get allocate size when the
		// PTS calls GetSGL.
		pMsg->AddSgl(	LISTEN_REPLY_PTABLE_SGI,
						NULL,
						NULL,
						SGL_DYNAMIC_REPLY
					);
					
	}

	// Add the return ID and optional row data buffer as an SGL item
	// NULL pb indicates data buffer to be alloc'd by transport.
	pMsg->AddSgl(	LISTEN_REPLY_ID_AND_OPT_ROW_SGI,
					NULL,								// &m_pModifiedRecordRet,
					NULL, 
					SGL_DYNAMIC_REPLY);  

	
	// Call the Table Service to listen on the table. Sepcify our callback handler.
	status = DdmServices::Send( pMsg, this,
                                  REPLYCALLBACK(TSListen, HandleListenReply) );

	// If the send failed delete the message before returning.
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		if( m_pDdmServices && m_pCallback)
			(m_pDdmServices->*m_pCallback)( m_pContext, status );
		delete pMsg;
		delete this;
	}
}	// TSListen::Send()


/**********************************************************************/
// HandleListenReply - This is the object's listen reply callback handler.
// It unpacks the Table Service TSListen reply payload.
// There are several types of replies that this handler will see but 
// they all have the same Reply payload structure a ListenerID and a rowID.
// The types of replies, how they are distinguished and what this code
// does with them are as follows:
//	1)	The first reply which returns the Listener ID and a null rowID.
//		This code just saves the ListenerID in the object.
//	2) 	Subsequent TSListen Replies which return the ListenerID and a rowID.
//		This code calls the user's specified callback function with the rowID.
//	3)	The Last Reply which returns a null ListenerID and a null rowID.
//		In this case this code deletes the object and then calls the 
//		user's specified callback function passing a null rowID. 
/**********************************************************************/
STATUS TSListen::HandleListenReply(MessageReply* pMsg)
{
	STATUS			status = pMsg->DetailedStatusCode;
	BOOL			fDeleteThis = false;
	ListenPayload	*pListenPayload;

	// Get a pointer to the reply payload and cast it to our reply structure.
	pListenPayload = (ListenPayload*)pMsg->GetPPayload();

	m_ListenerIDRet = pListenPayload->listenerIDRet;
	if (m_ppListenTypeRet)
		*m_ppListenTypeRet = &(pListenPayload->listenTypeRet);

		if ((m_ListenerIDRet != 0) && (pListenPayload->listenTypeRet == ListenInitialReply))
		{
		
			// If the client requested the table then handle that here.
			if (m_ppTableDataRet != NULL)
			{
				// Clear the return count lest we cause creation of a reply Sgl when none was intended.
				m_cbTableDataRet = 0;
				
				// Get the return record buffer and size in our instance variables.
				pMsg->GetSgl(	LISTEN_REPLY_PTABLE_SGI,
								&m_pTableDataRet,
								&m_cbTableDataRet
							);
	
				// Allocate a buffer for the client and copy the data there.
				*m_ppTableDataRet = new char[m_cbTableDataRet];
				memcpy(*m_ppTableDataRet, m_pTableDataRet, m_cbTableDataRet);
			}
			
			// return the size of the table if the client so desired.
			if (m_pcbTableDataRet != NULL)
				*m_pcbTableDataRet = m_cbTableDataRet;
		}
		else
		{
			// In all other cases, retreive the returned data and then 
			// call the user's callback.
			 
			// Get the return record buffer if the user wanted it.
			if ((m_ppModifiedRecordRet != NULL) && (m_pcbModifiedRecordRet))
				pMsg->GetSgl(	LISTEN_REPLY_ID_AND_OPT_ROW_SGI,
								m_ppModifiedRecordRet,
								m_pcbModifiedRecordRet);
		}
		
	// Return the Listener ID to the caller.
	if (m_pListenerIDRet != NULL)
	{
		*m_pListenerIDRet = m_ListenerIDRet;
	}	

	// Call the user's callback.
	if( m_pDdmServices && m_pCallback)
		status = (m_pDdmServices->*m_pCallback)( m_pContext, status );
		
	delete pMsg;
	
	fDeleteThis = ((m_ReplyMode & ReplyOnceOnly) == ReplyOnceOnly) && pMsg->IsLast();
	if (fDeleteThis)
		delete this;
		
	return status;
}	// TSListen::HandleListenReply


						
/**************************************************************************/
// Stop - Stops a listen operation previously begun with Send
//
// Parameters:
/**************************************************************************/
STATUS TSListen::Stop()
{
	StopListenPayload	myStopListeningPayload;
	STATUS				status;

	if (m_pListenerIDRet == NULL)
		return ercNoSuchListener;
		
	// Copy the values from the stack into message payload.
	myStopListeningPayload.listenerID = *m_pListenerIDRet;

	// Allocate a new private (define table) message to send to the table service.
	   // [Leave payload space for larger of stop-listen request/reply payloads.]
	Message *pMsg=new Message(TS_FUNCTION_STOP_LISTENING,
	                          sizeof(StopListenPayload));

	// Add the payload structure to the message.
	pMsg->AddPayload( &myStopListeningPayload, sizeof(myStopListeningPayload));

	// Call the Table Service to listen on the table.  Pass our object pointer as pContext
	// and specify our own callback handler to be called upon reply.
	status = DdmServices::Send(pMsg, this,
	                           REPLYCALLBACK(TSListen, HandleStopListenReply));

	// If the send failed delete the message before returning.
	if (status != OS_DETAIL_STATUS_SUCCESS)
		delete pMsg;

	return status;
}	// TSMessenger::TSListenTable

						
/**********************************************************************/
// HandleListenReply - This is the object's Stop TSListen reply callback
// handler. 
/**********************************************************************/
STATUS TSListen::HandleStopListenReply(MessageReply* pMsg)
{
STATUS				status = pMsg->DetailedStatusCode;

	// Delete the message
	delete pMsg;

	// Delete this object?
	delete this;

	return status;
}	// TSListen::HandleListenReply




//  TSTimedListen::Initialize()
//
//  Description:
//    Initialize all parameters necessary to start a timer and a Listen
//	operation.
//
//  Inputs:
//
//	pDdmServices	- Pointer to Caller's Ddm.  Used for reply dispatch.
//	ListenType 		- The type of operation for which you want to listen.
//					  NOTE: See ListenTypeEnum declaration in TableMsgs.h
//	prgbTableName	- The name of the table on which you want to listen. 
//	rgbFieldName	- The name of the field on which you want to listen.
//					  NOTE: rgbFieldName is only used for some ListenTypes. 
//  ReplyMode		- 0 = reply continuously 1 = reply once only.
//  pListenerIDRet	- Pointer to returned Listener ID number.
//					  NOTE: the ListenerID is used to in the call stop listening.
//	pCallback		- Caller's function to be executed upon listen reply.
//
STATUS TSTimedListen::Initialize(	
						DdmServices*		pDdmServices,
						U32					ListenType,
						String64			prgbTableName,
						String64			prgbRowKeyFieldName,
						void*				prgbRowKeyFieldValue,
						U32					cbRowKeyFieldValue,
						String64			prgbFieldName,
						void*				prgbFieldValue,
						U32					cbFieldValue,
						U32					ReplyMode,
						void**				ppTableDataRet,
						U32*				pcbTableDataRet,
						U32*				pListenerIDRet,
						U32**				ppListenTypeRet,
						void**				ppModifiedRecordRet,
						U32*				pcbModifiedRecordRet,
						pTSCallback_t		pCallback,
						U32					iTimeOut,
						void*				pContext
					 )
{
STATUS	status;

	// Initialize our base class.
	if( pDdmServices == NULL )
		return ercBadParameter;
	SetParentDdm(pDdmServices);

	// Even if the caller doesn't want listen reply type we do need it.
	// If no address for the listen reply type was supplied provide one.
	m_ppListenReplyType = ppListenTypeRet
		? ppListenTypeRet
		: &m_pListenReplyType;
	m_ReplyMode = ReplyMode;
			
	m_pListen = new TSListen;
	// Initialize our TSListen class.
	status = m_pListen->Initialize(	
		this,										// DdmServices*	pDdmServices
		ListenType,									// U32	ListenType,
 		prgbTableName,								// String64 prgbTableName,
		prgbRowKeyFieldName,						// String64 prgbRowKeyFieldName,
		prgbRowKeyFieldValue,						// void* prgbRowKeyFieldValue,
		cbRowKeyFieldValue,							// U32 cbRowKeyFieldValue,
		prgbFieldName,								// String64 prgbFieldName,
		prgbFieldValue,								// void* prgbFieldValue,
		cbFieldValue,								// U32 cbFieldValue,
		m_ReplyMode,								// U32 ReplyMode,
		ppTableDataRet,								// void** ppTableDataRet,
		pcbTableDataRet,							// U32* pcbTableDataRet,
		pListenerIDRet,								// U32* pListenerIDRet,
		m_ppListenReplyType,						// U32** ppListenTypeRet,
		ppModifiedRecordRet,						// void** ppModifiedRecordRet,
		pcbModifiedRecordRet,						// U32* pcbModifiedRecordRet,
		TSCALLBACK(TSTimedListen, ListenCallback),	// pTSCallback_t pCallback,
		pContext									// void* pContext
	);
	
	// Initialize our instance data:
	m_pDdmServices = pDdmServices;
	m_pUserCallback = pCallback;

	m_iTimeOut = iTimeOut;
	m_pListenReplyType = NULL;
	
	return status;
}

	
//  STATUS TSTimedListen::Send()
//
//  Description:
//    Start a Listen and a timer operation.
//	operation.
void TSTimedListen::Send()
{
STATUS status;

	// Remember that we have been called.
	m_fSendWasCalled = true;
	
	m_pListen->Send();
	
	// We also need to start a timer so we won't listen forever.
	// Allocate a message we'll send to the timer service to start a timer.
	// Initialize the payload structure for a timer with the specified
	// delay.
	m_pStartTimerMsg = new RqOsTimerStart(m_iTimeOut, 0);
	//									 ((m_ReplyMode & ReplyOnceOnly) == ReplyOnceOnly) ? 0 : m_iTimeOut);

	// Send the message off to the timer service.
	status = DdmServices::Send(m_pStartTimerMsg, NULL, REPLYCALLBACK(TSTimedListen, ListenTimerCallback));

	// Initialize our state data.
	m_fListenWasDetected = false;
	m_fListenTimerExpired = false;
	m_fSendWasCalled = false;
	m_fStopWasCalled = false;

//	return status;
}	// TSTimedListen::Send()



//  TSTimedListen::ListenCallback()
//
//  Description:
//    This method is called back when at several possible distinct times:
//	1) When Listen is initiated we are called back with our ListenerID,
//	2) When the specified Listen condition is detected by the PTS and
//	3) At StopListen time (if ListenReplyType was not ReplyOnceOnly).
//
//
//  Inputs:
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS TSTimedListen::ListenCallback(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)
U32	ListenReplyType = **m_ppListenReplyType;

	// If this is the initial Listen Reply then just call the user's callback.
	if (ListenReplyType == ListenInitialReply)
		status = (m_pDdmServices->*m_pUserCallback)( m_pContext, status );
	else
	// If this is not the final Listen Reply check if the timer has expired.
	if (ListenReplyType != ListenReturnedOnStopListen)
	{
		// If the timer has not gone off then we have an actual Listen Reply.
		m_fListenWasDetected = !m_fListenTimerExpired;
		
		// If we have a non-timed-out listen reply then...
		if (m_fListenWasDetected)
		{
			// If we were initialized in ReplyOnceOnly mode then the Listen will
			// have automatically deleted itself.
			BOOL	fReplyOnceOnly = ((m_ReplyMode & ReplyOnceOnly) == ReplyOnceOnly);
			if (fReplyOnceOnly) 
				m_pListen = NULL;
				
			// If we were initialized to ReplyOnceOnly OR
			// If we were initialized to ReplyContinuous and Stop hasn't been called 
			// then we need to stop our timer.
			if (fReplyOnceOnly || 
				(((m_ReplyMode & ReplyOnceOnly) == ReplyContinuous) && !m_fStopWasCalled)
			   )
			{
				// Send off a message to stop the outstanding timer we started. 
				m_pStopTimerMsg = new RqOsTimerStop( m_pStartTimerMsg );
				status = DdmServices::Send(m_pStopTimerMsg, this, REPLYCALLBACK(TSTimedListen, StopTimerCallback));
			}
			
			// If the user supplied a callback call it to let them know that
			// the listen happened.
			// Note that if we have a timed-out listen reply then we already
			// called the user's callback in our handling of our timer reply.
			status = (m_pDdmServices->*m_pUserCallback)( m_pContext, status );
		}
	}
	else
	// If this is the final Listen Reply clear our Listen object pointer.
	if (ListenReplyType == ListenReturnedOnStopListen)
	{
		m_pListen = NULL;
		// We can't know which reply will come last as we're cleaning up
		// so we have to check and see if it's safe to delete ourselves.
		if (!m_pListen && !m_pStartTimerMsg && !m_pStopTimerMsg)
			delete this;
	}
	
	return status;
}	
		
//  TSTimedListen::StopTimerReply(Message *pStopTimerReplyMsg)
//
//  Description:
//    Handle the reply to a previously issued Stop Timer Message.
//
//  Inputs:
//    pStopTimerReplyMsg - A pointer to our Stop Timer Message.  We need to 
//    delete this.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS TSTimedListen::StopTimerCallback(Message *pStopTimerReply)
{
STATUS		status = pStopTimerReply->DetailedStatusCode;

	// m_pStopTimerMsg had BETTER == pStopTimerReply;
	assert(pStopTimerReply == m_pStopTimerMsg);

	// Always delete the stop message.
	FreeAndClear(m_pStopTimerMsg);
	
	// It is possible that we missed the timer message in the mail and by the time
	// our stop message got there the timer had been replied to and was gone.
	// For this reason we forgive "NO_SUCH_TIMER" errors.  Any other error is an
	// error to be handled like any other.
	if (status == CTS_OS_INVALID_TIMER)
		status = OK;
		
	// If we are were initialized to ReplyOnceOnly or Stop() has been called
	// See if we're done and cleanup if we are..
	if (((m_ReplyMode & ReplyOnceOnly) == ReplyOnceOnly) || m_fStopWasCalled)
	{
		// We can't know which reply will come last as we're cleaning up
		// so we have to check and see if it's safe to delete ourselves.
		if (!m_pListen && !m_pStartTimerMsg && !m_pStopTimerMsg)
			delete this;
	}
	else
	// If we were initialized to ReplyContinuous and Stop hasn't 
	// been called then we need to restart our timer.
	if (((m_ReplyMode & ReplyOnceOnly) == ReplyContinuous) && !m_fStopWasCalled)
	{
		// New a message to start a timer with the user specified delay.
		m_pStartTimerMsg = new RqOsTimerStart(	m_iTimeOut, 0 );
	
		// Send the message off to the timer service.
		status = DdmServices::Send(m_pStartTimerMsg, this, REPLYCALLBACK(TSTimedListen, ListenTimerCallback));
	
		// Reinitialize our state data.
		m_fListenWasDetected = false;
		m_fListenTimerExpired = false;
		m_fStopWasCalled = false;
	}
	
	return status;
}	// end of TSTimedListen::StopTimerReply


//  TSTimedListen::ListenTimerReply(Message *pTimerMsgReply)
//
//  Description:
//	In the process of starting a TimedListen Operation, we started a 
//	timer to notify us if & when our Listen operation should be aborted.
//	This method is called when the timer expires, or when we have stopped
//	the timer.   If the listen condition has not yet been detected then 
//  we should abort and cleanup.  If it has been detected then all is well.
//
//  Inputs:
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS TSTimedListen::ListenTimerCallback(Message *pTimerMsgReply)
{
STATUS	status = pTimerMsgReply->DetailedStatusCode;

	// m_pStartTimerMsg had BETTER == pTimerMsgReply;
	assert(pTimerMsgReply == m_pStartTimerMsg);

	// Remember that we've been called.
	m_fListenTimerExpired = true;
	
	// Delete our start timer message which is also our reply message.
	FreeAndClear(m_pStartTimerMsg);

	// If the Listen Reply indicating the the Listen Condition has been
	// detected has occurred then we simply let the timer expire without
	// any action, otherwise we have a timeout error.
	if (!m_fListenWasDetected)
	{
		// Call the user supplied callback with a timeout error.
		status = (m_pDdmServices->*m_pUserCallback)( m_pContext, CTS_PTS_LISTEN_TIMED_OUT );

		// If we are in ReplyContinuous mode we must Abort the Listen and stop the timer. 
		// Be careful to avoid infinite recursion by not re-calling Stop().
		if (((m_ReplyMode & ReplyOnceOnly) == ReplyContinuous) && !m_fStopWasCalled)
			Stop();
	};

	// If we're in ReplyOnceOnly mode or our Stop method was called...
	if (((m_ReplyMode & ReplyOnceOnly) == ReplyOnceOnly) || m_fStopWasCalled) 
	{
		// We can't know which reply will come last as we're cleaning up
		// so we have to check and see if it's safe to delete ourselves.
		if (!m_pListen && !m_pStartTimerMsg && !m_pStopTimerMsg)
			delete this;
	}
	
	return status;
}	// end of TSTimedListen::ListenTimerReply



//  TSTimedListen::Stop()
//
//  Description:
//	In the process of starting a TimedListen Operation, we started a 
//	timer to notify us if & when our Listen operation should be aborted.
//	This method can called to abort ongoing listen and timer operations.
//	NOTE This method only NEEDs to be called if the TSTimedListen was
//	initialized with ReplyContinuous mode but can be used to stop any
//	outstanding TSTimedListen operation.
//
//  Inputs:
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS TSTimedListen::Stop()
{
STATUS status;

	// Remember that we've been called.
	m_fStopWasCalled = true;
	
	// If we have outstanding operations abort them.
	if (m_pListen)
		if (m_fSendWasCalled)
			status = m_pListen->Stop();
		else
			FreeAndClear(m_pListen);
			
	// If we have an outstanding timer...
	if (m_pStartTimerMsg && !m_fListenTimerExpired)
	{
		// If we don't have an outstanding message to stop it...
		if (!m_pStopTimerMsg)
		{
			// Allocate a stop timer message and then ...
			m_pStopTimerMsg = new RqOsTimerStop( m_pStartTimerMsg );
			// Send the message off to the timer service.
			status = DdmServices::Send(m_pStopTimerMsg, this, REPLYCALLBACK(TSTimedListen, StopTimerCallback));
		}
		
		return status;
	}
	
	// If we have no other messages outstanding then we're done
	if (!m_pStopTimerMsg)
		delete this;
		
	return OK;
}	// end of TSTimedListen::ListenTimerReply



