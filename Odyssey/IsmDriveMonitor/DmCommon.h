/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DmCommon.h
// 
// Description:
// This file contains macros and definitions used in all DriveMonitor modules.
// This file is included by every DriveMonitor module.
// 
// Update Log 
// 10/14/98 Michael G. Panas: Create file
// 01/24/99 Michael G. Panas: Use new TraceLevel[] array, normalize all trace
/*************************************************************************/
#if !defined(DMCommon_H)
#define DMCommon_H

// Turn on debugging
#ifdef _DEBUG
#define	DM_DEBUG
#endif


#include "Nucleus.h"

#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_DRIVE_MONITOR
#include "Odyssey_Trace.h"
#include "DmError.h"
#include "DmTrace.h"

#endif // DMCommon_H