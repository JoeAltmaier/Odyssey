/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: FCMLoopControl.cpp
// 
// Description:
// Implementation for loop control cmnds
// 
// $Log: /Gemini/Odyssey/FCMLoopControl.cpp $
// 
// 1     1/05/00 5:17p Dpatel
// Initial creation
// 
// 
//
/*************************************************************************/


#include "DdmFCM.h"


// Loop Control state machine 
enum {
	LOOP_DESCRIPTOR_RECORD_READ = 1,
};


//************************************************************************
//	ProcessLoopControlMsg
//		- The message based interface
//		
//		
//
//************************************************************************
STATUS DdmFCM::
ProcessLoopControlMsg(Message *_pLoopControlMsg)
{
	MsgFCMLoopControl *pLoopControlMsg = (MsgFCMLoopControl *)_pLoopControlMsg;
	
	STATUS		status;

	FCM_CMND_INFO	*pCmdInfo = NULL;
	pLoopControlMsg->GetFCMCmdInfo((void **)&pCmdInfo);
	pCmdInfo->isMessage = TRUE;
	status = LoopControlCommandValidation(
						pLoopControlMsg,
						pCmdInfo);
	return status;
}




//************************************************************************
//	LoopControlCommandValidation
//		
//		
//
//************************************************************************
STATUS DdmFCM::
LoopControlCommandValidation(
			HANDLE					h, 
			FCM_CMND_INFO			*_pCmdInfo)
{
	STATUS							status = FCM_SUCCESS;
	FCM_CMND_PARAMETERS				*pCmdParams = NULL;
	FCM_LOOP_CONTROL_INFO			*pLoopControlInfo = NULL;
	FCM_CMND_INFO					*pCmdInfo =NULL;

	CONTEXT	*pValidationContext	= new CONTEXT;

	// save the Cmd Info and the handle
	pValidationContext->cmdHandle	= h;

	pValidationContext->pData		= new (tZERO) FCM_CMND_INFO;
	memcpy(pValidationContext->pData, _pCmdInfo, sizeof(FCM_CMND_INFO));


	pCmdInfo			= (FCM_CMND_INFO *)pValidationContext->pData;
	pCmdParams			= &pCmdInfo->cmdParams;
	pLoopControlInfo	= 
		(FCM_LOOP_CONTROL_INFO *)&pCmdParams->loopControlInfo;

	// first read the Loop Descriptor record with the instance
	// number the command requests
	pValidationContext->state = LOOP_DESCRIPTOR_RECORD_READ;
	pValidationContext->pData1 = new (tZERO) LoopDescriptorRecord;
	TSReadRow *pReadRow = new TSReadRow;
	if (!pReadRow)
		status = CTS_OUT_OF_MEMORY;
	else		
		status = pReadRow->Initialize( 
				this,											// DdmServices *pDdmServices,
				LOOP_DESCRIPTOR_TABLE,							// String64 rgbTableName,
				fdLD_LOOP_NUM,									// String64 rgbKeyFieldName,
				&pLoopControlInfo->loopInstanceNumber,			// void *pKeyFieldValue,
				sizeof(U32),									// U32 cbKeyFieldValue,
				pValidationContext->pData1,					// void *prgbRowDataRet,
				sizeof(LoopDescriptorRecord),					// U32 cbRowDataRetMax,
				NULL,											// U32 *pcRowsReadRet,
				TSCALLBACK(DdmFCM,LoopControlValdiationReply),	// pTSCallback_t pCallback,
				pValidationContext								// void* pContext
			);
		
	if (status == OK)
		pReadRow->Send();
	else {
		if (pCmdInfo->isMessage){
			Reply((Message *)pValidationContext->cmdHandle, status);
		} else {
			m_pCmdServer->csrvReportCmdStatus(
					pValidationContext->cmdHandle,	// handle
					status,						// completion code
					NULL,						// result Data
					pCmdInfo);					// Orig cmd info
		}
		delete pValidationContext;
	}

	return status;
}


