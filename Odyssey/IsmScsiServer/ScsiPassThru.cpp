/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: ScsiPassThru.cpp
// 
// Description:
// This code handles passsing through SCSI commands to another
// device.
// 
// Update Log
//	$Log: /Gemini/Odyssey/IsmScsiServer/ScsiPassThru.cpp $
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
// 11/10/99 Michael G. Panas: Create file
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
// ScsiPassThru
// Pass through the current SCSI command.  The VDN of the target is in
// the vdLegacyScsi field of the Export table entry.  No Callback or 
// context is needed, so we delete the context when done.
/*************************************************************************/
STATUS ScsiServerIsm::ScsiPassThru(SCSI_CONTEXT *p_context)
{
	SCB_PAYLOAD				*pP = 
				(SCB_PAYLOAD *)((Message *)p_context->pMsg)->GetPPayload();
	U32				 status = 0;
	//U32				 length;
	
	TRACE_ENTRY(ScsiPassThru);
	
	// send to destination, no translation of ID or LUN is done
	// here.  Echo Scsi will add the correct ID/LUN
	status = Forward(p_context->pMsg,
					(VDN)pStsExport->vdLegacyScsi);
					
	if (status)
		TRACE_HEX(TRACE_L2, "\n\rScsiPassThru: send failed ", status);
	
	return(status);
	
} // ScsiPassThru


