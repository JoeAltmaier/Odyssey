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
// File: RmstrTestCommands.cpp
// 
// Description:
// Test Cmds for Raid master
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrTestCommands.cpp $
// 
// 22    8/27/99 5:24p Dpatel
// added event code..
// 
// 21    8/24/99 5:13p Dpatel
// added failover code, create array changes for "array name"
// 
// 20    8/24/99 3:57p Dpatel
// 
// 19    8/20/99 3:03p Dpatel
// added simulation for failover and failover code (CheckAnd...() methods)
// 
// 18    8/16/99 7:05p Dpatel
// Changes for alarms + using rowID * as handle instead of void*
// 
// 17    8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 16    8/06/99 2:07p Dpatel
// Test changes..
// 
// 15    8/05/99 2:34p Dpatel
// changed src to 2,0,1
// 
// 14    8/05/99 11:07a Dpatel
// internal delete array, fake raid ddm, hot copy auto break, removed
// array name code..
// 
// 13    8/02/99 3:01p Dpatel
// changes to create array, array FT processing...
// 
// 12    7/30/99 6:40p Dpatel
// Change preferred member, processing member down and stop util as
// internal cmds..
// 
// 11    7/28/99 6:35p Dpatel
// Added capability code, table services, add/remove members, preferred
// member and source member, hot copy etc...
// 
// 10    7/23/99 5:47p Dpatel
// Added internal cmds, hotcopy, changed commit spare etc.
// 
// 9     7/22/99 6:43p Dpatel
// Added unicode string names, changed validation
// 
// 8     7/20/99 6:49p Dpatel
// Some bug fixes and changed arrayName in ArrayDescriptor to rowID.
// 
// 7     7/09/99 5:26p Dpatel
// 
// 6     7/06/99 4:57p Dpatel
// fixed bugs found in the Utility testing process.
// 
// 5     6/30/99 11:15a Dpatel
// Changes for Abort Util and Chg Priority.
// 
// 4     6/28/99 5:16p Dpatel
// Implemented new methods, changed headers.
// 
//
// 06/11/99 Dipam Patel: Create file
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

#include "DdmRaidMstrTest.h"

#include "StorageRollCallTable.h"
#include "RaidUtilTable.h"
#include "ArrayDescriptor.h"

#include "RMgmtPolicies.h"
#include "RaidDefs.h"
#include "RmstrCmnds.h"


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////
STATUS DdmRAIDMstrTest
::TestCreateArray(
			UnicodeString32	arrayName,
			BOOL			isHotCopy,
			U32				numMembers,
			U32				numSpares)
{
	// Generate a Unicode String Object for the array name
	StringClass						scName;
	UnicodeString					ucArrayName;
	scName = StringClass(arrayName);
	ucArrayName = UnicodeString(scName);

	CONTEXT		*pContext = new CONTEXT;
	pContext->value  = isHotCopy;
	pContext->value1 = numMembers;
	pContext->value2 = numSpares;
	m_pStringResourceManager->WriteString(
			ucArrayName,
			&pContext->newRowId,
			(pTSCallback_t)&DdmRAIDMstrTest::ArrayNameWritten,
			pContext);
	return RMSTR_SUCCESS;
}