//************************************************************************
//	LoopControlCommandValidationReply
//		
//		
//
//************************************************************************
STATUS DdmFCM::
LoopControlValdiationReply(void *_pContext, STATUS status)
{
	CONTEXT							*pValidationContext = (CONTEXT *)_pContext;
	STATUS							rc;
	FCM_CMND_INFO					*pCmdInfo = NULL;
	FCM_CMND_PARAMETERS				*pCmdParams = NULL;
	FCM_LOOP_CONTROL_INFO			*pLoopControlInfo = NULL;

	BOOL							validationComplete = false;
	LoopDescriptorRecord			*pLoopDescriptorRecord = NULL;

	rc = FCM_SUCCESS;

	// pValdiationContext->pData = cmdInfo
	// pValdiationContext->pData1 = LoopDescriptorRecord
	pCmdInfo				= (FCM_CMND_INFO *)pValidationContext->pData;
	pCmdParams				= &pCmdInfo->cmdParams;
	pLoopControlInfo	= 
		(FCM_LOOP_CONTROL_INFO *)&pCmdParams->loopControlInfo;

	pLoopDescriptorRecord = (LoopDescriptorRecord *)pValidationContext->pData1;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		rc = CTS_FCM_ERR_INVALID_COMMAND;
		validationComplete = true;
	} else {
		switch(pValidationContext->state){
		case LOOP_DESCRIPTOR_RECORD_READ:
			// validate for each of our cmd, UP/ DOWN
			switch(pLoopControlInfo->cmnd){
			case LOOP_UP:
				if (pLoopDescriptorRecord->ActualLoopState == LoopUp){					
					// Resolve; enable checking later
					//rc = FCM_ERR_LOOP_ALREADY_UP;
				}
				break;
			case LOOP_DOWN:
				if (pLoopDescriptorRecord->ActualLoopState == LoopDown){					
					// Resolve; enable checking later
					//rc = FCM_ERR_LOOP_ALREADY_DOWN;
				}
			}			
			validationComplete = true;
			break;
		default:
			rc = CTS_FCM_ERR_INVALID_COMMAND;
			break;
		}
	}
	if (validationComplete){
		// if there was an error then report the status back
		if (rc){
			if (pCmdInfo->isMessage){
				Reply((Message *)pValidationContext->cmdHandle, status);
			} else {
				m_pCmdServer->csrvReportCmdStatus(
					pValidationContext->cmdHandle,	// handle
					rc,						// completion code
					NULL,					// result Data
					pCmdInfo);				// Orig cmd info
			}
			delete pValidationContext;
		} else {
			status = LoopControlCommand(
						pValidationContext->cmdHandle,	// handle
						pCmdInfo,
						pLoopDescriptorRecord);
		}
	}
	return status;
}



