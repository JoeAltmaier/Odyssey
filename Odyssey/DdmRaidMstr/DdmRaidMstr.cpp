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
// File: DdmRaidMstr.cpp
// 
// Description:
// Implementation for Raid Master
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/DdmRaidMstr.cpp $
// 
// 33    2/11/00 5:26p Dpatel
// DFCT12363 - RemoveMember was not being called. This will prevent the
// crash. But defect will be marked as fixed, when GMI disallows
// operation.
// 
// 32    1/07/00 5:39p Agusev
// 
// 31    11/23/99 6:46p Dpatel
// provided dummy vdn for WIN32
// 
// 30    11/03/99 3:31p Dpatel
// added trace..
// 
// 28    9/01/99 6:38p Dpatel
// added logging and alarm code..
// 
// 27    8/27/99 6:42p Dpatel
// start util takes SRC row id instead of ADT row id
// 
// 26    8/24/99 5:13p Dpatel
// added failover code, create array changes for "array name"
// 
// 25    8/20/99 3:03p Dpatel
// added simulation for failover and failover code (CheckAnd...() methods)
// 
// 24    8/16/99 7:04p Dpatel
// Changes for alarms + using rowID * as handle instead of void*
// 
// 23    8/12/99 1:56p Dpatel
// Added Array offline event processing code.
// 
// 22    8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 21    8/05/99 11:07a Dpatel
// internal delete array, fake raid ddm, hot copy auto break, removed
// array name code..
// 
// 20    8/03/99 6:22p Dpatel
// Check on member down, if source member down then change the source
// 
// 19    8/03/99 5:25p Dpatel
// Removed the service method to modify SRC, used table services..
// 
// 18    8/03/99 10:15a Jtaylor
// added RMSTR_RAID_DDM test ifdefs
// 
// 17    8/02/99 3:01p Dpatel
// changes to create array, array FT processing...
// 
// 16    7/30/99 6:40p Dpatel
// Change preferred member, processing member down and stop util as
// internal cmds..
// 
// 15    7/28/99 6:35p Dpatel
// Added capability code, table services, add/remove members, preferred
// member and source member, hot copy etc...
// 
// 14    7/23/99 5:47p Dpatel
// Added internal cmds, hotcopy, changed commit spare etc.
// 
// 13    7/22/99 6:43p Dpatel
// Added unicode string names, changed validation
// 
// 12    7/21/99 5:05p Dpatel
// Iniitialized numberOfDummyRecords to 0
// 
// 11    7/20/99 6:49p Dpatel
// Some bug fixes and changed arrayName in ArrayDescriptor to rowID.
// 
// 10    7/19/99 9:54a Dpatel
// pCmdData was not initialized.
// 
// 9     7/17/99 3:09p Dpatel
// added reading of PTS tables during init.
// 
// 8     7/17/99 1:20p Dpatel
// Queued up commands.
// 
// 7     7/16/99 10:28a Dpatel
// Added DownMember and Commit Spare code. Also removed the reads for
// validation.
// 
// 6     7/09/99 5:26p Dpatel
// 
// 5     7/06/99 4:57p Dpatel
// fixed bugs found in the Utility testing process.
// 
// 4     6/30/99 11:15a Dpatel
// Changes for Abort Util and Chg Priority.
// 
// 3     6/28/99 5:16p Dpatel
// Implemented new methods, changed headers.
// 
//
// 06/11/99 Dipam Patel: Create file
//
/*************************************************************************/

#include "Buildsys.h"

#include "DdmRaidMgmt.h"


CLASSNAME(DdmRAIDMstr,SINGLE);

