/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: ScsiReportLUNs.cpp
// 
// Description:
// This code handles the Report LUNS command
// 
// Update Log
//	$Log: /Gemini/Odyssey/IsmScsiServer/ScsiReportLUNs.cpp $
// 
// 2     1/09/00 2:05p Mpanas
// Fix compiler warnings
// 
// 1     11/15/99 4:01p Mpanas
// Re-organize sources
// - Add new files: ScsiInquiry.cpp, ScsiReportLUNS.cpp, 
//   ScsiReserveRelease.cpp
// - Remove unused headers: ScsiRdWr.h, ScsiMessage.h, ScsiXfer.h
// - New Listen code
// - Use Callbacks
// - All methods part of SCSI base class
// 
// 11/05/99 Michael G. Panas: Create file
/*************************************************************************/

#include "SCSIServ.h"
#include "ScsiSense.h"
#include "FcpMessageFormats.h"
#include "ScsiModes.h"
#include "Scsi.h"

#include "CDB.h"
#include "CTIdLun.h"

#include <string.h>

/*************************************************************************/
// Forward References
/*************************************************************************/

/*************************************************************************/
// ScsiServerReportLUNs
// Handle SCSI Report LUNs command.
/*************************************************************************/
void ScsiServerIsm::ScsiServerReportLUNs(SCSI_CONTEXT *p_context)
{
	SCB_PAYLOAD				*pP = 
				(SCB_PAYLOAD *)((Message *)p_context->pMsg)->GetPPayload();
	CDB12			*p_cmd = (CDB12 *)pP->CDB;
	U32				 status = 0;
	//U32				 length;
	
	TRACE_ENTRY(ScsiServerReportLUNs);
	
	// TODO:
	// Should only work on LUN 0?
	
	// build a list of all the active LUNs on this ID
	
	// transfer the list to the buffer reserved for this command
	
} // ScsiServerReportLUNs