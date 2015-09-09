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
// File: DdmFCMTest.cpp
// 
// Description:
// Test DDM for FCM
// 
// $Log: /Gemini/Odyssey/DdmFCM/DdmFCMTest/DdmFCMTest.cpp $
// 
// 1     1/05/00 5:20p Dpatel
// Initial creation
// 
//
/*************************************************************************/
#include "Buildsys.h"
#include "OsTypes.h"
#include "Message.h"
#include "CTTypes.h"
#include "OsStatus.h"
#include "Ddm.h"
#include "Fields.h"
#include "RequestCodes.h"
#include "DdmFCMTest.h"
#include "Rows.h"



#ifdef WIN32
#include <crtdbg.h>
#endif

CLASSNAME(DdmFCMTest,SINGLE);

/********************************************************************
*
* DdmFCMTest - Constructor
*
********************************************************************/

DdmFCMTest::DdmFCMTest(DID did): Ddm(did)
{
	m_pStringResourceManager = NULL;
}


/********************************************************************
*
* DdmFCMTest - Destructor
*
********************************************************************/
DdmFCMTest::~DdmFCMTest()
{
	if (m_pStringResourceManager){
		delete m_pStringResourceManager;
	}
}

/********************************************************************
*
* DdmFCMTest - Ctor
*
********************************************************************/

Ddm *DdmFCMTest::
Ctor(DID MyDID)
{
	Ddm *pMyDDM = new DdmFCMTest(MyDID);
	return pMyDDM;
}

/********************************************************************
*
* DdmFCMTest - Enable
*
********************************************************************/

STATUS DdmFCMTest::
Enable(Message *pMsg)
{
	// register to listen for new drives
	Ddm::Enable(pMsg);
	return OK;
}

/********************************************************************
*
*
*
********************************************************************/

STATUS DdmFCMTest::
Initialize(Message *pMsg)
{
#ifdef WIN32
	int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

	// Turn On (OR) - 
	tmpFlag |= _CRTDBG_CHECK_ALWAYS_DF;
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
	// Set the new state for the flag
	_CrtSetDbgFlag( tmpFlag );
	tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
#endif

	m_pCmdSender = 
			new CmdSender(
					FCM_CMD_QUEUE_TABLE,
					sizeof(FCM_CMND_INFO),
					sizeof(FCM_EVENT_INFO),	// union of status/event sizes
					this);
	m_pCmdSender->csndrInitialize(
			(pInitializeCallback_t)&DdmFCMTest::FCMObjectInitializedReply);
	m_pCmdSender->csndrRegisterForEvents(
		(pEventCallback_t)&DdmFCMTest::FCMEventHandler);
	m_pStringResourceManager = new StringResourceManager (
										this,
										(pTSCallback_t)&DdmFCMTest::StringResourceManagerInitializedReply);

	Reply(pMsg);
	return OK;
}



/********************************************************************
*
*
*
********************************************************************/

STATUS DdmFCMTest::
DispatchDefault(Message *pMsg)
{
	STATUS			status=OK;
	
	// Return success, we have already delivered the message.
	return status;
}




void DdmFCMTest
::FCMObjectInitializedReply(STATUS status)
{
	status = status;
}


//**************************************************************************
//
//	String Resource Manager Initialized reply
//
//**************************************************************************
STATUS DdmFCMTest
::StringResourceManagerInitializedReply(
			void			*pContext,
			STATUS			status)
{
	if (status == OK) {
		//TestLoopUp();
		//TestNacShutdown();
		//TestGetNextWWN();
		TestGetChassisWWN();
	}
	return status;
}


//**************************************************************************
//
//	Command Completion Reply
//
//**************************************************************************
void DdmFCMTest
::FCMCommandCompletionReply(
			STATUS				completionCode,
			void				*pStatusData,
			void				*pCmdData,
			void				*pCmdContext)
{

	TRACE_STRING(TRACE_RMSTR_1, "\nEnter: DdmFCMTest::rmstrCommandCompletionReply\n");
	FCM_CMND_INFO			*pInfo = (FCM_CMND_INFO *)pCmdData;
	FCM_CMND_INFO			*pInfo1 = (FCM_CMND_INFO *)pStatusData;
	FCM_CMND_PARAMETERS		*pParams = &pInfo->cmdParams;
	FCM_LOOP_CONTROL_INFO	*pLoopControlInfo = 
			(FCM_LOOP_CONTROL_INFO *)&pParams->loopControlInfo;
	FCM_GET_NEXT_WWN		*pGetNextWWN = 
			(FCM_GET_NEXT_WWN *)&pParams->getNextWWN;
	FCM_GET_CHASSIS_WWN		*pGetChassisWWN = 
			(FCM_GET_CHASSIS_WWN *)&pParams->getChassisWWN;


	TRACE_STRING(TRACE_RMSTR_1, "\t***Cmd Submitted***:\n");

	CONTEXT	*pContext = (CONTEXT *)pCmdContext;

	switch(completionCode){
	case FCM_SUCCESS:
		switch (pInfo->opcode){
		case FCM_CMND_LOOP_CONTROL:
			break;
		default:
			break;
		}
	}
	if (pContext){
		delete pContext;
		pContext = NULL;
	}
}


void DdmFCMTest
::FCMEventHandler(
			STATUS			eventCode,
			void			*pStatusData)
{

	TRACEF_NF(TRACE_RMSTR_1,("\nEnter: DdmFCMTest::rmstrEventHandler\n"));
	FCM_LOOP_STATUS				*pEvtLoopStatus = NULL;


	TRACE_STRING(TRACE_RMSTR_1, "\t<<<Event Received>>>:\n");

	switch(eventCode){
	case FCM_EVT_LOOP_UP:
		pEvtLoopStatus = (FCM_LOOP_STATUS *)pStatusData;
		break;
	case FCM_EVT_LOOP_DOWN:
		pEvtLoopStatus = (FCM_LOOP_STATUS *)pStatusData;
		break;
	case FCM_EVT_LIP:
		pEvtLoopStatus = (FCM_LOOP_STATUS *)pStatusData;
		break;
	default:
		break;
	}
}





