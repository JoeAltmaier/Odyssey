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
// File: WWN.cpp
// 
// Description:
// Implementation for loop control cmnds
// 
// $Log: /Gemini/Odyssey/DdmFCM/WWN.cpp $
// 
// 2     1/05/00 7:05p Dpatel
// 
// 1     1/05/00 5:17p Dpatel
// Initial creation
// 
// 
//
/*************************************************************************/


#include "DdmFCM.h"
#include "WWNTable.h"

/////////////////////////////////////////////////////////////////////////
//
//				INITIALIZATION
//
/////////////////////////////////////////////////////////////////////////

// Initialize Chassis wwn state machine 
enum {
	READ_CHASSIS_WWN_NAA2 = 1,
};

//************************************************************************
//	FCMInitializeChassisWWN
//
//		Resolve: Need to read the chassis WWN from where ever it
//		is stored. (Jeff. Nespor)
//	
//	
//************************************************************************
STATUS DdmFCM::
FCMInitializeChassisWWN(
			pTSCallback_t			pCallback,
			CONTEXT					*pParentContext)
{
	CONTEXT		*pContext = new CONTEXT();
	pContext->state = READ_CHASSIS_WWN_NAA2;
	pContext->pCallback = pCallback;
	pContext->pParentContext = pParentContext;

	pContext->pData = new (tZERO) WWNDescriptor;
	STATUS status = ReadChassisWWN(pContext, &m_chassisWWN_NAA2);
	return status;
}



//************************************************************************
//	FCMInitializeChassisWWNReply
//			
//		- Read the chassis wwn, store it in our class variables
//
//************************************************************************
STATUS DdmFCM::
FCMInitializeChassisWWNReply(void *_pContext, STATUS status)
{
	CONTEXT							*pContext = (CONTEXT *)_pContext;

	BOOL							cmdComplete = false;
	WWNDescriptor					*pWWNDescriptor = NULL;
	TSReadRow						*pReadRow = NULL;
	BOOL							insert = false;
	pTSCallback_t					cb;
	CONTEXT							*pParentContext = NULL;



	// pContext->pData = WWN descriptor
	if (status != OS_DETAIL_STATUS_SUCCESS){
		if (status == ercKeyNotFound){
			insert = true;
		} else {
			cmdComplete = true;
		}
	} 
	
	if (!cmdComplete){
		switch(pContext->state){
			case READ_CHASSIS_WWN_NAA2:
				// Resolve: 
				// Store the read values into our class members
				cmdComplete = true;
				break;

			default:
				cmdComplete = true;
				break;
		}
	} 
	
	if (cmdComplete) {
		pParentContext = pContext->pParentContext;
		cb = pContext->pCallback;
		(this->*cb)(pParentContext, status);
		delete pContext;
	}
	return status;
}


/////////////////////////////////////////////////////////////////////////
//
//				GET NEXT WWN
//
/////////////////////////////////////////////////////////////////////////

// Get Next WWN state machine 
enum {
	INSERT_NEW_WWN_RECORD =1,
};

//************************************************************************
//	ProcessGetWWNMsg
//		- Support for the message based interface
//		
//		
//
//************************************************************************
STATUS DdmFCM::
ProcessGetWWNMsg(Message *_pMsgFCMGetWWN)
{
	MsgFCMGetWWN *pMsgFCMGetWWN = (MsgFCMGetWWN *)_pMsgFCMGetWWN;
	
	STATUS		status;

	FCM_CMND_INFO	*pCmdInfo = NULL;
	pMsgFCMGetWWN->GetFCMCmdInfo((void **)&pCmdInfo);
	pCmdInfo->isMessage = TRUE;
	status = GetNextWWN(
						pMsgFCMGetWWN,
						pCmdInfo);
	return status;
}


//************************************************************************
//	GetNextWWN
//		
//		- Insert a new row
//		- Append the row id with the NAA5 chassis wwn
//
//************************************************************************
STATUS DdmFCM::
GetNextWWN(HANDLE h, FCM_CMND_INFO *_pCmdInfo)
{
	STATUS							status = FCM_SUCCESS;
	FCM_CMND_PARAMETERS				*pCmdParams = NULL;
	FCM_GET_NEXT_WWN				*pGetNextWWN = NULL;
	FCM_CMND_INFO					*pCmdInfo =NULL;

	CONTEXT	*pCmdContext	= new CONTEXT;

	// save the Cmd Info and the handle
	pCmdContext->cmdHandle	= h;

	pCmdContext->pData		= new (tZERO) FCM_CMND_INFO;
	memcpy(pCmdContext->pData, _pCmdInfo, sizeof(FCM_CMND_INFO));


	pCmdInfo			= (FCM_CMND_INFO *)pCmdContext->pData;
	pCmdParams			= &pCmdInfo->cmdParams;
	pGetNextWWN	= 
		(FCM_GET_NEXT_WWN *)&pCmdParams->getNextWWN;

	pCmdContext->pData1 = new (tZERO) WWNDescriptor;
	pCmdContext->state = INSERT_NEW_WWN_RECORD;
	InsertChassisWWN(
		pCmdContext->pData1,					// new record
		TSCALLBACK(DdmFCM,GetNextWWNReply),		// callback
		pCmdContext);								// context
	return status;
}