//************************************************************************
//	LoopControlCommand
//		Issue the acutal Loop control cmd, (UP/DOWN/LIP) to the Loop Monitor
//
//	handle		- Handle for the cmd
//	_pCmdInfo	- cmd packet for change source member
//	_pLoopDescriptorRecord	- the loop to send the cmd to
//
//************************************************************************
STATUS DdmFCM::
LoopControlCommand(
		HANDLE						handle,
		FCM_CMND_INFO				*_pCmdInfo,
		LoopDescriptorRecord		*_pLoopDescriptorRecord)
{
	STATUS							status = FCM_SUCCESS;
	FCM_CMND_INFO					*pCmdInfo = NULL;
	FCM_CMND_PARAMETERS				*pCmdParams = NULL;
	FCM_LOOP_CONTROL_INFO			*pLoopControlInfo = NULL;
	CONTEXT							*pCmdContext = NULL;
	LoopDescriptorRecord			*pLoopDescriptorRecord;

	LmLoopUp						*pLmLoopUp = NULL;
	LmLoopDown						*pLmLoopDown = NULL;
	LmLoopLIP						*pLmLoopLip = NULL;
	pCmdContext = new CONTEXT;

	pCmdContext->cmdHandle	= handle;
	pCmdContext->pData		= new FCM_CMND_INFO;
	memcpy(pCmdContext->pData, _pCmdInfo, sizeof(FCM_CMND_INFO));


	pCmdContext->pData1		= new (tZERO) LoopDescriptorRecord;
	memcpy(pCmdContext->pData1, _pLoopDescriptorRecord, sizeof(LoopDescriptorRecord));
	pLoopDescriptorRecord = (LoopDescriptorRecord *)pCmdContext->pData1;


	pCmdInfo				= (FCM_CMND_INFO *)pCmdContext->pData;
	pCmdParams				= &pCmdInfo->cmdParams;
	pLoopControlInfo	= 
		(FCM_LOOP_CONTROL_INFO *)&pCmdParams->loopControlInfo;

	switch(pLoopControlInfo->cmnd){
	case LOOP_UP:
		// Send a message to the IsmLoopMonitor
		pLmLoopUp = new LmLoopUp();
		pLmLoopUp->payload.instance = pLoopDescriptorRecord->LoopNumber;
		status = Send(
					pLoopDescriptorRecord->vdnLoopMonitor,	// vdn
					pLmLoopUp,								// message
					pCmdContext,							// context
					REPLYCALLBACK(DdmFCM,ProcessLoopControlReply));
		break;
	case LOOP_DOWN:
		pLmLoopDown = new LmLoopDown();
		pLmLoopDown->payload.instance = pLoopDescriptorRecord->LoopNumber;
		status = Send(
					pLoopDescriptorRecord->vdnLoopMonitor,	// vdn
					pLmLoopDown,							// message
					pCmdContext,							// context
					REPLYCALLBACK(DdmFCM,ProcessLoopControlReply));
		break;
	case LOOP_LIP:
		pLmLoopLip = new LmLoopLIP();
		pLmLoopLip->payload.instance = pLoopDescriptorRecord->LoopNumber;
		status = Send(
					pLoopDescriptorRecord->vdnLoopMonitor,	// vdn
					pLmLoopLip,								// message
					pCmdContext,							// context
					REPLYCALLBACK(DdmFCM,ProcessLoopControlReply));
		break;
	}

	if (pCmdInfo->isMessage){
		Reply((Message *)pCmdContext->cmdHandle, status);
	} else {
		m_pCmdServer->csrvReportCmdStatus(
			pCmdContext->cmdHandle,		// handle
			status,						// completion code
			NULL,						// result Data
			(void *)pCmdInfo);			// Orig cmd info
	}
	// clean up our context, other wise it will be cleaned up in the message reply
	if (status)
		delete pCmdContext;
	return status;
}	


