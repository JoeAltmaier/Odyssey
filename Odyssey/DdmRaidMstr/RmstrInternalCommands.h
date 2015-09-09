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
// File: RmstrInternalCommands.h
// 
// Description:
//
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrInternalCommands.h $
// 
// 5     8/12/99 1:56p Dpatel
// Added Array offline event processing code.
// 
// 4     8/05/99 11:07a Dpatel
// internal delete array, fake raid ddm, hot copy auto break, removed
// array name code..
// 
// 3     8/03/99 6:22p Dpatel
// Check on member down, if source member down then change the source
// 
// 2     7/30/99 6:40p Dpatel
// Change preferred member, processing member down and stop util as
// internal cmds..
// 
// 1     7/23/99 5:56p Dpatel
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

#include "DdmRaidMgmt.h"

#include "StorageRollCallTable.h"
#include "RaidUtilTable.h"
#include "ArrayDescriptor.h"

#include "RMgmtPolicies.h"
#include "RaidDefs.h"
#include "RmstrCmnds.h"



typedef enum {
	RMSTR_INTERNAL_CMND_START_UTIL = RMSTR_CMND_LAST_VALID + 1,
	RMSTR_INTERNAL_CMND_DELETE_ARRAY,
	RMSTR_INTERNAL_CMND_COMMIT_SPARE,
	RMSTR_INTERNAL_CMND_PROCESS_MEMBER_DOWN_EVENT,
	RMSTR_INTERNAL_CMND_PROCESS_ARRAY_OFFLINE_EVENT,
	RMSTR_INTERNAL_CMND_PROCESS_STOP_UTIL_EVENT,
	RMSTR_INTERNAL_CMND_CHANGE_SOURCE_MEMBER
} RMSTR_INTERNAL_CMND;