//************************************************************************
//	CONSTRUCTOR:
//		Initializes all the member variables.
//
//	did		- the did for this DDM
//
//************************************************************************
DdmRAIDMstr::DdmRAIDMstr(DID did): DdmMaster(did)
{	
	// save our did
	m_RmstrDID = did;

	// Initialize all the members
	m_pCmdServer			= NULL;
	m_pInternalCmdSender	= NULL;

	m_pRaidCmdSender		= NULL;
	m_pVCMCmdSender			= NULL;
#ifdef RMSTR_SSAPI_TEST
	m_pFakeRaidDdmCmdServer = NULL;
#endif

	m_RaidCmdSenderInitialized	= false;
	m_DescriptorTablesDefined	= false;

	m_isQuiesced		= false;
	m_SRCIsUsed			= false;

	m_pArrayData		= NULL;
	m_pMemberData		= NULL;
	m_pSpareData		= NULL;
	m_pUtilityData		= NULL;
	m_pCmdData			= NULL;
	m_pAlarmData		= NULL;

	m_pArrayDataTail	= NULL;
	m_pMemberDataTail	= NULL;
	m_pSpareDataTail	= NULL;
	m_pUtilityDataTail	= NULL;
	m_pCmdDataTail		= NULL;
	m_pAlarmDataTail	= NULL;

	m_pRmstrCapability	= NULL;
	m_pTableServices	= NULL;
	m_pHelperServices	= NULL;

	m_CommandInProgress = 0;
	m_failoverInProgress = false;

#ifdef WIN32
	m_dummyVDN = 0;
#endif

#ifdef TEST_TABLES
	m_dummyVDN = DUMMY_VDN;

	m_pSRCDefineTable = NULL;
	m_pSRCListen = NULL;
	
	m_pSRCRecord = NULL;
	m_sizeofSRCRecord = 0;

	m_pModifiedSRCRecord = NULL;
	m_sizeofModifiedSRCRecord = 0;

	m_SRCListenerID = 0;
	m_pNewSRCRecord = NULL;

	m_numberOfDummySRCRecords = 0;
#endif

}


//************************************************************************
//	DESTRUCTOR
//		Clean up all allocations done by us
//
//
//************************************************************************
DdmRAIDMstr::~DdmRAIDMstr()
{
	// Terminate our cmd server
	if (m_pCmdServer){
		m_pCmdServer->csrvTerminate();
	}

	// Terminate the Raid DDM Cmd Sender
	if (m_RaidCmdSenderInitialized){
		m_pRaidCmdSender->csndrTerminate();
	}

	if (m_pVCMCmdSender){
		m_pVCMCmdSender->csndrTerminate();
	}

#ifdef RMSTR_SSAPI_TEST
	// Terminate the Fake RAID DDM Cmd Server which we
	// use for simulation
	if (m_pFakeRaidDdmCmdServer){
		m_pFakeRaidDdmCmdServer->csrvTerminate();
	}
#endif

	// Terminate our internal cmd sender
	if (m_pInternalCmdSender){
		m_pInternalCmdSender->csndrTerminate();

	}
	if (m_pTableServices){
		delete m_pTableServices;
	}
	if (m_pHelperServices){
		delete m_pHelperServices;
	}
	// Delete the string resource mgr
	if(m_pStringResourceManager){
		delete m_pStringResourceManager;
	}
	// Delete all our local pts data
	CleanRmstrData();
}

//************************************************************************
//	Ctor
//		Static ctor
//
//	did		- the did for this DDM
//
//************************************************************************
Ddm *DdmRAIDMstr::
Ctor(DID MyDID)
{
	Ddm *pMyDDM = new DdmRAIDMstr(MyDID);
	return pMyDDM;
}


