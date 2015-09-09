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
// File: DdmRmstrTest.cpp
// 
// Description:
// Test DDM for Raid master
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/DdmRaidMstrTest.cpp $
// 
// 23    8/31/99 6:39p Dpatel
// events, util abort processing etc.
// 
// 22    8/24/99 5:13p Dpatel
// added failover code, create array changes for "array name"
// 
// 21    8/20/99 3:03p Dpatel
// added simulation for failover and failover code (CheckAnd...() methods)
// 
// 20    8/16/99 7:04p Dpatel
// Changes for alarms + using rowID * as handle instead of void*
// 
// 19    8/12/99 1:56p Dpatel
// Added Array offline event processing code.
// 
// 18    8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 17    8/05/99 11:07a Dpatel
// internal delete array, fake raid ddm, hot copy auto break, removed
// array name code..
// 
// 16    8/03/99 6:22p Dpatel
// Check on member down, if source member down then change the source
// 
// 15    8/03/99 5:25p Dpatel
// Removed the service method to modify SRC, used table services..
// 
// 14    8/02/99 3:01p Dpatel
// changes to create array, array FT processing...
// 
// 13    7/30/99 6:40p Dpatel
// Change preferred member, processing member down and stop util as
// internal cmds..
// 
// 12    7/28/99 6:35p Dpatel
// Added capability code, table services, add/remove members, preferred
// member and source member, hot copy etc...
// 
// 11    7/23/99 5:47p Dpatel
// Added internal cmds, hotcopy, changed commit spare etc.
// 
// 10    7/22/99 6:43p Dpatel
// Added unicode string names, changed validation
// 
// 9     7/20/99 6:49p Dpatel
// Some bug fixes and changed arrayName in ArrayDescriptor to rowID.
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
#include "OsTypes.h"
#include "Message.h"
#include "CTTypes.h"
#include "OsStatus.h"
#include "Ddm.h"
#include "Fields.h"
#include "RequestCodes.h"
#include "DdmRAIDMstrTest.h"
#include "Rows.h"



#ifdef WIN32
#include <crtdbg.h>
#endif

CLASSNAME(DdmRAIDMstrTest,SINGLE);

/********************************************************************
*
* DdmRAIDMstrTest - Constructor
*
********************************************************************/

DdmRAIDMstrTest::DdmRAIDMstrTest(DID did): Ddm(did)
{
	m_pStringResourceManager = NULL;
}


/********************************************************************
*
* DdmRAIDMstrTest - Destructor
*
********************************************************************/
DdmRAIDMstrTest::~DdmRAIDMstrTest()
{
	if (m_pStringResourceManager){
		delete m_pStringResourceManager;
	}
}

/********************************************************************
*
* DdmRAIDMstrTest - Ctor
*
********************************************************************/

Ddm *DdmRAIDMstrTest::
Ctor(DID MyDID)
{
	Ddm *pMyDDM = new DdmRAIDMstrTest(MyDID);
	return pMyDDM;
}

/********************************************************************
*
* DdmRAIDMstrTest - Enable
*
********************************************************************/

STATUS DdmRAIDMstrTest::
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

STATUS DdmRAIDMstrTest::
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
					RMSTR_CMD_QUEUE_TABLE,
					sizeof(RMSTR_CMND_INFO),
					sizeof(RMSTR_EVENT_INFO),	// union of status/event sizes
					this);
	m_pCmdSender->csndrInitialize(
			(pInitializeCallback_t)&DdmRAIDMstrTest::tstObjectInitializedReply);
	m_pCmdSender->csndrRegisterForEvents(
		(pEventCallback_t)&DdmRAIDMstrTest::rmstrEventHandler);
	m_pStringResourceManager = new StringResourceManager (
										this,
										(pTSCallback_t)&DdmRAIDMstrTest::StringResourceManagerInitializedReply);

	Ddm::Initialize(pMsg);
	return OK;
}



/********************************************************************
*
*
*
********************************************************************/

STATUS DdmRAIDMstrTest::
DispatchDefault(Message *pMsg)
{
	STATUS			status=OK;
	
	if (pMsg->reqCode != RMGMT_DDM_COMMAND)
		return OS_DETAIL_STATUS_INVALID_REQUEST;
	// Return success, we have already delivered the message.
	return status;
}




