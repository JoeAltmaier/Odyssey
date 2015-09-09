/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DmOther.cpp
// 
// Description:
// This module contains code to handle Other devices for DriveMonitor
// 

// Update Log:
//	$Log: /Gemini/Odyssey/IsmDriveMonitor/DmOther.cpp $
// 
// 2     1/11/00 7:27p Mpanas
// New PathTable changes
// 
// 1     9/14/99 8:40p Mpanas
// Complete re-write of DriveMonitor
// - Scan in sequence
// - Start Motors in sequence
// - LUN Scan support
// - Better table update
// - Re-organize sources
// 
// 
// 08/23/99 Michael G. Panas: Create file
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "OsTypes.h"
#include "Odyssey.h"
#include "Scsi.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"
#include "Listen.h"

#include "DriveMonitorIsm.h"
#include "DmCommon.h"

#include "Message.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"


/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/


/*************************************************************************/
// DM_Do_Other
// Inquiry found a type device that is unsupported, flag an error to
// the operator.
/*************************************************************************/
void DriveMonitorIsm::DM_Do_Other(DM_CONTEXT * pDmc)
{
	TRACE_ENTRY(DM_Do_Other);

	DM_PRINT_HEX(TRACE_L3, "\n\rDM: Invalid Device Type = ",
				((char *)pDmc->buffer)[0]);
	
	// delete the memory allocated
	delete pDmc->buffer;
	pDmc->buffer = NULL;
	
	// finish up
	DM_Complete_Plex_Request(pDmc, 0);
	
	
} // DM_Do_Disk