//************************************************************************
//	GetNextWWNReply
//		
//		- Insert a new row
//		- Append the row id with the NAA5 chassis wwn
//
//************************************************************************
STATUS DdmFCM::
GetNextWWNReply(void *_pContext, STATUS status)
{
	FCM_CMND_PARAMETERS				*pCmdParams = NULL;
	FCM_GET_NEXT_WWN				*pGetNextWWN = NULL;
	FCM_CMND_INFO					*pCmdInfo =NULL;
	unsigned char					temp[8];
	unsigned char					temp1[8];
	int								i = 0, k=0;
	BOOL							cmdComplete = false;

	CONTEXT	*pCmdContext	= (CONTEXT *)_pContext;



	pCmdInfo			= (FCM_CMND_INFO *)pCmdContext->pData;
	pCmdParams			= &pCmdInfo->cmdParams;
	pGetNextWWN	= 
		(FCM_GET_NEXT_WWN *)&pCmdParams->getNextWWN;

	WWNDescriptor *pWWNDescriptor = (WWNDescriptor *)pCmdContext->pData1;


	if (status != OS_DETAIL_STATUS_SUCCESS){
		cmdComplete = true;
	} else {
		switch(pCmdContext->state){
		case INSERT_NEW_WWN_RECORD:
			// Resolve: take care of endian ness
			memcpy(temp,&pWWNDescriptor->rid, 8);		// the lower order 8 bytes
			memcpy(temp1,&m_chassisWWN_NAA5, 8);		// the higher 8 bytes

			for (i=0; i < 8; i++){
				memcpy(&pGetNextWWN->wwn[i], &temp1[i], 1);
			}
			for (i=8; i < 16; i++){
				memcpy(&pGetNextWWN->wwn[i], &temp[k++], 1);
			}

			cmdComplete = true;
			break;
		default:
			cmdComplete = true;
			assert(0);
			break;
		}
	}
	if (cmdComplete){
		if (pCmdInfo->isMessage){
			// since we always use our context copy of the cmd info
			// update the copy of cmdinfo in the message with the 
			// return data
			FCM_CMND_INFO	*pMsgCmdInfo;
			MsgFCMGetWWN	*pGetWWNMsg = (MsgFCMGetWWN *)pCmdContext->cmdHandle;
			pGetWWNMsg->GetFCMCmdInfo((void **)&pMsgCmdInfo);
			*pMsgCmdInfo = *pCmdInfo;
			Reply((Message *)pGetWWNMsg, status);
		} else {
			m_pCmdServer->csrvReportCmdStatus(
				pCmdContext->cmdHandle,	// handle
				status,							// completion code
				NULL,							// result Data
				(void *)pCmdInfo);				// Orig cmd info
		}
		delete pCmdContext;
	}
	return status;
}




/////////////////////////////////////////////////////////////////////////
//
//				GET CHASSIS WWN
//
/////////////////////////////////////////////////////////////////////////
//************************************************************************
//	ProcessGetChassisWWNMsg
//		- Support for the message based interface
//		
//		
//
//************************************************************************
STATUS DdmFCM::
ProcessGetChassisWWNMsg(Message *_pMsgGetChassisWWN)
{
	MsgFCMGetChassisWWN *pMsgGetChassisWWN = (MsgFCMGetChassisWWN *)_pMsgGetChassisWWN;
	
	STATUS		status;

	FCM_CMND_INFO	*pCmdInfo = NULL;
	pMsgGetChassisWWN->GetFCMCmdInfo((void **)&pCmdInfo);
	pCmdInfo->isMessage = TRUE;
	status = GetChassisWWN(
						pMsgGetChassisWWN,
						pCmdInfo);
	return status;
}