void DdmRAIDMstrTest
::tstObjectInitializedReply(STATUS status)
{
	status = status;
}


//**************************************************************************
//
//	String Resource Manager Initialized reply
//
//**************************************************************************
STATUS DdmRAIDMstrTest
::StringResourceManagerInitializedReply(
			void			*pContext,
			STATUS			status)
{
	if (status == OK) {
		// Note : num spares = 0 for hot copy
		TestCreateArray("FirstArray",false, 2, 2);
		//TestCreateArray("FirstArray",true, 2, 0);
		//TestCreateArray("F", false, 2, 1);
	}
	return status;
}


//**************************************************************************
//
//	Command Completion Reply
//
//**************************************************************************
void DdmRAIDMstrTest
::rmstrCommandCompletionReply(
			STATUS				completionCode,
			void				*pStatusData,
			void				*pCmdData,
			void				*pCmdContext)
{
	UnicodeString				ucNewArrayName;
	StringClass					scNewArrayName;

	TRACE_STRING(TRACE_RMSTR_1, "\nEnter: DdmRAIDMstrTest::rmstrCommandCompletionReply\n");
	RMSTR_CMND_INFO *pInfo = (RMSTR_CMND_INFO *)pCmdData;
	RMSTR_CMND_PARAMETERS *pParams = &pInfo->cmdParams;
	RMSTR_CREATE_ARRAY_DEFINITION *pArrayDef = 
			(RMSTR_CREATE_ARRAY_DEFINITION *)&pParams->createArrayDefinition;
	RMSTR_DELETE_ARRAY_INFO *pDeleteArrayInfo = 
			(RMSTR_DELETE_ARRAY_INFO *)&pParams->deleteArrayInfo;

	RMSTR_START_UTIL_INFO		*pStartUtilInfo =
			(RMSTR_START_UTIL_INFO *)&pParams->startUtilInfo;
	RMSTR_ABORT_UTIL_INFO		*pAbortUtilInfo =
			(RMSTR_ABORT_UTIL_INFO *)&pParams->abortUtilInfo;
	RMSTR_CHANGE_PRIORITY_INFO		*pChangePriorityInfo =
			(RMSTR_CHANGE_PRIORITY_INFO *)&pParams->changePriorityInfo;


	TRACE_STRING(TRACE_RMSTR_1, "\t***Cmd Submitted***:\n");
	TRACEF_NF(TRACE_RMSTR_1,("\t\tCmd=%s\n", 
				dispCommandName[pInfo->opcode]));

	CONTEXT	*pContext = (CONTEXT *)pCmdContext;
	U32		x;
	switch(completionCode){
	case RMSTR_SUCCESS:
		switch (pInfo->opcode){
		case RMSTR_CMND_CREATE_ARRAY:
#if 0
			ucNewArrayName = UnicodeString(pArrayDef->arrayName);
			ucNewArrayName.GetAsciiString(scNewArrayName);
			TRACEF_NF(TRACE_RMSTR_1, ("\t\tArrayName=%s\n", scNewArrayName.CString()));
#endif
			break;

		case RMSTR_CMND_DELETE_ARRAY:
			//TRACEF_NF(TRACE_RMSTR_1, ("\t\t", pDeleteArrayInfo->arrayRowId));
			break;


		case RMSTR_CMND_CREATE_SPARE:
			x = pContext->value;
			break;

		case RMSTR_CMND_DELETE_SPARE:
			break;

		case RMSTR_CMND_START_UTIL:
			TRACEF_NF(TRACE_RMSTR_1,("\t\tUtil=%s\n", 
				dispUtilityName[pStartUtilInfo->utilityName]));
			break;

		case RMSTR_CMND_ABORT_UTIL:
			TRACEF_NF(TRACE_RMSTR_1,("\t\tUtil Abort Cmd Reply\n"));
			break;

		case RMSTR_CMND_CHANGE_UTIL_PRIORITY:
			TRACEF_NF(TRACE_RMSTR_1,("\t\tChange Priority Cmd Reply\n"));
			break;

		case RMSTR_CMND_ADD_MEMBER:
			TRACEF_NF(TRACE_RMSTR_1,("\t\tAdd Members Cmd Reply\n"));
			break;

		case RMSTR_CMND_CHANGE_PREFERRED_MEMBER:
			TRACEF_NF(TRACE_RMSTR_1,("\t\tChange Preferred Member Cmd Reply\n"));
			break;

		case RMSTR_CMND_CHANGE_SOURCE_MEMBER:
			TRACEF_NF(TRACE_RMSTR_1,("\t\tChange Source Member Cmd Reply\n"));
			break;

		default:
			TRACEF_NF(TRACE_RMSTR_1,("\tERROR: Unknown Cmd: Opcode=0x%x\n", pInfo->opcode));
			break;
		}
		break;
	default:
		TRACEF_NF(TRACE_RMSTR_1,("\t\tErrorCode=%s\n", 
				dispErrorName[completionCode]));
		break;
	}
	if (pContext){
		delete pContext;
		pContext = NULL;
	}
}


