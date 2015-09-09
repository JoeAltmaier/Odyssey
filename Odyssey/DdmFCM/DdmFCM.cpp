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
// File: DdmFCM.cpp
// 
// Description:
// Implementation for Fibre Channel Master
// 
// $Log: /Gemini/Odyssey/DdmFCM.cpp $
// 
// 1     1/05/00 5:17p Dpatel
// Initial creation
// 
//
/*************************************************************************/

#include "Buildsys.h"

#include "DdmFCM.h"


CLASSNAME(DdmFCM,SINGLE);

SERVELOCAL (DdmFCM, REQ_FCM_LOOP_CONTROL);
SERVELOCAL (DdmFCM, REQ_FCM_NAC_SHUTDOWN);
SERVELOCAL (DdmFCM, REQ_FCM_GET_WWN);
SERVELOCAL (DdmFCM, REQ_FCM_GET_CHASSIS_WWN);
//************************************************************************
//	CONSTRUCTOR:
//		Initializes all the member variables.
//
//	did		- the did for this DDM
//
//************************************************************************
DdmFCM::DdmFCM(DID did): DdmMaster(did)
{	
	// save our did
	m_FCMDID = did;

	// Initialize all the members
	m_pCmdServer				= NULL;
	m_pInternalCmdSender		= NULL;
	m_DescriptorTablesDefined	= false;

	m_isQuiesced		= false;


	m_CommandInProgress = 0;
	m_failoverInProgress = false;

	m_chassisWWN_NAA2 = 0x200000C04F000000;
	m_chassisWWN_NAA5 = 0x500C04F000000000;
}


//************************************************************************
//	DESTRUCTOR
//		Clean up all allocations done by us
//
//
//************************************************************************
DdmFCM::~DdmFCM()
{
	// Terminate our cmd server
	if (m_pCmdServer){
		m_pCmdServer->csrvTerminate();
	}

	if (m_pInternalCmdSender){
		m_pInternalCmdSender->csndrTerminate();
	}

	// Delete the string resource mgr
	if(m_pStringResourceManager){
		delete m_pStringResourceManager;
	}
}

//************************************************************************
//	Ctor
//		Static ctor
//
//	did		- the did for this DDM
//
//************************************************************************
Ddm *DdmFCM::
Ctor(DID MyDID)
{
	Ddm *pMyDDM = new DdmFCM(MyDID);
	return pMyDDM;
}


//************************************************************************
//	Quiesce
//		Do the quiesce processing by setting a flag for quiesce and
//		saving the reply msg. When FCM is quiesced, it completes
//		the "command in progress" and then queues up any further requests.
//		When enabled, it begins processing the queued cmds.
//
//	pMsg	- the quiesce reply message
//
//************************************************************************
STATUS	DdmFCM::
Quiesce(Message *pMsg)
{
	// set the Quiesce flag, so that no more commands are
	// processed after this is set
	m_isQuiesced = true;
	m_pQuiesceMessage = pMsg;
	return OS_DETAIL_STATUS_SUCCESS;
}


//************************************************************************
//	Enable:
//		Set the quiesce flag to "false" and proceed to check for
//		any queued cmd that needs to be processed.
//
//	pMsg	- the enable reply message
//
//************************************************************************
STATUS DdmFCM::
Enable(Message *pMsg)
{
	m_isQuiesced = false;
	// check for any more commands to run.
	Ddm::Enable(pMsg);
	return OK;
}


//************************************************************************
//	Intialize:
//		Initialize Capabilities
//		Initialize TableServices
//		Initialize StringResourceManager
//		Define the descriptor tables
//	Flow:
//		All tables are defined and capabilities read
//		FCM Data is initialized (reading the existing tables)
//		Command Queues are initialized (FCM)
//		
//	
//	pMsg	- the initialize reply message
//
//************************************************************************
STATUS DdmFCM::
Initialize(Message *pMsg)
{
	DispatchRequest(
			REQ_FCM_LOOP_CONTROL, 
			REQUESTCALLBACK(DdmFCM, ProcessLoopControlMsg));

	DispatchRequest(
			REQ_FCM_NAC_SHUTDOWN, 
			REQUESTCALLBACK(DdmFCM, ProcessNacShutdownMsg));

	DispatchRequest(
			REQ_FCM_GET_WWN, 
			REQUESTCALLBACK(DdmFCM, ProcessGetWWNMsg));

	DispatchRequest(
			REQ_FCM_GET_CHASSIS_WWN, 
			REQUESTCALLBACK(DdmFCM, ProcessGetChassisWWNMsg));

	// save our initialize message
	m_pInitializeMessage = pMsg;

	// create the string resource manager
	m_pStringResourceManager = new StringResourceManager (
										this,
										TSCALLBACK(DdmFCM,FCMStringResourceManagerInitializedReply));
	// Create the tables necessary for FCM	
	FCMDefineDescriptorTables();
	return OK;
}


