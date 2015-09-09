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
// File: NacShutdown.cpp
// 
// Description:
// Implementation for loop control cmnds
// 
// $Log: /Gemini/Odyssey/NacShutdown.cpp $
// 
// 1     1/05/00 5:17p Dpatel
// Initial creation
// 
// 
//
/*************************************************************************/


#include "DdmFCM.h"
#include "ExportTable.h"


// Nac shutdown state machine 
enum {
	EXPORT_RECORD_QUIESCED = 1,
};


//************************************************************************
//	ProcessNacShutdownMsg
//		- The message based interface
//		
//		
//
//************************************************************************
STATUS DdmFCM::
ProcessNacShutdownMsg(Message *_pNacShutdownMsg)
{
	MsgFCMNacShutdown *pNacShutdownMsg = (MsgFCMNacShutdown *)_pNacShutdownMsg;
	
	STATUS		status;

	FCM_CMND_INFO	*pCmdInfo = NULL;
	pNacShutdownMsg->GetFCMCmdInfo((void **)&pCmdInfo);
	pCmdInfo->isMessage = TRUE;
	status = ProcessNacShutdown(
						pNacShutdownMsg,
						pCmdInfo);
	return status;
}


//************************************************************************
//	ProcessNacShutdown
//		
//		
//
//************************************************************************
STATUS DdmFCM::
ProcessNacShutdown(HANDLE h, FCM_CMND_INFO *_pCmdInfo)
{
	STATUS							status = FCM_SUCCESS;
	FCM_CMND_PARAMETERS				*pCmdParams = NULL;
	FCM_NAC_SHUTDOWN_INFO			*pNacShutdownInfo = NULL;
	FCM_CMND_INFO					*pCmdInfo =NULL;
	CTReadyState					readyState;
	U32								loopInstanceNumber = 0;

	CONTEXT	*pCmdContext	= new CONTEXT;

	// save the Cmd Info and the handle
	pCmdContext->cmdHandle	= h;

	pCmdContext->pData		= new (tZERO) FCM_CMND_INFO;
	memcpy(pCmdContext->pData, _pCmdInfo, sizeof(FCM_CMND_INFO));


	pCmdInfo			= (FCM_CMND_INFO *)pCmdContext->pData;
	pCmdParams			= &pCmdInfo->cmdParams;
	pNacShutdownInfo	= 
		(FCM_NAC_SHUTDOWN_INFO *)&pCmdParams->nacShutdownInfo;

	// Modify all the export entries corresponding to the loop instances
	// for the slot number requested in the cmd
	pCmdContext->state = EXPORT_RECORD_QUIESCED;
	readyState = StateQuiesced;
	// calculate the loop instance of the 1st loop from the slot number
	// Resolve: find the #define for MAX_LOOPS_PER_BOARD = 3
	loopInstanceNumber = (pNacShutdownInfo->slotNumber * 3) + pCmdContext->numProcessed;
	pCmdContext->value = loopInstanceNumber;
	pCmdContext->value = 0;
	TSModifyField*	pModifyExportRecFields = new TSModifyField;
	
	if (!pModifyExportRecFields)
		status = CTS_OUT_OF_MEMORY;
	else		
		status = pModifyExportRecFields->Initialize(
			this,											// DdmServices *pDdmServices,
			EXPORT_TABLE,									// String64 rgbTableName,
			"FCInstance",									// String64 rgbKeyFieldName,
			&pCmdContext->value,							// void* pKeyFieldValue,
			sizeof(U32),									// U32 cbKeyFieldValue,
			"DesiredReadyState",							// String64 rgbFieldName,
			&readyState,									// void* pFieldValue,
			sizeof(CTReadyState),							// U32 cbFieldValue,
			0,												// U32 cRowsToModify (0 => all)
			NULL,											// U32* pcRowsModifiedRet,
			NULL,											// rowID *prowIDRet,
			0,												// U32 cbMaxRowID,
			TSCALLBACK(DdmFCM,ProcessNacShutdownReply),		// pTSCallback_t pCallback,
			pCmdContext										// void* pContext
		);

	if (status == OK)
		pModifyExportRecFields->Send();
	else {
		ProcessNacShutdownReply(pCmdContext, status);
	}
	return status;
}