void DdmRAIDMstrTest
::rmstrEventHandler(
			STATUS			eventCode,
			void			*pStatusData)
{
	U32					i=0;
	rowID				dedicatedSpareId;
	rowID				addMemberId;

	TRACEF_NF(TRACE_RMSTR_1,("\nEnter: DdmRAIDMstrTest::rmstrEventHandler\n"));
	RMSTR_EVT_ARRAY_ADDED_STATUS		*pEvtArrayAdded = NULL;
	RMSTR_EVT_ARRAY_DELETED_STATUS		*pEvtArrayDeleted = NULL;
	RMSTR_EVT_SPARE_ADDED_STATUS		*pEvtSpareAdded = NULL;
	RMSTR_EVT_SPARE_DELETED_STATUS		*pEvtSpareDeleted = NULL;
	RMSTR_EVT_UTIL_STARTED_STATUS		*pEvtUtilStarted = NULL;
	RMSTR_EVT_UTIL_STOPPED_STATUS		*pEvtUtilStopped = NULL;
	RMSTR_PRIORITY_CHANGED_STATUS		*pEvtPriorityChanged = NULL;
	RMSTR_PERCENT_COMPLETE_STATUS		*pEvtPercentComplete = NULL;
	RMSTR_EVT_MEMBER_DOWN_STATUS		*pEvtMemberDown = NULL;
	RMSTR_EVT_MEMBER_ADDED_STATUS		*pEvtMemberAdded = NULL;
	RMSTR_EVT_SPARE_ACTIVATED_STATUS	*pEvtSpareActivated = NULL;
	RMSTR_EVT_ARRAY_CRITICAL_STATUS		*pEvtArrayCriticalStatus = NULL;
	RMSTR_EVT_ARRAY_OFFLINE_STATUS		*pEvtArrayOfflineStatus = NULL;
	RMSTR_EVT_PREFERRED_MEMBER_CHANGED_STATUS		*pEvtPreferredMemberChanged = NULL;
	RMSTR_EVT_SOURCE_MEMBER_CHANGED_STATUS			*pEvtSourceMemberChanged = NULL;
	RMSTR_EVT_ARRAY_FAULT_TOLERANT_STATUS			*pEvtArrayFaultTolerant = NULL;

	RAID_UTIL_POLICIES					utilPolicy;
	rowID								tempRowId = {9,0,1};

	UnicodeString				ucNewArrayName;
	StringClass					scNewArrayName;


	TRACE_STRING(TRACE_RMSTR_1, "\t<<<Event Received>>>:\n");
	TRACEF_NF(TRACE_RMSTR_1,("\t\tEvent=%s\n", 
				dispEventName[eventCode]));
	switch(eventCode){
	case RMSTR_EVT_ARRAY_ADDED:
		pEvtArrayAdded = (RMSTR_EVT_ARRAY_ADDED_STATUS *)pStatusData;
		PrintArrayData(&pEvtArrayAdded->arrayData);
		// Display the array name
		ucNewArrayName = UnicodeString(pEvtArrayAdded->arrayName);
		ucNewArrayName.GetAsciiString(scNewArrayName);
		TRACEF_NF(TRACE_RMSTR_1, ("\t\tArrayName=%s\n", scNewArrayName.CString()));
#if 0
		addMemberId.Table = pEvtArrayAdded->arrayData.SRCTRID.Table;
		addMemberId.HiPart = 0;
		addMemberId.LoPart = 4;		// 5 is still free

		// Add a member
		TestAddMember(
			&pEvtArrayAdded->arrayData.thisRID,
			&addMemberId);
#endif

#if 0
		TestChangePreferredMember(
				&pEvtArrayAdded->arrayData.thisRID,
				&pEvtArrayAdded->arrayData.members[1]);
#endif
#if 0
		TestChangeSourceMember(
				&pEvtArrayAdded->arrayData.thisRID,
				&pEvtArrayAdded->arrayData.members[1]);
#endif

#if 1
		// Add a Dedicated Spare
		dedicatedSpareId.Table = pEvtArrayAdded->SRCData.rid.Table;
		dedicatedSpareId.HiPart = 0;
		dedicatedSpareId.LoPart = 5;		// 5 is still free
		TestAddSpare(
			//RAID_HOST_POOL_SPARE,
			//RAID_GENERAL_POOL_SPARE,
			RAID_DEDICATED_SPARE,
			&dedicatedSpareId,
			//NULL,
			&pEvtArrayAdded->arrayData.thisRID,	// target rid
			NULL);
#endif
#if 0
		memset(&utilPolicy,0,sizeof(RAID_UTIL_POLICIES));
		// Start a verify on the Array
		utilPolicy.SilentMode = 1;
		utilPolicy.RunThruErrors = 1;
		utilPolicy.SpecifyMembersToRunOn = 0;
		TestStartUtility(
			&pEvtArrayAdded->arrayData.thisRID,	// target rid
			RAID_UTIL_VERIFY,
			PRIORITY_HIGH,
			utilPolicy,
			5);							// %complete update rate
#endif

#if 0
		// Down a member
		rowID		memberRowId = pEvtArrayAdded->arrayData.members[1];
		TestDownAMember(&pEvtArrayAdded->arrayData.thisRID,&memberRowId);
#endif
		break;

	case RMSTR_EVT_ARRAY_DELETED:
		pEvtArrayDeleted = (RMSTR_EVT_ARRAY_DELETED_STATUS *)pStatusData;
		PrintArrayData(&pEvtArrayDeleted->arrayData);
#if 0
		TestDeleteArray(&pEvtArrayDeleted->arrayData.thisRID);
#endif

		break;


	case RMSTR_EVT_PREFERRED_MEMBER_CHANGED:
		pEvtPreferredMemberChanged = (RMSTR_EVT_PREFERRED_MEMBER_CHANGED_STATUS *)pStatusData;
		PrintArrayData(&pEvtPreferredMemberChanged->arrayData);
		break;

	case RMSTR_EVT_SOURCE_MEMBER_CHANGED:
		pEvtSourceMemberChanged = (RMSTR_EVT_SOURCE_MEMBER_CHANGED_STATUS *)pStatusData;
		PrintArrayData(&pEvtSourceMemberChanged->arrayData);
		break;


	case RMSTR_EVT_UTIL_STARTED:
		pEvtUtilStarted = (RMSTR_EVT_UTIL_STARTED_STATUS *)pStatusData;
		TRACEF_NF(TRACE_RMSTR_1,("\t\tUtil=%s\n", 
				dispUtilityName[pEvtUtilStarted->utilityData.utilityCode]));
		PrintUtilityData(&pEvtUtilStarted->utilityData);

#if 0
		TestDeleteArray(&pEvtUtilStarted->utilityData.targetRID);
#endif

#if 1
		//TestChangePriority(&pEvtUtilStarted->utilityData.thisRID, PRIORITY_LOW);
		TestAbortUtility(&pEvtUtilStarted->utilityData.thisRID);
#endif
		break;

	case RMSTR_EVT_UTIL_STOPPED:
		pEvtUtilStopped = (RMSTR_EVT_UTIL_STOPPED_STATUS *)pStatusData;
		TRACEF_NF(TRACE_RMSTR_1,("\t\tUtil=%s\n", 
				dispUtilityName[pEvtUtilStopped->utilityData.utilityCode]));
		switch(pEvtUtilStopped->reason){
		case RAID_UTIL_ABORTED:
			TRACEF_NF(TRACE_RMSTR_1, ("\t\tUTIL ABORTED BY USER\n"));
			break;
		case RAID_UTIL_ABORTED_IOERROR:
			TRACEF_NF(TRACE_RMSTR_1, ("\t\tUTIL ABORTED IOERROR\n"));
			break;
		case RAID_UTIL_COMPLETE:
			TRACEF_NF(TRACE_RMSTR_1, ("\t\tUTIL COMPLETED\n"));
			break;
		}
		PrintUtilityData(&pEvtUtilStopped->utilityData);
		switch(pEvtUtilStopped->utilityData.utilityCode){
		case RAID_UTIL_VERIFY:
			TRACEF_NF(TRACE_RMSTR_1, ("\t\tUtil Miscompare cnt=%x\n", 
				pEvtUtilStopped->miscompareCount));

			break;
		}
		break;
	

	case RMSTR_EVT_UTIL_PRIORITY_CHANGED:
		pEvtPriorityChanged = (RMSTR_PRIORITY_CHANGED_STATUS *)pStatusData;
		PrintUtilityData(&pEvtPriorityChanged->utilityData);
		TRACEF_NF(TRACE_RMSTR_1, ("\t\tUtil Old priority=%x\n", 
			pEvtPriorityChanged->oldPriority));
		TRACEF_NF(TRACE_RMSTR_1, ("\t\tUtil New priority=%x\n", 
			pEvtPriorityChanged->utilityData.priority));
		break;

	case RMSTR_EVT_UTIL_PERCENT_COMPLETE:
		pEvtPercentComplete = (RMSTR_PERCENT_COMPLETE_STATUS *)pStatusData;
		PrintUtilityData(&pEvtPercentComplete->utilityData);
		TRACEF_NF(TRACE_RMSTR_1, ("\t\tUtil Percent Complete=%x\n", 
			pEvtPercentComplete->percentComplete));
		break;

	case RMSTR_EVT_ARRAY_FAULT_TOLERANT:
		pEvtArrayFaultTolerant = (RMSTR_EVT_ARRAY_FAULT_TOLERANT_STATUS *)pStatusData;
		PrintArrayData(&pEvtArrayFaultTolerant->arrayData);
		break;

	case RMSTR_EVT_SPARE_ADDED:
		pEvtSpareAdded = (RMSTR_EVT_SPARE_ADDED_STATUS *)pStatusData;
		PrintSpareData(&pEvtSpareAdded->spareData);
#if 1
		// Down a member
		rowID		memberRowId;
		memberRowId.Table = 0xe;
		memberRowId.HiPart = 0;
		memberRowId.LoPart = 1;
		TestDownAMember(&pEvtSpareAdded->spareData.arrayRID,&memberRowId);
#endif
#if 0
		addMemberId.Table = pEvtSpareAdded->spareData.SRCTRID.Table;
		addMemberId.HiPart = 0;
		addMemberId.LoPart = 4;		// 5 is still free

		// Add a member
		TestAddMember(
			&pEvtSpareAdded->spareData.arrayRID,
			&addMemberId);
#endif

#if 0
		TestDeleteSpare(&pEvtSpareAdded->spareData.thisRID);
#endif
		break;
	
	case RMSTR_EVT_MEMBER_DOWN:
		pEvtMemberDown = (RMSTR_EVT_MEMBER_DOWN_STATUS *)pStatusData;
		PrintMemberData(&pEvtMemberDown->memberData);
#if 0
		// Add a Dedicated Spare
		dedicatedSpareId.Table = pEvtMemberDown->memberData.memberRID.Table;
		dedicatedSpareId.HiPart = 0;
		dedicatedSpareId.LoPart = 5;		// 5 is still free
		TestAddSpare(
			//RAID_HOST_POOL_SPARE,
			RAID_GENERAL_POOL_SPARE,
			//RAID_DEDICATED_SPARE,
			&dedicatedSpareId,
			NULL,
			//&pEvtMemberDown->memberData.arrayRID,	// target rid
			NULL);
#endif

		break;

	case RMSTR_EVT_MEMBER_ADDED:
		pEvtMemberAdded = (RMSTR_EVT_MEMBER_ADDED_STATUS *)pStatusData;
		PrintMemberData(&pEvtMemberAdded->memberData);
		break;

	case RMSTR_EVT_SPARE_ACTIVATED:
		pEvtSpareActivated = (RMSTR_EVT_SPARE_ACTIVATED_STATUS *)pStatusData;
		PrintSpareData(&pEvtSpareActivated->spareData);
#if 0
		memset(&utilPolicy,0,sizeof(RAID_UTIL_POLICIES));
		// Start a regenerate on the Array
		utilPolicy.SilentMode = 1;
		utilPolicy.RunThruErrors = 1;
		utilPolicy.SpecifyMembersToRunOn = 0;
		TestStartUtility(
			&pEvtSpareActivated->spareData.arrayRID,	// target rid
			RAID_UTIL_REGENERATE,
			PRIORITY_HIGH,
			utilPolicy,
			15);				// %complete update rate
#endif
#if 0
		// Add a Dedicated Spare
		dedicatedSpareId.Table = pEvtSpareActivated->spareData.SRCTRID.Table;
		dedicatedSpareId.HiPart = 0;
		dedicatedSpareId.LoPart = 5;		// 5 is still free
		TestAddSpare(
			//RAID_HOST_POOL_SPARE,
			//RAID_GENERAL_POOL_SPARE,
			RAID_DEDICATED_SPARE,
			&dedicatedSpareId,
			//NULL,
			&pEvtSpareActivated->spareData.arrayRID,	// target rid
			NULL);
#endif
		break;

	case RMSTR_EVT_SPARE_DELETED:	
		pEvtSpareDeleted = (RMSTR_EVT_SPARE_DELETED_STATUS *)pStatusData;
		PrintSpareData(&pEvtSpareDeleted->spareData);
#if 0
		TestDeleteArray(&pEvtSpareDeleted->spareData.arrayRID);
#endif
#if 0
		TestDeleteSpare(&pEvtSpareDeleted->spareData.thisRID);
#endif
		break;

	case RMSTR_EVT_ARRAY_CRITICAL:
		pEvtArrayCriticalStatus = (RMSTR_EVT_ARRAY_CRITICAL_STATUS *)pStatusData;
		PrintArrayData(&pEvtArrayCriticalStatus->arrayData);
		break;

	case RMSTR_EVT_ARRAY_OFFLINE:
		pEvtArrayOfflineStatus = (RMSTR_EVT_ARRAY_OFFLINE_STATUS *)pStatusData;
		PrintArrayData(&pEvtArrayOfflineStatus->arrayData);
		break;
	}
}