//************************************************************************
//	Quiesce
//		Do the quiesce processing by setting a flag for quiesce and
//		saving the reply msg. When Rmstr is quiesced, it completes
//		the "command in progress" and then queues up any further requests.
//		When enabled, it begins processing the queued cmds.
//
//	pMsg	- the quiesce reply message
//
//************************************************************************
STATUS	DdmRAIDMstr::
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
STATUS DdmRAIDMstr::
Enable(Message *pMsg)
{
#ifdef RMSTR_RAID_DDM_TEST
	// do a fake quiesce, so that cmds are not processed till
	// we hear the RAID_EVT_ENABLED from RAID DDM
	m_isQuiesced = true;
#else
	m_isQuiesced = false;
#endif	
	// check for any more commands to run.
	RunOutstandingCommand();
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
//		Rmstr Data is initialized (reading the existing tables)
//		Command Queues are initialized (RMSTR, Internal and RAID DDM)
//		
//	
//	pMsg	- the initialize reply message
//
//************************************************************************
STATUS DdmRAIDMstr::
Initialize(Message *pMsg)
{
	// save our initialize message
	m_pInitializeMessage = pMsg;

#ifdef	TEST_TABLES
	CreateSRCTable();
#endif

	// create our RmstrCapability object
	m_pRmstrCapability = new RmstrCapabilities(this);
	// create our TableServices object
	m_pTableServices = new TableServices(this);
	// create our HelperServices object
	m_pHelperServices = new HelperServices(this);
	// create the string resource manager
	m_pStringResourceManager = new StringResourceManager (
										this,
										TSCALLBACK(DdmRAIDMstr,rmstrStringResourceManagerInitializedReply));
	// Create the tables necessary for RMSTR	
	rmstrDefineDescriptorTables();
	return OK;
}


/********************************************************************
*
*
*
********************************************************************/
STATUS DdmRAIDMstr::
BuildVirtualDeviceList()
{
	return OK;
}

/********************************************************************
*
*
*
********************************************************************/
STATUS DdmRAIDMstr::
DispatchDefault(Message *pMsg)
{
	STATUS			status=OK;
	
	if (pMsg->reqCode != RMGMT_DDM_COMMAND)
		return OS_DETAIL_STATUS_INVALID_REQUEST;
	// Return success, we have already delivered the message.
	return status;
}


//**************************************************************************
//
//	String Resource Manager Initialized reply
//
//**************************************************************************
STATUS DdmRAIDMstr
::rmstrStringResourceManagerInitializedReply(
			void			*pContext,
			STATUS			status)
{
	pContext = pContext;
	status = status;
	return status;
}


//************************************************************************
//	InitializeCommandQueues
//		Initialize the RMSTR CMD QUEUE Cmd Server
//		Initialize the RAID_DDM	Cmd Sender
//
//************************************************************************
void DdmRAIDMstr::
InitializeCommandQueues()
{
	// Initialize the CmdServer for RMSTR
	m_pCmdServer = 
			new CmdServer(
					RMSTR_CMD_QUEUE_TABLE,
					sizeof(RMSTR_CMND_INFO),
					sizeof(RMSTR_EVENT_INFO),
					this,
					(pCmdCallback_t)&DdmRAIDMstr::rmstrCommandReceiver);
	m_pCmdServer->csrvInitialize((pInitializeCallback_t)&DdmRAIDMstr::rmstrCmdServerInitializedReply);	


#ifdef RMSTR_SSAPI_TEST
	m_pFakeRaidDdmCmdServer = 
			new CmdServer(
					RAID_COMMAND_QUEUE,
					sizeof(RaidRequest),
					sizeof(RaidEvent),
					this,
					(pCmdCallback_t)&DdmRAIDMstr::rmstrFakeRaidDdmCmdProcessor);
	m_pFakeRaidDdmCmdServer->csrvInitialize((pInitializeCallback_t)&DdmRAIDMstr::rmstrFakeRaidDdmCmdServerInitializedReply);	
#endif

	// Initialize a CmdSender for RAID DDM's
	m_pRaidCmdSender = 
			new CmdSender(
					RAID_COMMAND_QUEUE,
					sizeof(RaidRequest),
					sizeof(RaidEvent),
					this);
	m_pRaidCmdSender->csndrInitialize(
			(pInitializeCallback_t)&DdmRAIDMstr::RaidCmdSenderInitializedReply);
	// Register for events from the RAID ddm
	m_pRaidCmdSender->csndrRegisterForEvents(
		(pEventCallback_t)&DdmRAIDMstr::RaidDdmEventHandler);

	m_pVCMCmdSender	= new CmdSender(
					VCM_CONTROL_QUEUE,
					VCM_CONTROL_COMMAND_SIZE,
					VCM_CONTROL_STATUS_SIZE,
					this);
	m_pVCMCmdSender->csndrInitialize(
		(pInitializeCallback_t)&DdmRAIDMstr::VCMCmdSenderInitializedReply);

	// Do we really need an event handler??
	//m_pVCMCmdSender->csndrRegisterForEvents(
	//	(pEventCallback_t)&DdmRAIDMstr::VCMCEventHandler));
}


//************************************************************************
//	rmstrCmdServerInitializedReply
//		The Cmd Server initialize reply
//
//	status	- If Cmd Server Object is initailized properly
//
//************************************************************************
void DdmRAIDMstr
::rmstrCmdServerInitializedReply(STATUS status)
{
	assert(status == 0);
	TRACE_STRING(TRACE_RMSTR,("DdmRaidMstr : RMSTR CmdServer Initialized Successfully\n"));
	// Initialize a CmdSender for Internal RMSTR commands
	// i.e events or other internal cmds that rmstr might send
	m_pInternalCmdSender = 
			new CmdSender(
					RMSTR_CMD_QUEUE_TABLE,
					sizeof(RMSTR_CMND_INFO),
					sizeof(RMSTR_EVENT_INFO),
					this);
	m_pInternalCmdSender->csndrInitialize(
			(pInitializeCallback_t)&DdmRAIDMstr::rmstrInternalCmdSenderInitializedReply);
}


//************************************************************************
//	rmstrInternalCmdServerInitializedReply:
//		The Cmd Server initialize reply
//
//	status	- If Internal Cmd Server Object is initailized properly
//
//************************************************************************
void DdmRAIDMstr
::rmstrInternalCmdSenderInitializedReply(STATUS status)
{
	assert(status == 0);
	TRACE_STRING(TRACE_RMSTR,("DdmRaidMstr : RMSTR Internal CmdSender Initialized Successfully\n"));
	// Now Rmstr initialize is done, so reply to initialize msg
	Reply(m_pInitializeMessage);
}


//************************************************************************
//	rmstrCommandReceiver
//		This is the callback for any inserts into the RMSTR command
//		queue. So all commands for the RMSTR are received here.
//
//	handle		- Handle for the cmd
//	pCmdData	- Data associated with the cmd (RMSTR_CMND_INFO)
//
//************************************************************************
void DdmRAIDMstr
::rmstrCommandReceiver(HANDLE handle, void* pCmdData)
{
	RMSTR_CMND_INFO					*pCmdInfo;

	pCmdInfo = (RMSTR_CMND_INFO *)pCmdData;

	if (pCmdInfo->opcode > RMSTR_CMND_LAST_VALID){
		// internal cmd, needs to be processed before anything else
		// so add to head of queue
		AddCommandToQueue(handle, pCmdInfo, true);
	} else {
		// cmd from any RMSTR client, add to queue tail
		AddCommandToQueue(handle, pCmdInfo, false);
	}
}


//************************************************************************
//	AddCommandToQueue
//		Add the command to the queue, so that we can queue up cmds, while
//		we are quiesced or processing another request.
//
//	handle		- Handle for the cmd
//	pCmdInfo	- Data associated with the cmd (RMSTR_CMND_INFO)
//	addToHead	- true - add to head, false-add to tail
//
//************************************************************************
void DdmRAIDMstr::
AddCommandToQueue(
		HANDLE					handle,
		RMSTR_CMND_INFO			*pCmdInfo,
		BOOL					addToHead)
{
	RMSTR_QUEUED_CMND		queuedCmd;

	queuedCmd.rowId = *(rowID *)handle;
	memcpy(&queuedCmd.cmdInfo, pCmdInfo, sizeof(RMSTR_CMND_INFO));


	if (addToHead){
		AddRmstrDataToHead(
			RAID_CMND,
			&queuedCmd.rowId,
			&queuedCmd);
	} else {
		// Add the command to the Queue tail
		AddRmstrData(
			RAID_CMND,
			&queuedCmd.rowId,
			&queuedCmd);
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
void DdmRAIDMstr::
RunOutstandingCommand()
{
	// If Quiesce request, then dont send any further cmds,
	// wait for outstanding cmds to finish...
	if (m_isQuiesced == true){
		if (m_CommandInProgress == true){
			// wait for cmd to finish
		} else {
#ifdef RMSTR_RAID_DDM_TEST
#else		
			// Reply to Quiese, that we are done quiescing..
			Reply(m_pQuiesceMessage);
#endif			
		}
	} else {
		// If no command in queue, start the next command
		if (m_CommandInProgress == false){
			RMSTR_QUEUED_CMND		*pQueuedCmd;
			// get first command
			TraverseRmstrData(
				RAID_CMND,
				NULL,
				(void **)&pQueuedCmd);
			if (pQueuedCmd){
				StartCommandProcessing(
					&pQueuedCmd->rowId,
					&pQueuedCmd->cmdInfo);
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
//	pCmdInfo		- the cmd data (RMSTR_CMND_INFO)
//
//************************************************************************
void DdmRAIDMstr::
StartCommandProcessing(
			HANDLE				handle,
			RMSTR_CMND_INFO		*pCmdInfo)
{
	RMSTR_CMND_PARAMETERS			*pCmdParams = NULL;
	RMSTR_CREATE_ARRAY_DEFINITION	*pArrayDef = NULL;
	RMSTR_CREATE_SPARE_INFO			*pCreateSpareInfo = NULL;
	RMSTR_DELETE_SPARE_INFO			*pDeleteSpareInfo = NULL;
	RMSTR_START_UTIL_INFO			*pStartUtilInfo = NULL;
	RMSTR_ABORT_UTIL_INFO			*pAbortUtilInfo = NULL;

	pCmdParams = &pCmdInfo->cmdParams;

	TRACE_STRING(TRACE_RMSTR, "\nEnter: DdmRAIDMstr::rmstrCommandReceiver\n");
	TRACE_STRING(TRACE_RMSTR, "\t***Cmd Received***:\n");
	TRACEF_NF(TRACE_RMSTR,("\t\tCmd=%s\n", 
				dispCommandName[pCmdInfo->opcode]));

	assert(m_CommandInProgress == false);

	m_CommandInProgress = true;
	switch (pCmdInfo->opcode){
		case RMSTR_CMND_CREATE_ARRAY:
			pArrayDef = 
				(RMSTR_CREATE_ARRAY_DEFINITION *)&pCmdParams->createArrayDefinition;
			TRACEF_NF(TRACE_L1,("\n\t\tCREATE ARRAY\n"));				
			caCreateArrayValidation(handle, pCmdInfo);
			break;

		case RMSTR_CMND_DELETE_ARRAY:
		case RMSTR_INTERNAL_CMND_DELETE_ARRAY:
			TRACEF_NF(TRACE_L1,("\n\t\tDELETE ARRAY\n"));		
			DeleteArrayValidation(handle, pCmdInfo);
			break;

		case RMSTR_CMND_CREATE_SPARE:
			pCreateSpareInfo = 
				(RMSTR_CREATE_SPARE_INFO *)&pCmdParams->createSpareInfo;
			TRACEF_NF(TRACE_L1,("\n\t\tCREATE SPARE\n"));
			rmstrCreateSpareValidation(handle, pCmdInfo);
			break;

		case RMSTR_CMND_DELETE_SPARE:
		
			pDeleteSpareInfo = 
				(RMSTR_DELETE_SPARE_INFO *)&pCmdParams->deleteSpareInfo;
			TRACEF_NF(TRACE_L1,("\n\t\tDELETE SPARE: rowid=0x%x 0x%x 0x%x\n",
				pDeleteSpareInfo->spareId.Table,
				pDeleteSpareInfo->spareId.HiPart,
				pDeleteSpareInfo->spareId.LoPart));
			DeleteSpareValidation(handle, pCmdInfo);
			break;

		case RMSTR_CMND_START_UTIL:
		case RMSTR_INTERNAL_CMND_START_UTIL:
			pStartUtilInfo = 
				(RMSTR_START_UTIL_INFO *)&pCmdParams->startUtilInfo;

			TRACEF_NF(TRACE_L1,("\n\t\tSTART UTIL: =%s\n", 
					dispUtilityName[pStartUtilInfo->utilityName]));
			rmstrStartUtilReadTarget(
					handle, 
					pCmdInfo);
			break;

		case RMSTR_CMND_ABORT_UTIL:
			pAbortUtilInfo = 
				(RMSTR_ABORT_UTIL_INFO *)&pCmdParams->abortUtilInfo;

			TRACEF_NF(TRACE_L1, ("\n\t\tABORT UTIL: row id=%x %x %x\n", 
				pAbortUtilInfo->utilRowId.Table,
				pAbortUtilInfo->utilRowId.HiPart,
				pAbortUtilInfo->utilRowId.LoPart));

			AbortUtilValidation(handle, pCmdInfo);
			break;

		case RMSTR_CMND_CHANGE_UTIL_PRIORITY:
			TRACEF_NF(TRACE_L1,("\n\t\tCHANGE UTIL PRIORITY\n"));
			ChangePriorityValidation(handle, pCmdInfo);
			break;

		case RMSTR_CMND_DOWN_A_MEMBER:
			TRACEF_NF(TRACE_L1,("\n\t\tDOWN A MEMBER\n"));		
			DownAMemberValidation(handle, pCmdInfo);
			break;

		case RMSTR_CMND_ADD_MEMBER:
			TRACEF_NF(TRACE_L1,("\n\t\tADD A MEMBER\n"));		
			AddMemberValidation(handle, pCmdInfo);
			break;

		case RMSTR_CMND_REMOVE_MEMBER:
			TRACEF_NF(TRACE_L1,("\n\t\tREMOVE A MEMBER\n"));
			RemoveMember(handle, pCmdInfo);
			break;

		case RMSTR_CMND_CHANGE_SOURCE_MEMBER:
		case RMSTR_INTERNAL_CMND_CHANGE_SOURCE_MEMBER:
			TRACEF_NF(TRACE_L1,("\n\t\tCHANGE SOURCE MEMBER\n"));		
			ChangeSourceMemberValidation(handle,pCmdInfo);
			break;

		case RMSTR_CMND_CHANGE_PREFERRED_MEMBER:
			TRACEF_NF(TRACE_L1,("\n\t\tCHANGE PREFERRED MEMBER\n"));				
			ChangePreferredMemberValidation(handle,pCmdInfo);
			break;

		case RMSTR_INTERNAL_CMND_COMMIT_SPARE:
			// This is an internal command, cannot be issued thru interface opcodes
			TRACEF_NF(TRACE_L1,("\n\t\tPROCESS INTERNAL COMMIT SPARE\n"));					
			CommitSpare(handle, pCmdInfo);
			break;

		case RMSTR_INTERNAL_CMND_PROCESS_MEMBER_DOWN_EVENT:
			// This is an internal command, cannot be issued thru interface opcodes
			TRACEF_NF(TRACE_L1,("\n\t\tPROCESS MEMBER DOWN EVENT\n"));					
			ProcessDownMemberEvent(handle, pCmdInfo);
			break;

		case RMSTR_INTERNAL_CMND_PROCESS_ARRAY_OFFLINE_EVENT:
			// This is an internal command, cannot be issued thru interface opcodes
			TRACEF_NF(TRACE_L1,("\n\t\tPROCESS ARRAY OFFLINE EVENT\n"));			
			ProcessArrayOfflineEvent(handle, pCmdInfo);	
			break;

		case RMSTR_INTERNAL_CMND_PROCESS_STOP_UTIL_EVENT:
			// This is an internal command, cannot be issued thru interface opcodes
			TRACEF_NF(TRACE_L1,("\n\t\tPROCESS STOP UTIL EVENT\n"));			
			ProcessStopUtilEvent(handle, pCmdInfo);
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
void DdmRAIDMstr::
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
		RemoveRmstrData(
			RAID_CMND,
			(rowID *)handle);
		m_CommandInProgress = false;
		if (checkForNextCmd)
			RunOutstandingCommand();
	}
}






//************************************************************************
//	VCMCmdSenderInitializedReply
//
//************************************************************************
void DdmRAIDMstr
::VCMCmdSenderInitializedReply(STATUS status)
{
	assert(!status);
}

#ifdef WIN32
STATUS DdmRAIDMstr::StartHotCopy(
			RAID_ARRAY_DESCRIPTOR	*_pADTRecord,
			pTSCallback_t			pCallback,
			CONTEXT					*_pParentContext){

		return OK;
	}
#endif




