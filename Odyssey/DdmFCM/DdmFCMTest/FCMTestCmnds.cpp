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
// File: FCMTestCmnds.cpp
// 
// Description:
// Test Cmds for FCM
// 
// $Log: /Gemini/Odyssey/DdmFCM/DdmFCMTest/FCMTestCmnds.cpp $
// 
// 1     1/05/00 5:20p Dpatel
// Initial creation
// 
//
/*************************************************************************/

#include "OsTypes.h"
#include "Message.h"
#include "CTTypes.h"
#include "OsStatus.h"
#include "Ddm.h"
#include "Fields.h"
#include "Rows.h"
#include "Listen.h"
#include "ReadTable.h"
#include "Table.h"
#include "PTSCommon.h"
#include "RequestCodes.h"

#include "DdmFCMTest.h"

#include "StorageRollCallTable.h"
#include "LoopDescriptor.h"

#include "DdmFCMCmnds.h"
#include "DdmFCMMsgs.h"

//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////
STATUS DdmFCMTest
::TestLoopUp()
{
	MsgFCMLoopControl			*pLoopControlMsg;
	FCM_CMND_INFO				*pCmdInfo;
	FCM_CMND_PARAMETERS			*pCmdParams;
	FCM_LOOP_CONTROL_INFO		*pLoopControlInfo;
	STATUS						status;


	pCmdInfo = new (tZERO) FCM_CMND_INFO;
	pCmdInfo->opcode = FCM_CMND_LOOP_CONTROL;
	
	pCmdParams = 
			(FCM_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pLoopControlInfo = 
			(FCM_LOOP_CONTROL_INFO *) (&pCmdParams->loopControlInfo);

	pLoopControlInfo->cmnd = LOOP_UP;
	pLoopControlInfo->loopInstanceNumber = 0; //72;	// from the Loop Descriptor record
	pLoopControlInfo->flags = 0;				// scan fabric? etc


	// Issue a FCM Loop Control request
	pLoopControlMsg = new MsgFCMLoopControl(pCmdInfo);
	
	status = Send(
				pLoopControlMsg,
				NULL,
				REPLYCALLBACK(DdmFCM,FCM_LoopControlMsgReply));
	return status;

#if 0
	FCM_CMND_INFO				*pCmdInfo;
	FCM_CMND_PARAMETERS			*pCmdParams;
	FCM_LOOP_CONTROL_INFO		*pLoopControlInfo;


	pCmdInfo = new (tZERO) FCM_CMND_INFO;
	pCmdInfo->opcode = FCM_CMND_LOOP_CONTROL;
	
	pCmdParams = 
			(FCM_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pLoopControlInfo = 
			(FCM_LOOP_CONTROL_INFO *) (&pCmdParams->loopControlInfo);

	pLoopControlInfo->cmnd = LOOP_UP;
	pLoopControlInfo->loopInstanceNumber = 0; //72;	// from the Loop Descriptor record
	pLoopControlInfo->flags = 0;				// scan fabric? etc

	STATUS status = m_pCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmFCMTest::FCMCommandCompletionReply,
		NULL);
	delete pCmdInfo;
	pCmdInfo = NULL;
	return status;
#endif
}



//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////
STATUS DdmFCMTest
::TestNacShutdown()
{
	FCM_CMND_INFO				*pCmdInfo;
	FCM_CMND_PARAMETERS			*pCmdParams;
	FCM_NAC_SHUTDOWN_INFO		*pNacShutdownInfo;


	pCmdInfo = new (tZERO) FCM_CMND_INFO;
	pCmdInfo->opcode = FCM_CMND_NAC_SHUTDOWN;
	
	pCmdParams = 
			(FCM_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pNacShutdownInfo = 
			(FCM_NAC_SHUTDOWN_INFO *) (&pCmdParams->nacShutdownInfo);

	pNacShutdownInfo->slotNumber = 24; 

#if 0
	STATUS status = m_pCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmFCMTest::FCMCommandCompletionReply,
		NULL);
	delete pCmdInfo;
	pCmdInfo = NULL;
#else 
	// Issue a NAC shutdown request
	MsgFCMNacShutdown *pNacShutdownMsg = new MsgFCMNacShutdown(pCmdInfo);
	
	STATUS status = Send(
				pNacShutdownMsg,
				NULL,
				REPLYCALLBACK(DdmFCM,FCM_NacShutdownMsgReply));
#endif
	return status;
}




//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////
STATUS DdmFCMTest
::TestGetNextWWN()
{
	FCM_CMND_INFO				*pCmdInfo;
	FCM_CMND_PARAMETERS			*pCmdParams;
	FCM_GET_NEXT_WWN			*pGetNextWWN;


	pCmdInfo = new (tZERO) FCM_CMND_INFO;
	pCmdInfo->opcode = FCM_CMND_GET_NEXT_WWN;
	
	pCmdParams = 
			(FCM_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pGetNextWWN = 
			(FCM_GET_NEXT_WWN *) (&pCmdParams->getNextWWN);

#if 0
	STATUS status = m_pCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmFCMTest::FCMCommandCompletionReply,
		NULL);
	delete pCmdInfo;
	pCmdInfo = NULL;
#else 
	// Issue a Get wwn request
	MsgFCMGetWWN *pGetWWNMsg = new MsgFCMGetWWN(pCmdInfo);
	
	STATUS status = Send(
				pGetWWNMsg,
				NULL,
				REPLYCALLBACK(DdmFCM,FCM_GetWWNMsgReply));
#endif
	return status;
}




//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////
STATUS DdmFCMTest
::TestGetChassisWWN()
{
	FCM_CMND_INFO				*pCmdInfo;
	FCM_CMND_PARAMETERS			*pCmdParams;
	FCM_GET_CHASSIS_WWN			*pGetChassisWWN;


	pCmdInfo = new (tZERO) FCM_CMND_INFO;
	pCmdInfo->opcode = FCM_CMND_GET_CHASSIS_WWN;
	
	pCmdParams = 
			(FCM_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pGetChassisWWN = 
			(FCM_GET_CHASSIS_WWN *) (&pCmdParams->getChassisWWN);
	pGetChassisWWN->naaType = NAA2;

#if 1
	STATUS status = m_pCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmFCMTest::FCMCommandCompletionReply,
		NULL);
	delete pCmdInfo;
	pCmdInfo = NULL;
#else 
	// Issue a Get chassis request
	MsgFCMGetChassisWWN *pGetChassisWWNMsg = new MsgFCMGetChassisWWN(pCmdInfo);
	
	STATUS status = Send(
				pGetChassisWWNMsg,
				NULL,
				REPLYCALLBACK(DdmFCM,FCM_GetChassisWWNMsgReply));
#endif
	return status;
}






//************************************************************************
//	FCMMsgCompletionReply
//
//************************************************************************
STATUS DdmFCMTest
::FCM_LoopControlMsgReply(Message *pMsg)
{
	MsgFCMLoopControl	*pLoopControlMsg = (MsgFCMLoopControl *)pMsg;;

	STATUS				status = pMsg->Status();

	CONTEXT				*pContext = (CONTEXT *)pMsg->GetContext();
	FCM_CMND_INFO		*pCmdInfo;
	pLoopControlMsg->GetFCMCmdInfo((void **)&pCmdInfo);
	delete pMsg;
	return status;
}


STATUS DdmFCMTest
::FCM_NacShutdownMsgReply(Message *pMsg)
{
	MsgFCMNacShutdown	*pNacShutdownMsg = (MsgFCMNacShutdown *)pMsg;;

	STATUS				status = pMsg->Status();

	CONTEXT				*pContext = (CONTEXT *)pMsg->GetContext();
	FCM_CMND_INFO		*pCmdInfo;
	pNacShutdownMsg->GetFCMCmdInfo((void **)&pCmdInfo);
	delete pMsg;
	return status;
}


STATUS DdmFCMTest
::FCM_GetWWNMsgReply(Message *pMsg)
{
	MsgFCMGetWWN	*pGetWWNMsg = (MsgFCMGetWWN *)pMsg;;

	STATUS				status = pMsg->Status();

	CONTEXT				*pContext = (CONTEXT *)pMsg->GetContext();
	FCM_CMND_INFO		*pCmdInfo;
	pGetWWNMsg->GetFCMCmdInfo((void **)&pCmdInfo);
	delete pMsg;
	return status;
}


STATUS DdmFCMTest
::FCM_GetChassisWWNMsgReply(Message *pMsg)
{
	MsgFCMGetChassisWWN	*pGetChassisWWNMsg = (MsgFCMGetChassisWWN *)pMsg;

	STATUS				status = pMsg->Status();

	CONTEXT				*pContext = (CONTEXT *)pMsg->GetContext();
	FCM_CMND_INFO		*pCmdInfo;
	pGetChassisWWNMsg->GetFCMCmdInfo((void **)&pCmdInfo);


	FCM_CMND_PARAMETERS			*pCmdParams;
	FCM_GET_CHASSIS_WWN			*pGetChassisWWN;

	pCmdParams = 
			(FCM_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pGetChassisWWN = 
			(FCM_GET_CHASSIS_WWN *) (&pCmdParams->getChassisWWN);

	String16 test;
	memcpy(test, &pGetChassisWWN->chassisWWN, 8);
	delete pMsg;
	return status;
}