void DdmRAIDMstrTest
::PrintUtilityData(RAID_ARRAY_UTILITY		*pUtility)
{
	U32			i=0;

	TRACEF_NF(TRACE_RMSTR_1, ("\t\tArray ADT ID = %x %x %x\n", 
			pUtility->targetRID.Table,
			pUtility->targetRID.HiPart,
			pUtility->targetRID.LoPart));
	TRACEF_NF(TRACE_RMSTR_1, ("\t\tUtil UDT ID = %x %x %x\n", 
			pUtility->thisRID.Table,
			pUtility->thisRID.HiPart,
			pUtility->thisRID.LoPart));

	for (i=0; i < MAX_ARRAY_MEMBERS; i++){
		if (memcmp(
				&pUtility->sourceRowIds[i],
				&pUtility->sourceRowIds[MAX_ARRAY_MEMBERS-1],
				sizeof(rowID)) == 0){
			break;
		}
		TRACEF_NF(TRACE_RMSTR_1, ("\t\tSource RowID = %x %x %x\n", 
			pUtility->sourceRowIds[i].Table,
			pUtility->sourceRowIds[i].HiPart,
			pUtility->sourceRowIds[i].LoPart));
	}
	for (i=0; i < MAX_ARRAY_MEMBERS; i++){
		if (memcmp(
				&pUtility->destinationRowIds[i],
				&pUtility->sourceRowIds[MAX_ARRAY_MEMBERS-1],
				sizeof(rowID)) == 0){
			break;
		}
		TRACEF_NF(TRACE_RMSTR_1, ("\t\tDest RowID(s) = %x %x %x\n", 
			pUtility->destinationRowIds[i].Table,
			pUtility->destinationRowIds[i].HiPart,
			pUtility->destinationRowIds[i].LoPart));
	}
}



