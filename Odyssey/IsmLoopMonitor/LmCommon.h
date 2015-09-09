/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: LmCommon.h
// 
// Description:
// This module is the Loop Monitor common header
// 
// Update Log:
//	$Log: /Gemini/Odyssey/IsmLoopMonitor/LmCommon.h $
// 
// 3     8/14/99 9:28p Mpanas
// LoopMonitor version with most functionality
// implemented
// - Creates/Updates LoopDescriptor records
// - Creates/Updates FCPortDatabase records
// - Reads Export Table entries
// Note: This version uses the Linked List Container types
//           in the SSAPI Util code (SList.cpp)
// 
// 2     8/10/99 7:28p Mpanas
// _DEBUG cleanup
// 
// 1     7/23/99 1:39p Mpanas
// Add latest versions of the Loop code
// 
// 
// 07/17/99 Michael G. Panas: Create file
/*************************************************************************/

#ifndef __LmCommon_h
#define __LmCommon_h

#include <stdio.h>
#include <string.h>
#include "OsTypes.h"
#include "Odyssey.h"
#include "CtIDLUN.h"

#ifdef _DEBUG
#define	FCP_DEBUG
#endif
#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_FCP_LOOP
#include "Odyssey_Trace.h"


#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"
#include "Listen.h"

// Tables referenced
#include "ExportTable.h"
#include "LoopDescriptor.h"
#include "FCPortDatabaseTable.h"

#include "FC_Loop.h"
#include "FcpData.h"
#include "LoopMonitorIsm.h"

#include "Message.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"

#endif
