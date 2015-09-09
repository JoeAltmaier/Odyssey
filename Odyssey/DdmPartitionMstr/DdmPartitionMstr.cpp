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
// File: DdmPartitionMstr.cpp
// 
// Description:
// Implementation for Partition Master
// 
// $Log: /Gemini/Odyssey/DdmPartitionMstr/DdmPartitionMstr.cpp $
// 
// 1     9/15/99 4:00p Dpatel
// Initial creation
// 
//
/*************************************************************************/

#include "Buildsys.h"
#include "DdmPartitionMstr.h"


CLASSNAME(DdmPartitionMstr,SINGLE);

//************************************************************************
//	CONSTRUCTOR:
//		Initializes all the member variables.
//
//	did		- the did for this DDM
//
//************************************************************************
DdmPartitionMstr::DdmPartitionMstr(DID did): DdmMaster(did)
{
	

	m_PMSTRDID					= did;
	m_DescriptorTablesDefined	= false;
	m_pInitializeMessage		= NULL;
	m_pQuiesceMessage			= NULL;

	m_pCmdServer				= NULL;


	m_pTableServices			= NULL;
	m_pDataQueue				= NULL;
	m_pStringResourceManager	= NULL;
	m_pHelperServices			= NULL;

	m_isQuiesced				= false;
	m_CommandInProgress			= false;
	m_failoverInProgress		= false;
}