void DdmRAIDMstrTest
::PrintArrayData(RAID_ARRAY_DESCRIPTOR	*pADTRecord)
{
	U32		i=0;

		TRACEF_NF(TRACE_RMSTR_1, ("\t\tArrayRowId = %x %x %x\n", 
			pADTRecord->thisRID.Table,
			pADTRecord->thisRID.HiPart,
			pADTRecord->thisRID.LoPart));
		TRACEF_NF(TRACE_RMSTR_1, ("\t\tSRC RowId = %x %x %x\n", 
			pADTRecord->SRCTRID.Table,
			pADTRecord->SRCTRID.HiPart,
			pADTRecord->SRCTRID.LoPart));
		for (i=0; i < pADTRecord->numberMembers; i++){
			TRACEF_NF(TRACE_RMSTR_1, ("\t\tMember MDT RowID(s) = %x %x %x\n", 
				pADTRecord->members[i].Table,
				pADTRecord->members[i].HiPart,
				pADTRecord->members[i].LoPart));
		}
		for (i=0; i < pADTRecord->numberSpares; i++){
			TRACEF_NF(TRACE_RMSTR_1, ("\t\tSpare SDT RowID(s) = %x %x %x\n", 
				pADTRecord->spares[i].Table,
				pADTRecord->spares[i].HiPart,
				pADTRecord->spares[i].LoPart));
		}
		switch(pADTRecord->health){
		case RAID_CRITICAL:
			TRACEF_NF(TRACE_RMSTR_1, ("\t\tState = ARRAY CRITCAL\n"));
			break;
		case RAID_OFFLINE:
			TRACEF_NF(TRACE_RMSTR_1, ("\t\tState = ARRAY OFFLINE\n"));
			break;
		case RAID_FAULT_TOLERANT:
			TRACEF_NF(TRACE_RMSTR_1, ("\t\tState = ARRAY FAULT TOLERANT\n"));
			break;
		}
		TRACEF_NF(TRACE_RMSTR_1, ("\t\tPreferred Member Index = %x\n", 
			pADTRecord->preferredMemberIndex));
		TRACEF_NF(TRACE_RMSTR_1, ("\t\tSource Member Index = %x\n", 
			pADTRecord->sourceMemberIndex));
}


