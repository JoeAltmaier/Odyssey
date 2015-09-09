/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SmCommon.h
// 
// Description:
// This file contains macros and definitions used in all ScsiMonitor modules.
// This file is included by every ScsiMonitor module.
// 
// Update Log
//	$Log: /Gemini/Odyssey/IsmScsiMonitor/SmCommon.h $ 
// 
// 1     10/11/99 5:35p Cchan
// HSCSI (QL1040B) version of the Drive Monitor
//
/*************************************************************************/
#if !defined(SMCommon_H)
#define SMCommon_H

// Turn on debugging
#define	SM_DEBUG


#include "Nucleus.h"

#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_DRIVE_MONITOR
#include "Odyssey_Trace.h"
#include "SmError.h"
#include "SmTrace.h"

#endif // SMCommon_H