//************************************************************************
//	ProcessLoopControlReply
//		- Wait for the IsmLoopMonitor to reply for the loop up/down/rescan
//		- Read the Loop descriptor record 
//
//************************************************************************
STATUS DdmFCM::
ProcessLoopControlReply(Message* pMsg)
{
	STATUS	status = pMsg->Status();
	LmLoopUp			*pLmLoopUp = NULL;
	LmLoopDown			*pLmLoopDown = NULL;
	LmLoopLIP			*pLmLoopLip = NULL;
	U32					loopInstanceNumber = 0;
	FCM_CMND_INFO		*pCmdInfo = NULL;
	
	CONTEXT *pContext	= (CONTEXT *)pMsg->GetContext();
	pCmdInfo			= (FCM_CMND_INFO *)pContext->pData;

	if (!status){	
		switch(pMsg->reqCode){
		case LM_LOOP_LIP:
			pLmLoopLip = (LmLoopLIP *)pMsg;
			loopInstanceNumber = pLmLoopLip->payload.instance;
			break;
		case LM_LOOP_UP:
			pLmLoopUp = (LmLoopUp *)pMsg;
			loopInstanceNumber = pLmLoopUp->payload.instance;
			break;
		case LM_LOOP_DOWN:
			pLmLoopDown = (LmLoopDown *)pMsg;
			loopInstanceNumber = pLmLoopDown->payload.instance;
			break;
		}
		pContext->value = loopInstanceNumber;

		// read the loop descriptor again to get the current data
		TSReadRow *pReadRow = new TSReadRow;
		if (!pReadRow)
			status = CTS_OUT_OF_MEMORY;
		else		
			status = pReadRow->Initialize( 
				this,											// DdmServices *pDdmServices,
				LOOP_DESCRIPTOR_TABLE,							// String64 rgbTableName,
				fdLD_LOOP_NUM,									// String64 rgbKeyFieldName,
				&pContext->value,								// void *pKeyFieldValue,
				sizeof(U32),									// U32 cbKeyFieldValue,
				pContext->pData1,								// void *prgbRowDataRet,
				sizeof(LoopDescriptorRecord),					// U32 cbRowDataRetMax,
				NULL,											// U32 *pcRowsReadRet,
				TSCALLBACK(DdmFCM,LoopControlEventReply),		// pTSCallback_t pCallback,
				pContext										// void* pContext
			);
		
		if (status == OK)
			pReadRow->Send();
		else 
			delete pContext;
	}
	delete pMsg;
	return status;
}


//************************************************************************
//	LoopControlEventReply
//		- if the loop descriptor record is read successfully
//		- Generate the appropriate loop control event and log it
//		
//
//************************************************************************
STATUS DdmFCM::
LoopControlEventReply(void *_pContext, STATUS status)
{
	CONTEXT							*pContext = (CONTEXT *)_pContext;
	STATUS							rc;
	FCM_CMND_INFO					*pCmdInfo = NULL;
	FCM_CMND_PARAMETERS				*pCmdParams = NULL;
	FCM_LOOP_CONTROL_INFO			*pLoopControlInfo = NULL;
	LoopDescriptorRecord			*pLoopDescriptorRecord = NULL;

	// Event stuff
	FCM_LOOP_STATUS					*pLoopStatus = NULL;
	U32								loopEvent = 0;
	U32								logEventCode = 0;


	rc = FCM_SUCCESS;

	// pContext->pData = cmdInfo
	// pContext->pData1 = LoopDescriptorRecord
	pCmdInfo				= (FCM_CMND_INFO *)pContext->pData;
	pCmdParams				= &pCmdInfo->cmdParams;
	pLoopControlInfo	= 
		(FCM_LOOP_CONTROL_INFO *)&pCmdParams->loopControlInfo;
	pLoopDescriptorRecord = (LoopDescriptorRecord *)pContext->pData1;

	

	if (status != OS_DETAIL_STATUS_SUCCESS){
	} else {		
		// Resolve: define the msg compiler codes
		switch(pLoopControlInfo->cmnd){
			case LOOP_LIP:
				loopEvent = FCM_EVT_LIP;
				logEventCode = CTS_FCM_EVT_LOOP_LIP;
				break;
			case LOOP_UP:
				loopEvent = FCM_EVT_LOOP_UP;
				logEventCode = CTS_FCM_EVT_LOOP_UP;
				break;
			case LOOP_DOWN:
				loopEvent = FCM_EVT_LOOP_DOWN;
				logEventCode = CTS_FCM_EVT_LOOP_DOWN;
				break;
			default:
				assert(0);
		}	
		pLoopStatus = new (tZERO) FCM_LOOP_STATUS;
		pLoopStatus->loopData = *pLoopDescriptorRecord;
		m_pCmdServer->csrvReportEvent(
					loopEvent,			// event code
					pLoopStatus);		// event Data
		LogEvent(
				logEventCode,
				pLoopDescriptorRecord->slot,
				pLoopDescriptorRecord->LoopNumber);
		delete pLoopStatus;
	}
	delete pContext;
	return status;
}