//************************************************************************
//	GetChassisWWN
//		
//		- Check the type of format requested and return from our member 
//		variable
//
//************************************************************************
STATUS DdmFCM::
GetChassisWWN(HANDLE h, FCM_CMND_INFO *_pCmdInfo)
{
	STATUS							status = FCM_SUCCESS;
	FCM_CMND_PARAMETERS				*pCmdParams = NULL;
	FCM_GET_CHASSIS_WWN				*pGetChassisWWN = NULL;
	FCM_CMND_INFO					*pCmdInfo =NULL;

	CONTEXT	*pCmdContext	= new CONTEXT;

	// save the Cmd Info and the handle
	pCmdContext->cmdHandle	= h;

	pCmdContext->pData		= new (tZERO) FCM_CMND_INFO;
	memcpy(pCmdContext->pData, _pCmdInfo, sizeof(FCM_CMND_INFO));


	pCmdInfo			= (FCM_CMND_INFO *)pCmdContext->pData;
	pCmdParams			= &pCmdInfo->cmdParams;
	pGetChassisWWN	= 
		(FCM_GET_CHASSIS_WWN *)&pCmdParams->getChassisWWN;

	
	switch (pGetChassisWWN->naaType){
		// Resolve: endian ness
		case NAA2:
			memcpy(&pGetChassisWWN->chassisWWN, &m_chassisWWN_NAA2, 8);
			break;

		case NAA5:
			memcpy(&pGetChassisWWN->chassisWWN, &m_chassisWWN_NAA5, 8);
			break;

		default:
			assert(0);
			break;
	}
	if (pCmdInfo->isMessage){
		// since we always use our context copy of the cmd info
		// update the copy of cmdinfo in the message with the 
		// return data
		FCM_CMND_INFO	*pMsgCmdInfo;
		MsgFCMGetChassisWWN	*pGetChassisWWNMsg = (MsgFCMGetChassisWWN *)pCmdContext->cmdHandle;
		pGetChassisWWNMsg->GetFCMCmdInfo((void **)&pMsgCmdInfo);
		*pMsgCmdInfo = *pCmdInfo;
		Reply((Message *)pGetChassisWWNMsg, status);
	} else {
		m_pCmdServer->csrvReportCmdStatus(
				pCmdContext->cmdHandle,	// handle
				status,							// completion code
				NULL,							// result Data
				(void *)pCmdInfo);				// Orig cmd info
	}
	delete pCmdContext;
	return status;
}


/////////////////////////////////////////////////////////////////////////
//
//				UTILITY METHODS
//
/////////////////////////////////////////////////////////////////////////


//************************************************************************
//	Read the Chassis wwn
//			
//
//************************************************************************
STATUS DdmFCM::
ReadChassisWWN(CONTEXT *pContext, void *pKeyValue)
{
	STATUS			status = 0;

	TSReadRow *pReadRow = new TSReadRow;
	if (!pReadRow)
		status = CTS_OUT_OF_MEMORY;
	else {
		status = pReadRow->Initialize( 
					this,											// DdmServices *pDdmServices,
					WWN_TABLE,										// String64 rgbTableName,
					fdCHASSIS_WWN,									// String64 rgbKeyFieldName,
					pKeyValue,										// void *pKeyFieldValue,
					sizeof(U64),									// U32 cbKeyFieldValue,
					pContext->pData,								// void *prgbRowDataRet,
					sizeof(WWNDescriptor),							// U32 cbRowDataRetMax,
					NULL,											// U32 *pcRowsReadRet,
					TSCALLBACK(DdmFCM,FCMInitializeChassisWWNReply),		// pTSCallback_t pCallback,
					pContext										// void* pContext
				);
	}				
	if (status == OK) 
		pReadRow->Send();
	else 
		FCMInitializeChassisWWNReply(pContext, status);
	return status;
}



//************************************************************************
//	Insert the Chassis wwn
//			
//
//************************************************************************
STATUS DdmFCM::
InsertChassisWWN(void *pRowData, pTSCallback_t cb, CONTEXT *pContext)
{
	STATUS		status = 0;
	WWNDescriptor *pDesc = (WWNDescriptor *)pRowData;
	TSInsertRow *pInsertRow = new TSInsertRow;
	if (!pInsertRow)
		status = CTS_OUT_OF_MEMORY;
	else		
		status = pInsertRow->Initialize(
				this,									// DdmServices *pDdmServices,
				WWN_TABLE,								// String64 rgbTableName,
				pRowData,								// void *prgbRowData,
				sizeof(WWNDescriptor),					// U32 cbRowData,
				(rowID *)pRowData,						// rowID *prowIDRet,
				cb,										// pTSCallback_t pCallback,
				pContext								// void* pContext
			);
	
	if (status == OK)
		pInsertRow->Send();
	else 
		(this->*cb)(pContext, status);
	return status;
}