//************************************************************************
//	ProcessNacShutdownReply
//			
//		- Set DesiredReadyState in export record to quiesced
//		- Send a loop shutdown message to each of the loops
//		
//
//************************************************************************
STATUS DdmFCM::
ProcessNacShutdownReply(void *_pContext, STATUS status)
{
	CONTEXT							*pCmdContext = (CONTEXT *)_pContext;
	STATUS							rc;
	FCM_CMND_INFO					*pCmdInfo = NULL;
	FCM_CMND_PARAMETERS				*pCmdParams = NULL;
	FCM_NAC_SHUTDOWN_INFO			*pNacShutdownInfo = NULL;
	TSModifyField					*pModifyExportRecFields = NULL;
	CTReadyState					readyState;
	BOOL							cmdComplete = false;
	FCM_NAC_SHUTDOWN_STATUS			*pNacShutdownStatus = NULL;


	rc = FCM_SUCCESS;

	// pCmdContext->pData = cmdInfo
	pCmdInfo				= (FCM_CMND_INFO *)pCmdContext->pData;
	pCmdParams				= &pCmdInfo->cmdParams;
	pNacShutdownInfo	= 
		(FCM_NAC_SHUTDOWN_INFO *)&pCmdParams->nacShutdownInfo;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		if (status != ercKeyNotFound){
			rc = CTS_FCM_ERR_INVALID_COMMAND;
			cmdComplete = true;
		} else {
			status = 0; // we continue because error was key not found
		}
		// we need to check for all the loops
	} 
	
	if (!rc){
		switch(pCmdContext->state){
		case EXPORT_RECORD_QUIESCED:
			// Send an internal cmd to the FCM to shutdown this loop
			StartInternalLoopDown(pCmdContext->value);				

			pCmdContext->numProcessed++;
			if (pCmdContext->numProcessed < 3){
				pModifyExportRecFields = new TSModifyField;
				pCmdContext->value++;
				readyState = StateQuiesced;
				if (!pModifyExportRecFields)
					status = CTS_OUT_OF_MEMORY;
				else		
					status = pModifyExportRecFields->Initialize(
						this,											// DdmServices *pDdmServices,
						EXPORT_TABLE,									// String64 rgbTableName,
						"FCInstance",									// String64 rgbKeyFieldName,
						&pCmdContext->value,							// void* pKeyFieldValue,
						sizeof(U32),									// U32 cbKeyFieldValue,
						"DesiredReadyState",							// String64 rgbFieldName,
						&readyState,									// void* pFieldValue,
						sizeof(CTReadyState),							// U32 cbFieldValue,
						0,												// U32 cRowsToModify (0 => all)
						NULL,											// U32* pcRowsModifiedRet,
						NULL,											// rowID *prowIDRet,
						0,												// U32 cbMaxRowID,
						TSCALLBACK(DdmFCM,ProcessNacShutdownReply),		// pTSCallback_t pCallback,
						pCmdContext										// void* pContext
					);

				if (status == OK){
					pModifyExportRecFields->Send();
				} else {
					ProcessNacShutdownReply(pCmdContext, status);
				}
			} else {
				cmdComplete = true;
			}
			break;
		default:
			status = CTS_FCM_ERR_INVALID_COMMAND;
			cmdComplete = true;
			break;
		}
	}
	if (cmdComplete){
		if (pCmdInfo->isMessage){
			Reply((Message *)pCmdContext->cmdHandle, status);
		} else {
			m_pCmdServer->csrvReportCmdStatus(
					pCmdContext->cmdHandle,	// handle
					status,							// completion code
					NULL,							// result Data
					(void *)pCmdInfo);				// Orig cmd info
		}
		
		if (!rc){
			// Generate event and log event
			pNacShutdownStatus = new (tZERO) FCM_NAC_SHUTDOWN_STATUS;
			pNacShutdownStatus->slotNumber = pNacShutdownInfo->slotNumber;
			m_pCmdServer->csrvReportEvent(
					FCM_EVT_NAC_SHUTDOWN,	// event code
					pNacShutdownStatus);	// event Data
			LogEvent(
				CTS_FCM_EVT_NAC_SHUTDOWN,
				pNacShutdownStatus->slotNumber);
			delete pNacShutdownStatus;
		}
		delete pCmdContext;
	}
	return status;
}