/********************************************************************
*
*
*
********************************************************************/
STATUS DdmFCM::
BuildVirtualDeviceList()
{
	return OK;
}

/********************************************************************
*
*
*
********************************************************************/
STATUS DdmFCM::
DispatchDefault(Message *pMsg)
{
	STATUS			status=OK;
	
	// Return success, we have already delivered the message.
	return status;
}


//**************************************************************************
//
//	String Resource Manager Initialized reply
//
//**************************************************************************
STATUS DdmFCM
::FCMStringResourceManagerInitializedReply(
			void			*pContext,
			STATUS			status)
{
	pContext = pContext;
	status = status;
	return status;
}


//************************************************************************
//	InitializeCommandQueues
//		Initialize the FCM CMD QUEUE Cmd Server
//
//************************************************************************
void DdmFCM::
InitializeCommandQueues()
{
	// Initialize the CmdServer for RMSTR
	m_pCmdServer = 
			new CmdServer(
					FCM_CMD_QUEUE_TABLE,
					sizeof(FCM_CMND_INFO),
					sizeof(FCM_EVENT_INFO),
					this,
					(pCmdCallback_t)&DdmFCM::FCMCommandReceiver);
	m_pCmdServer->csrvInitialize((pInitializeCallback_t)&DdmFCM::FCMCmdServerInitializedReply);	
}


//************************************************************************
//	FCMCmdServerInitializedReply
//		The Cmd Server initialize reply
//
//	status	- If Cmd Server Object is initailized properly
//
//************************************************************************
void DdmFCM
::FCMCmdServerInitializedReply(STATUS status)
{
	assert(status == 0);

	TRACE_STRING(TRACE_RMSTR,("DdmFCM : CmdServer Initialized Successfully\n"));
	m_pInternalCmdSender = 
			new CmdSender(
					FCM_CMD_QUEUE_TABLE,
					sizeof(FCM_CMND_INFO),
					sizeof(FCM_EVENT_INFO),
					this);
	m_pInternalCmdSender->csndrInitialize(
			(pInitializeCallback_t)&DdmFCM::FCMInternalCmdSenderInitializedReply);
}


//************************************************************************
//	FCMCmdServerInitializedReply
//		The Cmd Server initialize reply
//
//	status	- If Cmd Server Object is initailized properly
//
//************************************************************************
void DdmFCM
::FCMInternalCmdSenderInitializedReply(STATUS status)
{
	assert(status == 0);

	TRACE_STRING(TRACE_RMSTR,("DdmFCM : Internal CmdSender Initialized Successfully\n"));
	Reply(m_pInitializeMessage);
}




//************************************************************************
//	FCMCommandReceiver
//		This is the callback for any inserts into the FCM command
//		queue. So all commands for the RMSTR are received here.
//
//	handle		- Handle for the cmd
//	pCmdData	- Data associated with the cmd (FCM_CMND_INFO)
//
//************************************************************************
void DdmFCM
::FCMCommandReceiver(HANDLE handle, void* pCmdData)
{
	FCM_CMND_INFO				*pCmdInfo;
	FCM_CMND_PARAMETERS			*pCmdParams = NULL;

	pCmdInfo = (FCM_CMND_INFO *)pCmdData;
	pCmdParams = &pCmdInfo->cmdParams;

	FCM_LOOP_CONTROL_INFO		*pLoopControlInfo = NULL;

	TRACE_STRING(TRACE_RMSTR, "\nEnter: DdmFCM::rmstrCommandReceiver\n");
	TRACE_STRING(TRACE_RMSTR, "\t***Cmd Received***:\n");

	// this is not a message based request
	pCmdInfo->isMessage = 0;

	switch (pCmdInfo->opcode){
		case FCM_CMND_LOOP_CONTROL:
			pLoopControlInfo = &(pCmdParams->loopControlInfo);
			switch(pLoopControlInfo->cmnd){
			case LOOP_LIP:
				TRACEF_NF(TRACE_L1,("\n\t\tRESCAN LOOP CMD RCVD\n"));				
				break;
			case LOOP_UP:
				TRACEF_NF(TRACE_L1,("\n\t\tLOOP UP CMD RCVD\n"));
				break;
			case LOOP_DOWN:
				TRACEF_NF(TRACE_L1,("\n\t\tLOOP DOWN CMD RCVD\n"));
				break;
			default:
				TRACEF_NF(TRACE_L1,("\n\t\tINVALID LOOP CMD RCVD\n"));
				break;
			}			
			LoopControlCommandValidation(handle, pCmdInfo);
			break;

		case FCM_CMND_NAC_SHUTDOWN:
			ProcessNacShutdown(handle, pCmdInfo);
			break;

		case FCM_CMND_GET_NEXT_WWN:
			GetNextWWN(handle, pCmdInfo);
			break;

		case FCM_CMND_GET_CHASSIS_WWN:
			GetChassisWWN(handle, pCmdInfo);
			break;

		default:
			assert(0);
			break;
	}
}