//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////
STATUS DdmRAIDMstrTest
::ArrayNameWritten(void *_pContext, STATUS status)
{
	RMSTR_CMND_INFO					*pCmdInfo;
	RMSTR_CMND_PARAMETERS			*pCmdParams;

	RMSTR_CREATE_ARRAY_DEFINITION	*pArrayDefinition;
	rowID							tempRow = {7,0,1};

	BOOL							isHotCopy;
	U32								numMembers;
	U32								numSpares;

	CONTEXT		*pContext = (CONTEXT *)_pContext;

	isHotCopy = pContext->value;
	numMembers = pContext->value1;
	numSpares = pContext->value2;

	pCmdInfo = (RMSTR_CMND_INFO *)new char[sizeof(RMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(RMSTR_CMND_INFO));

	pCmdInfo->opcode = RMSTR_CMND_CREATE_ARRAY;
	
	pCmdParams = 
			(RMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pArrayDefinition = 
			(RMSTR_CREATE_ARRAY_DEFINITION *) (&pCmdParams->createArrayDefinition);


	pArrayDefinition->raidLevel = RAID1;
	pArrayDefinition->totalCapacity = 100;
	pArrayDefinition->memberCapacity = 100;
	pArrayDefinition->dataBlockSize = 25;
	pArrayDefinition->parityBlockSize = 0;
	pArrayDefinition->peckingOrder = NEVER_PECK;
	pArrayDefinition->arrayNameRowId = pContext->newRowId;


	pArrayDefinition->numberMembers = numMembers;
	pArrayDefinition->numberSpares = numSpares;

	if (isHotCopy){
		//pArrayDefinition->createPolicy.StartHotCopyWithManualBreak = isHotCopy;
		pArrayDefinition->createPolicy.StartHotCopyWithAutoBreak = isHotCopy;
		pArrayDefinition->hotCopyPriority = PRIORITY_HIGH;
	}


	for (U32 k=0; k < numMembers + numSpares; k++){
			tempRow.LoPart = k + 1;
			pArrayDefinition->arrayMembers[k] = tempRow;	//{1,0,1};
	}

	tempRow.LoPart = 55;
	pArrayDefinition->arrayMembers[0x2f] = tempRow;

	pArrayDefinition->sourceMemberIndex = 0;

	status = m_pCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmRAIDMstrTest::rmstrCommandCompletionReply,
		NULL);
	delete pCmdInfo;
	pCmdInfo = NULL;
	return status;
}



//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////
STATUS DdmRAIDMstrTest::
TestDeleteArray(rowID	*pArrayRowId)
{
	RMSTR_CMND_INFO				*pCmdInfo;
	RMSTR_CMND_PARAMETERS		*pCmdParams;
	RMSTR_DELETE_ARRAY_INFO		*pDeleteArrayInfo;


	pCmdInfo = (RMSTR_CMND_INFO *)new char[sizeof(RMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(RMSTR_CMND_INFO));

	pCmdInfo->opcode = RMSTR_CMND_DELETE_ARRAY;
	
	pCmdParams = 
			(RMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pDeleteArrayInfo = 
			(RMSTR_DELETE_ARRAY_INFO *) (&pCmdParams->deleteArrayInfo);

	pDeleteArrayInfo->arrayRowId = *pArrayRowId;	// target ADT row id


	STATUS status = m_pCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmRAIDMstrTest::rmstrCommandCompletionReply,
		NULL);
	delete pCmdInfo;
	pCmdInfo = NULL;
	return status;
}


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////
STATUS DdmRAIDMstrTest::
TestChangePreferredMember(
		rowID				*pArrayRowId,
		rowID				*pMemberRowId)
{
	RMSTR_CMND_INFO							*pCmdInfo;
	RMSTR_CMND_PARAMETERS					*pCmdParams;
	RMSTR_CHANGE_PREFERRED_MEMBER_INFO		*pChangePreferredMemberInfo;



	pCmdInfo = (RMSTR_CMND_INFO *)new char[sizeof(RMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(RMSTR_CMND_INFO));

	pCmdInfo->opcode = RMSTR_CMND_CHANGE_PREFERRED_MEMBER;
	
	pCmdParams = 
			(RMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pChangePreferredMemberInfo = 
			(RMSTR_CHANGE_PREFERRED_MEMBER_INFO *) (&pCmdParams->changePreferredMemberInfo);

	pChangePreferredMemberInfo->arrayRowId = *pArrayRowId;	
	pChangePreferredMemberInfo->newMemberRowId = *pMemberRowId;

	STATUS status = m_pCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmRAIDMstrTest::rmstrCommandCompletionReply,
		NULL);
	delete pCmdInfo;
	pCmdInfo = NULL;
	return status;
}


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////
STATUS DdmRAIDMstrTest::
TestChangeSourceMember(
		rowID				*pArrayRowId,
		rowID				*pMemberRowId)
{
	RMSTR_CMND_INFO							*pCmdInfo;
	RMSTR_CMND_PARAMETERS					*pCmdParams;
	RMSTR_CHANGE_SOURCE_MEMBER_INFO			*pChangeSourceMemberInfo;



	pCmdInfo = (RMSTR_CMND_INFO *)new char[sizeof(RMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(RMSTR_CMND_INFO));

	pCmdInfo->opcode = RMSTR_CMND_CHANGE_SOURCE_MEMBER;
	
	pCmdParams = 
			(RMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pChangeSourceMemberInfo = 
			(RMSTR_CHANGE_SOURCE_MEMBER_INFO *) (&pCmdParams->changeSourceMemberInfo);

	pChangeSourceMemberInfo->arrayRowId = *pArrayRowId;	
	pChangeSourceMemberInfo->newMemberRowId = *pMemberRowId;

	STATUS status = m_pCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmRAIDMstrTest::rmstrCommandCompletionReply,
		NULL);
	delete pCmdInfo;
	pCmdInfo = NULL;
	return status;
}


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////
STATUS DdmRAIDMstrTest::
TestStartUtility(
			rowID				*pArrayRowId,	// target array rid (if array op)
			RAID_UTIL_NAME		utility,
			RAID_UTIL_PRIORITY	priority,
			RAID_UTIL_POLICIES	policy,
			U32					percentCompleteUpdateRate)
{
	RMSTR_CMND_INFO				*pCmdInfo;
	RMSTR_CMND_PARAMETERS		*pCmdParams;
	RMSTR_START_UTIL_INFO		*pStartUtilInfo;

	rowID						tempRowId = {1,0,1};

	CONTEXT						*pTestContext = new CONTEXT;
	pTestContext->value = 155;

	pCmdInfo = (RMSTR_CMND_INFO *)new char[sizeof(RMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(RMSTR_CMND_INFO));

	pCmdInfo->opcode = RMSTR_CMND_START_UTIL;
	
	pCmdParams = 
			(RMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pStartUtilInfo = 
			(RMSTR_START_UTIL_INFO *) (&pCmdParams->startUtilInfo);

	if (pArrayRowId){
		// could be null for Member hot copy
		pStartUtilInfo->targetRowId = *pArrayRowId;	// target ADT row id
	}

	pStartUtilInfo->utilityName = utility;	// Init, Expand, Verify, Regenerate
	pStartUtilInfo->priority = priority;
	pStartUtilInfo->updateRate = percentCompleteUpdateRate;

	// Policy is set to RunOnAllMembers for now
	pStartUtilInfo->policy = policy;

	switch (utility){
	case RAID_UTIL_VERIFY:
		// no data required for verify
		break;
	case RAID_UTIL_BKGD_INIT:
		// no data required for init
		break;
	case RAID_UTIL_REGENERATE:
		//pStartUtilInfo->sourceMembers[0] = tempRowId;
		break;
	case RAID_UTIL_MEMBER_HOTCOPY:
		// Resolve:
		break;
	default:
		break;
	}
	STATUS status = m_pCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmRAIDMstrTest::rmstrCommandCompletionReply,
		pTestContext);
	delete pCmdInfo;
	pCmdInfo = NULL;
	return status;
}



//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////
STATUS DdmRAIDMstrTest::
TestAbortUtility(rowID	*pUtilRowId)

{
	RMSTR_CMND_INFO				*pCmdInfo;
	RMSTR_CMND_PARAMETERS		*pCmdParams;
	RMSTR_ABORT_UTIL_INFO	*pAbortUtilInfo;

	pCmdInfo = (RMSTR_CMND_INFO *)new char[sizeof(RMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(RMSTR_CMND_INFO));

	pCmdInfo->opcode = RMSTR_CMND_ABORT_UTIL;
	
	pCmdParams = 
			(RMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pAbortUtilInfo = 
			(RMSTR_ABORT_UTIL_INFO *) (&pCmdParams->abortUtilInfo);

	pAbortUtilInfo->utilRowId = *pUtilRowId;	
	pAbortUtilInfo->policy.startNextRegenerate = 1;
	STATUS status = m_pCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmRAIDMstrTest::rmstrCommandCompletionReply,
		NULL);
	delete pCmdInfo;
	pCmdInfo = NULL;
	return status;
}



//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////
STATUS DdmRAIDMstrTest::
TestChangePriority(
			rowID				*pUtilRowId,
			RAID_UTIL_PRIORITY	newPriority)

{
	RMSTR_CMND_INFO				*pCmdInfo;
	RMSTR_CMND_PARAMETERS		*pCmdParams;
	RMSTR_CHANGE_PRIORITY_INFO	*pChangePriorityInfo;

	pCmdInfo = (RMSTR_CMND_INFO *)new char[sizeof(RMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(RMSTR_CMND_INFO));

	pCmdInfo->opcode = RMSTR_CMND_CHANGE_UTIL_PRIORITY;
	
	pCmdParams = 
			(RMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pChangePriorityInfo = 
			(RMSTR_CHANGE_PRIORITY_INFO *) (&pCmdParams->changePriorityInfo);

	pChangePriorityInfo->utilRowId = *pUtilRowId;	
	pChangePriorityInfo->newPriority = newPriority;
	STATUS status = m_pCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmRAIDMstrTest::rmstrCommandCompletionReply,
		NULL);
	delete pCmdInfo;
	pCmdInfo = NULL;
	return status;
}




STATUS DdmRAIDMstrTest::
TestAddSpare(
			RAID_SPARE_TYPE		type,
			rowID				*pSpareRowId,
			rowID				*pArrayRowId,	// target rid
			rowID				*pHostRowId)
{
	RMSTR_CMND_INFO				*pCmdInfo;
	RMSTR_CMND_PARAMETERS		*pCmdParams;
	RMSTR_CREATE_SPARE_INFO		*pCreateSpareInfo;

	if (type == RAID_DEDICATED_SPARE){
		assert(pArrayRowId != NULL);
	}

	assert(pSpareRowId != NULL);

	CONTEXT						*pTestContext = new CONTEXT;
	pTestContext->value = 255;

	pCmdInfo = (RMSTR_CMND_INFO *)new char[sizeof(RMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(RMSTR_CMND_INFO));

	pCmdInfo->opcode = RMSTR_CMND_CREATE_SPARE;
	
	pCmdParams = 
			(RMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pCreateSpareInfo = 
			(RMSTR_CREATE_SPARE_INFO *) (&pCmdParams->createSpareInfo);
	pCreateSpareInfo->spareType = type;
	if (pArrayRowId)
		pCreateSpareInfo->arrayRowId = *pArrayRowId;
	pCreateSpareInfo->spareId = *pSpareRowId;
	if (pHostRowId)
		pCreateSpareInfo->hostId = *pHostRowId;	

	STATUS status = m_pCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmRAIDMstrTest::rmstrCommandCompletionReply,
		pTestContext);
	delete pCmdInfo;
	pCmdInfo = NULL;
	return status;
}



STATUS DdmRAIDMstrTest::
TestDeleteSpare(
			rowID				*pSpareRowId)
{
	RMSTR_CMND_INFO				*pCmdInfo;
	RMSTR_CMND_PARAMETERS		*pCmdParams;
	RMSTR_DELETE_SPARE_INFO		*pDeleteSpareInfo;

	assert(pSpareRowId != NULL);


	pCmdInfo = (RMSTR_CMND_INFO *)new char[sizeof(RMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(RMSTR_CMND_INFO));

	pCmdInfo->opcode = RMSTR_CMND_DELETE_SPARE;
	
	pCmdParams = 
			(RMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pDeleteSpareInfo = 
			(RMSTR_DELETE_SPARE_INFO *) (&pCmdParams->createSpareInfo);
	pDeleteSpareInfo->spareId = *pSpareRowId;

	STATUS status = m_pCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmRAIDMstrTest::rmstrCommandCompletionReply,
		NULL);
	delete pCmdInfo;
	pCmdInfo = NULL;
	return status;
}




STATUS DdmRAIDMstrTest::
TestDownAMember(
			rowID				*pArrayRowId,
			rowID				*pMemberRowId)
{
	RMSTR_CMND_INFO					*pCmdInfo;
	RMSTR_CMND_PARAMETERS			*pCmdParams;
	RMSTR_DOWN_A_MEMBER_INFO		*pDownAMemberInfo;

	assert(pArrayRowId != NULL);
	assert(pMemberRowId != NULL);

	pCmdInfo = (RMSTR_CMND_INFO *)new char[sizeof(RMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(RMSTR_CMND_INFO));

	pCmdInfo->opcode = RMSTR_CMND_DOWN_A_MEMBER;
	
	pCmdParams = 
			(RMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pDownAMemberInfo = 
			(RMSTR_DOWN_A_MEMBER_INFO *) (&pCmdParams->downAMemberInfo);
	pDownAMemberInfo->arrayRowId = *pArrayRowId;
	pDownAMemberInfo->memberRowId = *pMemberRowId;

	STATUS status = m_pCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmRAIDMstrTest::rmstrCommandCompletionReply,
		NULL);
	delete pCmdInfo;
	pCmdInfo = NULL;
	return status;
}


STATUS DdmRAIDMstrTest::
TestAddMember(
			rowID		*pArrayRowId,
			rowID		*pSRCRowId)
{
	RMSTR_CMND_INFO					*pCmdInfo;
	RMSTR_CMND_PARAMETERS			*pCmdParams;
	RMSTR_ADD_MEMBER_INFO			*pAddMemberInfo;

	assert(pArrayRowId != NULL);
	assert(pSRCRowId != NULL);

	pCmdInfo = (RMSTR_CMND_INFO *)new char[sizeof(RMSTR_CMND_INFO)];
	memset(pCmdInfo,0,sizeof(RMSTR_CMND_INFO));

	pCmdInfo->opcode = RMSTR_CMND_ADD_MEMBER;
	
	pCmdParams = 
			(RMSTR_CMND_PARAMETERS *) (&pCmdInfo->cmdParams);
	pAddMemberInfo = 
			(RMSTR_ADD_MEMBER_INFO *) (&pCmdParams->addMemberInfo);
	pAddMemberInfo->arrayRowId = *pArrayRowId;
	pAddMemberInfo->newMemberRowId = *pSRCRowId;

	STATUS status = m_pCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&DdmRAIDMstrTest::rmstrCommandCompletionReply,
		NULL);
	delete pCmdInfo;
	pCmdInfo = NULL;
	return status;
}
