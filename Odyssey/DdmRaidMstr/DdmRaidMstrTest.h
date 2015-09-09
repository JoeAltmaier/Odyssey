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
// $Log: /Gemini/Odyssey/DdmRaidMstr/DdmRaidMstrTest.h $
// 
// 15    8/24/99 5:13p Dpatel
// added failover code, create array changes for "array name"
// 
// 14    8/16/99 2:48p Dpatel
// prototypes for Print methods..
// 
// 13    8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 12    8/02/99 3:01p Dpatel
// changes to create array, array FT processing...
// 
// 11    7/30/99 6:40p Dpatel
// Change preferred member, processing member down and stop util as
// internal cmds..
// 
// 10    7/28/99 6:35p Dpatel
// Added capability code, table services, add/remove members, preferred
// member and source member, hot copy etc...
// 
// 9     7/23/99 5:47p Dpatel
// Added internal cmds, hotcopy, changed commit spare etc.
// 
// 8     7/22/99 6:43p Dpatel
// Added unicode string names, changed validation
// 
// 7     7/20/99 6:49p Dpatel
// Some bug fixes and changed arrayName in ArrayDescriptor to rowID.
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
#ifndef _DdmRAIDMstrTest_h
#define _DdmRAIDMstrTest_h


#include "CtTypes.h"


#include "CmdSender.h"

#include "Message.h"
#include "Ddm.h"

#include "ArrayDescriptor.h"
#include "RaidSpareDescriptor.h"
#include "RaidMemberTable.h"

#include "RMgmtPolicies.h"
#include "RaidDefs.h"
#include "RmstrCmnds.h"
#include "RmstrEvents.h"
#include "RmstrErrors.h"
#include "RaidUtilTable.h"
#include "RmstrTestLevels.h"

#include "StorageRollCallTable.h"
#include "DdmClassDescriptorTable.h"
#include "VirtualClassDescriptorTable.h"

#include "Rows.h"
#include "Listen.h"
#include "ReadTable.h"
#include "Table.h"
#include "PTSCommon.h"

#include "stdio.h"

#include "odyssey_trace.h"
#include "Trace_Index.h"

#include "UnicodeString.h"
#include "StringResourceManager.h"

#define TRACE_INDEX			TRACE_RMSTR_1

#define UNUSED(p) p=p


extern char	*dispUtilityName[];
extern char	*dispCommandName[];
extern char	*dispEventName[];
extern char	*dispErrorName[];

#pragma	pack(4)


/********************************************************************
*
*
*
********************************************************************/

class DdmRAIDMstrTest: public Ddm
{
	struct CONTEXT
	{
		U32						cmnd;
		U32						state;
		U32						numProcessed;
		U32						value;		// some value
		U32						value1;		// some value
		U32						value2;		// some value
		void					*pData;		// general purpose ptr
		void					*pData1;	// general purpose ptr
		rowID					newRowId;	// for inserts
		CommandQueueRecord		*pCQRecord;
		struct CONTEXT			*pParentContext;
		pTSCallback_t			pCallback;
		CONTEXT()
		{
			cmnd = 0;
			state = 0;
			numProcessed = 0;
			value = 0;
			value1 = 0;
			value2 = 0;
			pData = NULL;
			pData1 = NULL;
			pCQRecord = NULL;
			pParentContext = NULL;
			pCallback = NULL;
		};
	};
public:
	DdmRAIDMstrTest(DID did);
	~DdmRAIDMstrTest();
	static Ddm *Ctor(DID MyDID);
	STATUS Enable(Message *pMsg);
	STATUS Initialize(Message *pMsg);
	STATUS DispatchDefault(Message *pMsg);

protected:


private:
	DID			MyDID;

	CmdSender			*m_pCmdSender;
	HANDLE				m_CmdHandle;

	StringResourceManager	*m_pStringResourceManager;

	void PrintUtilityData(RAID_ARRAY_UTILITY *pUtility);
	void PrintArrayData(RAID_ARRAY_DESCRIPTOR	*pADTRecord);
	void PrintSpareData(RAID_SPARE_DESCRIPTOR	*pSpare);
	void PrintMemberData(RAID_ARRAY_MEMBER	*pMember);
	STATUS StringResourceManagerInitializedReply(
			void			*pContext,
			STATUS			status);


	// TEST METHODS
	STATUS TestCreateArray(
			String64	arrayName,
			BOOL		isHotCopy,
			U32			numMembers,
			U32			numSpares);
	STATUS ArrayNameWritten(
			void		*_pContext, 
			STATUS		status);
	STATUS TestDeleteArray(
			rowID				*pArrayRowId);
	STATUS TestChangePreferredMember(
		rowID				*pArrayRowId,
		rowID				*pMemberRowId);
	STATUS TestChangeSourceMember(
		rowID				*pArrayRowId,
		rowID				*pMemberRowId);
	STATUS TestStartUtility(
			rowID				*pArrayRowId,	// target array rid (if array op)
			RAID_UTIL_NAME		utility,
			RAID_UTIL_PRIORITY	priority,
			RAID_UTIL_POLICIES	policy,
			U32					percentCompleteUpdateRate);
	STATUS TestAbortUtility(rowID	*pUtilRowId);
	STATUS TestChangePriority(
			rowID				*pUtilRowId,
			RAID_UTIL_PRIORITY	newPriority);


	STATUS TestAddSpare(
			RAID_SPARE_TYPE		type,
			rowID				*pSpareRowId,
			rowID				*pArrayRowId,	// target rid
			rowID				*pHostRowId);
	STATUS TestDeleteSpare(
			rowID				*pSpareRowId);

	STATUS TestDownAMember(
			rowID				*pArrayRowId,
			rowID				*pMemberRowId);
	STATUS TestAddMember(
			rowID				*pArrayRowId,
			rowID				*pMemberRowId);


	// GENERAL
	void tstObjectInitializedReply(STATUS status);
	void rmstrCommandCompletionReply(
				STATUS				statusCode,
				void				*pStatusData,
				void				*pCmdData,
				void				*pCmdContext);
	void rmstrEventHandler(
				STATUS			eventCode,
				void			*pEventData);
};


#endif

