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
// File: RmstrInternalCommands.cpp
// 
// Description:
// Implementation for the abort utility command
// 
// $Log: /Gemini/Odyssey/FCMInternalCmnds.cpp $
// 
// 1     1/05/00 5:17p Dpatel
// Initial creation
// 
// 
//
/*************************************************************************/

#include "DdmFCM.h"

//************************************************************************
//	StartInternalLoopDown
//
//************************************************************************
STATUS DdmFCM::
StartInternalLoopDown(U32 loopInstanceNumber)
{
	
	FCM_CMND_INFO				*pCmdInfo = NULL;
	FCM_CMND_PARAMETERS			*pCmdParams = NULL;
	FCM_LOOP_CONTROL_INFO		*pLoopControlInfo;


	pCmdInfo = new (tZERO) FCM_CMND_INFO;
	pCmdInfo->opcode = FCM_CMND_LOOP_CONTROL;
	
	pCmdParams = 
			(FCM_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pLoopControlInfo = 
			(FCM_LOOP_CONTROL_INFO *) (&pCmdParams->loopControlInfo);

	pLoopControlInfo->cmnd = LOOP_DOWN;
	pLoopControlInfo->loopInstanceNumber = loopInstanceNumber;
	pLoopControlInfo->flags = 0;				// scan fabric? etc

	STATUS status = m_pInternalCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmFCM::FCMInternalCmdCompletionReply,
		NULL);
	delete pCmdInfo;
	pCmdInfo = NULL;
	return status;
}	




//************************************************************************
//	FCMInternalCmdCompletionReply
//		This is the callback when our internal cmd sender sends a
//		command to the FCM Master itself. When the FCM reports the cmd status
//		this callback gets called. We dont need to do any processing here.
//	Note: We also dont register for any events, since we dont need to do any
//	processing on events.
//
//************************************************************************
void DdmFCM::
FCMInternalCmdCompletionReply (
			STATUS			completionCode,
			void			*pResultData,
			void			*pCmdData,
			void			*pCmdContext)
{
	FCM_CMND_INFO				*pCmdInfo = NULL;
	FCM_CMND_PARAMETERS			*pCmdParams = NULL;
	FCM_LOOP_CONTROL_INFO		*pLoopControlInfo = NULL;

	pCmdInfo = (FCM_CMND_INFO *)pCmdData; 
	pCmdParams = 
			(FCM_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);

	if (completionCode == FCM_SUCCESS){
		switch (pCmdInfo->opcode){
		case FCM_CMND_LOOP_CONTROL:
			pLoopControlInfo = 
				(FCM_LOOP_CONTROL_INFO *) (&pCmdParams->loopControlInfo);			
			break;
		default:
			break;
		}
	} else {
	}
}