//************************************************************************
//	DESTRUCTOR
//		Clean up all allocations done by us
//
//
//************************************************************************
DdmPartitionMstr::~DdmPartitionMstr()
{
	// Terminate our cmd server
	if (m_pCmdServer){
		m_pCmdServer->csrvTerminate();
	}
	if (m_pHelperServices){
		delete m_pHelperServices;
	}

	if (m_pTableServices){
		delete m_pTableServices;
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
Ddm *DdmPartitionMstr::
Ctor(DID MyDID)
{
	Ddm *pMyDDM = new DdmPartitionMstr(MyDID);
	return pMyDDM;
}


//************************************************************************
//	Quiesce
//		Do the quiesce processing by setting a flag for quiesce and
//		saving the reply msg. When Pmstr is quiesced, it completes
//		the "command in progress" and then queues up any further requests.
//		When enabled, it begins processing the queued cmds.
//
//	pMsg	- the quiesce reply message
//
//************************************************************************
STATUS	DdmPartitionMstr::
Quiesce(Message *pMsg)
{
	m_pQuiesceMessage = pMsg;
	m_isQuiesced = true;
	Reply(m_pQuiesceMessage);
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
STATUS DdmPartitionMstr::
Enable(Message *pMsg)
{
	m_isQuiesced = false;
	RunOutstandingCommand();
	Reply(pMsg);
	return OK;
}


//************************************************************************
//	Intialize:
//		Initialize TableServices
//		Initialize StringResourceManager
//		Define the descriptor tables
//	Flow:
//		
//	
//	pMsg	- the initialize reply message
//
//************************************************************************
STATUS DdmPartitionMstr::
Initialize(Message *pMsg)
{
	// save our initialize message
	m_pInitializeMessage = pMsg;

	// create our helper services object
	m_pHelperServices = new HelperServices(this);

	// create our TableServices object
	m_pTableServices = new TableServices(this);

	// initialize our Data Queue to hold our local data
	m_pDataQueue = new DataQueue(this, m_pTableServices);

	// create the string resource manager
	m_pStringResourceManager = new StringResourceManager (
										this,
										(pTSCallback_t)&DdmPartitionMstr::PmstrStringResourceManagerInitializedReply);
	// Create the tables necessary for Pmstr	
	DefineDescriptorTables();
	return OK;
}





//**************************************************************************
//
//	String Resource Manager Initialized reply
//
//**************************************************************************
STATUS DdmPartitionMstr
::PmstrStringResourceManagerInitializedReply(
			void			*pContext,
			STATUS			status)
{
	UNUSED(pContext);
	assert(status == 0);
	return status;
}


//************************************************************************
//	InitializeCommandQueues
//		Initialize the Pmstr CMD QUEUE Cmd Server
//		Initialize the RAID_DDM	Cmd Sender
//
//************************************************************************
void DdmPartitionMstr::
InitializeCommandQueues()
{
	// Initialize the CmdServer for Pmstr
	m_pCmdServer = 
			new CmdServer(
					PARTITION_MSTR_CMD_QUEUE,
					sizeof(PMSTR_CMND_INFO),
					sizeof(PMSTR_EVENT_INFO),
					this,
					(pCmdCallback_t)&DdmPartitionMstr::PmstrCommandReceiver);
	m_pCmdServer->csrvInitialize(
			(pInitializeCallback_t)&DdmPartitionMstr::PmstrCmdServerInitializedReply);
}


//************************************************************************
//	PmstrCmdServerInitializedReply
//		The Cmd Server initialize reply
//
//	status	- If Cmd Server Object is initailized properly
//
//************************************************************************
void DdmPartitionMstr
::PmstrCmdServerInitializedReply(STATUS status)
{
	assert(status == 0);
	Reply(m_pInitializeMessage);
}



//************************************************************************
//	PmstrCommandReceiver
//		This is the callback for any inserts into the Pmstr command
//		queue. So all commands for the Pmstr are received here.
//
//	handle		- Handle for the cmd
//	pCmdData	- Data associated with the cmd (Pmstr_CMND_INFO)
//
//************************************************************************
void DdmPartitionMstr
::PmstrCommandReceiver(HANDLE handle, void* pCmdData)
{
	PMSTR_CMND_INFO					*pCmdInfo;

	pCmdInfo = (PMSTR_CMND_INFO *)pCmdData;

	if (pCmdInfo->opcode > PMSTR_CMND_LAST_VALID){
		// internal cmd, needs to be processed before anything else
		// so add to head of queue
		m_pDataQueue->Add(
				PMSTR_CMND, 
				(rowID *)handle, 
				NULL,			// state Identifier
				pCmdInfo, 
				sizeof(PMSTR_CMND_INFO),
				false);
	} else {
		// cmd from any Pmstr client, add to queue tail
		m_pDataQueue->Add(
				PMSTR_CMND, 
				(rowID *)handle, 
				NULL,			// state identifier
				pCmdInfo, 
				sizeof(PMSTR_CMND_INFO));
	}
	RunOutstandingCommand();
}


	
//************************************************************************
//	RunOutstandingCommand
//		Pick the cmd from the head of the queue and run it. If
//		we are quiesced, or another command is in progress,
//		dont run.
//
//************************************************************************
void DdmPartitionMstr::
RunOutstandingCommand()
{
	rowID					*pDataKey = NULL;
	PMSTR_CMND_INFO			*pCmdInfo = NULL;

	// If Quiesce request, then dont send any further cmds,
	// wait for outstanding cmds to finish...
	if (m_isQuiesced == true){
		if (m_CommandInProgress == true){
			// wait for cmd to finish
		} else {
			// Reply to Quiese, that we are done quiescing..
			Reply(m_pQuiesceMessage);
		}
	} else {
		// If no command in queue, start the next command
		if (m_CommandInProgress == false){
			// get first command
			m_pDataQueue->Traverse(
				PMSTR_CMND,				// type
				NULL,					// row id (key)
				(void **)&pCmdInfo,		// Return data
				&pDataKey);				// return key
			if (pDataKey){
				StartCommandProcessing(
					pDataKey,
					pCmdInfo);
			}
		}
	}
}


//************************************************************************
//	StartCommandProcessing
//		It will process the cmd data.
//		It will set the m_commandInProgress to true.
//		It is required to set this flag to false, after command is
//		processed. Otherwise program flow will be blocked.
//
//	handle			- handle of the cmd to start
//	pCmdInfo		- the cmd data (Pmstr_CMND_INFO)
//
//************************************************************************
void DdmPartitionMstr::
StartCommandProcessing(
			HANDLE				handle,
			PMSTR_CMND_INFO		*pCmdInfo)
{
	PMSTR_CMND_PARAMETERS			*pCmdParams = NULL;

	pCmdParams = &pCmdInfo->cmdParams;

	TRACE_STRING(TRACE_RMSTR, "\nEnter: DdmPartitionMstr::PmstrCommandReceiver\n");
	TRACE_STRING(TRACE_RMSTR, "\t***Cmd Received***:\n");

	assert(m_CommandInProgress == false);

	m_CommandInProgress = true;

	switch (pCmdInfo->opcode){
		case PMSTR_CMND_CREATE_PARTITION:
			CreatePartitionValidation(handle, pCmdInfo);
			break;

		case PMSTR_CMND_MERGE_PARTITIONS:
			MergePartitionValidation(handle, pCmdInfo);
			break;

		default:
			m_CommandInProgress = false;
			break;
	}
}




//************************************************************************
//	StopCommandProcessing
//		This method is called after a command is completely processed.
//		It will set the m_commandInProgress to false.
//
//	checkForNextCmd		- if true, look for next cmd and run it.
//	handle				- handle of the cmd to stop
//
//************************************************************************
void DdmPartitionMstr::
StopCommandProcessing(
		BOOL				checkForNextCmd,
		HANDLE				handle)
{
	assert(handle);
	if (checkForNextCmd == START_FAILOVER_SIMULATION){
		m_CommandInProgress = false;
		m_failoverInProgress = true;
		RunOutstandingCommand();
	} else {
		m_failoverInProgress = false;
		m_pDataQueue->Remove(
				PMSTR_CMND,
				(rowID *)handle);
		m_CommandInProgress = false;
		if (checkForNextCmd){
			RunOutstandingCommand();
		}
	}
}


//************************************************************************
//		Simulate Fail over
//
//************************************************************************
BOOL DdmPartitionMstr
::SimulateFailover(PARTITION_CONTEXT *pCmdContext)
{
	// if failover was not in progress
	if (m_failoverInProgress == false){
		StopCommandProcessing(START_FAILOVER_SIMULATION, pCmdContext->cmdHandle);
		if (pCmdContext) {
			delete pCmdContext;
			pCmdContext = NULL;
		}
		return true;
	}
	return false;
}