void DdmRAIDMstrTest
::PrintSpareData(RAID_SPARE_DESCRIPTOR	*pSpare)
{

		TRACEF_NF(TRACE_RMSTR_1, ("\t\tSpare SRC RowID = %x %x %x\n", 
			pSpare->SRCTRID.Table,
			pSpare->SRCTRID.HiPart,
			pSpare->SRCTRID.LoPart));
		TRACEF_NF(TRACE_RMSTR_1, ("\t\tSpare SDT RowID = %x %x %x\n", 
			pSpare->thisRID.Table,
			pSpare->thisRID.HiPart,
			pSpare->thisRID.LoPart));
		TRACEF_NF(TRACE_RMSTR_1, ("\t\tSpare ADT ID = %x %x %x\n", 
			pSpare->arrayRID.Table,
			pSpare->arrayRID.HiPart,
			pSpare->arrayRID.LoPart));
		TRACEF_NF(TRACE_RMSTR_1, ("\t\tSpare Host ID = %x %x %x\n", 
			pSpare->hostRID.Table,
			pSpare->hostRID.HiPart,
			pSpare->hostRID.LoPart));		
}


void DdmRAIDMstrTest
::PrintMemberData(RAID_ARRAY_MEMBER	*pMember)
{
		TRACEF_NF(TRACE_RMSTR_1, ("\t\tMember SRC RowID = %x %x %x\n", 
			pMember->memberRID.Table,
			pMember->memberRID.HiPart,
			pMember->memberRID.LoPart));
		TRACEF_NF(TRACE_RMSTR_1, ("\t\tMember MDT RowID = %x %x %x\n", 
			pMember->thisRID.Table,
			pMember->thisRID.HiPart,
			pMember->thisRID.LoPart));
		TRACEF_NF(TRACE_RMSTR_1, ("\t\tMember ADT ID = %x %x %x\n", 
			pMember->arrayRID.Table,
			pMember->arrayRID.HiPart,
			pMember->arrayRID.LoPart));
}



